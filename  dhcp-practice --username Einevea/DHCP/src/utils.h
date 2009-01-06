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
#include <stdarg.h>

#include "constants.h"
#include "dhcp_state.h"
#include "f_messages.h"
#include "structs.h"

void printTrace(int xid, enum dhcp_message state, char* str);
void time_wait(int microsec);
void obtainHardwareAddress();
int obtain_ifindex();
void setMSGInfo(struct mdhcp_t ip_list[]);
int set_device_ip();
int set_device_netmask();
int set_device_router();
int up_device_if_down(const char* interface);
void device_down(const char* interface);
void printDebug(char* method, const char *string, ...);
int pow_utils(int, int);
void reset_timeout();
void get_next_timeout(struct timeval *tv);
void decrease_timeout(struct timeval *tv, struct timeval *init, struct timeval *end);
int compare_haddress(char *);

#endif /* UTILS_H_ */
