#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#include "spc_trash_mod.h"


static void _mvFileToTrashTemp()
{
	system(MVTOTRASHTEMPCMD);
	return;
}


static int _checkKeepList()
{
	return (access(KEEPLISTFILEPATH, R_OK));
}


struct keepFile_st
{
	unsigned char head;
	uint32_t length;
}__attribute__((__packed__));



static int _mvFileToDownloadDir()
{
	//open keep.list
	int fd = -1;	
	fd = open(KEEPLISTFILEPATH, O_RDONLY);
	if(fd < 0)
	{
		printf("_mvFileToDownloadDir():");
		printf("Failed to open keep.list.\n");
		return -1;
	}

	struct keepFile_st keepFile;
	memset(&keepFile, '\0', sizeof(keepFile));

	//loop
	//get one file
	int ret = -1;
	char *oldPath = NULL;
	oldPath = malloc(strlen(TRASHTEMPDIR)+50);
	if(NULL == oldPath)
	{
		printf("_mvFileToDownloadDir():");
		printf("Failed to malloc buffer for old path.\n");
		close(fd);
		return -1;
	}
	memset(oldPath, '\0', strlen(TRASHTEMPDIR) + 50);
	strncat(oldPath, TRASHTEMPDIR, strlen(TRASHTEMPDIR));

	char *newPath = NULL;
	newPath = malloc(strlen(DOWNLOADDIR)+50);
	if(NULL == newPath)
	{
		printf("_mvFileToDownloadDir():");
		printf("Failed to malloc buffer for new path.\n");
		free(oldPath);
		close(fd);
		return -1;
	}
	memset(newPath, '\0', strlen(DOWNLOADDIR) + 50);
	strncat(newPath, DOWNLOADDIR, strlen(DOWNLOADDIR));
	
	char fileName[50];

	while((ret = read(fd, &keepFile, sizeof(keepFile))) == sizeof(keepFile))
	{
		if(((keepFile.head) != 0xb1) || (keepFile.length > 100))
		{
			printf("_mvFileToDownloadDir():");
			printf("Got invalid keep file.\n");
			free(oldPath);
			free(newPath);
			close(fd);
			return -1;
		}

		memset(fileName, '\0', 50);
		memset(oldPath + strlen(TRASHTEMPDIR), '\0', 50);
		memset(newPath + strlen(DOWNLOADDIR), '\0', 50);
		//gen old path
		ret = read(fd, fileName, keepFile.length);
		if(ret != keepFile.length)
		{
			printf("_mvFileToDownloadDir():");
			printf("Got invalid file name.\n");
			free(oldPath);
			free(newPath);
			close(fd);
			return -1;
		}

		strncat(oldPath+strlen(TRASHTEMPDIR), fileName, keepFile.length);
		printf("%s\n", oldPath);

		//gen new path
		strncat(newPath+strlen(DOWNLOADDIR), fileName, keepFile.length);
		printf("%s\n", newPath);

		//rename
		ret = rename(oldPath, newPath);
		if(-1 == ret)
		{
			perror("_mvFileToDownloadDir():");
			printf("Failed to move file to download dir.\n");
		}
	}
	//close
	close(fd);
	free(oldPath);
	free(newPath);

	return 0;
}

static void _cleanTrashTemp()
{
	system(RMTRASHTEMPCMD);
	return;
}

//
int trash_keepFile()
{
	_mvFileToTrashTemp();
	int ret = -1;	
	ret = _checkKeepList();
	if(-1 == ret)
	{
		printf("trash_keepFile():");
		printf("Keep.list check failed.\n");
		return -1; 
	}

	_mvFileToDownloadDir();

	_cleanTrashTemp();

	return 0;
}


#if 0
int main()
{
	trash_keepFile();

	return 0;
}

#endif
