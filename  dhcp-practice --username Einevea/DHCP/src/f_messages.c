/*
 * f_messages.c
 *
 *  Created on: 21-oct-2008
 *      Author: dconde
 */
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include "f_messages.h"
#include "constants.h"

/*
 * Se reserva memoria
 */
struct msg_dhcp_t* from_mdhcp_to_message(struct mdhcp_t *str_dhcp) {
	int osize, size, p, iaux;
	short saux;
	unsigned char *msg;
	struct msg_dhcp_t * ret;

	ret = malloc(sizeof(struct msg_dhcp_t));

	osize = str_dhcp->opt_length;
	size = osize + DHCP_BSIZE; //En octetos

	msg = malloc(size);
	p = 0;
	memcpy(msg, &str_dhcp->op, 1);
	p += 1;
	memcpy(msg + p, &str_dhcp->htype, 1);
	p += 1;
	memcpy(msg + p, &str_dhcp->hlen, 1);
	p += 1;
	memcpy(msg + p, &str_dhcp->hops, 1);
	p += 1;
	iaux = htonl(str_dhcp->xid);
	memcpy(msg + p, &iaux, 4);
	p += 4;
	saux = htons(str_dhcp->secs);
	memcpy(msg + p, &saux, 2);
	p += 2;
	saux = htons(str_dhcp->flags);
	memcpy(msg + p, &saux, 2);
	p += 2;
	iaux = htonl(str_dhcp->ciaddr);
	memcpy(msg + p, &iaux, 4);
	p += 4;
	iaux = htonl(str_dhcp->yiaddr);
	memcpy(msg + p, &iaux, 4);
	p += 4;
	iaux = htonl(str_dhcp->siaddr);
	memcpy(msg + p, &iaux, 4);
	p += 4;
	iaux = htonl(str_dhcp->giaddr);
	memcpy(msg + p, &iaux, 4);
	p += 4;
	memcpy(msg + p, &str_dhcp->chaddr, 16);
	p += 16;
	memcpy(msg + p, &str_dhcp->sname, 64);
	p += 64;
	memcpy(msg + p, &str_dhcp->file, 128);
	p += 128;
	memcpy(msg + p, str_dhcp->options, osize);

	ret->length = size;
	ret->msg = msg;

	return ret;
}

struct mdhcp_t* from_message_to_mdhcp(struct msg_dhcp_t *message) {
	int p, osize, iaux;
	short saux;
	struct mdhcp_t *ret;
	unsigned char *msg;

	ret = malloc(sizeof(struct mdhcp_t));
	msg = message->msg;
	osize = message->length - DHCP_BSIZE;

	p = 0;
	memcpy(&ret->op, msg, 1);
	p += 1;
	memcpy(&ret->htype, msg + p, 1);
	p += 1;
	memcpy(&ret->hlen, msg + p, 1);
	p += 1;
	memcpy(&ret->hops, msg + p, 1);
	p += 1;
	memcpy(&iaux, msg + p, 4);
	ret->xid = ntohl(iaux);
	p += 4;
	memcpy(&saux, msg + p, 2);
	ret->secs = ntohs(saux);
	p += 2;
	memcpy(&saux, msg + p, 2);
	ret->flags = ntohs(saux);
	p += 2;
	memcpy(&iaux, msg + p, 4);
	ret->ciaddr = ntohl(iaux);
	p += 4;
	memcpy(&iaux, msg + p, 4);
	ret->yiaddr = ntohl(iaux);
	p += 4;
	memcpy(&iaux, msg + p, 4);
	ret->siaddr = ntohl(iaux);
	p += 4;
	memcpy(&iaux, msg + p, 4);
	ret->giaddr = ntohl(iaux);
	p += 4;
	memcpy(&ret->chaddr, msg + p, 16);
	p += 16;
	memcpy(&ret->sname, msg + p, 64);
	p += 64;
	memcpy(&ret->file, msg + p, 128);
	p += 128;

	ret->opt_length = osize;
	if (osize > 0) {
		ret->options = malloc(osize);
		memcpy(&ret->options, msg + p, osize);
	}

	return ret;
}

void free_mdhcp(struct mdhcp_t *str_dhcp) {
	if (str_dhcp->opt_length > 0)
		free(str_dhcp->options);
	free(str_dhcp);
}
void free_message(struct msg_dhcp_t *message) {
	if(message->length > 0)
		free(message->msg);
	free(message);
}
void print_mdhcp(struct mdhcp_t *str_dhcp){
	char cad[50]; //TODO CAMBIAR
	unsigned int op, htype, hlen, hops, xid, secs, flags, ciaddr, yiaddr, siaddr, giaddr;
//	char chaddr[16];
//	char sname[64];
//	char file[128];
//	unsigned int opt_length;
//	char *options;
	op = (unsigned int) str_dhcp-> op;
	htype = (unsigned int) str_dhcp-> htype;
	hlen = (unsigned int) str_dhcp-> hlen;
	hops = (unsigned int) str_dhcp-> hops;
	xid = (unsigned int) str_dhcp-> xid;
	secs = (unsigned int) str_dhcp-> secs;
	flags = (unsigned int) str_dhcp-> flags;
	ciaddr = (unsigned int) str_dhcp-> ciaddr;
	yiaddr = (unsigned int) str_dhcp-> yiaddr;
	siaddr = (unsigned int) str_dhcp-> siaddr;
	giaddr = (unsigned int) str_dhcp-> giaddr;
	sprintf(cad,"%u %u %u %u %u %u %u %u %u %u %u", op, htype, hlen, hops, xid, secs, flags, ciaddr, yiaddr, siaddr, giaddr);
	printf("---\n%s\n---\n",cad);
}
void print_message(struct msg_dhcp_t *message) {
	int i, l;
	unsigned char *msg = message->msg;
	l = message->length;
	printf("---\n");
	for (i = 0; i < l; i++) {
		printf("%c",msg[i]);
	}
	printf("---\n");
}
