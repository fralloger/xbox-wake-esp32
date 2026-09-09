#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <atomic>
#include <esp_netif.h>
#include <esp_netif_net_stack.h>
extern "C" {
#include <lwip/etharp.h>
#include <lwip/tcpip.h>
}
#include "wake_logic.h"

// ARP table access and requests run on lwIP's TCP/IP task, not the Arduino task.
// A single in-flight callback owns Worker until its release-store to busy.
class ArpResolver {
public:
    String state = "idle", ip, mac, error;
    uint32_t id = 0;

    bool start(const IPAddress& target, String& why) {
        if (state == "pending" || worker.busy.load(std::memory_order_acquire)) {
            why = "Поиск MAC уже выполняется"; return false;
        }
        if (WiFi.status() != WL_CONNECTED) { why = "Подключи ESP32 к домашнему Wi-Fi"; return false; }
        IPAddress local = WiFi.localIP(), mask = WiFi.subnetMask(), gateway = WiFi.gatewayIP();
        if (target == local || target == gateway) { why = "Нужен IP компьютера, не ESP32 и не роутера"; return false; }
        bool networkAddress = true, broadcastAddress = true;
        for (unsigned i = 0; i < 4; ++i) {
            if ((target[i] & mask[i]) != (local[i] & mask[i])) {
                why = "ARP определяет MAC только в подсети ESP32. Укажи локальный IP ПК"; return false;
            }
            const uint8_t host = target[i] & static_cast<uint8_t>(~mask[i]);
            networkAddress &= host == 0;
            broadcastAddress &= host == static_cast<uint8_t>(~mask[i]);
        }
        if (networkAddress || broadcastAddress) { why = "Это адрес сети или broadcast, нужен IP компьютера"; return false; }
        IP4_ADDR(&worker.address, target[0], target[1], target[2], target[3]);
        worker.found = false; worker.failed = false;
        ip = target.toString(); mac = ""; error = ""; state = "pending"; ++id;
        started = millis(); nextCheck = started + 200; attempts = 0;
        queue(true);
        return true;
    }

    void tick() {
        if (state != "pending" || worker.busy.load(std::memory_order_acquire)) return;
        if (worker.found) {
            char result[18];
            snprintf(result, sizeof(result), "%02X:%02X:%02X:%02X:%02X:%02X",
                     worker.mac[0], worker.mac[1], worker.mac[2], worker.mac[3], worker.mac[4], worker.mac[5]);
            mac = result; state = "success"; return;
        }
        if (worker.failed || WiFi.status() != WL_CONNECTED) {
            state = "error"; error = "Сетевой интерфейс ESP32 недоступен"; return;
        }
        if (millis() - started >= 4000) {
            state = "error";
            error = "MAC не найден. Включи ПК, проверь IP и общую подсеть без изоляции Wi-Fi. Можно ввести MAC вручную";
            return;
        }
        if (static_cast<int32_t>(millis() - nextCheck) >= 0) {
            nextCheck = millis() + 200;
            // Poll the table; re-send the ARP request every fifth poll.
            queue((++attempts % 5) == 0);
        }
    }

private:
    struct Worker {
        std::atomic<bool> busy{false};
        ip4_addr_t address{};
        uint8_t mac[6]{};
        bool request = false, found = false, failed = false;
    } worker;
    uint32_t started = 0, nextCheck = 0;
    unsigned attempts = 0;

    static void run(void* context) {
        Worker* work = static_cast<Worker*>(context);
        esp_netif_t* handle = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        netif* iface = handle ? static_cast<netif*>(esp_netif_get_netif_impl(handle)) : nullptr;
        if (!iface || !netif_is_up(iface) || !netif_is_link_up(iface)) work->failed = true;
        else if (work->request) etharp_request(iface, &work->address);
        else {
            eth_addr* ethernet = nullptr;
            const ip4_addr_t* address = nullptr;
            if (etharp_find_addr(iface, &work->address, &ethernet, &address) >= 0 && ethernet &&
                wake::ethernetMac(ethernet->addr)) {
                memcpy(work->mac, ethernet->addr, sizeof(work->mac)); work->found = true;
            }
        }
        work->busy.store(false, std::memory_order_release);
    }
    void queue(bool request) {
        worker.request = request;
        worker.busy.store(true, std::memory_order_release);
        if (tcpip_callback(run, &worker) != ERR_OK) {
            worker.busy.store(false, std::memory_order_release);
            worker.failed = true;
        }
    }
};
