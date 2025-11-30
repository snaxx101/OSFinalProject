#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

struct ChatPacket {
    uint8_t type;
    uint16_t groupID;
    uint32_t timestamp;
    char payload[256];
};

#endif
