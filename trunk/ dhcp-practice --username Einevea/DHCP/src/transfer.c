/*
 * transfer.c
 *
 *  Created on: 06-nov-2008
 *      Author: dconde
 */

#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <netpacket/packet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include "constants.h"
#include "dhcp_state.h"
#include "f_messages.h"
#include "utils.h"

//Metodos internos
int sendMSG(struct msg_dhcp_t *message);
int sendETH_Msg(struct mdhcp_t*, in_addr_t address);
int sendUDP_Msg(struct mdhcp_t*, in_addr_t address );

int sendDHCPDISCOVER(){
	int ret = EXIT_ERROR;
	unsigned int xid;
	double r;
	struct mdhcp_t* dhcpdiscover;
	char ** options;
	int opt_size;

	// Se genera un xid aleatorio
	r = (double)random()/(double)RAND_MAX;
	xid = UINT_MAX *r;

	// Se crea la estructura del mensaje con los datos adecuados
	dhcpdiscover = new_default_mdhcp();
	dhcpdiscover->op = DHCP_OP_BOOTREQUEST;
	dhcpdiscover->hlen = 6;
	dhcpdiscover->xid = xid;
	dhcpdiscover->secs = 0; //TODO preguntar al profesor
	memcpy(dhcpdiscover->chaddr, haddress, dhcpdiscover->hlen);

	options = malloc(4);
	opt_size = getDhcpOptions(options, DHCPDISCOVER);
	dhcpdiscover->options= *options; //TODO mirar options?
	dhcpdiscover->opt_length = opt_size;
// Prueba
	//print_mdhcp(dhcpdiscover); //TODO quitar

	// Se envia el mensaje dhcp discover a broadcast
	if(sendETH_Msg(dhcpdiscover, INADDR_BROADCAST) >= 0){
		//state = SELECTING; // TODO se necesita sincronización multihilo?
		ret = true;
	}else{
		fprintf(stderr,"ERROR: No se ha podido mandar el mensaje dhcpdiscover.\n");
	}

	free(options);
	free_mdhcp(dhcpdiscover);

	return ret;
}


int sendMSG(struct msg_dhcp_t *message){
	int ret = false;

	/*switch(state){
	case INIT:
	case SELECTING:
		ret = sendRAW_Msg(message);
		break;
	default:
		ret = sendUDP_Msg(message);
		break;
	}*/

	return ret;
}

/* ANTIGUO
int sendETH_Msg(struct mdhcp_t *dhcpStuct, in_addr_t address ){
	struct sockaddr_in	addr; // Direccion de envio
	unsigned char** msg;
	size_t size;
	int ret, sock, enviado;
	ret = 0;

	// Creamos el socket
	sock = socket (PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));
	if(sock == -1){
		perror("socket");
		ret = -1;
	}

	// Definimos parametros de configuracion para el envio
	///Se inicia la direccion de destino
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = address;
	addr.sin_port = htons(SERVER_PORT);

	/// Definimos el mensaje, inclusion de cabeceras...
	msg = malloc(4);
	size = getETHMessage(msg, address, dhcpStuct);
	printf("El tamaño del pakete es %d\n", size);


	// Se realiza el envio
	enviado = sendto(sock, msg, size, 0, (struct sockaddr *)&addr, sizeof (struct sockaddr_in));
	if(enviado == -1){
		perror("sendto");
		ret = -1;
	}
	printf("Enviado %d\n", enviado);

	return ret;
}*/

int sendETH_Msg(struct mdhcp_t *dhcpStuct, in_addr_t address ){
	struct sockaddr_ll	addr; // Direccion de envio
	unsigned char** msg;
	size_t size;
	int ret, sock, enviado;
	ret = 0;

	// Creamos el socket
	sock = socket (PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));
	if(sock == -1){
		perror("socket");
		ret = -1;
	}

	// Definimos parametros de configuracion para el envio
	///Se inicia la direccion de destino
	bzero(&addr, sizeof(struct sockaddr_ll));
	addr.sll_family = AF_PACKET;
	addr.sll_addr[0] = 255;
	addr.sll_addr[1] = 255;
	addr.sll_addr[2] = 255;
	addr.sll_addr[3] = 255;
	addr.sll_addr[4] = 255;
	addr.sll_addr[5] = 255;
	addr.sll_addr[6] = 0;
	addr.sll_addr[7] = 0;
	addr.sll_halen = 6;
	addr.sll_ifindex = obtain_ifindex();
	if(addr.sll_ifindex < 0)
		ret = -1;
	addr.sll_hatype = 0xFFFF;
	addr.sll_protocol = htons(ETH_P_IP);
	addr.sll_pkttype = PACKET_BROADCAST;

	if(addr.sll_ifindex == -1){
		ret = -1;
	}

	if(ret >= 0){
		/// Definimos el mensaje, inclusion de cabeceras...
		msg = malloc(4);
		size = getETHMessage(msg, address, dhcpStuct);
		//printf("El tamaño del pakete es %d\n", size);


		// Se realiza el envio
		enviado = sendto(sock, *msg, size, 0, (struct sockaddr *)&addr, sizeof (struct sockaddr_ll));
		if(enviado == -1){
			perror("sendto");
			ret = -1;
		}
		printf("Enviado %d\n", enviado);
	}

	return ret;
}


int sendUDP_Msg(struct mdhcp_t *message, in_addr_t address ){
	return true;
}

