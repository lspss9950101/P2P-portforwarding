#ifndef __UUID_H__
#define __UUID_H__

#include <random>
#include <string.h>

class UUID {
    private:
        unsigned long long high, low;
        void clear();
    public:
        static UUID zero();

        UUID();
        UUID(unsigned char *uuid);
        void random();
        void toBytes(unsigned char *uuid);
        void print();
        bool operator <(const UUID uuid) {
            return (this->high < uuid.high) || (this->high == uuid.high && this->low < uuid.low);
        }
        bool operator >(const UUID uuid) {
            return (this->high > uuid.high) || (this->high == uuid.high && this->low > uuid.low);
        }
        bool operator ==(const UUID uuid) {
            return (this->high == uuid.high) && (this->low == uuid.low);
        }
        bool operator <=(const UUID uuid) {
            return (*this < uuid) || (*this == uuid);
        }
        bool operator >=(const UUID uuid) {
            return (*this > uuid) || (*this == uuid);
        }
};

#endif