#ifndef STUN_MSG
#define STUN_MSG

#define MAGIC_COOKIE 0x2112A442

enum class STUN_MSG_TYPE {
    STUN_MSG_TYPE_BINDING_REQ=0x0001,
	STUN_MSG_TYPE_BINDING_RES=0x0101,
	STUN_MSG_TYPE_BINDING_ERR=0x0111,
	STUN_MSG_TYPE_SHARED_SECRET_REQ=0x0002,
	STUN_MSG_TYPE_SHARED_SECRET_RES=0x0102,
	STUN_MSG_TYPE_SHARED_SECRET_ERR=0x0112,
    SIZE=6
}

enum class STUN_MSG_TYPE {
    STUN_ATTR_TYPE_NULL=0x0000,
	STUN_ATTR_TYPE_MAPPED_ADDR=0x0001,
	STUN_ATTR_TYPE_RESPONSE_ADDR=0x0002,
	STUN_ATTR_TYPE_CHANGE_REQ=0x0003,
	STUN_ATTR_TYPE_SRC_ADDR=0x0004,
	STUN_ATTR_TYPE_CHANGED_ADDR=0x0005,
	STUN_ATTR_TYPE_USERNAME=0x0006,
	STUN_ATTR_TYPE_PASSWD=0x0007,
	STUN_ATTR_TYPE_MSG_INTEGRITY=0x0008,
	STUN_ATTR_TYPE_ERROR_CODE=0x0009,
	STUN_ATTR_TYPE_UNKNOWN=0x000A,
	STUN_ATTR_TYPE_REFLECTED_FROM=0x000B,
    SIZE=11
};

class StunMsg {
    private:
        STUN_MSG_TYPE type;
        unsigned char transaction_id[12];
        struct STUN_ATTR{
            STUN_ATTR_TYPE type;
            short len;
            unsigned char *val;
            bool set;
        } attr[(int)STUN_ATTR_TYPE::SIZE];

        short attrSize();
    public:
        StunMsg(STUN_MSG_TYPE, unsigned char*);
        
        ~StunMsg();

        short size();

        void setAttr(STUN_ATTR_TYPE, unsigned char *, short);

        void unsetAttr(STUN_ATTR_TYPE);

        std::string toString();
}

#endif