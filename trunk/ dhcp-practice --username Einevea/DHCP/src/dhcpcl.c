/*
 * dhcpcl.c
 *
 *  Created on: 17-oct-2008
 *      Author: dconde
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include <pthread.h>
#include <netpacket/packet.h>

#include "dhcpcl.h"
#include "constants.h"
#include "utils.h"
#include "pruebas/pruebas.h"
#include "dhcp_state.h"
#include "transfer.h"
#include "f_messages.h"

//DECLARACION DE METODOS INTERNOS
void printParamsError(int err);
int checkParams(int argc, const char* argv[]);
void pruebas();
int checkIFace(char* iface);
void getFileParams();
int initialize();
void *init(void *arg);
int selecting();
int requesting();
int bound();
void clean_close();
void run();

void pruebas() {
	//pruebaFormatoMsg();
}

/*
 * Funcion de inicio
 */
int main(int argc, const char* argv[]) {
	debug = DEBUG_OFF;
	no_exit = true;
	exit_value = checkParams(argc, argv);
	//pruebas();//TODO quitar
	if (exit_value == 0) {
		printTrace(0, PID, NULL);
		getFileParams();
		initialize();
		run();
		clean_close();
	}
	return exit_value;
}

void run() {
	pthread_t hilo;
	pthread_attr_t hilo_attr;
	int isbound = 0;
	int result = 0;

	while (!isbound) {
		//Se lanza un nuevo hilo para el envio
		pthread_attr_init(&hilo_attr);
		if(pthread_create(&hilo, &hilo_attr, init, NULL) < 0) //TODO mirar valor de retorno
			perror("pthread_create");

			/*if (init()) {*/
		result = selecting();
		pthread_join(hilo, NULL);
			if (result >= 0) {
				isbound = requesting();
			} else {
				//TODO
				printf("Error 1\n");
				exit(-1);
			}
		/*} else {
			//TODO
			printf("Error 2\n");
			exit(-1);
		}*/
	}
	bound();
}

void *init(void *arg) {
	printf("En init\n");
	// Se espera un numero aleatorio de segundos entre 1 y 10
	srandom(time(NULL));
	time_wait((random() % 9000) + 1000);
	sendDHCPDISCOVER();
	return NULL;
}

int selecting() {
	fd_set recvset;
	int sock_recv;
	struct sockaddr_ll addr; // Direccion de recepción
	int ret = 1;
	char * buf = malloc(1000); //TODO
	struct mdhcp_t *dhcp_recv;

	printf("En selecting\n");
	// Creación del socket de recepción
	sock_recv = socket (PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));
	if(sock_recv < 0){
		perror("socket");
		ret = -1;
	}
	if(ret >= 0){
		// Definición de la dirección de recepción
		bzero(&addr, sizeof(struct sockaddr_ll));
		addr.sll_family = AF_PACKET;
		addr.sll_addr[0] = 255;
		addr.sll_addr[1] = 255;
		addr.sll_addr[2] = 255;
		addr.sll_addr[3] = 255;
		addr.sll_addr[4] = 255;
		addr.sll_addr[5] = 255;
		addr.sll_addr[6] = 0;
		addr.sll_addr[7] = 0;
		addr.sll_halen = 6;
		addr.sll_ifindex = obtain_ifindex();
		if(addr.sll_ifindex < 0)
			ret = -1;
		addr.sll_hatype = 0xFFFF;
		addr.sll_protocol = htons(ETH_P_IP); //TODO raro que funcione con ese htons
		addr.sll_pkttype = PACKET_BROADCAST; // TODO no estoy seguro del broadcast

		if(addr.sll_ifindex == -1){
			ret = -1;
		}

		if(ret >= 0){
			ret = bind(sock_recv, (struct sockaddr *)&addr, sizeof(struct sockaddr_ll));
			if(ret < 0)
				perror("bind");

			int recv_size = recvfrom(sock_recv, buf, 1000, 0, NULL, NULL);
			printf("%d\n",recv_size);

			dhcp_recv = get_dhcpH_from_ethM(buf, recv_size);
			printf("ipOrigen %d\n",dhcp_recv->siaddr);
			printf("id %d\n",dhcp_recv->xid);

			/*FD_SET(sock_recv, &recvset); TODO hacer el select, ahora mismo solo coge 1 pakete

			//Recivimos multiples respuestas
			if(select(2,  &recvset,  NULL, NULL, NULL) < 0){
				perror("select");
				//TODO
				return -1;
			}
			printf("lala\n");
*/

		//Elegimos ip
		//Enviamos DHCPrequest
		}
	}
	return ret;
}

