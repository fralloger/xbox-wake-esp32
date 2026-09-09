// Portable regression tests. No ESP32, network, or C++ runtime dependency.
// Example: c++ -std=c++11 -Wall -Wextra -Werror test_wake_logic.cpp -o test_wake_logic
#include "../firmware/XboxWake/wake_logic.h"
#include <stdio.h>
#include <stdlib.h>

static unsigned checks = 0;
#define CHECK(condition) do { ++checks; if (!(condition)) { \
    fprintf(stderr, "%s:%d: failed: %s\n", __FILE__, __LINE__, #condition); \
    exit(1); } } while (0)

static void testMacParser() {
    uint8_t out[6] = {};
    const uint8_t expected[] = {0x14, 0xCB, 0x65, 0x8E, 0x09, 0xFB};
    CHECK(wake::parseMac("14:CB:65:8E:09:FB", out));
    CHECK(memcmp(out, expected, sizeof(out)) == 0);
    CHECK(wake::parseMac("14:cB:65:8e:09:fB", out));
    CHECK(memcmp(out, expected, sizeof(out)) == 0);
    CHECK(wake::parseMac("00:00:00:00:00:00", out));
    CHECK(wake::parseMac("FF:FF:FF:FF:FF:FF", out));
    CHECK(!wake::parseMac(nullptr, out));
    const char* invalid[] = {
        "", "1", "14", "14:", "14:CB", "14:CB:65:8E:09:F",
        "14:CB:65:8E:09:FB0", "14-CB-65-8E-09-FB", "14:CB:65:8E:09:FG",
        "1G:CB:65:8E:09:FB", "14;CB:65:8E:09:FB", "14:CB:65:8E:09 FB",
        " 4:CB:65:8E:09:FB", "14:CB:65:8E:09:F ", "14CB658E09FB",
        "14:CB:65:8E:9:FB", "14:CB:65:8E:009:FB", "14:CB:65:8E:09:FB\n"
    };
    for (const char* input : invalid) CHECK(!wake::parseMac(input, out));
    // Every truncated prefix must be rejected, including a buffer containing only NUL.
    const char full[] = "14:CB:65:8E:09:FB";
    for (size_t length = 0; length < 17; ++length) {
        char* input = static_cast<char*>(malloc(length + 1));
        CHECK(input != nullptr);
        memcpy(input, full, length);
        input[length] = '\0';
        CHECK(!wake::parseMac(input, out));
        free(input);
    }
}

static void testIPv4Parser() {
    uint8_t out[4] = {};
    CHECK(wake::parseIPv4("192.168.1.255", out));
    const uint8_t expected[] = {192, 168, 1, 255};
    CHECK(memcmp(out, expected, sizeof(out)) == 0);
    CHECK(wake::parseIPv4("0.0.0.0", out));
    CHECK(wake::parseIPv4("255.255.255.255", out));
    CHECK(wake::parseIPv4("001.002.003.004", out));
    const uint8_t decimal[] = {1, 2, 3, 4};
    CHECK(memcmp(out, decimal, sizeof(out)) == 0);
    CHECK(!wake::parseIPv4(nullptr, out));
    const char* invalid[] = {
        "", ".", "1", "1.", "1.2", "1.2.", "1.2.3", "1.2.3.",
        "1.2.3.4.", "1.2.3.4.5", ".1.2.3.4", "1..2.3", "1.2..3",
        "1.2.3.256", "256.2.3.4", "999.2.3.4", "1234.2.3.4", "0000.2.3.4",
        "-1.2.3.4", "+1.2.3.4", "1.2.3.-4", "1.2.3.+4", "a.2.3.4",
        "1,2,3,4", " 1.2.3.4", "1.2.3.4 ", "1.2.3.4\n", "1.2.3.4x",
        "9999999999999999999999999999999999999999.2.3.4"
    };
    for (const char* input : invalid) CHECK(!wake::parseIPv4(input, out));
    const char full[] = "1.2.3.4";
    for (size_t length = 0; length < 7; ++length) {
        char* input = static_cast<char*>(malloc(length + 1));
        CHECK(input != nullptr);
        memcpy(input, full, length);
        input[length] = '\0';
        CHECK(!wake::parseIPv4(input, out));
        free(input);
    }
}

