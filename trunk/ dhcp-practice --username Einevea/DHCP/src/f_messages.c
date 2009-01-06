/*
 * f_messages.c
 *
 *  Created on: 21-oct-2008
 *      Author: dconde
 */

#include "f_messages.h"

//METODOS PRIVADOS
char* StrToHexStr(char *str, int leng);

int get_UDPhdr(unsigned char **, struct msg_dhcp_t*);
int get_IPhdr(unsigned char **, in_addr_t, unsigned char *, int);
unsigned short in_cksum(unsigned short *ptr, int nbytes);

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
	ret = malloc(sizeof(struct mdhcp_t));
	ret->op = 0;
	ret->htype = 1;
	ret->hlen = 0;
	ret->hops = 0;
	ret->xid = 0;
	ret->secs = 0;
	ret->flags = 0;
	ret->ciaddr = 0;
	ret->yiaddr = 0;
	ret->siaddr = 0;
	ret->giaddr = 0;
	bzero(ret->chaddr, 16);
	bzero(ret->sname, 64);
	bzero(ret->file, 128);
	ret->opt_length = 0;
	ret->options = NULL;

	return ret;
}

int from_message_to_mdhcp(struct mdhcp_t * dhcp, struct msg_dhcp_t *message) {
	int p, osize, iaux;
	short saux;
	int ret = 0;
	unsigned char *msg;

	msg = message->msg;
	osize = message->length - DHCP_BSIZE;

	p = 0;
	memcpy(&dhcp->op, msg, 1);
	p += 1;
	memcpy(&dhcp->htype, msg + p, 1);
	p += 1;
	memcpy(&dhcp->hlen, msg + p, 1);
	p += 1;
	memcpy(&dhcp->hops, msg + p, 1);
	p += 1;
	memcpy(&iaux, msg + p, 4);
	dhcp->xid = ntohl(iaux);
	p += 4;
	memcpy(&saux, msg + p, 2);
	dhcp->secs = ntohs(saux);
	p += 2;
	memcpy(&saux, msg + p, 2);
	dhcp->flags = ntohs(saux);
	p += 2;
	memcpy(&iaux, msg + p, 4);
	dhcp->ciaddr = ntohl(iaux);
	p += 4;
	memcpy(&iaux, msg + p, 4);
	dhcp->yiaddr = ntohl(iaux);
	p += 4;
	memcpy(&iaux, msg + p, 4);
	dhcp->siaddr = ntohl(iaux);
	p += 4;
	memcpy(&iaux, msg + p, 4);
	dhcp->giaddr = ntohl(iaux);
	p += 4;
	memcpy(&dhcp->chaddr, msg + p, 16);
	p += 16;
	memcpy(&dhcp->sname, msg + p, 64);
	p += 64;
	memcpy(&dhcp->file, msg + p, 128);
	p += 128;
	dhcp->opt_length = osize;
	if (osize > 0) {
		dhcp->options = malloc(osize);
		memcpy(dhcp->options, msg + p, osize);
	}
	return ret;
}

