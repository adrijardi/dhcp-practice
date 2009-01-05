/*
 * utils.c
 *
 *  Created on: 21-oct-2008
 *      Author: dconde
 */

#include "utils.h"

char * getTimestamp() {
	char* timestamp;
	time_t t;
	struct tm *local, *gtm;
	int h, m;

	timestamp = malloc(50);
	t = time(NULL);
	local = localtime(&t);
	gtm = gmtime(&t);
	h = (int) local->tm_gmtoff;
	m = (h / 60) % 60; //TODO no funciona?
	h = h / 3600;
	if (h >= 0)
		strftime(timestamp, 50, "%Y-%m-%d %H:%M:%S+", local);
	else
		strftime(timestamp, 50, "%Y-%m-%d %H:%M:%S-", local);

	sprintf(timestamp, "%s%.2d:%.2d", timestamp, h, m);
	return timestamp;
}

void printDebug(char* method, const char *fmt, ...) {
	char *buf;
	char *timestamp;

	buf = malloc(1024);
	va_list ap;
	va_start (ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end (ap);

	timestamp = getTimestamp();
	if (DEBUG == TRUE) {
		printf("-[%s] {%s} %s\n", timestamp, method, buf);
	}
	free(timestamp);
	free(buf);
}

void printTrace(int xid, enum dhcp_message state, char* str) {
	char *timestamp;
	timestamp = getTimestamp();

	if (xid == -1) {

	} else if (xid == -2) {

	} else if (xid == -3) {

	} else {
		switch (state) {
		case DHCPDISCOVER:
			fprintf(stdout,"#[%s] (%u) DHCPDISCOVER sent.\n", timestamp, xid);
			break;
		case DHCPOFFER:
			fprintf(stdout,"#[%s] (%u) DHCPOFFER received from %s.\n",
					timestamp, xid, str);
			break;
		case DHCPREQUEST:
			fprintf(stdout,"#[%s] (%u) DHCPREQUEST sent to %s.\n", timestamp,
					xid, str);
			break;
		case DHCPACK:
			fprintf(stdout,"#[%s] (%u) DHCPACK received: %s.\n", timestamp,
					xid, str);
			break;
		case DHCPNACK:
			fprintf(stdout,"#[%s] (%u) DHCPNACK received.\n", timestamp, xid);
			break;
		case DHCPRELEASE:
			fprintf(stdout,"#[%s] (%u) DHCPRELEASE sent %s.\n", timestamp, xid,
					str);
			break;
		case PID:
			fprintf(stdout,"#[%s] PID=%d.\n", timestamp, getpid());
			break;
		case IP:
			fprintf(stdout,"#[%s] IP %s.\n", timestamp, str);
			break;
		case DHCPSIGINT:
			fprintf(stdout,"#[%s] SIGINT received.\n", timestamp);
			break;
		case DHCPSIGUSR2:
			fprintf(stdout,"#[%s] SIGUSR2 received.\n", timestamp);
			break;
		}
	}
	free(timestamp);
}

void time_wait(int microsec) {
	unsigned long time = microsec * 1000;
	usleep(time);
	time++;
}

void obtainHardwareAddress() {
	int fd;
	struct ifreq ifr;

	fd = socket(PF_INET,SOCK_DGRAM, 0);
	strcpy(ifr.ifr_name, IFACE);
	ioctl(fd, SIOCGIFHWADDR, &ifr);
	if (HADDRESS == NULL)
		HADDRESS = malloc(HADDRESS_SIZE);

	bzero(HADDRESS, HADDRESS_SIZE);
	memcpy(HADDRESS, ifr.ifr_hwaddr.sa_data, HADDRESS_SIZE);
	close(fd);
}

int obtain_ifindex() {
	int fd;
	struct ifreq ifr;
	//TODO cambiar para que solo obtenga mediante ioctl en la primera petición

	fd = socket(PF_INET,SOCK_DGRAM, 0);
	strcpy(ifr.ifr_ifrn.ifrn_name, IFACE);
	if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
		perror("ioctl");
		fprintf(stderr, "ERROR: La interfaz especificada no es válida.\n");

		return -1;
	}

	return ifr.ifr_ifru.ifru_ivalue;
}

