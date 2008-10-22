#include "../constants.h"
#include "../f_messages.h"

int pruebaFormatoMsg(){
	struct mdhcp_t str;
	struct msg_dhcp_t *message;
	struct mdhcp_t *mstr;

	str.op = 1;
	str.htype = 35;
	str.hlen = 134;
	str.hops = 254;
	str.xid = 323;
	str.secs = 2343;
	str.flags = 543;
	str.ciaddr = 33;
	str.yiaddr = 34543545;
	str.siaddr = 367;
	str.giaddr = 56566;
	str.opt_length = 0;

	print_mdhcp(&str);
	message = from_mdhcp_to_message(&str);
	print_message(message);
	mstr = from_message_to_mdhcp(message);
	print_mdhcp(mstr);
	return 0;
}
