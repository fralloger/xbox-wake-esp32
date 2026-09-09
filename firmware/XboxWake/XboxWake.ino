// XboxWake 0.3.0 — BLE observer + Wake-on-LAN, Seeed XIAO ESP32-S3.
// Pinned build dependencies are in ../../platformio.ini.
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <NimBLEDevice.h>
#include <esp_system.h>
#include <atomic>
#include "wake_logic.h"
#include "arp_resolver.h"
#include "web_ui.h"

constexpr unsigned MAX_CONTROLLERS = 16;
constexpr unsigned MAX_NEARBY = 48;
constexpr unsigned MAX_EVENTS = 20;
constexpr uint32_t BOOT_GUARD_MS = 5000;
constexpr uint32_t DISCOVERY_TTL_MS = 60000;

struct Controller {
    String mac, label;
    String addressType = "any";
    bool enabled = true;
};
struct Settings {
    bool enabled = false;
    String ssid, password;
    String targetIp, targetMac;
    String mode = "broadcast", broadcast = "auto";
    uint16_t port = 9;
    uint16_t quietSeconds = 10, cooldownSeconds = 30;
    Controller controllers[MAX_CONTROLLERS];
    unsigned count = 1;
};
struct ControllerRuntime {
    wake::Presence presence;
    int rssi = -127;
    uint32_t wakes = 0;
};
// Callback task sends POD records to the Arduino loop: no shared Strings/NVS/network I/O.
struct Advertisement {
    char mac[18] = {};
    char name[64] = {};
    uint32_t time = 0;
    uint32_t configGeneration = 0;
    int8_t rssi = -127;
    uint8_t addressType = 0;
    bool microsoft = false, xboxLikely = false;
};
struct Nearby {
    bool used = false;
    Advertisement advertisement;
};
struct LogEvent { uint32_t time = 0; String message; };
struct WakeJob {
    bool active = false;
    String controllerMac; // Empty for the explicitly requested web test.
    uint8_t packet[102] = {};
    uint8_t ethernetFrame[116] = {};
    bool ethernet = false;
    IPAddress destination;
    uint16_t port = 9;
    uint8_t remaining = 0, sent = 0;
    uint32_t nextSend = 0;
};

Settings config;
ControllerRuntime controllerRuntime[MAX_CONTROLLERS];
Nearby nearby[MAX_NEARBY];
LogEvent events[MAX_EVENTS];
unsigned eventCount = 0;
wake::WakeGate wakeGate;
WakeJob wakeJob;
ArpResolver arpResolver;
Preferences preferences;
WebServer server(80);
DNSServer dns;
WiFiUDP udp;
QueueHandle_t advertisementQueue = nullptr;
String sessionToken, apSsid, apPassword;
uint32_t startupTime = 0, totalWakes = 0, lastReconnect = 0;
uint32_t networkChangeAt = 0, lastScanCheck = 0;
std::atomic<uint32_t> configGeneration{1};
bool networkChangePending = false, wasConnected = false, mdnsStarted = false;
volatile uint32_t droppedAdvertisements = 0;

String addressTypeName(uint8_t type) { return (type & 1) ? "random" : "public"; }

void logEvent(const String& message) {
    for (unsigned i = min(eventCount, MAX_EVENTS - 1); i > 0; --i) events[i] = events[i - 1];
    events[0].time = millis();
    events[0].message = message;
    if (eventCount < MAX_EVENTS) ++eventCount;
    Serial.printf("[%lu] %s\n", static_cast<unsigned long>(millis() / 1000), message.c_str());
}

String normalizeMac(String value) {
    value.trim(); value.replace('-', ':'); value.toUpperCase();
    uint8_t bytes[6];
    return wake::parseMac(value.c_str(), bytes) ? value : String();
}
bool validIPv4(const String& value, IPAddress* output = nullptr) {
    uint8_t bytes[4];
    if (!wake::parseIPv4(value.c_str(), bytes)) return false;
    if (output) *output = IPAddress(bytes[0], bytes[1], bytes[2], bytes[3]);
    return true;
}
bool validTargetIp(const String& value) {
    IPAddress ip;
    return validIPv4(value, &ip) && ip[0] > 0 && ip[0] < 224 && ip[0] != 127;
}
bool validTargetMac(const String& value) {
    uint8_t mac[6];
    return wake::parseMac(value.c_str(), mac) && wake::ethernetMac(mac);
}
int findController(const String& mac) {
    for (unsigned i = 0; i < config.count; ++i) if (config.controllers[i].mac == mac) return i;
    return -1;
}
bool matches(const Controller& controller, const Advertisement& advertisement) {
    return controller.mac == advertisement.mac &&
        (controller.addressType == "any" || controller.addressType == addressTypeName(advertisement.addressType));
}

