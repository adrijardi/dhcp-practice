/*
 * dhcpcl.c
 *
 *  Created on: 17-oct-2008
 *      Author: dconde
 */
#include<stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dhcpcl.h"


//Parametros globales.
int exit_value, timeout, lease;
char* iface, hostname, address;
int debug = 0; // [Off: 0] [On: 1]


int main(int argc, const char* argv[]){
	exit_value = checkParams(argc, argv);
	if(exit_value==0){
		printf("HolaMundo%d\n", argc);
	}
	return exit_value;
}

int checkParams(int argc, const char* argv[]){
	int ret = 0;
	int i;
	char* param, *errPtr;
	if(argc >= 2 && argc <= 11){
		iface = (char*)argv[1];
		for (i = 2; i < argc && ret != -1; i+=2) {
			param = (char*)argv[i];
			if(strcmp(param,"-t")==0 && i+1 < argc){
				timeout = strtol(argv[i+1], &errPtr, 0);
				if(strlen(errPtr)!=0){
					printParamsError(1);
					ret = -1;
				}

			}else if(strcmp(param,"-h")==0 && i+1 < argc){
				param = (char*)argv[i+1];
				printf("-h: %s\n", param);

			}else if(strcmp(param,"-a")==0 && i+1 < argc){
				param = (char*)argv[i+1];
				printf("-a: %s\n", param);

			}else if(strcmp(param,"-l")==0 && i+1 < argc){
				lease = strtol(argv[i+1], &errPtr, 0);
				if(strlen(errPtr)!=0){
					printParamsError(2);
					ret = -1;
				}

			}else if(strcmp(param,"-d")==0){
				printf("Debug mode [ON]\n");
				debug = 1;
			}else{
				printParamsError(0);
				ret = -1;
			}
		}
	}else{
		printParamsError(0);
		ret = -1;
	}
	return ret;
}

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

