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
#include <netinet/in.h>
#include <arpa/inet.h>

#include "constants.h"
#include "dhcp_state.h"
#include "f_messages.h"

//Metodos internos
int sendMSG(struct msg_dhcp_t *message);
int sendRAW_Msg(struct mdhcp_t*);
int sendUDP_Msg(struct mdhcp_t*);

int sendDHCPDISCOVER(){
	printf("Vamos a petar\n");
	int ret = EXIT_ERROR;
	unsigned int xid;
	double r;
	struct mdhcp_t* dhcpdiscover;

	// Se genera un xid aleatorio
	r = (double)random()/(double)RAND_MAX;
	xid = UINT_MAX *r;

	// Se crea la estructura del mensaje con los datos adecuados
	dhcpdiscover = new_default_mdhcp();
	dhcpdiscover->op = DHCP_OP_BOOTREQUEST;
	dhcpdiscover->htype = 0;//TODO mirar el rfc.
	dhcpdiscover->hlen = 6;
	dhcpdiscover->xid = xid;
	dhcpdiscover->secs = 0; //TODO preguntar al profesor
	memcpy(dhcpdiscover->chaddr, haddress, dhcpdiscover->hlen);
	//dhcpdiscover->sname= ""; //TODO mirar options?
	//dhcpdiscover->file= ""; //TODO mirar options?
	dhcpdiscover->options= NULL; //TODO mirar options?
// Prueba
	print_mdhcp(dhcpdiscover); //TODO quitar

	// Se envia el mensaje
	if(sendRAW_Msg(dhcpdiscover) == true){
		//state = SELECTING; // TODO se necesita sincronización multihilo?
		ret = true;
		printf("guay\n");
	}else{
		fprintf(stderr,"ERROR: No se ha podido mandar el mensaje dhcpdiscover.\n");
	}

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

int sendRAW_Msg(struct mdhcp_t *dhcpStuct){
	struct ip_header_t* ipHeader;
	struct udp_header_t* udpHeader;
	unsigned char* msg;
	int size;
	struct sockaddr_in dest;
	int sock;
	int enviado;
	char algo[5];
	strcpy(algo, "hola");

	ipHeader = new_default_ipHeader();
	udpHeader = new_default_udpHeader();

	size = getRawMessage(msg, ipHeader, udpHeader, dhcpStuct);

	printf("El tamaño del pakete es %d\n", size);

	//dest = malloc(sizeof(struct sock_addr_in));
	dest.sin_family = PF_PACKET;
	//dest.sin_addr.s_addr = inet_addr("255.255.255.255");
	if(inet_aton("163.117.83.28", &dest.sin_addr) == 0){
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}
	dest.sin_port = htons(SERVER_PORT);

	sock = socket (PF_PACKET, SOCK_DGRAM, 0);
	printf("socket: %d\n",sock);
	enviado = sendto(sock,
			algo,
			5,
			0, //Routing flags
			(struct sockaddr*) &dest,
			sizeof(struct sockaddr));


	printf("Enviado %d\n", enviado);

	return true; // TODO devolver lo que tenga que ser
}
int sendUDP_Msg(struct mdhcp_t *message){
	return true;
}

