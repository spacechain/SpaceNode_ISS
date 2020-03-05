#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "spc_init_node.h"
#include "spc_cs_1.h"
#include "spc_cs_2.h"
#include "spc_start_work.h"

int main(int argc, char **argv)
{
#if 1
	if(argc != 2)
	{
		printf("SPC:wrong command!!!\n");
		return -1;
	}

	if((strlen(argv[1]) == 4) && (strncmp("init", argv[1], 4) == 0))
	{
		spc_init_node();
		return 0;
	}
	else if((strlen(argv[1]) == 3) && (strncmp("cs1", argv[1], 3) == 0))
	{
		spc_cs_1();
		return 0;
	}
	else if((strlen(argv[1]) == 3) && (strncmp("cs2", argv[1], 3) == 0))
	{
		spc_cs_2();
		return 0;
	}
	else if((strlen(argv[1]) == 5) && (strncmp("start", argv[1], 5) == 0))
	{
		spc_start_work();
		return 0;
	}
	else
	{
		printf("SPC:wrong input!!!\n");
		return -1;
	}
#endif
	//spc_start_work();
	return 0;
}