void encodeSettings(const Settings& value, JsonDocument& doc) {
    doc["version"] = 1;
    doc["enabled"] = value.enabled;
    doc["ssid"] = value.ssid; doc["password"] = value.password;
    doc["targetIp"] = value.targetIp; doc["targetMac"] = value.targetMac;
    doc["mode"] = value.mode; doc["broadcast"] = value.broadcast;
    doc["port"] = value.port;
    doc["quietSeconds"] = value.quietSeconds; doc["cooldownSeconds"] = value.cooldownSeconds;
    JsonArray list = doc.createNestedArray("controllers");
    for (unsigned i = 0; i < value.count; ++i) {
        JsonObject c = list.createNestedObject();
        c["mac"] = value.controllers[i].mac; c["label"] = value.controllers[i].label;
        c["enabled"] = value.controllers[i].enabled; c["addressType"] = value.controllers[i].addressType;
    }
}
bool persist(const Settings& value) {
    DynamicJsonDocument doc(8192);
    encodeSettings(value, doc);
    if (doc.overflowed()) return false;
    String json; serializeJson(doc, json);
    return preferences.putString("config", json) == json.length();
}
void loadSettings() {
    config.controllers[0].mac = "14:CB:65:8E:09:FB";
    config.controllers[0].label = "Xbox 1914";
    config.controllers[0].addressType = "public";
    String json = preferences.getString("config", "");
    if (json.isEmpty()) return;
    DynamicJsonDocument doc(8192);
    if (deserializeJson(doc, json) || doc["version"].as<int>() != 1) {
        logEvent("Настройки не прочитаны; пробуждение выключено."); return;
    }
    config.enabled = doc["enabled"] | false;
    config.ssid = doc["ssid"].as<String>(); config.password = doc["password"].as<String>();
    config.targetIp = doc["targetIp"].as<String>(); config.targetMac = normalizeMac(doc["targetMac"].as<String>());
    config.mode = doc["mode"] | "broadcast"; config.broadcast = doc["broadcast"] | "auto";
    config.port = doc["port"] | 9;
    config.quietSeconds = constrain(doc["quietSeconds"] | 10, 3, 300);
    config.cooldownSeconds = constrain(doc["cooldownSeconds"] | 30, 3, 3600);
    config.count = 0;
    for (JsonObject c : doc["controllers"].as<JsonArray>()) {
        if (config.count == MAX_CONTROLLERS) break;
        String mac = normalizeMac(c["mac"].as<String>());
        if (mac.isEmpty() || findController(mac) >= 0) continue;
        Controller& item = config.controllers[config.count++];
        item.mac = mac; item.label = c["label"] | "Геймпад";
        item.enabled = c["enabled"] | true;
        item.addressType = c["addressType"] | "any";
    }
}

String readyReason() {
    if (!config.enabled) return "Общий переключатель выключен";
    if (millis() - startupTime < BOOT_GUARD_MS) return "Запуск: наблюдаем эфир 5 секунд";
    if (WiFi.status() != WL_CONNECTED) return "Нет подключения ESP32 к домашнему Wi-Fi";
    if (!validTargetIp(config.targetIp)) return "Укажи IPv4 пробуждаемого ПК";
    if (!validTargetMac(config.targetMac)) return "Укажи MAC сетевой карты ПК для WoL";
    if (config.mode != "broadcast" && config.mode != "ethernet") return "Неверный режим отправки";
    if (!config.port) return "Неверный UDP-порт";
    if (config.mode == "broadcast") {
        if (config.broadcast == "auto") {
            IPAddress target, local = WiFi.localIP(), mask = WiFi.subnetMask();
            validIPv4(config.targetIp, &target);
            for (unsigned i = 0; i < 4; ++i)
                if ((target[i] & mask[i]) != (local[i] & mask[i]))
                    return "ПК вне подсети ESP32: проверь IP или задай адрес broadcast вручную";
        } else if (!validIPv4(config.broadcast)) return "Неверный адрес broadcast";
    }
    return String();
}