int requesting() {
	printf("En requesting\n");
	//Escuchamos lo que venga
	//Enviamos dhcpAck dhcpNAck
	//Si falta algo mas tambien lo hacemos
	return true;
}

int bound() {
	printf("En bound\n");
	//Temporizador de leasetime
	//escuchar señales
	//Si recive señal se envia DHCPRELEASE ??
	return true;
}

void clean_close() {
	free(haddress);
}

int initialize() {
	// Se inicializan los parametros del estado.
	state = INIT;
	haddress = NULL;
	haddress_size = 6;
	obtainHardwareAddress();
	return EXIT_NORMAL; //TODO
}

void getFileParams() {
	fprintf(stdout,"Obtenemos parametros?\n");
}

/*
 * Funcion que comprueba el numero de parametros de entrada, el formato de los mismos
 * y asigna los valores a las variables globales.
 */
int checkParams(int argc, const char* argv[]) {
	int ret = EXIT_NORMAL;
	int i, iface_state;
	char *param, *errPtr;

	//Se comprueba el numero de parametros
	if (argc >= 2 && argc <= 11) {

		//Se asigna el iface

		iface_state = checkIFace((char*) argv[1]);
		if (iface_state == true) {
			iface = (char*) argv[1];

			for (i = 2; i < argc && ret != -1; i += 2) {

				// Se comprueba que la cadena sea de un parametro acordado
				param = (char*) argv[i];
				if (strcmp(param, "-t") == 0 && i + 1 < argc) {
					//Se asigna el parametro timeout
					timeout = strtol(argv[i + 1], &errPtr, 0);
					//Se comprueba que no haya habido un error de formato
					if (strlen(errPtr) != 0) {
						printParamsError(1);
						ret = EXIT_ERROR;
					}

				} else if (strcmp(param, "-h") == 0 && i + 1 < argc) {
					param = (char*) argv[i + 1];
					printf("-h: %s\n", param);

				} else if (strcmp(param, "-a") == 0 && i + 1 < argc) {
					param = (char*) argv[i + 1];
					printf("-a: %s\n", param);

				} else if (strcmp(param, "-l") == 0 && i + 1 < argc) {
					//Se asigna el parametro de lease
					lease = strtol(argv[i + 1], &errPtr, 0);
					//Se comprueba que no haya habido un error de formato
					if (strlen(errPtr) != 0) {
						printParamsError(2);
						ret = EXIT_ERROR;
					}

				} else if (strcmp(param, "-d") == 0) {
					//Se activa el modo debug
					printf("Debug mode [ON]\n");
					debug = DEBUG_ON;

				} else {
					//El parametro recibido no es un parametro acordado
					printParamsError(0);
					ret = EXIT_ERROR;
				}
			}
		} else {
			ret = EXIT_ERROR;
		}
	} else {
		printParamsError(0);
		ret = EXIT_ERROR;
	}
	return ret;
}

int checkIFace(char* iface) {
	int ret;
	ret = true;
	//TODO comprobar iface
	printf("%d\n", ret);
	return ret;
}

/*
 * Funcion que imprime los mensajes de error de los parametros de entrada.
 */
void printParamsError(int err) {
	switch (err) {
	case 0:
		printf(
				"Error en el numero de parametros, el formato de ejecion es:\n\n dhcpcl interface [-t timeout] [-h hostname] [-a IP address] [-l leasetime] [-d]\n\n");
		printf(
				"  −t timeout:\n\tEspecifica durante cuanto tiempo el cliente intenta conseguir una ip.\n\tEl valor por defecto son 64 segundos.\n");
		printf(
				"  −h hostname:\n\tIndica la cadena a usar en el campo de opciones host_name del datagrama DHCP.\n");
		printf(
				"  −a address:\n\tIndica la ultima direccion IP conocida, para ser enviada en el DHCPDISCOVER.\n");
		printf(
				"  −l lease:\n\tEspecifica el valor del temporizador de arriendo sugerido al servidor, el servidor puede sobreescribir este valor.\n\tEl valor por defecto es infinito\n");
		printf("  −d:\n\tModo de depuracion.\n");
		break;
	case 1:
		printf("Error en el formato del valor de timeout.\n");
		printf(
				"  −t timeout: utiliza valores enteros que representan segundos.\n");
		break;
	case 2:
		printf("Error en el formato del valor de lease.\n");
		printf(
				"  −l lease: utiliza valores enteros que representan segundos.\n");
		break;
	default:
		break;
	}
}