static void testEthernetMac() {
    const uint8_t global[] = {0x14, 0xCB, 0x65, 0x8E, 0x09, 0xFB};
    const uint8_t local[] = {0x02, 0, 0, 0, 0, 1};
    const uint8_t startsWithZero[] = {0, 0, 0, 0, 0, 1};
    const uint8_t zero[] = {0, 0, 0, 0, 0, 0};
    const uint8_t multicast[] = {0x01, 0, 0x5E, 0, 0, 1};
    const uint8_t localMulticast[] = {0x03, 0, 0, 0, 0, 1};
    const uint8_t broadcast[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    CHECK(wake::ethernetMac(global));
    CHECK(wake::ethernetMac(local));
    CHECK(wake::ethernetMac(startsWithZero));
    CHECK(!wake::ethernetMac(zero));
    CHECK(!wake::ethernetMac(multicast));
    CHECK(!wake::ethernetMac(localMulticast));
    CHECK(!wake::ethernetMac(broadcast));
}

static void testMagicPacket() {
    const uint8_t target[] = {0x14, 0xCB, 0x65, 0x8E, 0x09, 0xFB};
    // Complete wire-format fixture: six FF bytes, followed by sixteen target MACs.
    const uint8_t expected[] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0x14, 0xCB, 0x65, 0x8E, 0x09, 0xFB,
        0x14, 0xCB, 0x65, 0x8E, 0x09, 0xFB,
        0x14, 0xCB, 0x65, 0x8E, 0x09, 0xFB,
        0x14, 0xCB, 0x65, 0x8E, 0x09, 0xFB,
        0x14, 0xCB, 0x65, 0x8E, 0x09, 0xFB,
        0x14, 0xCB, 0x65, 0x8E, 0x09, 0xFB,
        0x14, 0xCB, 0x65, 0x8E, 0x09, 0xFB,
        0x14, 0xCB, 0x65, 0x8E, 0x09, 0xFB,
        0x14, 0xCB, 0x65, 0x8E, 0x09, 0xFB,
        0x14, 0xCB, 0x65, 0x8E, 0x09, 0xFB,
        0x14, 0xCB, 0x65, 0x8E, 0x09, 0xFB,
        0x14, 0xCB, 0x65, 0x8E, 0x09, 0xFB,
        0x14, 0xCB, 0x65, 0x8E, 0x09, 0xFB,
        0x14, 0xCB, 0x65, 0x8E, 0x09, 0xFB,
        0x14, 0xCB, 0x65, 0x8E, 0x09, 0xFB,
        0x14, 0xCB, 0x65, 0x8E, 0x09, 0xFB
    };
    static_assert(sizeof(expected) == 102, "Golden packet must contain 102 bytes");
    uint8_t guarded[104];
    memset(guarded, 0xA5, sizeof(guarded));
    wake::magicPacket(target, guarded + 1);
    CHECK(guarded[0] == 0xA5);
    CHECK(guarded[103] == 0xA5);
    CHECK(memcmp(guarded + 1, expected, sizeof(expected)) == 0);
    CHECK(memcmp(target, expected + 6, sizeof(target)) == 0);
}

static void testPresence() {
    const uint32_t quiet = 5000;
    wake::Presence presence;
    CHECK(presence.observe(0, quiet));
    CHECK(!presence.observe(0, quiet));
    CHECK(!presence.observe(4999, quiet));
    CHECK(presence.lastSeen == 4999);
    CHECK(!presence.observe(9998, quiet));
    CHECK(presence.observe(14998, quiet)); // Exactly the silence threshold.
    CHECK(!presence.observe(14999, quiet));
    CHECK(presence.observe(25000, quiet));
    wake::Presence second;
    CHECK(second.observe(25000, quiet)); // Controllers have independent presence.
    CHECK(!presence.observe(25001, quiet));
}

static void testWakeGate() {
    wake::WakeGate gate;
    const uint32_t cooldown = 10000;
    CHECK(gate.accept(0, true, true, true, true, cooldown));
    CHECK(!gate.sent); // Inspection must not consume the opportunity.
    CHECK(!gate.accept(0, false, true, true, true, cooldown));
    CHECK(!gate.accept(0, true, false, true, true, cooldown));
    CHECK(!gate.accept(0, true, true, false, true, cooldown));
    CHECK(!gate.accept(0, true, true, true, false, cooldown));
    gate.mark(100);
    CHECK(gate.sent && gate.lastWake == 100);
    CHECK(!gate.accept(100, true, true, true, true, cooldown));
    CHECK(!gate.accept(10099, true, true, true, true, cooldown));
    CHECK(gate.accept(10100, true, true, true, true, cooldown));
    // Master disable wins both before and after cooldown expiry.
    CHECK(!gate.accept(10100, true, false, true, true, cooldown));
    CHECK(!gate.accept(60000, true, false, true, true, cooldown));
    CHECK(!gate.accept(60000, true, true, false, true, cooldown));
    CHECK(!gate.accept(60000, true, true, true, false, cooldown));
    CHECK(!gate.accept(60000, false, true, true, true, cooldown));
    CHECK(gate.lastWake == 100);
}