bool beginWake(const String& controllerMac, String& error) {
    error = readyReason();
    if (!error.isEmpty()) return false;
    if (wakeJob.active) { error = "Отправка уже выполняется"; return false; }
    uint32_t now = millis();
    if (!wakeGate.accept(now, true, config.enabled, true, true, config.cooldownSeconds * 1000UL)) {
        error = "Действует интервал между пробуждениями"; return false;
    }
    if (!controllerMac.isEmpty()) {
        int index = findController(controllerMac);
        if (index < 0 || !config.controllers[index].enabled) { error = "Контроллер выключен"; return false; }
    }
    uint8_t targetMac[6];
    wake::parseMac(config.targetMac.c_str(), targetMac);
    wake::magicPacket(targetMac, wakeJob.packet);
    wakeJob.ethernet = config.mode == "ethernet";
    if (wakeJob.ethernet) {
        uint8_t sourceMac[6]; WiFi.macAddress(sourceMac);
        wake::ethernetMagicPacket(targetMac, sourceMac, wakeJob.ethernetFrame);
    } else {
        // AP and STA coexist. Explicitly bind the sender to the home-network interface.
        udp.stop();
        if (!udp.begin(WiFi.localIP(), 0)) { error = "Не удалось открыть UDP на интерфейсе домашней сети"; return false; }
    }
    if (!wakeJob.ethernet) {
        if (config.broadcast != "auto") validIPv4(config.broadcast, &wakeJob.destination);
        else {
            IPAddress local = WiFi.localIP(), mask = WiFi.subnetMask();
            for (unsigned i = 0; i < 4; ++i) wakeJob.destination[i] = local[i] | static_cast<uint8_t>(~mask[i]);
        }
    }
    wakeJob.controllerMac = controllerMac;
    wakeJob.port = config.port;
    wakeJob.active = true; wakeJob.remaining = 3; wakeJob.sent = 0; wakeJob.nextSend = now;
    return true;
}

void serviceWakeJob() {
    if (!wakeJob.active) return;
    int index = findController(wakeJob.controllerMac);
    // Check again for EVERY datagram, including any queued repeats after a UI toggle.
    if (!config.enabled || WiFi.status() != WL_CONNECTED ||
        (!wakeJob.controllerMac.isEmpty() && (index < 0 || !config.controllers[index].enabled))) {
        wakeJob.active = false;
        logEvent("Оставшиеся WoL-пакеты отменены"); return;
    }
    uint32_t now = millis();
    if (static_cast<int32_t>(now - wakeJob.nextSend) < 0) return;
    bool delivered = false;
    if (wakeJob.ethernet) {
        esp_netif_t* sta = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        delivered = sta && esp_netif_transmit(sta, wakeJob.ethernetFrame, sizeof(wakeJob.ethernetFrame)) == ESP_OK;
    } else if (udp.beginPacket(wakeJob.destination, wakeJob.port)) {
        size_t written = udp.write(wakeJob.packet, sizeof(wakeJob.packet));
        delivered = udp.endPacket() == 1 && written == sizeof(wakeJob.packet);
    }
    if (delivered) {
        if (!wakeJob.sent) {
            wakeGate.mark(now); ++totalWakes;
            if (index >= 0) ++controllerRuntime[index].wakes;
        }
        ++wakeJob.sent;
    }
    --wakeJob.remaining;
    wakeJob.nextSend = now + 150;
    if (!wakeJob.remaining) {
        wakeJob.active = false;
        String origin = wakeJob.controllerMac.isEmpty() ? "Тест" : wakeJob.controllerMac;
        String destination = wakeJob.ethernet ? "Ethernet FF:FF:FF:FF:FF:FF (0x0842)" :
            wakeJob.destination.toString() + ":" + String(wakeJob.port);
        logEvent(origin + ": отправлено " + String(wakeJob.sent) + "/3 WoL → " + destination +
                 ", MAC ПК " + config.targetMac + ", " + config.mode + ".");
        udp.stop();
    }
}