void setMSGInfo(struct mdhcp_t ip_list[]) {
	u_int case_;
	int p, i, h, size, lenght;
	u_int32_t aux32;
	u_int32_t aux;
	SELECTED_ADDRESS.s_addr = ntohl(ip_list[0].yiaddr);
	SERVER_ADDRESS.s_addr = ntohl(ip_list[0].siaddr);

	p = 4;
	lenght = (ip_list[0].opt_length) - 2;
	printDebug("setMSGInfo", "l%d", lenght);
	while (p < lenght) {
		case_ = ip_list[0].options[p++];
		printDebug("setMSGInfo", "%u \t %d", case_, p);
		switch (case_) {
		case 1:
			//Subnet Mask
			if (SUBNET_MASK != NULL) {
				free(SUBNET_MASK);
			}
			SUBNET_MASK = malloc(sizeof(struct sockaddr));
			bzero(SUBNET_MASK, sizeof(struct sockaddr_in));
			h = ip_list[0].options[p++];
			i = 0;
			aux = 0;
			while (i < h) {
				aux = aux << 8;
				aux += (u_int8_t)ip_list[0].options[p++];
				i++;
			}
			SUBNET_MASK->sin_addr.s_addr = ntohl(aux);
			break;
		case 3:
			//Router List
			if (ROUTERS_LIST != NULL) {
				free(ROUTERS_LIST);
			}
			ROUTER_LIST_SIZE = ip_list[0].options[p++] / 4;
			size = sizeof(struct in_addr);
			ROUTERS_LIST = malloc(size * ROUTER_LIST_SIZE);
			i = 0;
			while (i < ROUTER_LIST_SIZE) {
				memcpy(&aux32, &ip_list[0].options[p + (i * size)], size);
				ROUTERS_LIST[i * size].s_addr = ntohl(aux32);
				p += size;
				i++;
			}
			break;
		case 6:
			//Domain Name Server
			if (DOMAIN_NAME_SERVER_LIST != NULL) {
				free(DOMAIN_NAME_SERVER_LIST);
			}

			DOMAIN_LIST_SIZE = ip_list[0].options[p++] / 4;
			size = sizeof(struct in_addr);
			DOMAIN_NAME_SERVER_LIST = malloc(size * DOMAIN_LIST_SIZE);
			i = 0;
			while (i < DOMAIN_LIST_SIZE) {
				memcpy(&aux32, &ip_list[0].options[p + (i * size)], size);
				printDebug("setMSGInfo", "domain: %u", ntohl(aux32));
				DOMAIN_NAME_SERVER_LIST[i * size].s_addr = ntohl(aux32);
				p += size;
				i++;
			}
			break;
		case 15:
			//Domain Name
			if (DOMAIN_NAME != NULL) {
				free(DOMAIN_NAME);
			}

			h = ip_list[0].options[p++];
			DOMAIN_NAME = malloc(h);
			i = 0;
			while (i < h) {
				DOMAIN_NAME[i] = ip_list[0].options[p++];
				i++;
			}
			break;
		case 51:
			//Lease time
			h = ip_list[0].options[p++];
			i = 0;
			aux32 = 0;
			while (i < h) {
				aux32 = aux32 << 8;
				aux32 += ip_list[0].options[p++];
				i++;
			}
			LEASE = aux32;
			break;
		case 0xff:
			//End Options
			printf("paso por end\n");
			break;

		default:
			// Se ignora el campo
			h = ip_list[0].options[p++];
			p += h;
			break;
		}
	}
}

