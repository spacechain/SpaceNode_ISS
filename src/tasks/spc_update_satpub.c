#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>


#include "spc_server_mod.h"
#include "spc_hotp_mod.h"
#include "spc_seed_mod.h"
//#include "spc_sign_mod.h"
#include "spc_hotp_mod.h"
#include "spc_crypt_mod.h"

#include "spc_init_server.h"

#define SECRETLEN	16
#define MAKEMAX		10

void spc_update_satpub(uint32_t serverId, char *uuid)
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

	ret = seed_makeSeed(info.server_id, info.server_start, uuid, UUIDLEN);
	if(ret != 0)
	{
		printf("Failed to make new seed for server.\n");
		return;
	}

	info.server_start += MAKEMAX;
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
