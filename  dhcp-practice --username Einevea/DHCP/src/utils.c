/*
 * utils.c
 *
 *  Created on: 21-oct-2008
 *      Author: dconde
 */

#include "utils.h"

char* getNetConfigTrace();

// Obtiene la marca de tiempo en una cadena de caracteres.
char * getTimestamp() {
	char* timestamp;
	time_t t;
	struct tm local, gtm;
	struct tm *tmp;
	int h, m;

	timestamp = malloc(50);
	t = time(NULL);
	tmp = localtime(&t);
	memcpy(&local, tmp, sizeof(struct tm));

	tmp = gmtime(&t);
	memcpy(&gtm, tmp, sizeof(struct tm));
	h = (int) local.tm_gmtoff;
	m = (h / 60) % 60; //TODO no funciona?
	h = h / 3600;
	if (h >= 0)
		strftime(timestamp, 50, "%Y-%m-%d %H:%M:%S+", &local);
	else
		strftime(timestamp, 50, "%Y-%m-%d %H:%M:%S-", &local);

	sprintf(timestamp, "%s%.2d:%.2d", timestamp, h, m);
	return timestamp;
}

void reset_timeout(){
	ACTUAL_TIMEOUT = 4;
	SEC_TIMEOUT = 0;
	USEC_TIMEOUT = 0;
}

void get_next_timeout(struct timeval *tv){
	double  r;
	r = (((double)rand()/(double)RAND_MAX)*2)-1;
	SEC_TIMEOUT = (ACTUAL_TIMEOUT + r);
	USEC_TIMEOUT = ((ACTUAL_TIMEOUT +r) - SEC_TIMEOUT) *1000000;
	printDebug("get_next_timeout","r %e, AT %d, ST %d, UT %d",r, ACTUAL_TIMEOUT, SEC_TIMEOUT, USEC_TIMEOUT);
	tv->tv_sec = SEC_TIMEOUT;
	tv->tv_usec = USEC_TIMEOUT;
	ACTUAL_TIMEOUT *=2;
}

void decrease_timeout(struct timeval *tv, struct timeval *init, struct timeval *end){
	int sec, usec;
	sec = end->tv_sec - init->tv_sec;
	if(end->tv_usec >= init->tv_usec){
		usec = end->tv_usec - init->tv_usec;
	}else{
		sec -= 1;
		usec = 1000000 +(end->tv_usec-init->tv_usec);
	}
	tv->tv_sec = SEC_TIMEOUT - sec;
	if(USEC_TIMEOUT >= usec){
		tv->tv_usec = USEC_TIMEOUT - usec;
	} else {
		if (tv->tv_sec <= 0) {
			tv->tv_sec = 0;
			tv->tv_usec = 0;
		} else {
			tv->tv_sec -= 1;
			tv->tv_usec = 1000000 + (USEC_TIMEOUT - usec);
		}
	}
	printDebug("decrease_timeout","TO %d.%06d - res %d.%06d = tv %d.%06d ",SEC_TIMEOUT,USEC_TIMEOUT, sec, usec,tv->tv_sec,tv->tv_usec);
}

// Imprime trazas de debuf, solo en el caso de que la aplicación esté en modo DEBUG.
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

// Compara dos direcciones hardware, devuelve 1 en caso de que sean iguales y 0 si no lo son.
int compare_haddress(char * had){
	int i, ret;
	ret = TRUE;
	for(i = 0; ret == TRUE && i < HADDRESS_SIZE; i++){
		if(had[i]!= HADDRESS[i])
			ret = FALSE;
	}
	printDebug("compare_haddress","ret %d",ret);
	return ret;
}