class ScanCallbacks : public NimBLEScanCallbacks {
    void onResult(const NimBLEAdvertisedDevice* device) override {
        Advertisement packet;
        String mac(device->getAddress().toString().c_str()); mac.toUpperCase();
        strlcpy(packet.mac, mac.c_str(), sizeof(packet.mac));
        strlcpy(packet.name, device->getName().c_str(), sizeof(packet.name));
        packet.time = millis(); packet.rssi = device->getRSSI(); packet.addressType = device->getAddressType();
        packet.configGeneration = configGeneration.load(std::memory_order_relaxed);
        for (uint8_t i = 0; i < device->getManufacturerDataCount(); ++i) {
            std::string data = device->getManufacturerData(i);
            if (data.size() >= 2 && static_cast<uint8_t>(data[0]) == 0x06 &&
                static_cast<uint8_t>(data[1]) == 0x00) packet.microsoft = true;
        }
        // A manufacturer hint, never a wake permission. Public-address OUI only.
        if (!(packet.addressType & 1) && mac.startsWith("14:CB:65:")) packet.microsoft = true;
        String name(packet.name); name.toLowerCase();
        packet.xboxLikely = name.indexOf("xbox") >= 0 || (packet.microsoft &&
            ((device->haveAppearance() && device->getAppearance() == 0x03C4) ||
             device->isAdvertisingService(NimBLEUUID(static_cast<uint16_t>(0x1812)))));
        if (xQueueSend(advertisementQueue, &packet, 0) != pdTRUE) ++droppedAdvertisements;
    }
} scanCallbacks;

void processAdvertisement(const Advertisement& packet) {
    int selected = -1;
    uint32_t oldestAge = 0;
    for (unsigned i = 0; i < MAX_NEARBY; ++i) {
        if (nearby[i].used && strcmp(nearby[i].advertisement.mac, packet.mac) == 0 &&
            nearby[i].advertisement.addressType == packet.addressType) { selected = i; break; }
        if (!nearby[i].used) { selected = i; oldestAge = UINT32_MAX; }
        else if (oldestAge != UINT32_MAX && millis() - nearby[i].advertisement.time >= oldestAge) {
            oldestAge = millis() - nearby[i].advertisement.time; selected = i;
        }
    }
    if (selected >= 0) { nearby[selected].used = true; nearby[selected].advertisement = packet; }
    for (unsigned i = 0; i < config.count; ++i) {
        if (!matches(config.controllers[i], packet)) continue;
        ControllerRuntime& runtime = controllerRuntime[i];
        bool fresh = runtime.presence.observe(packet.time, config.quietSeconds * 1000UL);
        runtime.rssi = packet.rssi;
        if (!fresh) continue;
        // HTTP mutations run before queue draining. Consume old observations as presence,
        // but never authorize a packet that arrived before registration/re-enabling.
        if (packet.configGeneration != configGeneration.load(std::memory_order_relaxed)) continue;
        if (!config.enabled || !config.controllers[i].enabled) {
            logEvent(config.controllers[i].label + ": обнаружен, пробуждение выключено"); continue;
        }
        // Stale observations from a congested queue must never cause a delayed wake.
        if (millis() - packet.time > 1500) continue;
        String error;
        if (!beginWake(config.controllers[i].mac, error)) logEvent(config.controllers[i].label + ": " + error);
    }
}

void sendJson(JsonDocument& doc, int code = 200) {
    String body; serializeJson(doc, body);
    server.sendHeader("Cache-Control", "no-store");
    server.sendHeader("X-Content-Type-Options", "nosniff");
    server.send(code, "application/json; charset=utf-8", body);
}
void sendError(const String& message, int code = 400) {
    StaticJsonDocument<384> doc; doc["error"] = message; sendJson(doc, code);
}
void sendOk() { StaticJsonDocument<32> doc; doc["ok"] = true; sendJson(doc); }
bool readBody(JsonDocument& doc) {
    if (server.header("X-Wake-Token") != sessionToken) { sendError("Обнови страницу: токен сессии изменился", 403); return false; }
    if (!server.header("Content-Type").startsWith("application/json")) { sendError("Ожидается JSON", 415); return false; }
    String body = server.arg("plain");
    if (body.length() > 4096) { sendError("Слишком большой запрос", 413); return false; }
    if (deserializeJson(doc, body) || !doc.is<JsonObject>()) { sendError("Неверный JSON"); return false; }
    return true;
}
bool commitSettings(const Settings& next) {
    if (!persist(next)) { sendError("Не удалось сохранить настройки во flash", 500); return false; }
    config = next;
    configGeneration.fetch_add(1, std::memory_order_relaxed);
    return true;
}