void free_mdhcp(struct mdhcp_t *str_dhcp) {
	if (str_dhcp->opt_length > 0){
		free(str_dhcp->options);
	}
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
	xchaddr = StrToHexStr(chaddr, HADDRESS_SIZE);
	sprintf(cad, "%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%u-%s-%s-%s-%u-%s", op, htype,
			hlen, hops, xid, secs, flags, ciaddr, yiaddr, siaddr, giaddr,
			xchaddr, sname, file, opt_length, options);
	printf("---\n%s\n---\n", cad);

	free(xchaddr);
	free(cad);
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

/*
 * Obtiene el mensaje en formato char* a partir de las cabeceras
 * Establece los valores de tamaño de las cabeceras
 */
int getETHMessage(unsigned char** msg, in_addr_t hostname,
		struct mdhcp_t* mdhcp) {
	int total_size;
	struct msg_dhcp_t* dhcp_msg;
	unsigned char** udp_msg;

	udp_msg = malloc(4);

	total_size = 0;
	dhcp_msg = from_mdhcp_to_message(mdhcp);
	total_size = get_UDPhdr(udp_msg, dhcp_msg);
	total_size = get_IPhdr(msg, hostname, *udp_msg, total_size);

	free(*udp_msg);
	free(udp_msg);
	free_message(dhcp_msg);
	return total_size;
}

int get_UDPhdr(unsigned char ** msg, struct msg_dhcp_t* dhcp_msg) {
	int size, p;
	struct udphdr *udph;

	size = 8 + dhcp_msg->length;
	*msg = malloc(size);
	udph = malloc(sizeof(struct udphdr));
	udph->source = htons(CLIENT_PORT);
	udph->dest = htons(SERVER_PORT);
	udph->len = htons(size);
	udph->check = 0;

	p = 0;
	memcpy(*msg, &udph->source, 2);
	p += 2;
	memcpy(*msg + p, &udph->dest, 2);
	p += 2;
	memcpy(*msg + p, &udph->len, 2);
	p += 2;
	memcpy(*msg + p, &udph->check, 2);
	p += 2;
	memcpy(*msg + p, dhcp_msg->msg, dhcp_msg->length);

	free(udph);

	return size;
}

int get_IPhdr(unsigned char ** msg, in_addr_t hostname, unsigned char * udp_msg, int udp_size) {
	int size, p;
	u_int8_t caux;
	struct iphdr *iph;

	size = 20 + udp_size;
	*msg = malloc(size);
	iph = malloc(sizeof(struct iphdr));

	/* fillout ip-header */
	bzero(iph, sizeof(struct iphdr));

	iph->version = 4;
	iph->ihl = 5;
	iph->tos = 0;
	iph->tot_len = htons(size);
	iph->id = getuid();
	iph->ttl = 64;
	iph->frag_off = 0;
	iph->protocol = IPPROTO_UDP;
	iph->saddr = 0;
	iph->daddr = hostname;
	iph->check = 0;
	iph->check = in_cksum((unsigned short *) iph, sizeof(struct iphdr));

	p = 0;
	caux = iph->version;
	caux = caux << 4;
	caux += iph->ihl;
	memcpy(*msg, &caux, 1);
	p += 1;
	memcpy(*msg + p, &iph->tos, 1);
	p += 1;
	memcpy(*msg + p, &iph->tot_len, 2);
	p += 2;
	memcpy(*msg + p, &iph->id, 2);
	p += 2;
	memcpy(*msg + p, &iph->frag_off, 2);
	p += 2;
	memcpy(*msg + p, &iph->ttl, 1);
	p += 1;
	memcpy(*msg + p, &iph->protocol, 1);
	p += 1;
	memcpy(*msg + p, &iph->check, 2);
	p += 2;
	memcpy(*msg + p, &iph->saddr, 4);
	p += 4;
	memcpy(*msg + p, &iph->daddr, 4);
	p += 4;

	memcpy(*msg + p, udp_msg, udp_size);

	free(iph);
	return size;
}

unsigned short in_cksum(unsigned short *addr, int len) {
	int nleft = len;
	int sum = 0;
	unsigned short *w = addr;
	unsigned short answer = 0;

	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1) {
		*(unsigned char *) (&answer) = *(unsigned char *) w;
		sum += answer;
	}

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	answer = ~sum;
	return (answer);
}

