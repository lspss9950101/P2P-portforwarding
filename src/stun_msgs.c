#include <stun_msgs.h>

int create_stun_packet(STUN_PACKET* packet, STUN_MSG_TYPE type, unsigned char transaction_id[12]) {
    packet->type = type;
    memcpy(packet->transaction_id, transaction_id, 12);
    packet->attr_count = 0;
}

int delete_stun_packet(STUN_PACKET* packet) {
    for(int i = 0; i < packet->attr_count; i++)
        delete_stun_attr(&packet->attr[i]);
}

int create_stun_attr(STUN_ATTR* attr, STUN_ATTR_TYPE type, short len, unsigned char* val) {
    attr->attr_type = type;
    attr->attr_len = len;
    attr->attr_val = (unsigned char *)malloc(len);
    memcpy(attr->attr_val, val, len);
}

int delete_stun_attr(STUN_ATTR* attr) {
    free(attr->attr_val);
    attr->attr_len = 0;
    attr->attr_type = STUN_ATTR_TYPE_NULL;
}

int append_attr(STUN_PACKET* packet, STUN_ATTR* attr) {
    create_stun_attr(&packet->attr[packet->attr_count++], attr->attr_type, attr->attr_len, attr->attr_val);
}

int packet_len(STUN_PACKET* packet) {
    int len = 2 + 2 + 4 + 12;
    len += attr_len(packet);
    return len;
}

int attr_len(STUN_PACKET* packet) {
    int len = 0;
    for(int i = 0; i < packet->attr_count; i++)
        len += (2 + 2 + packet->attr[i].attr_len);
    return len;
}

int stun_packet_to_str(STUN_PACKET* packet, unsigned char* buf) {
    *(short *)(&buf[0]) = htons(packet->type);
    *(short *)(&buf[2]) = htons(attr_len(packet));
    *(int *)(&buf[4]) = htonl((int)MAGIC_COOKIE);
    *(int *)(&buf[8]) = htonl(*(int *)(&packet->transaction_id[0]));
    *(int *)(&buf[12]) = htonl(*(int *)(&packet->transaction_id[4]));
    *(int *)(&buf[16]) = htonl(*(int *)(&packet->transaction_id[8]));

    int idx = 20;
    for(int i = 0; i < packet->attr_count; i++) {
        *(short *)(&buf[idx]) = htons(packet->attr[i].attr_type);
        *(short *)(&buf[idx+2]) = htons(packet->attr[i].attr_len);
        for(int j = 0; j < packet->attr[i].attr_len; j++) {
            buf[idx + 4 + j] = packet->attr[i].attr_val[j];
        }
    }
}
