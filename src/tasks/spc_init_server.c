#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "spc_server_mod.h"
#include "spc_hotp_mod.h"
#include "spc_seed_mod.h"
//#include "spc_sign_mod.h"
#include "spc_hotp_mod.h"
#include "spc_crypt_mod.h"
#include "spc_init_mod.h"
#include "spc_secret_mod.h"

#include "spc_init_server.h"

#define SECRETLEN	16
#define MAKEMAX		10

void spc_init_server(char *uuid, char *userEpub, uint32_t userEpubLen)
{
	int ret = -1;
	struct server_info info;
	memset(&info, '\0', sizeof(struct server_info));

	ret = server_getNewInfo(&info);
	if(ret != 0)
	{
		printf("Failed to get new info.\n");
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

	printf("DEBUG:GEN NEW SECRET DONE\n");

	ret = server_initNewServer(&info);
	if(ret != 0)
	{
		printf("Failed to init new server.\n");
		return;
	}

	printf("DEBUG:NEW SERVER INIT DONE\n");
	
	ret = seed_makeSeed(info.server_id, info.server_start, uuid, UUIDLEN);
	if(ret != 0)
	{
		printf("Failed to make seed for new server.\n");
		return;
	}

	info.server_start += MAKEMAX;
	
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
