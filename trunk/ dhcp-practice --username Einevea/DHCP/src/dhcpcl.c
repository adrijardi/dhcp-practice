/*
 * dhcpcl.c
 *
 *  Created on: 17-oct-2008
 *      Author: dconde
 */
#include<stdio.h>
#include "dhcpcl.h"


//Parametros globales.
int exit_value;

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
	if(argc >= 2 && argc <= 11){
		for (i = 1; i < argc; i++) {
			// Se comprueba el numero de parametros.
						/*if ( argc >= 1 && argc <= 6 ){
							x = atoi( argv[1] );
							y = atoi( argv[2] );

							printf( "%d + %d = %d\n", x, y, x + y );

							// Will print something like: 3 + 2 = 5
						}*/
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
		default:
			break;
	}
}