int getDhcpDiscoverOptions(char** opt) {
	int p, size;
	u_int32_t aux_lease;
	u_int8_t magic_c[4];
	u_int8_t msg_type[3];
	u_int8_t address[6]; //50
	u_int8_t host_name[2]; //12
	int host_name_length;
	u_int8_t msg_lease[6];
	u_int8_t end_opt;

	aux_lease = LEASE;

	magic_c[0] = 99;
	magic_c[1] = 130;
	magic_c[2] = 83;
	magic_c[3] = 99;

	msg_type[0] = 0x35;
	msg_type[1] = 0x1;
	msg_type[2] = 0x1;

	if(PARAM_ADDRESS != NULL){
		printDebug("getDhcpDiscoverOptions", "PARAM_ADDRESS");
		address[0] = 50;
		address[1] = 4;
		memcpy(&address[2], &PARAM_ADDRESS->s_addr, 4);
	}

	if(PARAM_HOSTNAME != NULL){
		host_name_length = strlen(PARAM_HOSTNAME);
		printDebug("getDhcpDiscoverOptions", "PARAM_HOSTNAME %d", host_name_length);
		host_name[0] = 12;
		host_name[1] = host_name_length;
	}

	msg_lease[0] = 51;
	msg_lease[1] = 4;
	msg_lease[5] = aux_lease;
	aux_lease = aux_lease >> 8;
	msg_lease[4] = aux_lease;
	aux_lease = aux_lease >> 8;
	msg_lease[3] = aux_lease;
	aux_lease = aux_lease >> 8;
	msg_lease[2] = aux_lease;

	end_opt = 0xFF;

	size = 64;
	*opt = malloc(size);

	bzero(*opt, size);
	p = 0;
	memcpy(*opt, magic_c, 4);
	p += 4;
	memcpy(*opt + p, msg_type, 3);
	p += 3;
	if (PARAM_ADDRESS != NULL) {
		memcpy(*opt + p, address, 6);
		p += 6;
	}

	if (PARAM_HOSTNAME != NULL) {
		memcpy(*opt + p, host_name, 6);
		p += 2;
		memcpy(*opt + p, PARAM_HOSTNAME, host_name_length);
		p += host_name_length;
	}
	memcpy(*opt + p, msg_lease, 6);
	p += 6;
	memcpy(*opt + p, &end_opt, 1);
	p += 1;

	return size;
}

