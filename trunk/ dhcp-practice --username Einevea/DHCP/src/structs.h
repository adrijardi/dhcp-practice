/*
 * structs.h
 *
 *  Created on: 24-dic-2008
 *      Author: adri
 */

#ifndef STRUCTS_H_
#define STRUCTS_H_

struct offerIP{
	uint offered_ip;
	uint server_ip;
	struct sockaddr* subnet_mask;
	struct in_addr* routers_list;
	struct in_addr* domain_name_server_list;
	struct in_addr* server_address;
	char * domain_name;
	u_int32_t lease;
};

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

#endif /* STRUCTS_H_ */
