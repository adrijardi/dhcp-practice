/*
 * f_messages.h
 *
 *  Created on: 21-oct-2008
 *      Author: dconde
 */

#ifndef F_MESSAGES_H_
#define F_MESSAGES_H_
// Tamanyo base de dhcp
#define DHCP_BSIZE 236;
// Valores de las variables de un mensaje dhcp
#define DHCP_OP_BOOTREQUEST 1;
#define DHCP_OP_BOOTREPLY 2;

#define DHCP_FLAGS_NO_BROADCAST 0;
#define DHCP_FLAGS_BROADCAST 32768;

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include "utils.h"
#include "constants.h"
#include "dhcp_state.h"
#include "structs.h"

struct mdhcp_t* new_default_mdhcp();
struct msg_dhcp_t* from_mdhcp_to_message(struct mdhcp_t *str_dhcp);
int from_message_to_mdhcp(struct mdhcp_t * dhcp, struct msg_dhcp_t *message);
void free_mdhcp(struct mdhcp_t *str_dhcp);
void free_message(struct msg_dhcp_t *message);
void print_mdhcp(struct mdhcp_t *str_dhcp);
void print_message(struct msg_dhcp_t *msg);

int getDhcpDiscoverOptions(char** opt);
int getDhcpRequestOptions(char** opt);
int getDhcpReleaseOptions(char** opt);

int getETHMessage(unsigned char** , in_addr_t , struct mdhcp_t* );

//Obtiene la estructura dhcp a partir del mensaje ethernet
int get_dhcpH_from_ipM(struct mdhcp_t * dhcp, char * ip_msg, int ip_msg_len);
int isDhcp( char* ip, int len);
int isUdp( char* ip, int len);
int getIpPacketLen( char* buf, int len);

#endif /* F_MESSAGES_H_ */
