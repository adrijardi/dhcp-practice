/*
 * transfer.c
 *
 *  Created on: 06-nov-2008
 *      Author: dconde
 */

#include <limits.h>
#include <string.h>
#include <stdio.h>

#include "constants.h"
#include "dhcp_state.h"
#include "f_messages.h"

//Metodos internos
int sendMSG(struct msg_dhcp_t *message);
int sendRAW_Msg(struct msg_dhcp_t *message);
int sendUDP_Msg(struct msg_dhcp_t *message);

int sendDHCPDISCOVER(){
	printf("Vamos a petar\n");
	int ret = EXIT_ERROR;
	unsigned int xid;
	double r;
	struct mdhcp_t* dhcpdiscover;
	struct msg_dhcp_t* message;

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

	// Se extrae el mensaje
	message = from_mdhcp_to_message(dhcpdiscover);
	// Se envia el mensaje
	if(sendMSG(message) == true){
		//state = SELECTING; // TODO se necesita sincronización multihilo?
		ret = EXIT_NORMAL;
	}else{
		fprintf(stderr,"ERROR: No se ha podido mandar el mensaje dhcpdiscover.");
	}

	free_message(message);
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

int sendRAW_Msg(struct msg_dhcp_t *message){
	return true;
}
int sendUDP_Msg(struct msg_dhcp_t *message){
	return true;
}

