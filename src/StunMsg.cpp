#include <StunMsg.h>

StunMsg::StunMsg(STUN_MSG_TYPE type, unsigned char transaction_id[12]) {
    this->type = type;
    memcpy(this->transaction_id, transaction_id, 12);
    for(int i = 0; i < (int)STUN_ATTR_TYPE::SIZE; i++) {
        this->attr[i].set = false;
    }
}

StunMsg::StunMsg(uvector &buf) {
    parseBuffer(buf);
}

StunMsg::~StunMsg() {
    
}

void StunMsg::parseBuffer(uvector &buf) {
    for(int i = 0; i < (int)STUN_ATTR_TYPE::SIZE; i++) {
        this->attr[i].set = false;
    }
    this->type = (STUN_MSG_TYPE)ntohs(*(short *)&buf[0]);
    short msg_len = ntohs(*(short *)&buf[2]);
    memcpy(this->transaction_id, &buf[8], 12);
    for(short idx = 20; idx < msg_len + 20;) {
        STUN_ATTR_TYPE attr_type = (STUN_ATTR_TYPE)ntohs(*(short *)&buf[idx]);
        short attr_len = ntohs(*(short *)&buf[idx+2]);

        int index = StunMsg::attrTypeToIndex((STUN_ATTR_TYPE)attr_type);
        this->attr[index].val = uvector(&buf[idx+4], &buf[idx+4+attr_len]);
        /*for(int i = idx + 4 + attr_len - 1; i >= idx + 4; i--)
            this->attr[index].val.push_back(buf[i]);*/
        this->attr[index].set = true;
        idx += (4 + attr_len);    
    }
}

short StunMsg::attrTypeToIndex(STUN_ATTR_TYPE type) {
    switch(type) {
        case STUN_ATTR_TYPE::STUN_ATTR_TYPE_MAPPED_ADDR:
            return 0;
        case STUN_ATTR_TYPE::STUN_ATTR_TYPE_RESPONSE_ADDR:
            return 1;
        case STUN_ATTR_TYPE::STUN_ATTR_TYPE_CHANGE_REQ:
            return 2;
        case STUN_ATTR_TYPE::STUN_ATTR_TYPE_SRC_ADDR:
            return 3;
        case STUN_ATTR_TYPE::STUN_ATTR_TYPE_CHANGED_ADDR:
            return 4;
        case STUN_ATTR_TYPE::STUN_ATTR_TYPE_USERNAME:
            return 5;
        case STUN_ATTR_TYPE::STUN_ATTR_TYPE_PASSWD:
            return 6;
        case STUN_ATTR_TYPE::STUN_ATTR_TYPE_XOR_MAPPED_ADDRESS:
            return 7;
        case STUN_ATTR_TYPE::STUN_ATTR_TYPE_XOR_ONLY:
            return 8;
        case STUN_ATTR_TYPE::STUN_ATTR_TYPE_SERVER:
            return 9;
        case STUN_ATTR_TYPE::STUN_ATTR_TYPE_ERROR_CODE:
            return 10;
        case STUN_ATTR_TYPE::STUN_ATTR_TYPE_UNKNOWN:
            return 11;
        case STUN_ATTR_TYPE::STUN_ATTR_TYPE_REFLECTED_FROM:
            return 12;
        case STUN_ATTR_TYPE::STUN_ATTR_TYPE_REALM:
            return 13;
        case STUN_ATTR_TYPE::STUN_ATTR_TYPE_NONCE:
            return 14;
        case STUN_ATTR_TYPE::STUN_ATTR_TYPE_MSG_INTEGRITY:
            return 15;
    }
    throw;
    return -1;
}

