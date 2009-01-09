/*
 * dhcpcl.c
 *
 *  Created on: 17-oct-2008
 *      Author: Daniel Conde y Adrián López
 */
#include "dhcpcl.h"

//DECLARACION DE METODOS INTERNOS
void printParamsError(int err);
int checkParams(int argc, const char* argv[]);
void pruebas();
void getFileParams();
int initialize();
int init();
int selecting();
int requesting();
int bound();
void clean_close();
void run();
void finalize_all();
void SIGINT_controller(int sigint);
void SIGUSR2_controller(int sigusr2);
void defaultValues();

void pruebas() {
	//pruebaFormatoMsg();
}

/*
 * Funcion de inicio
 */
int main(int argc, const char* argv[]) {
	signal(SIGINT, SIGINT_controller);
	signal(SIGUSR2, SIGUSR2_controller);
	defaultValues();
	EXIT_VALUE = checkParams(argc, argv);

	if (EXIT_VALUE == 0) {
		printTrace(0, PID, NULL);
		getFileParams();
		if (EXIT_VALUE == 0) {
			initialize();
			printDebug("main", "Inicializado");
			run();
			finalize_all();
		}
	}
	return EXIT_VALUE;
}

void defaultValues() {
	DEBUG = DEBUG_OFF;
	NO_EXIT = TRUE;
	LEASE = 0xffffffff;
	SUBNET_MASK = NULL;
	ROUTERS_LIST = NULL;
	DOMAIN_NAME_SERVER_LIST = NULL;
	DOMAIN_NAME = NULL;
	TIMEOUT = 64;
	reset_timeout();
	PARAM_HOSTNAME = NULL;
	PARAM_ADDRESS = NULL;
	SERVER_HOSTNAME = NULL;
}

// Bucle rincipal del programa.
void run() {
	int is_requesting, is_ack, time_left;
	// Se espera un numero aleatorio de segundos entre 1 y 10
	if(DEBUG == DEBUG_OFF){
		srandom(time(NULL));
		time_wait((random() % 9000) + 1000);
	}

	is_ack = FALSE;
	is_requesting = FALSE;
	time_left = TIMEOUT - ACTUAL_TIMEOUT;
	while ((is_ack == FALSE) && (EXIT_VALUE == EXIT_NORMAL)) {
		is_requesting = FALSE;
		reset_timeout();
		time_left = TIMEOUT - ACTUAL_TIMEOUT;
		printDebug("run","time_left REQUESTING %d", time_left);
		while (!is_requesting && time_left >= 0) {
			printDebug("run","init");
			if (init() >= 0){
				printDebug("run","selecting");
				is_requesting = selecting();
			}
			printDebug("run","time_left= %d - %d", TIMEOUT, ACTUAL_TIMEOUT);
			time_left = TIMEOUT - ACTUAL_TIMEOUT;
		}
		if (time_left > 0) {
			printDebug("run","requesting");
			reset_timeout();
			time_left = TIMEOUT - ACTUAL_TIMEOUT;
			printDebug("run","time_left ack %d", time_left);
			is_ack= -1;
			while ((is_ack== -1) && time_left >= 0) {
				is_ack = requesting();
				time_left = TIMEOUT - ACTUAL_TIMEOUT;
			}			
			if(is_ack <0){
				is_ack = TRUE;
				fprintf(stderr,"FAILURE: Waiting for ACK Timeout reached\n");
				EXIT_VALUE=EXIT_NO_RESPONSE;
			}
		} else {
			EXIT_VALUE = EXIT_NO_RESPONSE;
			fprintf(stderr,"FAILURE: Waiting for DHCPOFFER Timeout reached\n");
		}
	}
	if (EXIT_VALUE == EXIT_NORMAL) {
		printDebug("run","bound");
		EXIT_VALUE = bound();
	}
}

// Estado init del programa.
int init() {
	printDebug("init", "");
	return sendDHCPDISCOVER();
}

