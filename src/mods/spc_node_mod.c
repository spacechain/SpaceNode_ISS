#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "spc_node_mod.h"

#define	READSTEP	20


static int _checkEwallet()
{
	return(access(EWALLETFILEPATH, F_OK));
}


static int _rmEwallet()
{
	return (remove(EWALLETFILEPATH));
}


static void _runCreate()
{
	system(CREATECMD);
	return;
}


int node_checkEwallet()
{
	return (_checkEwallet());
}


int node_rmEwallet()
{
	return (_rmEwallet());
}


int node_createEwallet()
{
	if(0 == _checkEwallet())
	{
		printf("node_createEwallet():");
		printf("Ewallet file exists, remove it first.\n");
		return -1;
	}

	_runCreate();

	return 0;
}


#if 0
int main(int argc, char **argv)
{
	if(-1 == node_checkEwallet())
	{
		printf("Ewallet does not exist.\n");
	}
	else
	{
		printf("Ewallet exists.\n");
	}

	if(-1 == node_rmEwallet())
	{
		printf("Ewallet has not been removed.\n");
	}
	else
	{
		printf("Ewallet has been removed.\n");
	}

	node_createEwallet();

	if(-1 == node_checkEwallet())
	{
		printf("Ewallet does not exist.\n");
	}
	else
	{
		printf("Ewallet exists.\n");
	}

	if(-1 == node_rmEwallet())
	{
		printf("Ewallet has not been removed.\n");
	}
	else
	{
		printf("Ewallet has been removed.\n");
	}

	return 0;
}

#endif 