int getDhcpRequestOptions(char** opt) {
	int i, p, size;
	u_int32_t aux32;
	u_int8_t magic_c[4];
	u_int8_t msg_type[3];
	u_int8_t sub_mask[6];
	char* rout_list;
	char* dn_list;
	int addr_size;
	u_int8_t serv_ident[6];
	u_int8_t host_name[2]; //12
	int host_name_length;
	u_int8_t msg_lease[6];
	u_int8_t end_opt;

	magic_c[0] = 99;
	magic_c[1] = 130;
	magic_c[2] = 83;
	magic_c[3] = 99;

	msg_type[0] = 0x35;
	msg_type[1] = 0x1;
	msg_type[2] = 0x3;

	// Subnet Mask
	sub_mask[0] = 1;
	sub_mask[1] = 4;
	memcpy(&sub_mask[2], &SUBNET_MASK->sin_addr.s_addr, 4);

	// Router List
	addr_size = sizeof(struct in_addr);
	rout_list = malloc(2 + (ROUTER_LIST_SIZE * addr_size));
	rout_list[0] = 3;
	rout_list[1] = ROUTER_LIST_SIZE*addr_size;

	i = 0;
	printDebug("getDhcpRequestOptions", "Router_list_size: %d", ROUTER_LIST_SIZE);
	printDebug("getDhcpRequestOptions", "Router_list_size2: %d", 2 + (ROUTER_LIST_SIZE * addr_size));
	while (i < ROUTER_LIST_SIZE) {
		printDebug("getDhcpRequestOptions", "Trace: %d", 1);
		aux32 = htonl(ROUTERS_LIST[i*addr_size].s_addr);
		printDebug("getDhcpRequestOptions", "Trace: %d", 2);
		memcpy(&rout_list[2+(i*addr_size)], &aux32, addr_size);
		printDebug("getDhcpRequestOptions", "Trace: %d", 3);
		i++;
	}

	// Domain name List
	printDebug("getDhcpRequestOptions", "Trace: %d, %u, %u", 4, DOMAIN_LIST_SIZE, addr_size);
	dn_list = malloc(2 + (DOMAIN_LIST_SIZE * addr_size));
	printDebug("getDhcpRequestOptions", "Trace: %d", 5);
	dn_list[0] = 6;
	dn_list[1] = DOMAIN_LIST_SIZE * addr_size;

	i = 0;
	printDebug("getDhcpRequestOptions", "domain_list_size: %d", DOMAIN_LIST_SIZE);
	printDebug("getDhcpRequestOptions", "domain_list_size: %d", 2 + (DOMAIN_LIST_SIZE * addr_size));
	while (i < DOMAIN_LIST_SIZE) {
		aux32 = htonl(DOMAIN_NAME_SERVER_LIST[i*addr_size].s_addr);
		printDebug("getDhcpRequestOptions", "domain: %u", ntohl(aux32));
		memcpy(&dn_list[2+(i*addr_size)], &aux32, addr_size);
		i++;
	}

	if (PARAM_HOSTNAME != NULL) {
		host_name_length = strlen(PARAM_HOSTNAME);
		printDebug("getDhcpDiscoverOptions", "PARAM_HOSTNAME %d",
				host_name_length);
		host_name[0] = 12;
		host_name[1] = host_name_length;
	}

	aux32 = LEASE;
	msg_lease[0] = 51;
	msg_lease[1] = 4;
	msg_lease[5] = aux32;
	aux32 = aux32 >> 8;
	msg_lease[4] = aux32;
	aux32 = aux32 >> 8;
	msg_lease[3] = aux32;
	aux32 = aux32 >> 8;
	msg_lease[2] = aux32;

	serv_ident[0] = 54;
	serv_ident[1] = 4;
	memcpy(&serv_ident[2], &SERVER_ADDRESS, 4);

	end_opt = 0xFF;

	size = 64;
	*opt = malloc(size);

	bzero(*opt, size);
	p = 0;
	memcpy(*opt, magic_c, 4);
	p += 4;
	memcpy(*opt + p, msg_type, 3);
	p += 3;
	memcpy(*opt + p, sub_mask, 6);
	p += 6;
	memcpy(*opt + p, rout_list, (2 + (ROUTER_LIST_SIZE * addr_size)));
	p += (2 + (ROUTER_LIST_SIZE * addr_size));
	memcpy(*opt + p, dn_list, (2 + (DOMAIN_LIST_SIZE * addr_size)));
	p += (2 + (DOMAIN_LIST_SIZE * addr_size));
	if (PARAM_HOSTNAME != NULL) {
		memcpy(*opt + p, host_name, 6);
		p += 2;
		memcpy(*opt + p, PARAM_HOSTNAME, host_name_length);
		p += host_name_length;
	}
	memcpy(*opt + p, msg_lease, 6);
	p += 6;
	memcpy(*opt + p, serv_ident, 6);
	p += 6;
	memcpy(*opt + p, &end_opt, 1);
	p += 1;

	free(rout_list);
	free(dn_list);
	return size;
}

int getDhcpReleaseOptions(char** opt) {
	int p, size;
	u_int32_t aux_lease;
	u_int8_t magic_c[4];
	u_int8_t msg_type[3];
	u_int8_t srv_ident[6];
	u_int8_t end_opt;

	aux_lease = LEASE;

	magic_c[0] = 99;
	magic_c[1] = 130;
	magic_c[2] = 83;
	magic_c[3] = 99;

	msg_type[0] = 0x35;
	msg_type[1] = 0x1;
	msg_type[2] = 0x7;

	srv_ident[0] = 0x36;
	srv_ident[1] = 0x4;
	memcpy(&srv_ident[2], &SERVER_ADDRESS, 4);

	end_opt = 0xFF;

	size = 64;
	*opt = malloc(size);

	bzero(*opt, size);
	p = 0;
	memcpy(*opt, magic_c, 4);
	p += 4;
	memcpy(*opt + p, msg_type, 3);
	p += 3;
	memcpy(*opt + p, srv_ident, 6);
	p += 6;
	memcpy(*opt + p, &end_opt, 1);
	p += 1;

	return size;
}

