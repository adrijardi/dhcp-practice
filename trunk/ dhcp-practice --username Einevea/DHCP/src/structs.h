/*
 * structs.h
 *
 *  Created on: 24-dic-2008
 *       Author: Daniel Conde y Adrián López
 */

#ifndef STRUCTS_H_
#define STRUCTS_H_

// Estructura para dhcp
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

// Estructura para mensaje dhcp en cadena de caracteres.
struct msg_dhcp_t{
	unsigned int length;
	unsigned char * msg;
};

#endif /* STRUCTS_H_ */