// Estado selecting.
int selecting() {
	struct mdhcp_t dhcpMessages[MAXDHCPOFFERS];
	int numMessages;
	int ret = FALSE;
	int i;
	printDebug("selecting", "");

	// Recive los mensaje Offer
	numMessages = get_selecting_messages(dhcpMessages);
	if (numMessages > 0) {
		//Elegimos ip - no se requiere reservar espacio, se reserva dentro
		setMSGInfo(dhcpMessages);
		printDebug("selecting", "termina selección ip");

		// Envia dhcp request
		ret = sendDHCPREQUEST();

		for (i = 0; i < numMessages; i++) {
			free(dhcpMessages[i].options);
		}
	}
	return ret;
}

// Estado requesting
// Devuelve 1 si recibe ack, 0 si recibe nak, y -1 si se produce algún error
int requesting() {
	int ack_ok;
	printDebug("requesting", "");

	// Recive los mensaje Offer
	// ack_ok 1 -> ack, 0 -> nak, -1 error
	ack_ok = get_ACK_message();
	printDebug("requesting", "ack_ok %d", ack_ok);
	if (ack_ok > 0) {
		// Establece la configuracion de red del dispositivo con ioctl
			set_device_ip();
			set_device_netmask();
		if(ROUTER_LIST_SIZE >0){
			set_device_router();
		}else{
			fprintf(stderr,"WARNING: Waiting no gateway entry\n");
		}
		printTrace(XID,IP,NULL);
	}
	return ack_ok;
}

// Estado bound
int bound() {
	printDebug("bound", "");
	close_sockets();
	sleep(LEASE);
	device_down(IFACE);
	return EXIT_NORMAL;
}

// Inicializa el programa.
int initialize() {
	int ret = 0;
	// Se inicializan los parametros del estado.
	HADDRESS = NULL;
	HADDRESS_SIZE = 6;
	obtainHardwareAddress();
	ret = init_sockets();
	return ret;
}

void getFileParams() {
	printDebug("getFileParams", "¿¿Obtenemos parametros de fichero??");
}


// Funcion que comprueba el numero de parametros de entrada, el formato de los mismos
// y asigna los valores a las variables globales.
int checkParams(int argc, const char* argv[]) {
	int ret = EXIT_NORMAL;
	int i, iface_state;
	char *param, *errPtr;
	//DEBUG = DEBUG_ON;
	//Se comprueba el numero de parametros
	if (argc >= 2 && argc <= 11) {

		//Se asigna el iface

		IFACE = (char*) argv[1];
		if(up_device_if_down(IFACE) < 0){
			iface_state = FALSE;
			EXIT_VALUE = EXIT_ERROR;
			ret = EXIT_ERROR;
		}else{
			iface_state = TRUE;
			for (i = 2; i < argc && ret != -1; i += 2) {

				// Se comprueba que la cadena sea de un parametro acordado
				param = (char*) argv[i];
				if (strcmp(param, "-t") == 0 && i + 1 < argc) {
					//Se asigna el parametro timeout
					TIMEOUT = strtol(argv[i + 1], &errPtr, 0);
					//Se comprueba que no haya habido un error de formato
					if (strlen(errPtr) != 0) {
						printParamsError(1);
						ret = EXIT_ERROR;
					}
					printDebug("checkParams", "-t: %d\n", TIMEOUT);

				} else if (strcmp(param, "-h") == 0 && i + 1 < argc) {
					param = (char*) argv[i + 1];
					PARAM_HOSTNAME = param;
					printDebug("checkParams", "-h: %s\n", PARAM_HOSTNAME);

				} else if (strcmp(param, "-a") == 0 && i + 1 < argc) {
					param = (char*) argv[i + 1];
					PARAM_ADDRESS = malloc(sizeof(struct in_addr));
					if(inet_aton(param, PARAM_ADDRESS)==0){
						printParamsError(3);
						ret = EXIT_ERROR;
					}
					printDebug("checkParams", "-a: %s\n", param);

				} else if (strcmp(param, "-l") == 0 && i + 1 < argc) {
					//Se asigna el parametro de lease
					if(strcmp(argv[i+1], "inf") != 0){
						LEASE = strtol(argv[i + 1], &errPtr, 0);
						//Se comprueba que no haya habido un error de formato
						if (strlen(errPtr) != 0) {
							printParamsError(2);
							ret = EXIT_ERROR;
						}
					}
					printDebug("checkParams", "-l: %d\n", LEASE);

				} else if (strcmp(param, "-d") == 0) {
					//Se activa el modo debug
					DEBUG = DEBUG_ON;
					printDebug("checkParams", "Debug mode <ON>");

				} else {
					//El parametro recibido no es un parametro acordado
					printParamsError(0);
					ret = EXIT_ERROR;
				}
			}
		}
	} else {
		printParamsError(0);
		ret = EXIT_ERROR;
	}
	return ret;
}

