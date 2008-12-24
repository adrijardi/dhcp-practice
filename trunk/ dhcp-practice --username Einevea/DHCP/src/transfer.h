/*
 * transfer.h
 *
 *  Created on: 06-nov-2008
 *      Author: dconde
 */

#ifndef TRANSFER_H_
#define TRANSFER_H_

#include "f_messages.h"

int sendDHCPDISCOVER();
int sendDHCPREQUEST(struct offerIP* selected_ip);
int get_selecting_messages(struct mdhcp_t *** messages);



#endif /* TRANSFER_H_ */