//////////////////////////////////////////////
// Set IP adress of interface to value adress
//////////////////////////////////////////////
int set_device_ip() {
	int test_sock = 0;
	struct sockaddr_in* ip_addr = NULL;
	struct ifreq ifr;

	// Limpia la estructura
	memset(&ifr, 0, sizeof(struct ifreq));

	// Establece la ip del cliente
	ip_addr = (struct sockaddr_in *) &(ifr.ifr_addr);
	bzero(ip_addr, sizeof(struct sockaddr_in));
	ip_addr->sin_family = AF_INET;
	ip_addr->sin_addr.s_addr = SELECTED_ADDRESS.s_addr;

	test_sock = socket(PF_INET,SOCK_DGRAM, 0);
	if (test_sock == -1) {
		perror("socket");
		return (-1);
	}

	// Establece el nombre del dispositivo
	strncpy(ifr.ifr_name, IFACE, IFNAMSIZ);
	if (ioctl(test_sock, SIOCSIFADDR, &ifr) != 0) {
		perror("ioctl");
		close(test_sock);
		return (-1);
	} else {
		printDebug("set_device_ip", "IP address of '%s' set to '%d'\n",
				IFACE, SELECTED_ADDRESS.s_addr);
		printDebug("set_device_ip", "pos:%s\n", inet_ntoa(SELECTED_ADDRESS));
	}
	close(test_sock);
	return (0);
}

//////////////////////////////////////////////
// Set netmask of interface
//////////////////////////////////////////////
int set_device_netmask() {
	int test_sock = 0;
	struct sockaddr_in* netmask = NULL;
	struct ifreq ifr;

	// Limpia la estructura
	memset(&ifr, 0, sizeof(struct ifreq));

	// Establece la máscara de red
	netmask = (struct sockaddr_in *) &(ifr.ifr_netmask);
	bzero(netmask, sizeof(struct sockaddr_in));
	netmask->sin_family = AF_INET;
	netmask->sin_addr.s_addr = SUBNET_MASK->sin_addr.s_addr;

	test_sock = socket(PF_INET,SOCK_DGRAM, 0);
	if (test_sock == -1) {
		perror("socket");
		return (-1);
	}

	// Establece el nombre del dispositivo
	strncpy(ifr.ifr_name, IFACE, IFNAMSIZ);
	if (ioctl(test_sock, SIOCSIFNETMASK, &ifr) != 0) {
		perror("ioctl");
		close(test_sock);
		return (-1);
	} else {
		printDebug("set_device_netmask", "netmask of '%s' set to '%d'\n",
				IFACE, SUBNET_MASK->sin_addr.s_addr);
		printDebug("set_device_netmask", "pos:%s\n", inet_ntoa(SELECTED_ADDRESS));
	}
	close(test_sock);
	return (0);
}

// Comprueba si el interfaz está desactivado, y en ese caso lo activa
// si el interfaz no estaba desactivado previamente devuelve -1
int up_device_if_down(const char* interface) {
	int fd;
	struct ifreq ifr;
	int is_up;
	int ret = 0;

	fd = socket(PF_INET,SOCK_DGRAM, 0);
	strcpy(ifr.ifr_name, IFACE);
	ioctl(fd, SIOCGIFFLAGS, &ifr);

	is_up = ifr.ifr_ifru.ifru_flags & IFF_UP;

	if(is_up == 0){
		ifr.ifr_ifru.ifru_flags = ifr.ifr_ifru.ifru_flags | IFF_UP;
		ioctl(fd, SIOCSIFFLAGS, &ifr);
	}

	else{
		ret = -1;
		fprintf(stderr,"ERROR: %s is already enabled\n", interface);
	}
	close(fd);
	ret = 1;//TODO poner bien luego
	return ret;
}

void device_down(const char* interface) {
	int fd;
	struct ifreq ifr;
	int is_up;

	fd = socket(PF_INET,SOCK_DGRAM, 0);
	strcpy(ifr.ifr_name, IFACE);
	ioctl(fd, SIOCGIFFLAGS, &ifr);

	is_up = ifr.ifr_ifru.ifru_flags & IFF_UP;

	if(is_up == 1){
		ifr.ifr_ifru.ifru_flags = ifr.ifr_ifru.ifru_flags ^ IFF_UP;
		ioctl(fd, SIOCSIFFLAGS, &ifr);
	}

	close(fd);
}

int pow_utils(int x, int y){
	int ret,i;
	ret = 1;
	for(i = 0; i < y; i++){
		ret *= x;
	}
	return ret;
}
