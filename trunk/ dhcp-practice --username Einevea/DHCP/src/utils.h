/*
 * utils.h
 *
 *  Created on: 21-oct-2008
 *      Author: dconde
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if_ether.h>
#include <string.h>
#include <stdlib.h>

#include "constants.h"
#include "dhcp_state.h"
#include "f_messages.h"
#include "structs.h"

void printTrace(int xid, enum dhcp_message state, char* str);
void time_wait(int microsec);
void obtainHardwareAddress();
int obtain_ifindex();
struct offerIP* select_ip(struct mdhcp_t ** ip_list);

pthread_mutex_t * lock;
pthread_mutex_t * lock_params;

#endif /* UTILS_H_ */
