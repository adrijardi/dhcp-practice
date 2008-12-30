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
	if (debug == true) {
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
	strcpy(ifr.ifr_name, iface);
	ioctl(fd, SIOCGIFHWADDR, &ifr);
	if (haddress == NULL)
		haddress = malloc(haddress_size);

	bzero(haddress, haddress_size);
	memcpy(haddress, ifr.ifr_hwaddr.sa_data, haddress_size);
	close(fd);
}

int obtain_ifindex() {
	int fd;
	struct ifreq ifr;
	//TODO cambiar para que solo obtenga mediante ioctl en la primera petición

	fd = socket(PF_INET,SOCK_DGRAM, 0);
	strcpy(ifr.ifr_ifrn.ifrn_name, iface);
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
	selected_address.s_addr = ntohl(ip_list[0].yiaddr);
	server_address.s_addr = ntohl(ip_list[0].siaddr);

	p = 4;
	lenght = (ip_list[0].opt_length) - 2;
	printDebug("setMSGInfo","l%d",lenght);
	while (p < lenght) {
		case_ = ip_list[0].options[p++];
		printDebug("setMSGInfo","%u \t %d",case_, p);
		switch (case_) {
		case 1:
			//Subnet Mask
			if (subnet_mask != NULL) {
				free(subnet_mask);
			}
			subnet_mask = malloc(sizeof(struct sockaddr));
			h = ip_list[0].options[p++];
			i = 0;
			while (i < h) {
				subnet_mask->sa_data[i] = ip_list[0].options[p++];
				i++;
			}
			break;
		case 3:
			//Router List
			if (routers_list != NULL) {
				free(routers_list);
			}
			h = ip_list[0].options[p++];
			size = sizeof(struct in_addr);
			routers_list = malloc(size * (h / 4));

			i = 0;
			aux32 = 0;
			while (i < h) {
				aux32 = aux32 << 8;
				aux32 += ip_list[0].options[p++];
				if (((i + 1) % 4) == 0) {
					aux32 = 0;
					(routers_list + (i / 4) * size)->s_addr = aux32;
				}
				i++;
			}
			break;
		case 6:
			//Domain Name Server
			if (domain_name_server_list != NULL) {
				free(domain_name_server_list);
			}
			h = ip_list[0].options[p++];
			size = sizeof(struct in_addr);
			domain_name_server_list = malloc(size * (h / 4));
			i = 0;
			aux32 = 0;
			while (i < h) {
				aux32 = aux32 << 8;
				aux32 += ip_list[0].options[p++];
				if (((i + 1) % 4) == 0) {
					aux32 = 0;
					(domain_name_server_list + (i / 4) * size)->s_addr = aux32;
				}
				i++;
			}
			break;
		case 15:
			//Domain Name
			if (domain_name != NULL) {
				free(domain_name);
			}

			h = ip_list[0].options[p++];
			domain_name = malloc(h);
			i = 0;
			while (i < h) {
				domain_name[i] = ip_list[0].options[p++];
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
			lease = aux32;
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
int set_device_ip(const char* interface, struct in_addr ip_address) {
	int test_sock = 0;
	struct sockaddr_in* addr = NULL;
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(struct ifreq));
	addr = (struct sockaddr_in *) &(ifr.ifr_addr);
	memset(addr, 0, sizeof(struct sockaddr_in));
	//addr->sin_len=sizeof(struct sockaddr_in);
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = ip_address.s_addr;

	test_sock = socket(PF_INET,SOCK_DGRAM, 0);
	if (test_sock == -1) {
		perror("socket");
		return (-1);
	}

	strncpy(ifr.ifr_name, interface, IFNAMSIZ);
	if (ioctl(test_sock, SIOCSIFADDR, &ifr) != 0) {
		perror("ioctl");
		close(test_sock);
		return (-1);
	} else {
		printDebug("set_device_ip", "IP address of '%s' set to '%d'\n",
				interface, ip_address.s_addr);
		printDebug("set_device_ip", "pos:%s\n", inet_ntoa(ip_address));
	}
	close(test_sock);
	return (0);
}