// Funcion que imprime los mensajes de error de los parametros de entrada.
void printParamsError(int err) {
	switch (err) {
	case 0:
		fprintf(stderr,
				"Error en el numero de parametros, el formato de ejecion es:\n\n dhcpcl interface [-t timeout] [-h hostname] [-a IP address] [-l leasetime] [-d]\n\n");
		fprintf(stderr,
				"  −t timeout:\n\tEspecifica durante cuanto tiempo el cliente intenta conseguir una ip.\n\tEl valor por defecto son 64 segundos.\n");
		fprintf(stderr,
				"  −h hostname:\n\tIndica la cadena a usar en el campo de opciones host_name del datagrama DHCP.\n");
		fprintf(stderr,
				"  −a address:\n\tIndica la ultima direccion IP conocida, para ser enviada en el DHCPDISCOVER.\n");
		fprintf(stderr,
				"  −l lease:\n\tEspecifica el valor del temporizador de arriendo sugerido al servidor, el servidor puede sobreescribir este valor.\n\tEl valor por defecto es infinito\n");
		fprintf(stderr,"  −d:\n\tModo de depuracion.\n");
		break;
	case 1:
		fprintf(stderr,"Error en el formato del valor de timeout.\n");
		fprintf(stderr,
				"  −t timeout: utiliza valores enteros que representan segundos.\n");
		break;
	case 2:
		fprintf(stderr,"Error en el formato del valor de lease.\n");
		fprintf(stderr,
				"  −l lease: utiliza valores enteros que representan segundos.\n");
		break;
	case 3:
		fprintf(stderr,"Error en el formato o del valor de IP address.\n");
		fprintf(stderr,
				"  −a IP address: utiliza notacion decimal separada por puntos.\n");
		break;
	default:
		break;
	}
}

// Finaliza la aplicación liberando la memória necesaría.
void finalize_all(){
	printDebug("finalize_all", "");
	free(HOSTNAME);
	free(ADDRESS);
	free(HADDRESS);
	free(SUBNET_MASK);
	free(ROUTERS_LIST);
	free(DOMAIN_NAME_SERVER_LIST);
	free(DOMAIN_NAME);
	if(PARAM_ADDRESS != NULL){
		free(PARAM_ADDRESS);
	}
	if(SERVER_HOSTNAME != NULL){
		free(SERVER_HOSTNAME);
	}
}

// Sale del programa de manera "abrupta"
void SIGINT_controller(int sigint){
	printTrace(0, DHCPSIGINT, NULL);
	finalize_all();
	exit(EXIT_NORMAL);
}

// Hace DHCPRELEASE y baja la interfaz
void SIGUSR2_controller(int sigusr2){
	printTrace(0, DHCPSIGUSR2, NULL);
	//Si recive señal se envia DHCPRELEASE nuevo socket ip
	sendDHCPRELEASE();
	device_down(IFACE);
	finalize_all();
	exit(EXIT_NORMAL);
}
