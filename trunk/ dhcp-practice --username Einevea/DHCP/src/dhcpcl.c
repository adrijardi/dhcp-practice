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

#include "dhcpcl.h"
#include "constants.h"
#include "utils.h"
#include "pruebas/pruebas.h"
#include "dhcp_state.h"
#include "transfer.h"


//DECLARACION DE METODOS INTERNOS
void printParamsError(int err);
int checkParams(int argc, const char* argv[]);
void pruebas();
int checkIFace( char* iface);
void getFileParams();
int init();
void clean_close();



void pruebas(){
	//pruebaFormatoMsg();
}

/*
 * Funcion de inicio
 */
int main(int argc, const char* argv[]){
	debug = DEBUG_OFF;
	exit_value = checkParams(argc, argv);
	pruebas();//TODO quitar
	if(exit_value==0){
		printTrace(0, PID, NULL);
		getFileParams();
		exit_value=init();
		clean_close();
	}
	return exit_value;
}
void clean_close(){
	free(haddress);
}

int init(){
	// Se inicializan los parametros del estado.
	state = INIT;
	haddress=NULL;
	haddress_size = 6;
	// Se espera un numero aleatorio de segundos entre 1 y 10
	srandom(time(NULL));
	time_wait((random()%9000)+1000);
	//Se van a necesitar varios hilos a partir de este punto, envio de tramas y recepción de tramas bloqueantes.
	//sendMessage();
	obtainHardwareAddress();
	sendDHCPDISCOVER();
	return EXIT_NORMAL; //TODO
}

void getFileParams(){
	fprintf(stdout,"Obtenemos parametros?\n");
}

/*
 * Funcion que comprueba el numero de parametros de entrada, el formato de los mismos
 * y asigna los valores a las variables globales.
 */
int checkParams(int argc, const char* argv[]){
	int ret = EXIT_NORMAL;
	int i, iface_state;
	char *param, *errPtr;

	//Se comprueba el numero de parametros
	if(argc >= 2 && argc <= 11){

		//Se asigna el iface

		iface_state = checkIFace((char*)argv[1]);
		if(iface_state == true){
			iface = (char*)argv[1];

			for (i = 2; i < argc && ret != -1; i+=2) {

				// Se comprueba que la cadena sea de un parametro acordado
				param = (char*)argv[i];
				if(strcmp(param,"-t")==0 && i+1 < argc){
					//Se asigna el parametro timeout
					timeout = strtol(argv[i+1], &errPtr, 0);
					//Se comprueba que no haya habido un error de formato
					if(strlen(errPtr)!=0){
						printParamsError(1);
						ret = EXIT_ERROR;
					}

				}else if(strcmp(param,"-h")==0 && i+1 < argc){
					param = (char*)argv[i+1];
					printf("-h: %s\n", param);

				}else if(strcmp(param,"-a")==0 && i+1 < argc){
					param = (char*)argv[i+1];
					printf("-a: %s\n", param);

				}else if(strcmp(param,"-l")==0 && i+1 < argc){
					//Se asigna el parametro de lease
					lease = strtol(argv[i+1], &errPtr, 0);
					//Se comprueba que no haya habido un error de formato
					if(strlen(errPtr)!=0){
						printParamsError(2);
						ret = EXIT_ERROR;
					}

				}else if(strcmp(param,"-d")==0){
					//Se activa el modo debug
					printf("Debug mode [ON]\n");
					debug = DEBUG_ON;

				}else{
					//El parametro recibido no es un parametro acordado
					printParamsError(0);
					ret = EXIT_ERROR;
				}
			}
		}else{
			ret = EXIT_ERROR;
		}
	}else{
		printParamsError(0);
		ret = EXIT_ERROR;
	}
	return ret;
}

int checkIFace( char* iface){
	int ret;
	ret = true;
	//TODO comprobar iface
	printf("%d\n",ret);
	return ret;
}

/*
 * Funcion que imprime los mensajes de error de los parametros de entrada.
 */
void printParamsError(int err){
	switch (err) {
		case 0:
			printf("Error en el numero de parametros, el formato de ejecion es:\n\n dhcpcl interface [-t timeout] [-h hostname] [-a IP address] [-l leasetime] [-d]\n\n");
			printf("  −t timeout:\n\tEspecifica durante cuanto tiempo el cliente intenta conseguir una ip.\n\tEl valor por defecto son 64 segundos.\n");
			printf("  −h hostname:\n\tIndica la cadena a usar en el campo de opciones host_name del datagrama DHCP.\n");
			printf("  −a address:\n\tIndica la ultima direccion IP conocida, para ser enviada en el DHCPDISCOVER.\n");
			printf("  −l lease:\n\tEspecifica el valor del temporizador de arriendo sugerido al servidor, el servidor puede sobreescribir este valor.\n\tEl valor por defecto es infinito\n");
			printf("  −d:\n\tModo de depuracion.\n");
			break;
		case 1:
			printf("Error en el formato del valor de timeout.\n");
			printf("  −t timeout: utiliza valores enteros que representan segundos.\n");
			break;
		case 2:
			printf("Error en el formato del valor de lease.\n");
			printf("  −l lease: utiliza valores enteros que representan segundos.\n");
			break;
		default:
			break;
	}
}

