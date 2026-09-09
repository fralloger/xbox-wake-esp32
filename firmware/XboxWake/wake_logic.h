#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>

// Portable logic shared by the firmware and the host tests.
namespace wake {
inline int hexDigit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}
inline bool parseMac(const char* text, uint8_t out[6]) {
    if (!text || strlen(text) != 17) return false;
    for (unsigned i = 0; i < 6; ++i) {
        int h = hexDigit(text[3 * i]), l = hexDigit(text[3 * i + 1]);
        if (h < 0 || l < 0 || (i < 5 && text[3 * i + 2] != ':')) return false;
        out[i] = static_cast<uint8_t>((h << 4) | l);
    }
    return true;
}
inline bool ethernetMac(const uint8_t mac[6]) {
    uint8_t any = 0;
    for (unsigned i = 0; i < 6; ++i) any |= mac[i];
    return any && !(mac[0] & 1); // Unicast, nonzero. Locally assigned MACs are valid.
}
inline bool parseIPv4(const char* text, uint8_t out[4]) {
    if (!text) return false;
    for (unsigned part = 0; part < 4; ++part) {
        unsigned value = 0, digits = 0;
        while (*text >= '0' && *text <= '9') {
            value = value * 10 + static_cast<unsigned>(*text++ - '0');
            if (++digits > 3 || value > 255) return false;
        }
        if (!digits) return false;
        out[part] = static_cast<uint8_t>(value);
        if (part < 3) { if (*text++ != '.') return false; }
    }
    return *text == '\0';
}
inline void magicPacket(const uint8_t mac[6], uint8_t packet[102]) {
    memset(packet, 0xFF, 6);
    for (unsigned repeat = 0; repeat < 16; ++repeat) memcpy(packet + 6 + 6 * repeat, mac, 6);
}
inline void ethernetMagicPacket(const uint8_t target[6], const uint8_t source[6], uint8_t frame[116]) {
    memset(frame, 0xFF, 6); // Ethernet broadcast; target identity remains inside the magic payload.
    memcpy(frame + 6, source, 6);
    frame[12] = 0x08; frame[13] = 0x42;
    magicPacket(target, frame + 14);
}
struct Presence {
    bool seen = false;
    uint32_t lastSeen = 0;
    // Every observation updates presence, even while disabled or rate-limited.
    // Enabling a toggle must never turn an existing advertisement stream into a new press.
    bool observe(uint32_t now, uint32_t quietMs) {
        bool fresh = !seen || static_cast<uint32_t>(now - lastSeen) >= quietMs;
        seen = true;
        lastSeen = now;
        return fresh;
    }
};
struct WakeGate {
    bool sent = false;
    uint32_t lastWake = 0;
    bool accept(uint32_t now, bool fresh, bool master, bool controller,
                bool networkReady, uint32_t cooldownMs) const {
        return fresh && master && controller && networkReady &&
               (!sent || static_cast<uint32_t>(now - lastWake) >= cooldownMs);
    }
    void mark(uint32_t now) { sent = true; lastWake = now; }
};
} // namespace wake