// Devuelve el tamaño del mensaje de la estructura dhcp delvuelta mediante el puntero al primer parámetro
// -1 Si no correspondía a una estructura dhcp
int get_dhcpH_from_ipM(struct mdhcp_t * dhcp, char * ip_msg, int ip_msg_len) {
	u_int16_t packet_total_len;
	int ip_udp_header_size = 28;
	struct msg_dhcp_t dhcpM;
	int ret = 0;

	// Obtenemos el tamaño del paquete desde la cabecera ip
	packet_total_len = (ip_msg[2] << 8) + ip_msg[3];
	printDebug("get_dhcpH_from_ipM", "tamaño total del paquete %d\n",packet_total_len);

	// Comprueba que sea udp y en ese caso que sea dhcp
	if(ip_msg[9] == 0x11){ // UDP
		if(ip_msg[20] == 0 && ip_msg[21] == 67 && ip_msg[22] == 0 && ip_msg[23] == 68){ // DHCP
			dhcpM.msg = (unsigned char *) ip_msg + ip_udp_header_size;
			dhcpM.length = ip_msg_len - ip_udp_header_size;
			printDebug("get_dhcpH_from_ipM", "el paquete es dhcp\n");
		}
		else ret = -1;
	}
	else ret = -1;

	if(ret == -1){
		printDebug("get_dhcpH_from_ipM", "el paquete no es dhcp\n");
		return -1;
	}

	if(from_message_to_mdhcp(dhcp, &dhcpM) >= 0)
		return packet_total_len;
	return -1;
}

// si es dhcp devuelve 0, en caso contrario -1
int isDhcp( char* ip, int len){
	int ret = -1;
	if(len >= 28 && ip[9] == 0x11){ // UDP
			if(ip[20] == 0 && ip[21] == 67 && ip[22] == 0 && ip[23] == 68){
				ret = 0;
			}
	}
	return ret;
}

// si es udp devuelve 0, en caso contrario -1
int isUdp( char* ip, int len){
	int ret = -1;
	if(len >= 20 && ip[9] == 0x11){ // UDP
				ret = 0;
	}
	return ret;
}

// Devuelve el tamaño del paquete ip contenido dentro del buffer pasado
int getIpPacketLen( char* buf, int len){
	int ret = -1;
	if(len >= 20){
		// Obtenemos el tamaño del paquete desde la cabecera ip
		ret = (buf[2] << 8) + buf[3];
	}
	return ret;
}

// 1 si es Ack, 0 en caso contrario, -1 error
int isAckMsg(struct mdhcp_t* dhcp){
	int ret = -1;
	int pos = 0;
	char actual;

	if(((u_int8_t)dhcp->options[0]) == 0x63 && ((u_int8_t)dhcp->options[1]) == 0x82 && ((u_int8_t)dhcp->options[2]) == 0x53 && ((u_int8_t)dhcp->options[3]) == 0x63){
		pos += 4;
		while(pos < dhcp -> opt_length && ret == -1){
			actual = dhcp->options[pos];
			if(actual == 53){ // Tipo de mensaje
				// es ACK
				if(dhcp->options[pos+1] == 1 && dhcp->options[pos+2] == 05)
					ret = 1;
				// es NACK
				else if(dhcp->options[pos+1] == 1 && dhcp->options[pos+2] == 06)
					ret = 0;
			}
			// Se salta el resto de la opción
			else pos += dhcp->options[pos+1]+2;
		}
	}
	else printDebug("isAckMsg", "Wrong Magic cookie");
	return ret;
}

// 0 si es offer, -1 en caso contrario
int isOfferMsg(struct mdhcp_t* dhcp){
	int ret = -1;
	int pos = 0;
	char actual;

	if(((u_int8_t)dhcp->options[0]) == 0x63 && ((u_int8_t)dhcp->options[1]) == 0x82 && ((u_int8_t)dhcp->options[2]) == 0x53 && ((u_int8_t)dhcp->options[3]) == 0x63){
		pos += 4;
		while(pos < dhcp -> opt_length && ret == -1){
			actual = dhcp->options[pos];
			printf("actual%d\n",actual);
			if(actual == 53){ // Tipo de mensaje
				// es Offer
				if(dhcp->options[pos+1] == 1 && dhcp->options[pos+2] == 02)
					ret = 0;
			}
			// Se salta el resto de la opción
			else pos += dhcp->options[pos+1]+2;
		}
	}
	else printDebug("isOfferMsg", "Wrong Magic cookie");
	return ret;
}
