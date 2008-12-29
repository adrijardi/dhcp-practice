/*
 * utils.c
 *
 *  Created on: 21-oct-2008
 *      Author: dconde
 */

#include "utils.h"

char * getTimestamp(){
	char* timestamp;
	time_t t;
	struct tm *local, *gtm;
	int h, m;

	timestamp = malloc(50);
	t=time(NULL);
	local=localtime(&t);
	gtm=gmtime(&t);
	h = (int)local->tm_gmtoff;
	m = (h/60)%60; //TODO no funciona?
	h = h/3600;
	if(h>=0)
		strftime(timestamp, 50, "%Y-%m-%d %H:%M:%S+", local);
	else
		strftime(timestamp, 50, "%Y-%m-%d %H:%M:%S-", local);

	sprintf(timestamp,"%s%.2d:%.2d",timestamp,h,m);
	return timestamp;
}

void printTrace(int xid, enum dhcp_message state, char* str){
	char *timestamp;
	timestamp = getTimestamp();

	if(xid == -1){

	}else if(xid == -2){

	}else if(xid == -3){

	}else{
		switch(state){
		case DHCPDISCOVER:
			fprintf(stdout,"#[%s] (%d) DHCPDISCOVER sent.\n",timestamp,xid);
			break;
		case DHCPOFFER:
			fprintf(stdout,"#[%s] (%d) DHCPOFFER received from %s.\n",timestamp,xid,str);
			break;
		case DHCPREQUEST:
			fprintf(stdout,"#[%s] (%d) DHCPREQUEST sent to %s.\n",timestamp,xid,str);
			break;
		case DHCPACK:
			fprintf(stdout,"#[%s] (%d) DHCPACK received: %s.\n",timestamp,xid,str);
			break;
		case DHCPNACK:
			fprintf(stdout,"#[%s] (%d) DHCPNACK received.\n",timestamp,xid);
			break;
		case DHCPRELEASE:
			fprintf(stdout,"#[%s] (%d) DHCPRELEASE sent %s.\n",timestamp,xid,str);
			break;
		case PID:
			fprintf(stdout,"#[%s] PID=%d.\n",timestamp,getpid());
			break;
		case IP:
			fprintf(stdout,"#[%s] IP %s.\n",timestamp,str);
			break;
		case DHCPSIGINT:
			fprintf(stdout,"#[%s] SIGINT received.\n",timestamp);
			break;
		case DHCPSIGUSR2:
			fprintf(stdout,"#[%s] SIGUSR2 received.\n",timestamp);
			break;
		}
	}
	free(timestamp);
}

void time_wait(int microsec){
	unsigned long time = microsec * 1000;
	usleep(time);
	time++;
}

void obtainHardwareAddress(){
	int fd;
	struct ifreq ifr;

	fd = socket(PF_INET, SOCK_DGRAM,0);
	strcpy(ifr.ifr_name, iface);
	ioctl(fd, SIOCGIFHWADDR, &ifr);
	if(haddress == NULL)
		haddress = malloc(haddress_size);

	bzero(haddress, haddress_size);
	memcpy(haddress,ifr.ifr_hwaddr.sa_data, haddress_size);
	close(fd);
}

int obtain_ifindex(){
	int fd;
	struct ifreq ifr;
	//TODO cambiar para que solo obtenga mediante ioctl en la primera petición

	fd = socket(PF_INET, SOCK_DGRAM,0);
	strcpy(ifr.ifr_ifrn.ifrn_name, iface);
	if(ioctl(fd, SIOCGIFINDEX, &ifr) < 0){
		perror("ioctl");
		printf("La interfaz especificada no es válida\n");
		return -1;
	}

	return ifr.ifr_ifru.ifru_ivalue;
}

struct offerIP* select_ip(struct mdhcp_t ip_list[]){
	struct offerIP *oIp;
	struct sockaddr *snM;
	struct in_addr* roL;
	struct in_addr* dnL;
	struct in_addr* seA;
	char * doN;
	int case_;
	int p, i, h, size;
	u_int32_t aux32;

