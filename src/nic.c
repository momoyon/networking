#include <nic.h>

Mac_addresses assigned_mac_addresses = {0};

void get_unique_mac_address(uint8 *mac_address) {
    while (true) {
        uint8 random_mac_address[6] = {
            (uint8)randomi(0,255),
            (uint8)randomi(0,255),
            (uint8)randomi(0,255),
            (uint8)randomi(0,255),
            (uint8)randomi(0,255),
            (uint8)randomi(0,255),
        };

        // @SPEED: Yea this is fucking slow
        bool found = false;
        for (size_t i = 0; i < assigned_mac_addresses.count; ++i) {
            Mac_address m = assigned_mac_addresses.items[i];
            if (m.addr[0] == random_mac_address[0] &&
                m.addr[1] == random_mac_address[1] &&
                m.addr[2] == random_mac_address[2] &&
                m.addr[3] == random_mac_address[3] &&
                m.addr[4] == random_mac_address[4] &&
                m.addr[5] == random_mac_address[5]) {
                found = true;
                break;
            }
        }
        if (!found) {
            mac_address[0] = random_mac_address[0];
            mac_address[1] = random_mac_address[1];
            mac_address[2] = random_mac_address[2];
            mac_address[3] = random_mac_address[3];
            mac_address[4] = random_mac_address[4];
            mac_address[5] = random_mac_address[5];

            Mac_address m = {0};
            m.addr[0] = random_mac_address[0];
            m.addr[1] = random_mac_address[1];
            m.addr[2] = random_mac_address[2];
            m.addr[3] = random_mac_address[3];
            m.addr[4] = random_mac_address[4];
            m.addr[5] = random_mac_address[5];

            darr_append(assigned_mac_addresses, m);
            return;
        }
    }
}

