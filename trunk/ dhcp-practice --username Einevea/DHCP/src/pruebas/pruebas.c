#include "../constants.h"
#include "../f_messages.h"

int pruebaFormatoMsg(){
	struct msg_dhcp_t *message;
	struct mdhcp_t *str, *mstr;

	//Se crea un mensaje
	str =new_default_mdhcp();

	//Se imprimen y transforman los mensajes
	print_mdhcp(str);
	message = from_mdhcp_to_message(str);
	print_message(message);
	mstr = from_message_to_mdhcp(message);
	print_mdhcp(mstr);

	//Se libera la memoria
	free_mdhcp(str);
	free_message(message);
	free_mdhcp(mstr);
	return 0;
}