// Imprime una traza de programa, solo está definidas aquí las trazas estandar de salida.
void printTrace(int xid, enum dhcp_message state, char* str) {
	char *timestamp;
	char *aux;
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
		case DHCPNAK:
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

			aux = getNetConfigTrace();
			fprintf(stdout,"#[%s]%s.\n",timestamp, aux);
			free(aux);
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


char* getNetConfigTrace(){
	int ret;
	char *msg, *aux;
	ret = 0;
	aux = malloc(200);
	msg = malloc(200);
	bzero(aux, 200);
	bzero(msg,200);

	//Se rellena la ip y el lease;
	if (LEASE == 0xffffffff) {
		sprintf(aux, " IP: %s; leasetime: inf;", inet_ntoa(SELECTED_ADDRESS));
	}else{
		sprintf(aux, " IP: %s; leasetime: %u;", inet_ntoa(SELECTED_ADDRESS), LEASE);
	}
	//Se rellena la mascara de red
	sprintf(msg, "%s subnet mask: %s;",aux, inet_ntoa(SUBNET_MASK->sin_addr));

	//Se rellena el router y el hostname del servidor
	sprintf(aux, "%s router: %s; server hostname %s;",msg, inet_ntoa(ROUTERS_LIST[0]), SERVER_HOSTNAME);

	//Se rellenan los DNS
	if(DOMAIN_LIST_SIZE >= 1){
		sprintf(msg, "%s primary DNS: %s;",aux, inet_ntoa(DOMAIN_NAME_SERVER_LIST[0]));

		if(DOMAIN_LIST_SIZE >= 2){
			sprintf(aux, "%s secondary DNS: %s",msg, inet_ntoa(DOMAIN_NAME_SERVER_LIST[1]));
		}else{
			sprintf(aux, "%s secondary DNS: -",msg);
		}
	}else{
		sprintf(msg, "%s primary DNS: -; secondary DNS: -",aux);

	}


	//IP %s; leasetime %s; subnet mask %s; router: %s; server hostname %s; primary DNS: %s; secondary DNS: %s

	free(msg);
	return aux;
}

void time_wait(int microsec) {
	unsigned long time = microsec * 1000;
	usleep(time);
	time++; //TODO LOL
}

// Obtiene la dirección hardware del actual equipo.
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

// Obtiene el identificador de la interfaz de red.
int obtain_ifindex() {
	int fd;
	struct ifreq ifr;

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
				ROUTERS_LIST[i * size].s_addr = aux32;
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
				memcpy(&aux32, &ip_list[0].options[p], size);
				printDebug("setMSGInfo", "domain: %u", ntohl(aux32));
				DOMAIN_NAME_SERVER_LIST[i].s_addr = aux32;
				p += size;
				i++;
			}
			break;
		case 12:
			//Server Hostname
			if (SERVER_HOSTNAME != NULL) {
				free(SERVER_HOSTNAME);
			}

			h = ip_list[0].options[p++]+1;
			printDebug("setMSGInfo", "Server hostname length: %d", h);
			SERVER_HOSTNAME = malloc(h);
			i = 0;
			while (i < h-1) {
				SERVER_HOSTNAME[i] = ip_list[0].options[p++];
				i++;
			}
			SERVER_HOSTNAME[i] = '\0';
			break;
		case 15:
			//Domain Name
			if (DOMAIN_NAME != NULL) {
				free(DOMAIN_NAME);
			}

			h = ip_list[0].options[p++]+1;
			DOMAIN_NAME = malloc(h);
			i = 0;
			while (i < h-1) {
				DOMAIN_NAME[i] = ip_list[0].options[p++];
				i++;
			}
			DOMAIN_NAME[i] = '\0';
			break;
		case 51:
			//Lease time
			h = ip_list[0].options[p++];
			i = 0;
			aux32 = 0;
			memcpy(&aux32, &ip_list[0].options[p], 4);
			i+=4;
			LEASE = ntohl(aux32);
			break;
		case 0xff:
			//End Options
			printDebug("setMSGInfo", "paso por end");
			break;

		default:
			// Se ignora el campo
			h = ip_list[0].options[p++];
			p += h;
			break;
		}
	}
}

// Establece la dirección ip del intefaz.
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
		printDebug("set_device_ip", "IP address of '%s' set to '%s'",
				IFACE, inet_ntoa(SELECTED_ADDRESS));
	}
	close(test_sock);
	return (0);
}

// Establece la máscara de subred del interfaz.
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
		printDebug("set_device_netmask", "netmask of '%s' set to '%s'",
				IFACE, inet_ntoa(SUBNET_MASK->sin_addr));
	}
	close(test_sock);
	return (0);
}

// Establece la puerta de enlace del interfaz.
int set_device_router() {
	struct rtentry route;
	int test_sock = 0;
	struct sockaddr_in singw, sindst;

	// Limpia las estructuras
	memset(&singw, 0, sizeof(struct sockaddr));
	memset(&sindst, 0, sizeof(struct sockaddr));
	memset(&route, 0, sizeof(struct rtentry));

	// Establecemos los parámetros del gateway
	singw.sin_family = AF_INET;
	singw.sin_addr.s_addr = ROUTERS_LIST[0].s_addr;
	sindst.sin_family = AF_INET;
	sindst.sin_addr.s_addr = INADDR_ANY;

	// Rellenamos la ruta
	route.rt_dst = *(struct sockaddr *) &sindst;
	route.rt_gateway = *(struct sockaddr *) &singw;
	route.rt_flags = RTF_GATEWAY;
	route.rt_dev = IFACE;

	test_sock = socket(AF_INET,SOCK_DGRAM, 0);
	if (test_sock < 0) {
		perror("socket");
		return (-1);
	}

	// Se borran la misma entrada primero.
	ioctl(test_sock, SIOCDELRT, &route);
	if (ioctl(test_sock, SIOCADDRT, &route) < 0) {
		perror("ioctl");
		close(test_sock);
		return (-1);
	}else{
		printDebug("set_device_router", "route d %s\tp %s\ti %s ", "0.0.0.0", inet_ntoa(singw.sin_addr),	IFACE);
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
	return ret;
}

// Desactiva el interfaz.
void device_down() {
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

// Eleva un número al otro, parece que no sirve para nada, ya que se podría hacer con x^y
// pero tiene funcionamientos ocultos.
int pow_utils(int x, int y){
	int ret,i;
	ret = 1;
	for(i = 0; i < y; i++){
		ret *= x;
	}
	return ret;
}
