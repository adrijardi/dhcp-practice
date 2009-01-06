/*
 * transfer.c
 *
 *  Created on: 06-nov-2008
 *      Author: dconde
 */
#include "transfer.h"

//Metodos internos
int sendMSG(struct msg_dhcp_t *message);
int sendETH_Msg(struct mdhcp_t*, in_addr_t address);
int sendUDP_Msg(unsigned char* msg, uint len, struct in_addr * address);

int sock_packet; // Socket de envio / recepción a nivel packet
struct sockaddr_ll addr_packet; // Dirección de envio

int init_sockets() {
	int ret = 0;
	int on = 1;
	// Creación del socket de recepción
	sock_packet = socket(PF_PACKET,SOCK_DGRAM, htons(ETH_P_IP));
	setsockopt(sock_packet, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
	if (sock_packet < 0) {
		perror("socket");
		ret = -1;
	}
	if (ret >= 0) {
		// Definición de la dirección de recepción
		bzero(&addr_packet, sizeof(struct sockaddr_ll));
		addr_packet.sll_family = AF_PACKET;
		addr_packet.sll_addr[0] = 255;
		addr_packet.sll_addr[1] = 255;
		addr_packet.sll_addr[2] = 255;
		addr_packet.sll_addr[3] = 255;
		addr_packet.sll_addr[4] = 255;
		addr_packet.sll_addr[5] = 255;
		addr_packet.sll_addr[6] = 0;
		addr_packet.sll_addr[7] = 0;
		addr_packet.sll_halen = 6;
		addr_packet.sll_ifindex = obtain_ifindex();
		if (addr_packet.sll_ifindex < 0)
			ret = -1;
		addr_packet.sll_hatype = 0xFFFF;
		addr_packet.sll_protocol = htons(ETH_P_IP);
		addr_packet.sll_pkttype = PACKET_BROADCAST;

		if (addr_packet.sll_ifindex == -1) {
			ret = -1;
		}

		if (ret >= 0) {
			ret = bind(sock_packet, (struct sockaddr *) &addr_packet,
					sizeof(struct sockaddr_ll));
			if (ret < 0)
				perror("bind");
		}
	}
	return ret;
}

void close_sockets() {
	if (close(sock_packet) < 0)
		perror("close");
}

int sendDHCPDISCOVER() {
	int ret = EXIT_ERROR;
	double r;
	struct mdhcp_t* dhcpdiscover;
	char ** options;
	int opt_size;

	// Se genera un xid aleatorio
	r = (double) random() / (double) RAND_MAX;
	XID = UINT_MAX * r;

	// Se crea la estructura del mensaje con los datos adecuados
	dhcpdiscover = new_default_mdhcp();
	dhcpdiscover->op = DHCP_OP_BOOTREQUEST;
	dhcpdiscover->hlen = 6;
	dhcpdiscover->xid = XID;
	dhcpdiscover->secs = 0;
	memcpy(dhcpdiscover->chaddr, HADDRESS, dhcpdiscover->hlen);

	options = malloc(4);
	opt_size = getDhcpDiscoverOptions(options);

	dhcpdiscover->options = *options;
	dhcpdiscover->opt_length = opt_size;

	// Se envia el mensaje dhcp discover a broadcast
	printTrace(XID, DHCPDISCOVER, NULL);
	if (sendETH_Msg(dhcpdiscover, INADDR_BROADCAST) >= 0) {
		ret = TRUE;
	} else {
		fprintf(stderr,
		"ERROR: No se ha podido mandar el mensaje dhcpdiscover.\n");
	}

	free(options);
	free_mdhcp(dhcpdiscover);
	return ret;
}

int sendDHCPREQUEST() {
	int ret = EXIT_ERROR;
	struct mdhcp_t* dhcpRequest;
	char ** options;
	int opt_size;

	printDebug("sendDHCPREQUEST", "enviando dhcpRequest");

	// Se crea la estructura del mensaje con los datos adecuados
	dhcpRequest = new_default_mdhcp();

	dhcpRequest->op = DHCP_OP_BOOTREQUEST;
	dhcpRequest->hlen = 6;
	dhcpRequest->xid = XID;
	dhcpRequest->secs = 0;
	memcpy(dhcpRequest->chaddr, HADDRESS, dhcpRequest->hlen);

	options = malloc(4);
	opt_size = getDhcpRequestOptions(options);
	dhcpRequest->options = (*options);
	dhcpRequest->opt_length = opt_size;

	// Se envia el mensaje dhcp request a broadcast
	if (sendETH_Msg(dhcpRequest, INADDR_BROADCAST) >= 0) {
		printTrace(XID, DHCPREQUEST, inet_ntoa(SERVER_ADDRESS));
		ret = TRUE;
	} else {
		fprintf(stderr,
		"ERROR: No se ha podido mandar el mensaje dhcpRequest.\n");
	}

	free(options);
	free_mdhcp(dhcpRequest);
	return ret;
}

int sendDHCPRELEASE() {
	struct mdhcp_t * dhcp_msg;
	struct msg_dhcp_t * msg;
	char ** options;
	int opt_size;
	int ret;

	printDebug("sendDHCPRELEASE", "");
	dhcp_msg = new_default_mdhcp();

	// Se crea la estructura del mensaje con los datos adecuados
	dhcp_msg->op = DHCP_OP_BOOTREQUEST;
	dhcp_msg->hlen = 6;
	dhcp_msg->xid = XID;
	dhcp_msg->secs = 0;
	memcpy(&dhcp_msg->ciaddr, &SELECTED_ADDRESS.s_addr, sizeof(in_addr_t));
	memcpy(dhcp_msg->chaddr, HADDRESS, dhcp_msg->hlen);

	options = malloc(4);
	opt_size = getDhcpReleaseOptions(options);
	dhcp_msg->options = (*options);
	dhcp_msg->opt_length = opt_size;

	msg = from_mdhcp_to_message(dhcp_msg);

	ret = sendUDP_Msg(msg->msg, msg->length, &SERVER_ADDRESS);
	//ret = sendETH_Msg(dhcp_msg, SERVER_ADDRESS.s_addr);

	if (ret >= 0) {
		printTrace(XID, DHCPRELEASE, "Pedazo cara de culo!");
	}

	free_mdhcp(dhcp_msg);
	free_message(msg);
	free(options);
	return ret;
}

int sendETH_Msg(struct mdhcp_t *dhcpStuct, in_addr_t address) {
	unsigned char** msg;
	size_t size;
	int ret, enviado;
	ret = 0;
	/// Definimos el mensaje, inclusion de cabeceras...
	msg = malloc(4);
	size = getETHMessage(msg, address, dhcpStuct);
	//printDebug("sendETH_Msg", "El tamaño del pakete es %d", size);

	// Se realiza el envio
	enviado = sendto(sock_packet, *msg, size, 0,
			(struct sockaddr *) &addr_packet, sizeof(struct sockaddr_ll));
	if (enviado == -1) {
		perror("sendto");
		ret = -1;
	}
	printDebug("sendETH_Msg", "Enviado %d", enviado);

	free(*msg);
	free(msg);
	return ret;
}

int sendUDP_Msg(unsigned char* msg, uint len, struct in_addr * ip_address) {
	int sock_inet;
	struct sockaddr_in addr_inet;
	struct sockaddr_in local_addr;
	int ret = 0;
	int on = 1;

	sock_inet = socket(AF_INET,SOCK_DGRAM, 0);
	setsockopt(sock_inet, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
	if (sock_inet < 0)
		perror("socket");
	else {
		addr_inet.sin_addr = *ip_address;
		addr_inet.sin_family = AF_INET;
		addr_inet.sin_port = htons(SERVER_PORT);

		local_addr.sin_addr = SELECTED_ADDRESS;
		local_addr.sin_family = AF_INET;
		local_addr.sin_port = htons(CLIENT_PORT);

		ret = bind(sock_inet, (struct sockaddr*) &local_addr,
				sizeof(struct sockaddr_in));
		if (ret < 0)
			perror("bind");
		else {
			ret = sendto(sock_inet, msg, len, 0, (struct sockaddr*) &addr_inet,
					sizeof(struct sockaddr_in));
			if (ret < 0) {
				perror("sendto");
			}
		}
	}
	close(sock_inet);
	return ret;
}

// TODO comprobar que se cierran todos los socket

// Recive todos los mensajes de dhcpOffer
int get_selecting_messages(struct mdhcp_t messages[]) {
	fd_set recvset;
	struct timeval tv, init, end;
	int ret = 1;
	char * buf = malloc(1000);
	int num_dhcp = 0;
	char * msg_string;
	char *str_serv_addr;
	char *str_ip_addr;
	struct in_addr serv_addr_temp;
	struct in_addr ip_addr_temp;
	char * straux;
	//int packet_size;

	// Se establecen los sets de descriptores
	FD_ZERO(&recvset);
	FD_SET(sock_packet, &recvset);

	//Recibimos multiples respuestas
	ret = 1;

	// Tiempo de espera del select - Hay que hacerlo en cada iteracción del buble
	get_next_timeout(&tv);
	printDebug("get_selecting_messages", "Timeout sec:%d, usec:%d", tv.tv_sec, tv.tv_usec);
	gettimeofday(&init, NULL);

	while (ret > 0 && num_dhcp < MAXDHCPOFFERS) {
		ret = select(sock_packet + 1, &recvset, NULL, NULL, &tv);
		gettimeofday(&end, NULL);
		decrease_timeout(&tv, &init, &end);
		printDebug("get_selecting_messages", "Timeout sec:%d, usec:%d", tv.tv_sec, tv.tv_usec);

		if(ret < 0) {
			perror("select");
			printDebug("get_selecting_messages", "Fin timeout?");
		} else if(ret> 0) {
			int recv_size = recvfrom(sock_packet, buf, 1000, 0, NULL, NULL);

			if(get_dhcpH_from_ipM(&messages[num_dhcp], buf, recv_size) >= 0){

				// Se comprueba que el mensaje responda al ultimo Xid y que tenga el mismo chaddr
				if(messages[num_dhcp].xid == XID && compare_haddress(messages[num_dhcp].chaddr)) {
					if(isOfferMsg(&messages[num_dhcp]) == 0){
						msg_string = malloc(60);

						ip_addr_temp.s_addr = ntohl(messages[num_dhcp].yiaddr);
						serv_addr_temp.s_addr = ntohl(messages[num_dhcp].siaddr);

						straux = inet_ntoa(serv_addr_temp);
						str_serv_addr = malloc(strlen(straux)+1);
						bzero(str_serv_addr, strlen(straux)+1);
						memcpy(str_serv_addr, straux, strlen(straux));

						straux = inet_ntoa(ip_addr_temp);
						str_ip_addr = malloc(strlen(straux)+1);
						bzero(str_ip_addr, strlen(straux)+1);
						memcpy(str_ip_addr, straux, strlen(straux));

						sprintf(msg_string, "%s (offered %s)",str_serv_addr, str_ip_addr);

						printTrace(messages[num_dhcp].xid, DHCPOFFER, msg_string);

						free(msg_string);
						printDebug("get_selecting_messages", "ipOrigen %d",messages[num_dhcp].siaddr);
						printDebug("get_selecting_messages", "id %d",messages[num_dhcp].xid);

						num_dhcp++;
					}else{
						printDebug("get_selecting_messages", "Mensaje no DHCP Offer");
					}
				}else{
					printDebug("get_selecting_messages", "Distinto Xid %d o chaddr %s", messages[num_dhcp].xid, messages[num_dhcp].chaddr);
					//free(messages[num_dhcp].options); TODO mirar memoria
				}
			}else printDebug("get_selecting_messages", "El mensaje no es dhcp\n");
		}
	}
		ret = num_dhcp;
		free(buf);
		free(str_serv_addr);
		free(str_ip_addr);

		return ret;
	}

// 1 en caso de ACK, 0 en caso de NACK, -1 en caso de error
int get_ACK_message() {
	fd_set recvset;
	struct timeval tv, init, end;
	int ret = 1;
	char * buf = malloc(1000);
	//uint buf_ocupation = 0;
	struct mdhcp_t dhcp_recv;
	int ack = -1;
	int packet_size;
	char tempmsg[50];

	// Se establecen los sets de descriptores
	FD_ZERO(&recvset);
	FD_SET(sock_packet, &recvset);

	// Tiempo de espera del select TODO- Hay que hacerlo en cada iteracción del buble
	get_next_timeout(&tv);
	printDebug("get_ACK_message", "Timeout sec:%d, usec:%d", tv.tv_sec, tv.tv_usec);
	gettimeofday(&init, NULL);

	//Recibimos multiples respuestas
	ret = 1;
	while (ret > 0 && ack == -1) {
		// Select
		ret = select(sock_packet + 1, &recvset, NULL, NULL, &tv);
		gettimeofday(&end, NULL);
		decrease_timeout(&tv, &init, &end);
		printDebug("get_ACK_message", "Timeout sec:%d, usec:%d", tv.tv_sec, tv.tv_usec);

		if(ret < 0) {
			perror("select");
		} else if(ret> 0) {

			// Lee un buffer de 1000 bytes
			int recv_size = recvfrom(sock_packet, buf, 1000, 0, NULL, NULL);
			printDebug("get_ACK_message", "recibido %d",recv_size);

			packet_size = getIpPacketLen(buf, recv_size);
			if(isUdp(buf, recv_size) == 0){
				if(isDhcp(buf, recv_size) == 0){
					if(get_dhcpH_from_ipM(&dhcp_recv, buf, recv_size) > 0){
						// Se comprueba que el mensaje responda al ultimo Xid
						if(dhcp_recv.xid == XID) {
							ack = isAckMsg(&dhcp_recv);
							if(ack == -1){
								printDebug("get_ACK_message", "El paquete ack no es válido\n");
							}

							printDebug("get_ACK_message", "ipOrigen %d",dhcp_recv.siaddr);
							printDebug("get_ACK_message", "id %d",dhcp_recv.xid);
						} else {
							printDebug("get_ACK_message", "Distinto Xid");
							//free(&dhcp_recv); TODO posible memory leak?
						}
					}
				}
				else printDebug("get_ACK_message", "El paquete no es dhcp\n");
			}
			else printDebug("get_ACK_message", "El paquete no es udp\n");
		}
	}
	bzero(tempmsg, 50);
	if(ack == 0){
		printTrace(dhcp_recv.xid, DHCPNAK, NULL);
	}
	else if(ack == 1){
		if( LEASE == 0xffffffff){
			sprintf(tempmsg, "%s with leasing inf seconds", inet_ntoa(SELECTED_ADDRESS));
		}else{
			sprintf(tempmsg, "%s with leasing %u seconds", inet_ntoa(SELECTED_ADDRESS), LEASE);
		}
		printTrace(dhcp_recv.xid, DHCPACK, tempmsg);
	}
	ret = ack;
	if(dhcp_recv.opt_length> 0)
		free(dhcp_recv.options);
	free(buf);

	return ret;
}

