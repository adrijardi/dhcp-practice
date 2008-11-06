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

int sendDHCPDISCOVER(){
	unsigned int xid;
	double r;
	r = (double)random()/(double)RAND_MAX;
	xid = UINT_MAX *r;
	struct mdhcp_t* dhcpdiscover;
	dhcpdiscover = new_default_mdhcp();
	dhcpdiscover->op = DHCP_OP_BOOTREQUEST;
	dhcpdiscover->htype = 0;//TODO mirar el rfc.
	dhcpdiscover->hlen = 6;
	dhcpdiscover->xid = xid;
	dhcpdiscover->secs = 0; //TODO preguntar al profesor
	memcpy(dhcpdiscover->chaddr, haddress, dhcpdiscover->hlen); //TODO pillar dirección mac de mi maquina
	//dhcpdiscover->sname= ""; //TODO mirar options?
	//dhcpdiscover->file= ""; //TODO mirar options?
	dhcpdiscover->options= NULL; //TODO mirar options?

	print_mdhcp(dhcpdiscover);

	free_mdhcp(dhcpdiscover);
	return EXIT_NORMAL; //TODO
}

