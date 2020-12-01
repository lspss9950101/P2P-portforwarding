#ifndef STUN_MSGS
#define STUN_MSGS

#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAGIC_COOKIE 0x2112A442

typedef enum {
	STUN_MSG_TYPE_BINDING_REQ=0x0001,
	STUN_MSG_TYPE_BINDING_RES=0x0101,
	STUN_MSG_TYPE_BINDING_ERR=0x0111,
	STUN_MSG_TYPE_SHARED_SECRET_REQ=0x0002,
	STUN_MSG_TYPE_SHARED_SECRET_RES=0x0102,
	STUN_MSG_TYPE_SHARED_SECRET_ERR=0x0112
} STUN_MSG_TYPE;

typedef enum {
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
	STUN_ATTR_TYPE_REFLECTED_FROM=0x000B
} STUN_ATTR_TYPE;

typedef struct {
	STUN_ATTR_TYPE attr_type;
	short attr_len;
	unsigned char *attr_val;
} STUN_ATTR;

typedef struct {
	STUN_MSG_TYPE type;
	unsigned char transaction_id[12];
	STUN_ATTR attr[11];
	int attr_count;
} STUN_PACKET;

int create_stun_packet(STUN_PACKET*, STUN_MSG_TYPE, unsigned char[12]);

int delete_stun_packet(STUN_PACKET*);

int create_stun_attr(STUN_ATTR*, STUN_ATTR_TYPE, short, unsigned char*);

int delete_stun_attr(STUN_ATTR*);

int append_attr(STUN_PACKET*, STUN_ATTR*);

int packet_len(STUN_PACKET*);

int attr_len(STUN_PACKET*);

int stun_packet_to_str(STUN_PACKET*, unsigned char*);

#endif