void handleState() {
    uint32_t now = millis();
    DynamicJsonDocument doc(16384);
    doc["token"] = sessionToken; doc["enabled"] = config.enabled; doc["version"] = "0.3.0";
    JsonObject wifi = doc.createNestedObject("wifi");
    wifi["connected"] = WiFi.status() == WL_CONNECTED;
    wifi["ssid"] = config.ssid; wifi["ip"] = WiFi.localIP().toString();
    wifi["subnet"] = WiFi.subnetMask().toString(); wifi["gateway"] = WiFi.gatewayIP().toString();
    wifi["apSsid"] = apSsid; wifi["apIp"] = WiFi.softAPIP().toString();
    JsonObject target = doc.createNestedObject("target");
    target["ip"] = config.targetIp; target["mac"] = config.targetMac;
    target["mode"] = config.mode; target["broadcast"] = config.broadcast; target["port"] = config.port;
    JsonObject timing = doc.createNestedObject("timing");
    timing["quietSeconds"] = config.quietSeconds; timing["cooldownSeconds"] = config.cooldownSeconds;
    JsonArray list = doc.createNestedArray("controllers");
    for (unsigned i = 0; i < config.count; ++i) {
        JsonObject item = list.createNestedObject();
        item["mac"] = config.controllers[i].mac; item["label"] = config.controllers[i].label;
        item["enabled"] = config.controllers[i].enabled; item["addressType"] = config.controllers[i].addressType;
        if (controllerRuntime[i].presence.seen) item["ageSeconds"] = (now - controllerRuntime[i].presence.lastSeen) / 1000;
        else item["ageSeconds"] = nullptr;
        item["rssi"] = controllerRuntime[i].rssi; item["wakes"] = controllerRuntime[i].wakes;
    }
    JsonObject status = doc.createNestedObject("status");
    String reason = readyReason();
    status["ready"] = reason.isEmpty(); status["reason"] = reason.isEmpty() ? "Готов к пробуждению по новому появлению контроллера" : reason;
    status["scanActive"] = NimBLEDevice::getScan()->isScanning();
    status["uptimeSeconds"] = now / 1000; status["wakeCount"] = totalWakes;
    status["droppedAdvertisements"] = droppedAdvertisements;
    if (wakeGate.sent) status["lastWakeAgeSeconds"] = (now - wakeGate.lastWake) / 1000;
    else status["lastWakeAgeSeconds"] = nullptr;
    JsonArray logs = doc.createNestedArray("events");
    for (unsigned i = 0; i < eventCount; ++i) {
        JsonObject entry = logs.createNestedObject();
        entry["ageSeconds"] = (now - events[i].time) / 1000; entry["message"] = events[i].message;
    }
    sendJson(doc);
}
void handleScan() {
    DynamicJsonDocument doc(20000);
    JsonArray list = doc.createNestedArray("devices");
    for (const Nearby& entry : nearby) {
        if (!entry.used || millis() - entry.advertisement.time > DISCOVERY_TTL_MS) continue;
        const Advertisement& adv = entry.advertisement;
        JsonObject item = list.createNestedObject();
        item["mac"] = adv.mac; item["addressType"] = addressTypeName(adv.addressType);
        item["name"] = adv.name; item["rssi"] = adv.rssi;
        item["ageSeconds"] = (millis() - adv.time) / 1000;
        item["microsoft"] = adv.microsoft; item["xboxLikely"] = adv.xboxLikely;
        int index = findController(adv.mac);
        item["registered"] = index >= 0 && matches(config.controllers[index], adv);
    }
    sendJson(doc);
}
void handleMaster() {
    StaticJsonDocument<256> doc; if (!readBody(doc)) return;
    if (!doc["enabled"].is<bool>()) { sendError("Нужно логическое поле enabled"); return; }
    Settings next = config; next.enabled = doc["enabled"].as<bool>();
    if (!commitSettings(next)) return;
    if (!config.enabled) wakeJob.active = false;
    logEvent(config.enabled ? "Общий переключатель включён" : "Общий переключатель выключен; WoL запрещён");
    sendOk();
}
void handleController() {
    StaticJsonDocument<1024> doc; if (!readBody(doc)) return;
    String mac = normalizeMac(doc["mac"].as<String>());
    String label = doc["label"].as<String>(); label.trim();
    if (mac.isEmpty() || label.isEmpty() || label.length() > 96 || !doc["enabled"].is<bool>()) {
        sendError("Укажи MAC, имя до 96 байт и состояние контроллера"); return;
    }
    int index = findController(mac);
    if (index < 0 && config.count == MAX_CONTROLLERS) { sendError("Максимум 16 контроллеров"); return; }
    Settings next = config;
    bool isNew = index < 0;
    if (isNew) index = next.count++;
    String type = doc["addressType"] | (isNew ? "any" : config.controllers[index].addressType.c_str());
    if (type != "any" && type != "public" && type != "random") { sendError("Неверный тип Bluetooth-адреса"); return; }
    next.controllers[index].mac = mac; next.controllers[index].label = label;
    next.controllers[index].enabled = doc["enabled"].as<bool>(); next.controllers[index].addressType = type;
    if (!commitSettings(next)) return;
    // Registering a device currently in the list must not itself wake the PC.
    if (isNew) {
        controllerRuntime[index] = ControllerRuntime();
        for (const Nearby& entry : nearby) if (entry.used && matches(config.controllers[index], entry.advertisement) &&
            millis() - entry.advertisement.time < config.quietSeconds * 1000UL) {
            controllerRuntime[index].presence.observe(entry.advertisement.time, config.quietSeconds * 1000UL);
            controllerRuntime[index].rssi = entry.advertisement.rssi;
        }
    }
    if (!config.controllers[index].enabled && wakeJob.controllerMac == mac) wakeJob.active = false;
    logEvent(label + ": " + (config.controllers[index].enabled ? "разрешён" : "выключен")); sendOk();
}
void handleDeleteController() {
    StaticJsonDocument<256> doc; if (!readBody(doc)) return;
    String mac = normalizeMac(doc["mac"].as<String>()); int index = findController(mac);
    if (index < 0) { sendError("Контроллер не найден", 404); return; }
    Settings next = config;
    for (unsigned i = index; i + 1 < next.count; ++i) next.controllers[i] = next.controllers[i + 1];
    --next.count;
    if (!commitSettings(next)) return;
    for (unsigned i = index; i < config.count; ++i) controllerRuntime[i] = controllerRuntime[i + 1];
    controllerRuntime[config.count] = ControllerRuntime();
    if (wakeJob.controllerMac == mac) wakeJob.active = false;
    logEvent("Удалён контроллер " + mac); sendOk();
}
void handleTarget() {
    StaticJsonDocument<1024> doc; if (!readBody(doc)) return;
    Settings next = config;
    next.targetIp = doc["ip"].as<String>(); next.targetIp.trim();
    next.targetMac = normalizeMac(doc["mac"].as<String>());
    next.mode = doc["mode"].as<String>(); next.broadcast = doc["broadcast"] | "auto";
    next.broadcast.trim(); if (next.broadcast.isEmpty()) next.broadcast = "auto";
    int port = doc["port"] | 0, quiet = doc["quietSeconds"] | 0, cooldown = doc["cooldownSeconds"] | 0;
    if (!validTargetIp(next.targetIp) || !validTargetMac(next.targetMac)) {
        sendError("Нужны IPv4 ПК и MAC его Ethernet/WoL-адаптера; MAC геймпада сюда не подходит"); return;
    }
    if ((next.mode != "broadcast" && next.mode != "ethernet") ||
        (next.broadcast != "auto" && !validIPv4(next.broadcast)) || port < 1 || port > 65535 ||
        quiet < 3 || quiet > 300 || cooldown < 3 || cooldown > 3600) {
        sendError("Проверь режим, broadcast, порт 1–65535, тишину 3–300 с и интервал 3–3600 с"); return;
    }
    next.port = port; next.quietSeconds = quiet; next.cooldownSeconds = cooldown;
    if (!commitSettings(next)) return;
    wakeJob.active = false; logEvent("Настройки WoL сохранены"); sendOk();
}
void handleWifi() {
    StaticJsonDocument<1024> doc; if (!readBody(doc)) return;
    Settings next = config;
    next.ssid = doc["ssid"].as<String>();
    if (next.ssid.isEmpty() || next.ssid.length() > 32 || !doc["password"].is<const char*>()) {
        sendError("Укажи SSID до 32 байт и пароль (пустой только для открытой сети)"); return;
    }
    next.password = doc["password"].as<String>();
    if ((!next.password.isEmpty() && next.password.length() < 8) || next.password.length() > 63) {
        sendError("Пароль Wi-Fi: 8–63 символа либо пустой для открытой сети"); return;
    }
    if (!commitSettings(next)) return;
    wakeJob.active = false; networkChangePending = true; networkChangeAt = millis() + 600;
    logEvent("Параметры Wi-Fi сохранены; подключаемся"); sendOk();
}
void handleResolveStatus() {
    StaticJsonDocument<1536> doc;
    doc["id"] = arpResolver.id; doc["state"] = arpResolver.state;
    doc["ip"] = arpResolver.ip; doc["mac"] = arpResolver.mac;
    doc["error"] = arpResolver.error;
    doc["source"] = "ARP — запись локальной таблицы после запроса; не подтверждение WoL";
    sendJson(doc);
}
void handleResolveStart() {
    StaticJsonDocument<256> doc; if (!readBody(doc)) return;
    String ip = doc["ip"].as<String>(); ip.trim(); IPAddress target;
    if (!validTargetIp(ip) || !validIPv4(ip, &target)) { sendError("Укажи IPv4 компьютера"); return; }
    String error;
    if (!arpResolver.start(target, error)) { sendError(error, 409); return; }
    logEvent("Ищем MAC компьютера " + ip + " по ARP");
    handleResolveStatus();
}

