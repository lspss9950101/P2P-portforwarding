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
        static UUID random();

        UUID();
        UUID(unsigned char *uuid);
        void toBytes(unsigned char *uuid) const;
        void toString(char *buf) const;
        void print() const;
        bool operator<(const UUID uuid) const {
            return (this->high < uuid.high) || (this->high == uuid.high && this->low < uuid.low);
        }
        bool operator>(const UUID uuid) const {
            return (this->high > uuid.high) || (this->high == uuid.high && this->low > uuid.low);
        }
        bool operator==(const UUID uuid) const {
            return (this->high == uuid.high) && (this->low == uuid.low);
        }
        bool operator<=(const UUID uuid) const {
            return (*this < uuid) || (*this == uuid);
        }
        bool operator>=(const UUID uuid) const {
            return (*this > uuid) || (*this == uuid);
        }
};

#endif