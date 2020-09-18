#include <stdio.h>

#include "spc_node_mod.h"
#include "spc_init_mod.h"
#include "spc_init_node.h"

int spc_init_node()
//void spc_init_node()
{
	//rm ewallet
	remove(EWALLETPATH);	

	int ret = -1;
	ret = node_checkEwallet();
	if(-1 == ret)
	{
		ret = node_createEwallet();
		if(-1 == ret)
		{
			printf("spc_init_node():Failed to create node ewallet.\n");
			return -1;
		}
	}

	ret = init_checkSpcEpubFile();
	if(-1 == ret)
	{
		printf("spc_init_node():No spc epub file found.\n");
		return -1;
	}

	ret = init_exchangeEpub();
	if(-1 == ret)
	{
		printf("spc_init_node():Failed to exchange epubkey.\n");
		return -1;
	}


	return 0;
}