void beginHttp() {
    const char* headers[] = {"X-Wake-Token", "Content-Type"};
    server.collectHeaders(headers, 2);
    server.on("/", HTTP_GET, []() {
        server.sendHeader("Cache-Control", "no-store");
        server.sendHeader("X-Frame-Options", "DENY");
        server.sendHeader("Content-Security-Policy", "default-src 'self'; script-src 'self' 'unsafe-inline'; style-src 'self' 'unsafe-inline'; img-src 'self' data:; connect-src 'self'; frame-ancestors 'none'");
        server.send_P(200, "text/html; charset=utf-8", WEB_UI);
    });
    server.on("/api/state", HTTP_GET, handleState);
    server.on("/api/scan", HTTP_GET, handleScan);
    server.on("/api/master", HTTP_POST, handleMaster);
    server.on("/api/controller", HTTP_POST, handleController);
    server.on("/api/controller/delete", HTTP_POST, handleDeleteController);
    server.on("/api/target", HTTP_POST, handleTarget);
    server.on("/api/wifi", HTTP_POST, handleWifi);
    server.on("/api/resolve", HTTP_POST, handleResolveStart);
    server.on("/api/resolve", HTTP_GET, handleResolveStatus);
    server.onNotFound([]() {
        if (server.uri().startsWith("/api/")) { sendError("Нет такого API", 404); return; }
        server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + "/");
        server.send(302, "text/plain", "Open the setup page");
    });
    server.begin();
}