static void testObservationsWhileDisabled() {
    wake::Presence presence;
    wake::WakeGate gate;
    const uint32_t quiet = 5000, cooldown = 10000;
    bool fresh = presence.observe(100, quiet);
    CHECK(fresh);
    CHECK(!gate.accept(100, fresh, false, true, true, cooldown));
    fresh = presence.observe(4100, quiet);
    CHECK(!fresh);
    CHECK(!gate.accept(4100, fresh, false, true, true, cooldown));
    // Enabling master during an existing advertisement stream is not a press.
    fresh = presence.observe(8100, quiet);
    CHECK(!gate.accept(8100, fresh, true, true, true, cooldown));
    fresh = presence.observe(13100, quiet);
    CHECK(gate.accept(13100, fresh, true, true, true, cooldown));
    gate.mark(13100);
    // Observations blocked by cooldown still move lastSeen forward.
    fresh = presence.observe(18100, quiet);
    CHECK(fresh);
    CHECK(!gate.accept(18100, fresh, true, true, true, cooldown));
    fresh = presence.observe(22000, quiet);
    CHECK(!fresh);
    fresh = presence.observe(23100, quiet);
    CHECK(!gate.accept(23100, fresh, true, true, true, cooldown));
    // A disabled individual controller follows the same rule.
    fresh = presence.observe(29000, quiet);
    CHECK(!gate.accept(29000, fresh, true, false, true, cooldown));
    fresh = presence.observe(30000, quiet);
    CHECK(!gate.accept(30000, fresh, true, true, true, cooldown));
    fresh = presence.observe(35000, quiet);
    CHECK(gate.accept(35000, fresh, true, true, true, cooldown));
}

static void testMillisRollover() {
    wake::Presence presence;
    const uint32_t before = UINT32_MAX - 999; // 1000 ms before wrap.
    CHECK(presence.observe(before, 5000));
    CHECK(!presence.observe(3999, 5000)); // Only 4999 ms elapsed across wrap.
    CHECK(presence.observe(8999, 5000));
    wake::Presence exact;
    CHECK(exact.observe(before, 5000));
    CHECK(exact.observe(4000, 5000)); // Exactly 5000 ms across wrap.
    wake::WakeGate gate;
    gate.mark(before);
    CHECK(!gate.accept(8999, true, true, true, true, 10000));
    CHECK(gate.accept(9000, true, true, true, true, 10000));
    CHECK(!gate.accept(9000, true, false, true, true, 10000));
    gate.mark(9000);
    CHECK(!gate.accept(9001, true, true, true, true, 10000));
}

int main() {
    {
        const uint8_t target[] = {0x04,0xD9,0xF5,0xBB,0xCE,0x00};
        const uint8_t source[] = {0x10,0x20,0x30,0x40,0x50,0x60};
        uint8_t guarded[118]; memset(guarded,0xA5,sizeof(guarded));
        wake::ethernetMagicPacket(target,source,guarded+1);
        CHECK(guarded[0]==0xA5 && guarded[117]==0xA5);
        for (unsigned i=0;i<6;++i) CHECK(guarded[1+i]==0xFF);
        CHECK(memcmp(guarded+7,source,6)==0);
        CHECK(guarded[13]==0x08 && guarded[14]==0x42);
        for (unsigned i=0;i<6;++i) CHECK(guarded[15+i]==0xFF);
        for (unsigned repeat=0;repeat<16;++repeat) CHECK(memcmp(guarded+21+repeat*6,target,6)==0);
    }
    testMacParser();
    testIPv4Parser();
    testEthernetMac();
    testMagicPacket();
    testPresence();
    testWakeGate();
    testObservationsWhileDisabled();
    testMillisRollover();
    printf("PASS: %u checks (MAC/IP parsing, WoL packet, enable gates, presence, rollover)\n", checks);
    return 0;
}
