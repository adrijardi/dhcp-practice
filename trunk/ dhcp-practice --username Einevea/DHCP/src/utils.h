/*
 * utils.h
 *
 *  Created on: 21-oct-2008
 *      Author: dconde
 */

#ifndef UTILS_H_
#define UTILS_H_

#include "constants.h"
#include "f_messages.h"

void printTrace(int xid, enum dhcp_message state, char* str);
void time_wait(int microsec);
void obtainHardwareAddress();
int obtain_ifindex();
struct offerIP* select_ip(struct mdhcp_t ** ip_list);

struct offerIP{
	uint offered_ip;
	uint server_ip;
};

#endif /* UTILS_H_ */
