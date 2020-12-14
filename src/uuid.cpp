#include <uuid.h>

UUID::UUID() {
    this->random();
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

void UUID::random() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<> dis1(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);

    this->high = 0;
    this->low = 0;

    for(int i = 0; i < 16; i++) {
        this->high <<= 4;
        if(i == 12) this->high |= 4;
        else this->high |= dis1(gen);
    }

    this->low = dis2(gen);

    for(int i = 0; i < 16; i++) {
        this->low <<= 4;
        this->low |= dis1(gen);
    }
}

void UUID::toBytes(unsigned char *uuid) {
    memcpy(uuid, &this->high, 8);
    memcpy(uuid+8, &this->low, 8);
}

void UUID::print() {
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
    printf("%04X%08X\n", parsed_low->y2, parsed_low->z2);
}