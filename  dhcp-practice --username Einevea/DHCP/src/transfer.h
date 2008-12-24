/*
 * transfer.h
 *
 *  Created on: 06-nov-2008
 *      Author: dconde
 */

#ifndef TRANSFER_H_
#define TRANSFER_H_

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
#include "utils.h"
#include "f_messages.h"

int sendDHCPDISCOVER();
void* sendDHCPREQUEST(void * arg);
int get_selecting_messages(struct mdhcp_t *** messages);



#endif /* TRANSFER_H_ */
