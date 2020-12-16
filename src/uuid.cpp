#include <uuid.h>

UUID::UUID() {
    
}

UUID::UUID(unsigned char *uuid) {
    high = *(unsigned long long *)&uuid[0];
    low = *(unsigned long long *)&uuid[8];
}

void UUID::clear() {
    this->high = 0;
    this->low = 0;
}

UUID UUID::zero() {
    UUID uuid;
    uuid.clear();
    return uuid;
}

UUID UUID::random() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<> dis1(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);

    UUID uuid;
    uuid.high = 0;
    uuid.low = 0;

    for(int i = 0; i < 16; i++) {
        uuid.high <<= 4;
        if(i == 12) uuid.high |= 4;
        else uuid.high |= dis1(gen);
    }

    uuid.low = dis2(gen);

    for(int i = 0; i < 16; i++) {
        uuid.low <<= 4;
        uuid.low |= dis1(gen);
    }

    return uuid;
}

void UUID::toBytes(unsigned char *uuid) const {
    memcpy(uuid, &this->high, 8);
    memcpy(uuid+8, &this->low, 8);
}

void UUID::toString(char *buf) const {
    union parser {
        struct {
            unsigned short z1;
            unsigned short y1;
            unsigned int x1;
        };
        struct {
            unsigned int z2;
            unsigned short y2;
            unsigned short x2;
        };
    } *parsed_high, *parsed_low;

    parsed_high = (union parser *)&this->high;
    parsed_low = (union parser *)&this->low;

    sprintf(buf, "%08X-%04X-%04X-%04X-%04X%08X",
        parsed_high->x1, parsed_high->y1, parsed_high->z1,
        parsed_low->x2, parsed_low->y2, parsed_low->z2);
}

void UUID::print() const {
    union parser {
        struct {
            unsigned short z1;
            unsigned short y1;
            unsigned int x1;
        };
        struct {
            unsigned int z2;
            unsigned short y2;
            unsigned short x2;
        };
    } *parsed_high, *parsed_low;

    parsed_high = (union parser *)&this->high;
    parsed_low = (union parser *)&this->low;

    printf("%08X-", parsed_high->x1);
    printf("%04X-", parsed_high->y1);
    printf("%04X-", parsed_high->z1);
    printf("%04X-", parsed_low->x2);
    printf("%04X%08X", parsed_low->y2, parsed_low->z2);
}