void setup() {
    Serial.begin(115200); // Never wait for a USB host: the board must work while the PC is off.
    startupTime = millis();
    if (!preferences.begin("xboxwake", false)) logEvent("Ошибка NVS; сохранение настроек недоступно");
    loadSettings();
    char token[33];
    snprintf(token, sizeof(token), "%08lx%08lx%08lx%08lx", (unsigned long)esp_random(),
             (unsigned long)esp_random(), (unsigned long)esp_random(), (unsigned long)esp_random());
    sessionToken = token;
    apPassword = "likeValve";
    char suffix[7]; snprintf(suffix, sizeof(suffix), "%06lx", (unsigned long)((ESP.getEfuseMac() >> 24) & 0xFFFFFF));
    apSsid = String("XboxWake-") + suffix;
    WiFi.persistent(false);
    WiFi.mode(WIFI_AP_STA);
    WiFi.setHostname("xboxwake");
    WiFi.setAutoReconnect(true);
    if (!WiFi.softAP(apSsid.c_str(), apPassword.c_str())) logEvent("Не удалось запустить точку Wi-Fi");
    dns.start(53, "*", WiFi.softAPIP());
    if (!config.ssid.isEmpty()) WiFi.begin(config.ssid.c_str(), config.password.c_str());
    lastReconnect = millis();
    advertisementQueue = xQueueCreate(64, sizeof(Advertisement));
    if (!advertisementQueue) { logEvent("Ошибка: не создана очередь BLE"); ESP.restart(); }
    NimBLEDevice::init("");
    NimBLEScan* scan = NimBLEDevice::getScan();
    scan->setScanCallbacks(&scanCallbacks, true); // Duplicates are needed to measure real silence.
    scan->setActiveScan(false); // Observe advertisements only; no pairing or connection.
    scan->setInterval(100); scan->setWindow(60);
    scan->setMaxResults(0); // Keep memory bounded even in a crowded radio environment.
    if (!scan->start(0, false, true)) logEvent("Сканирование BLE не запустилось; повторим");
    beginHttp();
    logEvent("XboxWake 0.3.0 запущен. BLE-наблюдение без сопряжения.");
    Serial.printf("\nWi-Fi setup SSID: %s\nWi-Fi setup password: %s\nSetup URL: http://%s/\n"
                  "Type INFO and Enter to show these details again.\n\n",
                  apSsid.c_str(), apPassword.c_str(), WiFi.softAPIP().toString().c_str());
}

