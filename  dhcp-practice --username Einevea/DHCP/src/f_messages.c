/*
 * f_messages.c
 *
 *  Created on: 21-oct-2008
 *      Author: dconde
 */
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "f_messages.h"
#include "constants.h"
#include "dhcp_state.h"

//METODOS PRIVADOS
int mdhcp_to_message_size(struct mdhcp_t *str_dhcp);
char* StrToHexStr(char *str, int leng);

//CODIGO
int mdhcp_to_message_size(struct mdhcp_t *str_dhcp) {
	int ret;
	ret = str_dhcp->opt_length + DHCP_BSIZE; //En octetos
	return ret;
}

/*
 * Se reserva memoria
 */
struct msg_dhcp_t* from_mdhcp_to_message(struct mdhcp_t *str_dhcp) {
	int size, p, iaux;
	short saux;
	unsigned char *msg;
	struct msg_dhcp_t * ret;

	ret = malloc(sizeof(struct msg_dhcp_t));

	size = mdhcp_to_message_size(str_dhcp); //En octetos

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
	memcpy(msg + p, str_dhcp->options, str_dhcp->opt_length);

	ret->length = size;
	ret->msg = msg;

	return ret;
}

struct mdhcp_t* new_default_mdhcp() {
	struct mdhcp_t *ret;
	int s;
	ret = malloc(sizeof(struct mdhcp_t));
	ret->op = 0;
	ret->htype = 0;
	ret->hlen = 0;
	ret->hops = 0;
	ret->xid = 0;
	ret->secs = 0;
	ret->flags = 0;
	ret->ciaddr = 0;
	ret->yiaddr = 0;
	ret->siaddr = 0;
	ret->giaddr = 0;
	s = strlen(ret->chaddr);
	bzero(ret->chaddr, s);
	s = strlen(ret->sname);
	bzero(ret->sname, s);
	s = strlen(ret->file);
	bzero(ret->file, s);
	ret->opt_length = 0;
	ret->options = NULL;

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
	if (message->length > 0)
		free(message->msg);
	free(message);
}
void print_mdhcp(struct mdhcp_t *str_dhcp) {
	char *cad, *chaddr, *sname, *file, *options, *xchaddr;
	unsigned int op, htype, hlen, hops, xid, secs, flags, ciaddr, yiaddr,
			siaddr, giaddr, opt_length;
	int size;
	size = sizeof(struct mdhcp_t) + str_dhcp->opt_length;
	cad = malloc(size);

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
	opt_length = (unsigned int) str_dhcp-> opt_length;
	chaddr = str_dhcp->chaddr;
	sname = str_dhcp->sname;
	file = str_dhcp->file;
	options = str_dhcp->options;
	xchaddr = StrToHexStr(chaddr, haddress_size);
	sprintf(cad, "%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%s-%s-%s-%u-%s", op, htype,
			hlen, hops, xid, secs, flags, ciaddr, yiaddr, siaddr, giaddr,
			xchaddr, sname, file, opt_length, options);
	free(xchaddr);
	printf("---\n%s\n---\n", cad);
}

char* StrToHexStr(char *str, int leng) {
	int i;
	char *newstr = malloc((leng * 2) + 1);
	char *cpold = str;
	char *cpnew = newstr;

	for (i = 0; i < leng; i++) {
		sprintf(cpnew, "%02x", (unsigned char) (*cpold++));
		cpnew += 2;
	}
	*(cpnew) = '\0';
	return newstr;
}

void print_message(struct msg_dhcp_t *message) {
	int i, l;
	unsigned char *msg = message->msg;
	l = message->length;
	printf("---\n");
	for (i = 0; i < l; i++) {
		printf("%c", msg[i]);
	}
	printf("---\n");
}

struct ip_header_t* new_default_ipHeader() {
	struct ip_header_t *ret;
	ret = malloc(sizeof(struct ip_header_t));
	ret->source_ip = malloc(sizeof(struct in_addr));
	ret->dest_ip = malloc(sizeof(struct in_addr));
	ret->ver_ihl = 128 + 5;
	ret->tos = 0;
	ret->tLen = 0; //Tamaño total del datagrama
	ret->id = 0; //Identificador para los fragmentos
	ret->flags_fragments = 0; //??
	ret->ttl = 50; //Time to live, 50 por poner algo
	ret->protocol = 17; //UDP
	ret->checksum = 0; //CheckSum, digo yo que habrá que ponerlo
	inet_aton("0.0.0.0", ret->source_ip);
	inet_aton("255.255.255.255", ret->dest_ip);
	return ret;
}
int from_ipHeader_to_char(char* msg, struct ip_header_t *ipHeader) {
	// TODO
	return NULL;
}
void free_ipHeader(struct ip_header_t *ipHeader) {
	//TODO

}

struct udp_header_t* new_default_udpHeader() {
	struct udp_header_t *ret;
	ret = malloc(sizeof(struct udp_header_t));
	ret->source = CLIENT_PORT;
	ret->dest = SERVER_PORT;
	ret->len = 8; //Valor mínimo, hay que incluir el dhcp
	ret->checksum = 0;
	return ret;
}

int from_udpHeader_to_char(char* msg, struct udp_header_t *ipHeader) {
	//TODO
	return NULL;
}

void free_udpHeader(struct udp_header_t *ipHeader) {
	//TODO
}

/*
 * Obtiene el mensaje en formato char* a partir de las cabeceras
 * Establece los valores de tamaño de las cabeceras
 *
 */
int set_udpLenght(char* message, struct ip_header_t* ipHeader,
		struct udp_header_t* udpHeader, struct mdhcp_t* dhcpStruct) {
	int totalSize;
	int messagePos;
	char* tempString;
	struct msg_dhcp_t* dhcpMessage;
	int tempSize;

	udpHeader->len = dhcpMessage->opt_length + 8;
	// Es posible que haya que meter padding en función del tamaño udp.
	totalSize = udpHeader->len + 20;
	ipHeader-> tLen = totalSize;

	message = malloc(totalSize);

	// ¿Comprobar tamaños? por si ha petado
	tempSize = from_ipHeader_to_char(tempString, ipHeader);
	memcpy(message, tempString, tempSize);
	messagePos = tempSize;
	free(tempString);

	tempSize = from_udpHeader_to_char(tempString, udpHeader);
	memcpy(message + messagePos, tempString, tempSize);
	messagePos += tempSize;
	free(tempString);

	dhcpMessage = from_mdhcp_to_message(dhcpStruct);
	tempSize = dhcpMessage->length;
	memcpy(message + messagePos, dhcpMessage->msg, tempSize);
	free_message(dhcpMessage);

	return -1;
}

