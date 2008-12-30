/*
 * dhcp_state.h
 *
 *  Created on: 06-nov-2008
 *      Author: dconde
 */

#ifndef DHCP_STATE_H_
#define DHCP_STATE_H_
#include "constants.h"

char *iface, *hostname, *address, *haddress;
enum dhcp_states state;
int exit_value, timeout, debug, haddress_size, dhcp_socket, no_exit;
u_int32_t lease;
unsigned int xid;

// Direcciones y Options recibidas;
struct in_addr selected_address;
struct in_addr server_address;
struct sockaddr* subnet_mask;
struct in_addr* routers_list;
struct in_addr* domain_name_server_list;
char * domain_name;
u_int32_t lease;



#endif /* DHCP_STATE_H_ */