void loop() {
    dns.processNextRequest();
    server.handleClient(); // Apply the master switch before handling pending BLE events/WoL repeats.
    arpResolver.tick();
    static String previousResolveState = "idle";
    if (arpResolver.state != previousResolveState) {
        previousResolveState = arpResolver.state;
        if (arpResolver.state == "success") logEvent("ARP: " + arpResolver.ip + " → " + arpResolver.mac + ". Сохрани настройки ПК.");
        if (arpResolver.state == "error") logEvent(arpResolver.error);
    }
    if (networkChangePending && static_cast<int32_t>(millis() - networkChangeAt) >= 0) {
        networkChangePending = false;
        WiFi.disconnect(false, false);
        WiFi.begin(config.ssid.c_str(), config.password.c_str());
        lastReconnect = millis();
    }
    bool connected = WiFi.status() == WL_CONNECTED;
    if (connected != wasConnected) {
        wasConnected = connected;
        logEvent(connected ? String("Wi-Fi подключён. Панель: http://") + WiFi.localIP().toString() + "/" : "Wi-Fi потерян; WoL временно недоступен");
        if (connected && !mdnsStarted) { mdnsStarted = MDNS.begin("xboxwake"); if (mdnsStarted) MDNS.addService("http", "tcp", 80); }
    }
    if (!connected && !config.ssid.isEmpty() && millis() - lastReconnect >= 30000) {
        lastReconnect = millis(); WiFi.reconnect();
    }
    Advertisement packet;
    for (unsigned count = 0; count < 64 && xQueueReceive(advertisementQueue, &packet, 0) == pdTRUE; ++count)
        processAdvertisement(packet);
    serviceWakeJob();
    if (millis() - lastScanCheck >= 5000) {
        lastScanCheck = millis();
        if (!NimBLEDevice::getScan()->isScanning()) {
            logEvent("Перезапуск BLE-сканирования"); NimBLEDevice::getScan()->start(0, false, true);
        }
    }
    // Nonblocking serial diagnostics. No Wi-Fi secrets in HTTP responses or the event log.
    static String serialLine;
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            serialLine.trim();
            if (serialLine.equalsIgnoreCase("INFO"))
                Serial.printf("AP: %s\nAP password: %s\nAP URL: http://%s/\nLAN URL: http://%s/\n",
                    apSsid.c_str(), apPassword.c_str(), WiFi.softAPIP().toString().c_str(), WiFi.localIP().toString().c_str());
            serialLine = "";
        } else if (serialLine.length() < 32) serialLine += c;
    }
    delay(2);
}
