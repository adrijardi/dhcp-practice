/*
 * dhcp_state.h
 *
 *  Created on: 06-nov-2008
 *      Author: dconde
 */

#ifndef DHCP_STATE_H_
#define DHCP_STATE_H_
#include "constants.h"

char *IFACE, *HOSTNAME, *ADDRESS, *HADDRESS;
enum dhcp_states STATE;
int EXIT_VALUE, TIMEOUT, DEBUG, HADDRESS_SIZE, DHCP_SOCKET, NO_EXIT;
u_int32_t LEASE;
unsigned int XID;

// Direcciones y Options recibidas;
struct in_addr SELECTED_ADDRESS;
struct in_addr SERVER_ADDRESS;
struct sockaddr_in* SUBNET_MASK;
int ROUTER_LIST_SIZE;
struct in_addr* ROUTERS_LIST;
int DOMAIN_LIST_SIZE;
struct in_addr* DOMAIN_NAME_SERVER_LIST;
char * DOMAIN_NAME;



#endif /* DHCP_STATE_H_ */
