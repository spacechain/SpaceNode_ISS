#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "spc_do_cmd.h"
#include "spc_server_mod.h"
#include "spc_init_server.h"
#include "spc_update_satpub.h"
#include "spc_update_secret.h"

void spc_do_cmd()
{
	//get count of cmd file
	int ret = -1;
	ret = server_countCmdFile();

	//loop do the job
	int i = 0;
	int pRet = -1;
	char uuid[UUIDLEN+1];
	uint32_t serverId = 0;
	char *userEpub = NULL;
	uint32_t userEpubLen = 0;

	for(i = 0; i < ret; i ++)
	{
		memset(uuid, '\0', UUIDLEN+1);
		serverId = 0;
		userEpub = NULL;
		userEpubLen = 0;

		pRet = server_parseCmdFile(uuid, UUIDLEN, &serverId, &userEpub, &userEpubLen);
		if(-1 == pRet)
		{
			if(userEpub != NULL)
				free(userEpub);
			continue;
		}
	
		//parse ret
		//init
		if((0 == serverId) && (0 != userEpubLen) && (NULL != userEpub))
		{
			printf("spc_do_cmd():");
			printf("do init work.\n");
			//init server
			spc_init_server(uuid, userEpub, userEpubLen);

		}
		else if((serverId != 0) && (0 != userEpubLen) && (NULL != userEpub))
		{
			printf("spc_do_cmd():");
			printf("do secret update.\n");
			//update secret
			spc_update_secret(serverId, userEpub, userEpubLen, uuid);
		}
		else if((serverId != 0) && (userEpubLen == 0))
		{
			printf("spc_do_cmd():");
			printf("do satpub update.\n");
			//update satpub
			spc_update_satpub(serverId, uuid);
		}
		else
		{
			printf("spc_do_cmd():");
			printf("got invalid ret.\n");
			if(userEpub != NULL)
				free(userEpub);
			continue;
		}
		free(userEpub);
		

	}	



	return;
}


