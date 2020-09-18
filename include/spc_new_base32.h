#ifndef __SPC_NEW_BASE32_H__
#define __SPC_NEW_BASE32_H__
#include <stdlib.h>


/*
	Default characters used in BASE32 digests.
	For use with otp_random_base32()
*/
static const char BASE32_CHARS[32] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
	'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
	'U', 'V', 'W', 'X', 'Y', 'Z', '2', '3', '4', '5',
	'6', '7'
};

int spc_new_base32(char *outBuf, uint32_t len, uint32_t seed);



#endif












