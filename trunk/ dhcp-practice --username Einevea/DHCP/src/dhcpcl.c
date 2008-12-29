/*
 * dhcpcl.c
 *
 *  Created on: 17-oct-2008
 *      Author: dconde
 */
#include "dhcpcl.h"

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
void finalize_all();
void SIGINT_controller(int sigint);
void SIGUSR2_controller(int sigusr2);

void pruebas() {
	//pruebaFormatoMsg();
}

/*
 * Funcion de inicio
 */
int main(int argc, const char* argv[]) {
	signal(SIGINT, SIGINT_controller);
	signal(SIGUSR2, SIGUSR2_controller);
	debug = DEBUG_OFF;
	no_exit = true;
	exit_value = checkParams(argc, argv);
	//pruebas();//TODO quitar
	if (exit_value == 0) {
		printTrace(0, PID, NULL);
		getFileParams();
		initialize();
		printf("inicializado\n");
		run();
		finalize_all();
	}
	return exit_value;
}

void run() {
	pthread_t hilo;
	pthread_attr_t hilo_attr;
	int isbound = 0;
	int result = 0;

	while (!isbound) {
		// Se cierra el lock de control de envio ( no se envia hasta que no se está preparado para recivir )
		pthread_mutex_lock(lock);

		//Se lanza un nuevo hilo para el envio
		pthread_attr_init(&hilo_attr);
		if (pthread_create(&hilo, &hilo_attr, init, NULL) < 0) //TODO mirar valor de retorno
			perror("pthread_create");

		result = selecting();

		// Se espera por el hilo de envio
		pthread_join(hilo, NULL);

		if (result >= 0) {
			if(requesting() >= 0)
				isbound = 1;
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
	struct mdhcp_t dhcpMessages[MAXDHCPOFFERS];
	int numMessages;
	struct offerIP *selected_ip;
	int ret = 0;
	pthread_t hilo;
	pthread_attr_t hilo_attr;

	printf("En selecting\n");

	// Recive los mensaje Offer
	numMessages = get_selecting_messages(dhcpMessages);
	if (numMessages > 0) {
		//Elegimos ip - no se requiere reservar espacio, se reserva dentro
		selected_ip = select_ip(dhcpMessages);
		selected_address.s_addr = ntohl(selected_ip->offered_ip);
		printf("termina selección ip\n");

		// Envia dhcp request

		// Se cierra el lock de control de envio ( no se envia hasta que no se está preparado para recivir )
		pthread_mutex_lock(lock);
		pthread_mutex_lock(lock_params);

		printf("se crea hilo\n");
		//Se lanza un nuevo hilo para el envio
		pthread_attr_init(&hilo_attr);
		if (pthread_create(&hilo, &hilo_attr, sendDHCPREQUEST,
				(void *) selected_ip) < 0) //TODO mirar valor de retorno
			perror("pthread_create");

		pthread_mutex_lock(lock_params);
		free(selected_ip);
		pthread_mutex_unlock(lock_params);
	}
	return ret;
}

int requesting() {
	int ret = 0;
	int ack_ok;

	printf("En requesting\n");

	// Recive los mensaje Offer
	ack_ok = get_ACK_message();
	if (ack_ok > 0) {
		// Establece la ip del dispositivo con ioctl
		set_device_ip(iface, selected_address);
	}
	return ret;

	//Escuchamos lo que venga
	//Enviamos dhcpAck dhcpNAck
	//Si falta algo mas tambien lo hacemos

}

int bound() {
	printf("En bound\n");
	sleep(3600);

	//Temporizador de leasetime
	//escuchar señales
	//Si recive señal se envia DHCPRELEASE ??
	return true;
}

int initialize() {
	int ret = 0;
	// Se inicializan los parametros del estado.
	state = INIT;
	lock = malloc(sizeof(pthread_mutex_t));
	lock_params = malloc(sizeof(pthread_mutex_t));
	if (pthread_mutex_init(lock, NULL) < 0) {
		perror("pthread_mutex_init");
		exit(-1);
	}
	if (pthread_mutex_init(lock_params, NULL) < 0) {
		perror("pthread_mutex_init");
		exit(-1);
	}
	haddress = NULL;
	haddress_size = 6;
	obtainHardwareAddress();
	ret = init_sockets();
	return ret;
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

void finalize_all(){
	close_sockets();
	free(lock);
	free(lock_params);
	//free(iface);
	free(hostname);
	free(address);
	free(haddress);
}

void SIGINT_controller(int sigint){
	printf("SIGINT\n");
}

void SIGUSR2_controller(int sigusr2){
	printf("SIGUSR2\n");
}