	oIp = malloc(sizeof(struct offerIP));
	snM = malloc(sizeof(struct sockaddr));

	oIp->offered_ip = ip_list[0].yiaddr;
	oIp->server_ip = ip_list[0].siaddr;
	oIp->subnet_mask = snM;
	oIp->routers_list = NULL;
	oIp->domain_name_server_list = NULL;
	oIp->domain_name = NULL;
	oIp->server_address = NULL;

	while(p < ip_list[0].opt_length){
		case_ = ip_list[0].options[p++];
		switch(case_){
		case 1:
			//Subnet Mask
			h = ip_list[0].options[p++];
			i = 0;
			while(i < h){
				snM->sa_data[i] = ip_list[0].options[p++];
				i++;
			}
			break;
		case 3:
			//Router List
			h = ip_list[0].options[p++];
			size = sizeof(struct in_addr);
			roL = malloc(size * (h/4));
			oIp->routers_list = roL;
			i = 0;
			aux32 = 0;
			while(i < h){
				aux32 = aux32 << 8;
				aux32 += ip_list[0].options[p++];
				if(((i+1)%4)== 0){
					aux32 = 0;
					(roL+(i/4)*size)->s_addr = aux32;
				}
				i++;
			}
			break;
		case 6:
			//Domain Name Server
			h = ip_list[0].options[p++];
			size = sizeof(struct in_addr);
			dnL = malloc(size * (h/4));
			oIp->domain_name_server_list = dnL;
			i = 0;
			aux32 = 0;
			while(i < h){
				aux32 = aux32 << 8;
				aux32 += ip_list[0].options[p++];
				if(((i+1)%4)== 0){
					aux32 = 0;
					(dnL+(i/4)*size)->s_addr = aux32;
				}
				i++;
			}
			break;
		case 15:
			//Domain Name
			h = ip_list[0].options[p++];
			doN = malloc(h);
			oIp->domain_name = doN;
			i = 0;
			while(i < h){
				doN[i] = ip_list[0].options[p++];
				i++;
			}
			break;
		case 51:
			//Lease time
			h = ip_list[0].options[p++];
			i = 0;
			aux32 = 0;
			while(i < h){
				aux32 = aux32 << 8;
				aux32 += ip_list[0].options[p++];
				i++;
			}
			oIp->lease = aux32;
			break;
		case 53:
			//Type
			break;
		case 54:
			//Server Identifier
			h = ip_list[0].options[p++];
			seA = malloc(sizeof(struct in_addr));
			oIp->server_address = seA;
			i = 0;
			aux32 = 0;
			while(i < h){
				aux32 = aux32 << 8;
				aux32 += ip_list[0].options[p++];
				i++;
			}
			break;
		case 255:
			//End Options
			break;
		}
	}

	return oIp;
}

//////////////////////////////////////////////
// Set IP adress of interface to value adress
//////////////////////////////////////////////
int set_device_ip(const char* interface,struct in_addr ip_address){
 int test_sock=0;
 struct sockaddr_in* addr=NULL;
 struct ifreq ifr;

 memset( &ifr, 0, sizeof( struct ifreq ) );
 addr= (struct sockaddr_in *)&(ifr.ifr_addr);
 memset(addr, 0, sizeof( struct sockaddr_in) );
 //addr->sin_len=sizeof(struct sockaddr_in);
 addr->sin_family=AF_INET;
 addr->sin_addr.s_addr= ip_address.s_addr;

 test_sock = socket( PF_INET, SOCK_DGRAM, 0 );
 if( test_sock == -1 )
 {
  perror("socket");
  return (-1);
 }

 strncpy( ifr.ifr_name,interface,IFNAMSIZ);
 if( ioctl( test_sock, SIOCSIFADDR, &ifr ) != 0 )
 {
  perror("ioctl");
  close(test_sock);
  return (-1);
 }
 else{
	 printf("IP address of '%s' set to '%d'\n",interface,ip_address.s_addr);
	 printf("pos:%s\n",inet_ntoa(ip_address));
 }
 close(test_sock);
 return(0);
}


