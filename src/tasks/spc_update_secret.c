#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>


#include "spc_secret_mod.h"
#include "spc_server_mod.h"
#include "spc_hotp_mod.h"
#include "spc_seed_mod.h"
//#include "spc_sign_mod.h"
#include "spc_hotp_mod.h"
#include "spc_crypt_mod.h"
#include "spc_init_mod.h"
#include "spc_init_server.h"

#define SECRETLEN	16
#define MAKEMAX		10

void spc_update_secret(uint32_t serverId, char *userEpub, uint32_t userEpubLen, char *uuid)
{
	int ret = -1;
	struct server_info info;
	memset(&info, '\0', sizeof(struct server_info));

	ret = server_getServerInfo(&info, serverId);
	if(ret != 0)
	{
		printf("Failed to get no.%d info.\n", serverId);
		return;
	}

	char secretBuf[SECRETLEN+1] = {'\0'};
	uint32_t secretSeed = time(NULL);

	ret = hotp_getNewSecret(secretBuf, SECRETLEN+1, secretSeed);
	if(ret != 0)
	{
		printf("Failed to generate new secret.\n");
		return;
	}

	memcpy(info.server_secret, secretBuf, SECRETLEN);

	//gen sec file
	char *spcEpub = NULL;
	uint32_t spcEpubLen = 0;
	spcEpub = init_getSpcEpub(&spcEpubLen);
	if((NULL == spcEpub) || (spcEpubLen == 0))
	{
		if(spcEpub != NULL)
			free(spcEpub);
		printf("Failed to get spc epub.\n");
		return;
	}

	ret = secret_genSecFile(userEpub, userEpubLen, spcEpub, spcEpubLen, secretBuf, SECRETLEN, uuid, UUIDLEN, info.server_id);
	if(ret == -1)
	{
		printf("Failed to generate sec file.\n");
		free(spcEpub);
		return;
	}

	//BUG FIX:
	//clean counter to 1 after update OTP secret
	info.server_counter = 1;	

	ret = server_setServerInfo(&info);
	if(ret != 0)
	{
		printf("Failed to set server info into server.list.\n");
		//if failed to write into server.list should delete dir and struct in server.list 
		return;
	}	

	return;
}

#if 0
int main(void)
{
	spc_server_init();
	return 0;
}

#endif
