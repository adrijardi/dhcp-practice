/*
 * f_messages.h
 *
 *  Created on: 21-oct-2008
 *      Author: dconde
 */

#ifndef F_MESSAGES_H_
#define F_MESSAGES_H_

// Tamanyo base de dhcp
#define DHCP_BSIZE 236;
// Valores de las variables de un mensaje dhcp
#define DHCP_OP_BOOTREQUEST 1;
#define DHCP_OP_BOOTREPLY 2;

#define DHCP_FLAGS_NO_BROADCAST 0;
#define DHCP_FLAGS_BROADCAST 32768;

struct mdhcp_t{
	unsigned char op;
	unsigned char htype;
	unsigned char hlen;
	unsigned char hops;
	unsigned int xid;
	unsigned short secs;
	unsigned short flags;
	unsigned int ciaddr;
	unsigned int yiaddr;
	unsigned int siaddr;
	unsigned int giaddr;
	char chaddr[16];
	char sname[64];
	char file[128];
	unsigned int opt_length;
	char *options;
};

struct msg_dhcp_t{
	unsigned int length;
	unsigned char * msg;
};

struct ip_header_t{
	unsigned char ver_ihl;
	unsigned char tos;
	unsigned short tLen;
	unsigned short id;
	unsigned short flags_fragments;
	unsigned char ttl;
	unsigned char protocol;
	unsigned short checksum;
	unsigned int source_ip;
	unsigned int dest_ip;
};

struct mdhcp_t* new_default_mdhcp();
struct msg_dhcp_t* from_mdhcp_to_message(struct mdhcp_t *str_dhcp);
struct mdhcp_t* from_message_to_mdhcp(struct msg_dhcp_t *message);
void free_mdhcp(struct mdhcp_t *str_dhcp);
void free_message(struct msg_dhcp_t *message);
void print_mdhcp(struct mdhcp_t *str_dhcp);
void print_message(struct msg_dhcp_t *msg);

struct ip_header_t new_default_ipHeader();
char* from_ipHeader_to_char(struct ip_header_t *ipHeader);
void free_ipHeader(struct ip_header_t *ipHeader);

#endif /* F_MESSAGES_H_ */
