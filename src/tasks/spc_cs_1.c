#include <stdio.h>

#include "spc_cs_1.h"
#include "spc_init_mod.h"


int spc_cs_1()
{
	int ret = -1;
	ret = init_checkSpcCs1();
	if(-1 == ret)
	{
		printf("spc_cs_1():No cs1 file found.\n");
		return -1;
	}

	ret = init_cs1();
	if(-1 == ret)
	{
		printf("spc_cs_1():Check failed in stage 1.\n");
		return -1;
	}
	
	return 0;
}

