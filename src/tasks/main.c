#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "spc_init_node.h"
#include "spc_cs_1.h"
#include "spc_cs_2.h"
#include "spc_start_work.h"

int main()
{
#if 0
	int ret = -1;
	if(spc_init_node() == 0)
	{
		return 0;
	}
	else if(spc_cs_1() == 0)
	{
		return 0;
	}
	else if(spc_cs_2() == 0)
	{
		return 0;
	}
	else if(spc_start_work() == 0)
	{
		return 0;
	}
	else
	{
		printf("SPC:wrong input!!!\n");
		return -1;
	}
#endif
	spc_start_work();
	return 0;
}
