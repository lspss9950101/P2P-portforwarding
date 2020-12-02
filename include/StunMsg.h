#ifndef STUN_MSG
#define STUN_MSG

#define MAGIC_COOKIE 0x2112A442

#include <netinet/in.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <Type.h>
#include <Crypto.h>

enum class STUN_MSG_TYPE {
    STUN_MSG_TYPE_BINDING_REQ=0x0001,
	STUN_MSG_TYPE_BINDING_RES=0x0101,
	STUN_MSG_TYPE_BINDING_ERR=0x0111,
	STUN_MSG_TYPE_SHARED_SECRET_REQ=0x0002,
	STUN_MSG_TYPE_SHARED_SECRET_RES=0x0102,
	STUN_MSG_TYPE_SHARED_SECRET_ERR=0x0112,
    SIZE=6
};

enum class STUN_ATTR_TYPE {
    STUN_ATTR_TYPE_NULL=0x0000,
	STUN_ATTR_TYPE_MAPPED_ADDR=0x0001,
	STUN_ATTR_TYPE_RESPONSE_ADDR=0x0002,
	STUN_ATTR_TYPE_CHANGE_REQ=0x0003,
	STUN_ATTR_TYPE_SRC_ADDR=0x0004,
	STUN_ATTR_TYPE_CHANGED_ADDR=0x0005,
	STUN_ATTR_TYPE_USERNAME=0x0006,
	STUN_ATTR_TYPE_PASSWD=0x0007,
	STUN_ATTR_TYPE_ERROR_CODE=0x0009,
	STUN_ATTR_TYPE_UNKNOWN=0x000A,
	STUN_ATTR_TYPE_REFLECTED_FROM=0x000B,
    STUN_ATTR_TYPE_REALM=0x0014,
    STUN_ATTR_TYPE_NONCE=0x0015,
    STUN_ATTR_TYPE_XOR_MAPPED_ADDRESS=0x8020,
    STUN_ATTR_TYPE_XOR_ONLY=0x8021,
    STUN_ATTR_TYPE_SERVER=0x8022,
	STUN_ATTR_TYPE_MSG_INTEGRITY=0x0008,
    SIZE=16
};

class StunMsg {
    private:
        STUN_MSG_TYPE type;
        unsigned char transaction_id[12];
        struct STUN_ATTR{
            uvector val;
            bool set;
        };
        
        STUN_ATTR attr[(int)STUN_ATTR_TYPE::SIZE];

        short attrSize();

        static short attrTypeToIndex(STUN_ATTR_TYPE);
        
        static STUN_ATTR_TYPE indexToAttrType(short);
    public:
        StunMsg(STUN_MSG_TYPE, unsigned char*);

        StunMsg(uvector&);
        
        ~StunMsg();

        void parseBuffer(uvector&);

        STUN_MSG_TYPE getType();

        short size();

        void setAttr(STUN_ATTR_TYPE, uvector&);

        void unsetAttr(STUN_ATTR_TYPE);

        uvector getAttr(STUN_ATTR_TYPE);

        uvector toPacket(bool, bool);

        void dump(bool, bool);
};

#endif