STUN_ATTR_TYPE StunMsg::indexToAttrType(short index) {
    STUN_ATTR_TYPE list[] = {
        STUN_ATTR_TYPE::STUN_ATTR_TYPE_MAPPED_ADDR,
        STUN_ATTR_TYPE::STUN_ATTR_TYPE_RESPONSE_ADDR,
        STUN_ATTR_TYPE::STUN_ATTR_TYPE_CHANGE_REQ,
        STUN_ATTR_TYPE::STUN_ATTR_TYPE_SRC_ADDR,
        STUN_ATTR_TYPE::STUN_ATTR_TYPE_CHANGED_ADDR,
        STUN_ATTR_TYPE::STUN_ATTR_TYPE_USERNAME,
        STUN_ATTR_TYPE::STUN_ATTR_TYPE_PASSWD,
        STUN_ATTR_TYPE::STUN_ATTR_TYPE_XOR_MAPPED_ADDRESS,
        STUN_ATTR_TYPE::STUN_ATTR_TYPE_XOR_ONLY,
        STUN_ATTR_TYPE::STUN_ATTR_TYPE_SERVER,
        STUN_ATTR_TYPE::STUN_ATTR_TYPE_ERROR_CODE,
        STUN_ATTR_TYPE::STUN_ATTR_TYPE_UNKNOWN,
        STUN_ATTR_TYPE::STUN_ATTR_TYPE_REFLECTED_FROM,
        STUN_ATTR_TYPE::STUN_ATTR_TYPE_REALM,
        STUN_ATTR_TYPE::STUN_ATTR_TYPE_NONCE,
        STUN_ATTR_TYPE::STUN_ATTR_TYPE_MSG_INTEGRITY
    };
    return list[index];
}

short StunMsg::attrSize() {
    short size = 0;
    for(int i = 0; i < (int)STUN_ATTR_TYPE::SIZE; i++) {
        if(this->attr[i].set) {
            size += (4 + this->attr[i].val.size());
        }
    }
    return size;
}

STUN_MSG_TYPE StunMsg::getType() {
    return this->type;
}

short StunMsg::size() {
    return 20 + this->attrSize();
}

void StunMsg::setAttr(STUN_ATTR_TYPE type, uvector &val) {
    short index = StunMsg::attrTypeToIndex(type);
    this->attr[index].set = true;
    this->attr[index].val = val;
}

void StunMsg::unsetAttr(STUN_ATTR_TYPE type) {
    short index = StunMsg::attrTypeToIndex(type);
    this->attr[index].set = false;
}

uvector StunMsg::getAttr(STUN_ATTR_TYPE type) {
    short index = StunMsg::attrTypeToIndex(type);
    return this->attr[index].set ? this->attr[index].val : uvector();
}

uvector StunMsg::toPacket(bool msg_integrity, bool finger_print) {
    int size = this->size();
    unsigned char buf[size];
    *(short *)(&buf[0]) = htons((short)this->type);
    *(short *)(&buf[2]) = htons(size - 20);
    *(int *)(&buf[4]) = htonl((int)MAGIC_COOKIE);
    *(int *)(&buf[8]) = htonl(*(int *)(&this->transaction_id[0]));
    *(int *)(&buf[12]) = htonl(*(int *)(&this->transaction_id[4]));
    *(int *)(&buf[16]) = htonl(*(int *)(&this->transaction_id[8]));

    for(int i = 0, idx = 20; i < (int)STUN_ATTR_TYPE::SIZE; i++) {
        if(!this->attr[i].set) continue;
        *(short *)(&buf[idx]) = htons((short)StunMsg::indexToAttrType(i));
        *(short *)(&buf[idx+2]) = htons(this->attr[i].val.size());
        for(int j = 0; j < this->attr[i].val.size(); j++) {
            buf[idx + 4 + j] = this->attr[i].val[this->attr[i].val.size() - j - 1];
        }
        idx += (4 + this->attr[i].val.size());
    }

    if(msg_integrity) {
        uvector key = {0x02, 0x01, 0x01, 0x02};
        uvector before_sha = uvector(buf, buf + size);
        uvector after_sha;
        Crypto::SHA1Encrypt(key, before_sha, after_sha);
        this->setAttr(STUN_ATTR_TYPE::STUN_ATTR_TYPE_MSG_INTEGRITY, after_sha);
        uvector result = this->toPacket(false, false);
        this->unsetAttr(STUN_ATTR_TYPE::STUN_ATTR_TYPE_MSG_INTEGRITY);
        return result;
    }

    return uvector(buf, buf + size);
}

void StunMsg::dump(bool msg_integrity, bool finger_print) {
    uvector buf = this->toPacket(msg_integrity, finger_print);
    int size = buf.size();
    short cnt = 0;
    for(int i = 0; i < size; i++) {
        printf("%02X ", (unsigned char)buf[i]);
        if(cnt == 3) printf("\n");
        cnt = (cnt + 1) % 4;
    }
    printf("\n");
}