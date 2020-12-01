#include <StunMsg.h>

StunMsg::StunMsg(STUN_MSG_TYPE type, unsigned char transaction_id[12]) {
    this->type = type;
    memcpy(this.transaction_id, transaction_id, 12);
    for(int i = 0; i < (int)STUN_ATTR_TYPE::SIZE; i++) {
        this->attr[i].set = false;
        this->attr[i].type = (STUN_MSG_TYPE)(i + 1);
        this->attr[i].len = 0;
    }
}

StunMsg::~StunMsg() {
    for(int i = 0; i < (int)STUN_ATTR_TYPE::SIZE; i++) {
        if(this->attr[i].set) {
            delete this->attr[i].val;
        }
    }
}

short StunMsg::attrSize() {
    short size = 0;
    for(int i = 0; i < (int)STUN_ATTR_TYPE::SIZE; i++) {
        if(this->attr[i].set)
            size += StunMsg::attr_len(this->attr[i]);
    }
    return size;
}

short StunMsg::size() {
    return 20 + this->attrSize();
}

void StunMsg::setAttr(STUN_ATTR_TYPE type, unsigned char *val, short len) {
    if(this->attr[(int)type].set) {
        delete [] this->attr[(int)type].val;
    }
    this->attr[(int)type].type = type;
    this->attr[(int)type].len = len;
    this->attr[(int)type].set = true;
    this->attr[(int)type].val = new unsigned char[len];
    memcpy(this->attr[(int)type].val, val, len);
}

void StunMsg::unsetAttr(STUN_ATTR_TYPE type) {
    if(this->attr[(int)type].set)
        delete [] this->attr[(int)type].val;
    this->attr[(int)type].set = false;
}

std::string StunMsg::toString() {
    char buf[this->size()];
    *(short *)(&buf[0]) = htons((short)this->type);
    *(short *)(&buf[2]) = htons(this->attrSize());
    *(int *)(&buf[4]) = htonl((int)MAGIC_COOKIE);
    *(int *)(&buf[8]) = htonl(*(int *)(&this->transaction_id[0]));
    *(int *)(&buf[12]) = htonl(*(int *)(&this->transaction_id[4]));
    *(int *)(&buf[16]) = htonl(*(int *)(&this->transaction_id[8]));

    for(int i = 0, idx = 20; i < (int)STUN_ATTR_TYPE::SIZE; i++) {
        if(!this->attr[i].set) continue;
        *(short *)(&buf[idx]) = htons(this->attr[i].type);
        *(short *)(&buf[idx+2]) = htons(this->attr[i].len);
        for(int j = 0; j < this->attr[i].len; j++) {
            buf[idx + 4 + j] = this->attr[i].val[this->attr[i].len - j - 1];
        }
        idx += (4 + this->attr[i].len);
    }

    return std::string(buf);
}