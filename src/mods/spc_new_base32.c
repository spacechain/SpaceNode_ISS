#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdint.h>

#include "spc_new_base32.h"

/*
	Generates a valid base32 number given len as size, chars as a charset,
	and out_str as output. out_str's size should be precomputed and
	null-terminated.
	
	Returns
			1 on success
			if out_str != 0, writes generated base32 as string to out_str
		error, 0

*/
int spc_new_base32(char *outBuf, uint32_t len, uint32_t seed) 
{
	if((NULL == outBuf) || (len <= 16))
	{
		printf("spc_new_base32():");
		printf("Got invalid inputs.\n");
		return -1;
	}

	srand(seed);

	memset(outBuf, '\0', len);

	for (unsigned int i = 0; i < 16; i++)
	{
		outBuf[i] = BASE32_CHARS[rand()%32];
	}
	return 0;
}




