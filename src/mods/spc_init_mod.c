#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>


#include "spc_init_mod.h"

#define READSTEP	10	


static int _checkStage()
{
	return (access(STAGEFILEPATH, R_OK | W_OK));
}


static int _validStage(uint32_t stage)
{
	if((stage < 1) || (stage > 4))
	{
		return -1;
	}
	else
	{
		return 0;
	}
}


static int _initStage()
{
	//open and create
	int fd = -1;
	fd = open(STAGEFILEPATH, O_WRONLY | O_CREAT, 0777);
	if(fd < 0)
	{
		perror("_initStage():open()");
		printf("Failed to create stage file.\n");
		return -1;
	}
	
	close(fd);
	return 0;
}


static int _setStage(uint32_t stage)
{
#if 0
	//check input
	if((stage < 1) || (stage > 4))
	{
		printf("_setStage():Got invalid input.\n");
		return -1;
	}

#endif	
	//open stage file
	int fd = -1;
	fd = open(STAGEFILEPATH, O_WRONLY);
	if(fd < 0)
	{
		perror("_setStage():open()");
		printf("Failed to open stage file.\n");
		return -1;
	}
	
	//write stage file
	int ret = -1;
	ret = write(fd, &stage, sizeof(stage));
	if(ret != sizeof(stage))
	{
		close(fd);
		if(-1 == ret)
		{
			perror("_setStage():write()");
		}
		else
		{
			printf("_setStage():write():");
		}
		printf("Failed to write value to stage file.\n");
		return -1;
	}
	
	//close satge file
	close(fd);
	return 0;

}


static int _getStage(uint32_t *pStage)
{
	if(NULL == pStage)
	{
		printf("_getStage():Got invalid address of return value.\n");
		return -1;
	}
	
	//open stage file
	int fd = -1;
	fd = open(STAGEFILEPATH, O_RDONLY);
	if(fd < 0)
	{
		perror("_getStage():open()");
		printf("Failed to open stage file.\n");
		*pStage = 0;
		return -1;
	}
	
	//read stage value from stage file
	uint32_t stage = 0;
	int ret = -1;
	ret = read(fd, &stage, sizeof(stage));
	if(ret != sizeof(stage))
	{
		close(fd);
		if(-1 == ret)
		{
			perror("_getStage():read()");
		}
		else
		{
			printf("_getStage():read():");
		}
		printf("Failed to read stage value from stage file.\n");
		*pStage = 0;
		return -1;
	}

	//close stage file
	close(fd);

	//return stage value	
	*pStage = stage;
	return 0;

}


static int _removeStage()
{
	return (remove(STAGEFILEPATH));
}


static int _pushStage(uint32_t stagePrv, uint32_t *stageNext)
{
	if(NULL == stageNext)
	{
		printf("_pushStage():");
		printf("Got invalid stageNext input.\n");
		return -1;
	}

	if((stagePrv > 0) && (stagePrv < 4))
	{
		*stageNext = stagePrv + 1;
		return 0;
	}

	*stageNext = 0;
	printf("_pushStage():");
	printf("Got invalid stagePrv input.\n");
	return -1;
}


//
//
int init_checkStage()
{
	//check stage file existence
	if(-1 == _checkStage())
	{
		printf("init_checkStage():");
		printf("No stage file detected, please call init_initStage() first.\n");
		return -1;
	}
	
	//get stage value
	uint32_t stage = 0;
	if(-1 == _getStage(&stage))
	{
		printf("init_checkStage():");
		printf("Failed to get stage value from stage file.\n");
		return -1;
	}

	//valid stage
	if(-1 == _validStage(stage))
	{
		printf("init_checkStage():");
		printf("Got invalid value from stage file.\n");
		return -1;
	}

	return 0;
}



//
//
int init_initStage()
{
	//_init() to create file
	if(-1 == _initStage())
	{
		printf("init_initStage():");
		printf("Failed to create stage file.\n");
		return -1;
	}

	//_set() to write init stage value, remove
	if(-1 == _setStage(INITSTAGEVALUE))
	{
		printf("init_initStage():");
		printf("Failed to write init stage value new stage file.\n");
		printf("Removing trash file now...\n");
		printf("init_initStage():");
		if(-1 == _removeStage())
		{
			printf("Failed to remove trash file.\n");
		}
		else
		{
			printf("Removing trash file done.\n");	
		}
		return -1;
	}

	return 0;
}


//
//
int init_getStage(uint32_t *pStage)
{
	//_check() check existence and input
	if(NULL == pStage)
	{
		printf("init_getStage():");
		printf("Got invalid input ");
		return -1;
	}

	if(-1 == _checkStage())
	{
		printf("init_getStage():");
		printf("No stage file detected, call init_initStage() first.\n");
		return -1;
	}
	
	//_get() get value
	uint32_t stage = 0;
	if(-1 == _getStage(&stage))
	{
		printf("init_getStage():");
		printf("Failed to get value from stage file.\n");
		*pStage = 0;
		return -1;
	}

	//_valid() valid value
	if(-1 == _validStage(stage))
	{
		printf("init_getStage():");
		printf("Got invalid value from stage file.\n");
		*pStage = 0;
		return -1;
	}

	*pStage = stage;
	return 0;
}


int init_pushStage()
{
	//_check() check existence
	if(-1 == _checkStage())
	{
		printf("init_pushStage():");
		printf("No stage file detected, call init_initStage() first.\n");
		return -1;
	}

	//_get() get value
	uint32_t stagePrv = 0;
	if(-1 == _getStage(&stagePrv))
	{
		printf("init_pushStage():");
		printf("Failed to get value from stage file.\n");
		return -1;
	}

	//_valid()
	if(-1 == _validStage(stagePrv))
	{
		printf("init_pushStage():");
		printf("Got invalid value of stagePrv.\n");
		return -1;
	}

	//_push() push stage
	uint32_t stageNext = 0;
	if(-1 == _pushStage(stagePrv, &stageNext))
	{
		printf("init_pushStage():");
		printf("Failed to get stageNext.\n");
		return -1;
	}

	//_valid()
	if(-1 == _validStage(stageNext))
	{
		printf("init_pushStage():");
		printf("Got invalid value of stageNext.\n");
		return -1;
	}
	
	//_set() write new value to file
	if(-1 == _setStage(stageNext))	
	{
		printf("init_pushStage():");
		printf("Failed to write stageNext to stage file.\n");
		return -1;
	}

	return 0;
}


//pubkeys exchange

static int _checkEpubFile()
{
	return (access(EPUBFILEPATH, R_OK));
}


static char *_runCreateNewAddr(uint32_t *pRetLen)
{
	//check input
	if(NULL == pRetLen)
	{
		printf("_runCreateNewAddr():");
		printf("Got invalid inputs.\n");
		return NULL;
	}

	//perform popen
	FILE *fRet = NULL;
	int ret = -1;

	fRet = popen(CREATENEWADDRCMD, "r");
	if(NULL == fRet)
	{
		perror("_runCreateNewAddr():popen()");
		printf("Failed to run create new addr cmd.\n");
		*pRetLen = 0;
		return NULL;
	}	

	int num = 0;
	int numSum = 0;
	void *buf = NULL;
	void *newBuf = NULL;
	buf = malloc(READSTEP);
	if(NULL == buf)
	{
		printf("_runCreateNewAddr():malloc():");
		printf("Failed to allocate memory for buffer.\n");
		if(-1 == pclose(fRet))
		{
			perror("_runCreateNewAddr():pclose()");
			printf("Failed to close result pipe file.\n");
		}
		*pRetLen = 0;
		return NULL;
	}
	memset(buf, '\0', READSTEP);
	while(numSum < 1024)
	{
		num = 0;
		num = fread(buf+numSum, 1, READSTEP, fRet);
		if((num < READSTEP) && (ferror(fRet) != 0))
		{
			printf("_runCreateNewAddr():fread():");
			printf("Failed to read data from result pipe file.\n");
			free(buf);
			if(-1 == pclose(fRet))
			{
				perror("_runCreateNewAddr():pclose()");
				printf("Failed to close result pipe file.\n");
			}
			*pRetLen = 0;
			return NULL;
		}
		else if((num <= READSTEP) && (ferror(fRet) == 0) && (feof(fRet) != 0))
		{
			//printf("num <= step, no error, eof.\n");
			numSum += num;
			break;
		}
		else if((num == READSTEP) && (ferror(fRet) == 0) && (feof(fRet) == 0))
		{
			printf("num == step, no error, no eof.\n");
			numSum += num;
			newBuf = realloc(buf, numSum+READSTEP);
			if(NULL == newBuf)
			{
				printf("_runCreateNewAddr():realloc():");
				printf("Failed to realloc for buffer.\n");
				free(buf);
				if(-1 == pclose(fRet))
				{
					perror("_runCreateNewAddr():pclose()");
					printf("Failed to close result pipe file.\n");
				}
				*pRetLen = 0;
				return NULL;	
			}
			
			memset(newBuf+numSum, '\0', READSTEP);
			buf = newBuf;
			newBuf = NULL;
			continue;
		}

	}		

	if(-1 == pclose(fRet))
	{
		perror("_runCreateNewAddr():pclose()");
		printf("Failed to close result pipe file.\n");
	}

	*pRetLen = numSum;
	return buf;

}


static void _freeAddrRet(char *pAddrRet)
{
	free(pAddrRet);
	return;
}


static char * _parseAddrRet(char *addrRet, uint32_t addrRetLen, uint32_t *pAddrLen)
{
	//check input
	if((NULL == addrRet) || (0 == addrRetLen) || (NULL == pAddrLen))
	{
		printf("_parseAddrRet():");
		printf("Got invalid input.\n");
		return NULL;
	}	

	//copy addrRet
	void *buf = NULL;
	buf = malloc(addrRetLen);
	if(NULL == buf)
	{
		printf("_parseAddrRet():");
		printf("Failed to allocate memory for buffer.\n");
		*pAddrLen = 0;
		return NULL;
	}
	memset(buf, '\0', addrRetLen);
	memcpy(buf, addrRet, addrRetLen);

	//delete new line 
	if(((char *)buf)[addrRetLen - 1] == '\n')
	{
		printf("_parseAddrRet():");
		printf("Got useless new line.\n");
		((char *)buf)[addrRetLen -1] = '\0';
		*pAddrLen = addrRetLen - 1;
		return buf;
	}

	//no new line 
	*pAddrLen = addrRetLen;
	return buf;

}


static void _freeAddr(char *pAddr)
{
	free(pAddr);
	return;
}


static char *_runGenEpub(char *addr, uint32_t addrLen, uint32_t *pEpubRetLen)
{
	//check inputs
	if((NULL == addr) || (addrLen == 0) || (NULL == pEpubRetLen))
	{
		printf("_runGenEpub():");
		printf("Got invalid inputs.\n");
		return NULL;
	}
	printf("addr:%s\n", addr);

	//strcat whole cmd 
	void *pCmd = NULL;
	pCmd = malloc(strlen(GETPUBKEYSCMDHEADER) + addrLen + 5);
	if(NULL == pCmd)
	{
		printf("_runGenEpub():");
		printf("Failed to allocate memory for cmd buffer.\n");
		*pEpubRetLen = 0;
		return NULL;
	}
	memset(pCmd, '\0', strlen(GETPUBKEYSCMDHEADER) + addrLen + 5);

	memcpy(pCmd, GETPUBKEYSCMDHEADER, strlen(GETPUBKEYSCMDHEADER));
	memcpy(pCmd+strlen(GETPUBKEYSCMDHEADER), addr, addrLen);

	//perform popen 
	FILE *fRet = NULL;
	int ret = -1;

	printf("%s\n", pCmd);
	fRet = popen(pCmd, "r");
	if(NULL == fRet)
	{
		perror("_runGenEpub():popen()");
		printf("Failed to run getpubkeys cmd.\n");
		*pEpubRetLen = 0;
		return NULL;
	}	

	int num = 0;
	int numSum = 0;
	void *buf = NULL;
	void *newBuf = NULL;
	buf = malloc(READSTEP);
	if(NULL == buf)
	{
		printf("_runGenEpub():malloc():");
		printf("Failed to allocate memory for buffer.\n");
		if(-1 == pclose(fRet))
		{
			perror("_runGenEpub():pclose()");
			printf("Failed to close result pipe file.\n");
		}
		*pEpubRetLen = 0;
		return NULL;
	}
	memset(buf, '\0', READSTEP);
	while(numSum < 1024)
	{
		num = 0;
		num = fread(buf+numSum, 1, READSTEP, fRet);
		if((num < READSTEP) && (ferror(fRet) != 0))
		{
			printf("_runGenEpub():fread():");
			printf("Failed to read data from result pipe file.\n");
			free(buf);
			if(-1 == pclose(fRet))
			{
				perror("_runGenEpub():pclose()");
				printf("Failed to close result pipe file.\n");
			}
			*pEpubRetLen = 0;
			return NULL;
		}
		else if((num <= READSTEP) && (ferror(fRet) == 0) && (feof(fRet) != 0))
		{
			printf("num <= step, no error, eof.\n");
			numSum += num;
			break;
		}
		else if((num == READSTEP) && (ferror(fRet) == 0) && (feof(fRet) == 0))
		{
			printf("num == step, no error, no eof.\n");
			numSum += num;
			newBuf = realloc(buf, numSum+READSTEP);
			if(NULL == newBuf)
			{
				printf("_runGenEpub():realloc():");
				printf("Failed to realloc for buffer.\n");
				free(buf);
				if(-1 == pclose(fRet))
				{
					perror("_runGenEpub():pclose()");
					printf("Failed to close result pipe file.\n");
				}
				*pEpubRetLen = 0;
				return NULL;	
			}
			
			memset(newBuf+numSum, '\0', READSTEP);
			buf = newBuf;
			newBuf = NULL;
			continue;
		}

	}		

	if(-1 == pclose(fRet))
	{
		perror("_runGenEpub():pclose()");
		printf("Failed to close result pipe file.\n");
	}

	*pEpubRetLen = numSum;
	return buf;

}


static void _freeEpubRet(char *pEpubRet)
{
	free(pEpubRet);
	return;
}


static char * _parseEpubRet(char *ePubRet, uint32_t ePubRetLen, uint32_t *pEpubLen)
{
	//check input
	if((NULL == ePubRet) || (0 == ePubRetLen) || (NULL == pEpubLen))
	{
		printf("_parseEpubRet():");
		printf("Got invalid input.\n");
		return NULL;
	}	

	//copy ePubRet
	void *buf = NULL;
	buf = malloc(ePubRetLen);
	if(NULL == buf)
	{
		printf("_parseEpubRet():");
		printf("Failed to allocate memory for buffer.\n");
		*pEpubLen = 0;
		return NULL;
	}
	memset(buf, '\0', ePubRetLen);
	memcpy(buf, ePubRet, ePubRetLen);

	//delete new line 
	if(((char *)buf)[ePubRetLen - 1] == '\n')
	{
		printf("_parseEpubRet():");
		printf("Got useless new line.\n");
		((char *)buf)[ePubRetLen -1] = '\0';
		*pEpubLen = ePubRetLen - 1;
		return buf;
	}

	//no new line 
	*pEpubLen = ePubRetLen;
	return buf;

}


static void _freeEpub(char *pEpub)
{
	free(pEpub);
	return;
}


static int _saveSatEpubToTempFile(char *satEpub, uint32_t satEpubLen)
{
	//check input
	if((NULL == satEpub) || (0 == satEpubLen))
	{
		printf("_saveSatEpubToFile():");
		printf("Got invalid inputs.\n");
		return -1;
	}

	//open file
	int fd = -1;
	fd = open(TEMPEPUBFILEPATH, O_CREAT | O_WRONLY, 0777);
	if(fd < 0)
	{
		printf("_saveSatEpubToFile():");
		printf("Failed to create and open epub.temp.\n");
		return -1;
	}

	//write satEpub into epub.temp
	int ret = -1;
	ret = write(fd, &satEpubLen, sizeof(satEpubLen));
	if(ret != sizeof(satEpubLen))
	{
		if(-1 == ret)
		{
			perror("_saveSatEpubToFile():write()");
		}
		else
		{
			printf("_saveSatEpubToFile():write():");
		}	
		printf("Failed to write length of sat Epub to epub.temp.\n");
		close(fd);
		return -1;
	}

	ret = write(fd, satEpub, satEpubLen);
	if(ret != satEpubLen)
	{
		if(-1 == ret)
		{
			perror("_saveSatEpubToFile():write()");
		}
		else
		{
			printf("_saveSatEpubToFile():write():");
		}
		printf("Failed to write satEpub to epub.temp.\n");
		close(fd);
		return -1;
	}
	
	//close file
	close(fd);
	return 0;
}


static char *_findSpcEpubFile(char *dirPath, uint32_t *retNameLen)
{
	//check inputs
	if((NULL == dirPath) || (NULL == retNameLen))
	{
		printf("_findSpcEpubFile():");
		printf("Got invalid inputs.\n");
		*retNameLen = 0;
		return NULL;
	}
	//printf("CHECKDONE.\n");

	//open dir 
	DIR *dUl = NULL;
	dUl = opendir(dirPath);
	if(NULL == dUl)
	{
		perror("_findSpcEpubFile():opendir()");
		printf("Failed to open upload directory.\n");
		*retNameLen = 0;
		return NULL;
	}	
	//printf("OPENDIRDONE.\n");
	
	//read dir 
	struct dirent *pD_st = NULL;
	void *pFileName = NULL;

	pD_st = readdir(dUl);
	while(NULL != pD_st)
	{
		if(strncmp(((pD_st->d_name)+strlen(pD_st->d_name)-strlen(SPCEPUBFILETAIL)), SPCEPUBFILETAIL, strlen(SPCEPUBFILETAIL)) == 0)
		{
			pFileName = malloc(strlen(pD_st->d_name)+1);
			if(NULL == pFileName)
			{
				printf("_findSpcEpubFile():readdir():malloc():");
				printf("Failed to allocate memory for ret name buffer.\n");

				if(-1 == closedir(dUl))
				{
					perror("_findSpcEpubFile():readdir():closedir()");
					printf("Failed to excute closedir().\n");
				}
				
				*retNameLen = 0;
				return NULL;
			}

			memset(pFileName, '\0', strlen(pD_st->d_name)+1);
			memcpy(pFileName, (pD_st->d_name), strlen(pD_st->d_name));
			break;			
		}	
		memset(pD_st->d_name, '\0', strlen(pD_st->d_name));	
		pD_st = readdir(dUl);
	}
	//printf("READDIRDONE.\n");	


	//close dir
	if(-1 == closedir(dUl))
	{
		perror("_findSpcEpubFile():readdir():closedir()");
		printf("Failed to excute closedir().\n");
		free(pFileName);
		*retNameLen = 0;
		return NULL;
	}
	 
	//return
	if(NULL == pFileName)
	{
		*retNameLen = 0;
		return NULL;
	}

	*retNameLen = strlen(pD_st->d_name);
	return pFileName; 

}


static void _freeSpcEpubFileName(char *pName)
{
	free(pName);
	return;
}


static int _countSpcEpubFile(char *dirPath)
{
	//check inputs
	if((NULL == dirPath))
	{
		printf("_countSpcEpubFile():");
		printf("Got invalid inputs.\n");
		return -1;
	}
	//printf("CHECKDONE.\n");

	//open dir 
	DIR *dUl = NULL;
	dUl = opendir(dirPath);
	if(NULL == dUl)
	{
		perror("_countSpcEpubFile():opendir()");
		printf("Failed to open upload directory.\n");
		return -1;
	}	
	//printf("OPENDIRDONE.\n");
	
	//read dir 
	struct dirent *pD_st = NULL;
	int retCount = 0;

	pD_st = readdir(dUl);
	while(NULL != pD_st)
	{
		if(strncmp(((pD_st->d_name)+strlen(pD_st->d_name)-strlen(SPCEPUBFILETAIL)), SPCEPUBFILETAIL, strlen(SPCEPUBFILETAIL)) == 0)
		{
			retCount++;
		}	
		
		memset(pD_st->d_name, '\0', strlen(pD_st->d_name));	
		pD_st = readdir(dUl);
	}
	//printf("READDIRDONE.\n");	


	//close dir
	if(-1 == closedir(dUl))
	{
		perror("_countSpcEpubFile():readdir():closedir()");
		printf("Failed to excute closedir().\n");
		return -1;
	}
	 
	//return
	return retCount; 

}


static int _mvSpcEpubFileToTempDir(char *fileName, uint32_t fileNameLen)
{
	//check 
	if((NULL == fileName) || (0 == fileNameLen))
	{
		printf("_mvSpcEpubFileToTempDir():");
		printf("Got invalid inputs.\n");
		return -1;
	}

	//gen old path and new path
	void *pOldPath = NULL;
	pOldPath = malloc(strlen(UPLOADDIR)+fileNameLen+1); 
	if(NULL == pOldPath)
	{	
		printf("_mvSpcEpubFileToTempDir():");
		printf("Failed to allocate memory for spc epub file old path.\n");
		return -1;
	}
	memset(pOldPath, '\0', strlen(UPLOADDIR)+fileNameLen+1);
	memcpy(pOldPath, UPLOADDIR, strlen(UPLOADDIR));
	strncat(pOldPath, fileName, fileNameLen);
	printf("OLDPATH:%s.\n", (char *)pOldPath);

	void *pNewPath = NULL;
	pNewPath = malloc(strlen(SPCEPUBFILETEMPPATH)+1); 
	if(NULL == pNewPath)
	{	
		printf("_mvSpcEpubFileToTempDir():");
		printf("Failed to allocate memory for spc epub file new path.\n");
		free(pOldPath);
		return -1;
	}
	memset(pNewPath, '\0', strlen(SPCEPUBFILETEMPPATH)+1);
	memcpy(pNewPath, SPCEPUBFILETEMPPATH, strlen(SPCEPUBFILETEMPPATH));
	printf("NEWPATH:%s.\n", (char *)pNewPath);

	//move
	if(-1 == rename(pOldPath, pNewPath))
	{
		perror("_mvSpcEpubFileToTempDir()");
		printf("Failed to move spc epub file from upload directory to temp directory.\n");
		free(pOldPath);
		free(pNewPath);
		return -1;
	}
	
	free(pOldPath);
	free(pNewPath);
	return 0;
}


static int _checkSpcEpubFileInTempDir()
{
	return (access(SPCEPUBFILETEMPPATH, R_OK));	
}


static int _parseSpcEpubFile(char *uuidBuf, uint32_t uuidBufLen, char **ppSpcEpub, uint32_t *pSpcEpubLen)
{
	//check input
	if((NULL == uuidBuf) || (uuidBufLen < 32) || (NULL == ppSpcEpub) || (NULL == pSpcEpubLen))
	{
		printf("_parseSpcEpubFile():");
		printf("Got invalid inputs.\n");
		*ppSpcEpub = NULL;
		*pSpcEpubLen = 0;
		return -1;
	}

	//open file
	int fd = -1;
	fd = open(SPCEPUBFILETEMPPATH, O_RDONLY);
	if(fd < 0)
	{
		perror("_parseSpcEpubFile():open()");
		printf("Failed to open spc.epub in temp dir.\n");
		*ppSpcEpub = NULL;
		*pSpcEpubLen = 0;
		return -1;	
	}
	
	
	//read uuid
	int ret = -1;
	memset(uuidBuf, '\0', UUIDLEN);
	ret = read(fd, uuidBuf, UUIDLEN);
	if(ret != UUIDLEN)
	{
		if(-1 == ret)
		{
			perror("_parseSpcEpubFile()");
		}
		else
		{
			printf("_parseSpcEpubFile():");
		}
		printf("Failed to read uuid from spc.epub.\n");
		close(fd);
		*ppSpcEpub = NULL;
		*pSpcEpubLen = 0;
		return -1;	
			
	}

	//read spc pubkey len
	uint32_t spcEpubLen = 0;
	ret = read(fd, &spcEpubLen, sizeof(spcEpubLen));
	if(ret != sizeof(spcEpubLen))
	{
		if(-1 == ret)
		{
			perror("_parseSpcEpubFile()");
		}
		else
		{
			printf("_parseSpcEpubFile():");
		}
		printf("Failed to read spc epub length from spc.epub.\n");
		close(fd);
		*ppSpcEpub = NULL;
		*pSpcEpubLen = 0;
		return -1;	
			
	}

	if(spcEpubLen > 256)
	{
		printf("_parseSpcEpubFile():");
		printf("Failed to read spc epub length from spc.epub.\n");
		close(fd);
		*ppSpcEpub = NULL;
		*pSpcEpubLen = 0;
		return -1;	
	}
	
	//read spc pubkey
	void *pSpcEpub = NULL;
	pSpcEpub = malloc(spcEpubLen);
	if(NULL == pSpcEpub)
	{
		printf("_parseSpcEpubFile():");
		printf("Failed to allocate memory for spc epub buffer.\n");
		close(fd);
		*ppSpcEpub = NULL;
		*pSpcEpubLen = 0;
		return -1;	
		
	}
	memset(pSpcEpub, '\0', spcEpubLen);

	ret = read(fd, pSpcEpub, spcEpubLen);
	if(ret != spcEpubLen)
	{
		if(-1 == ret)
		{
			perror("_parseSpcEpubFile()");
		}
		else
		{
			printf("_parseSpcEpubFile():");
		}
		printf("Failed to read spc epub from spc.epub.\n");
		close(fd);
		free(pSpcEpub);
		*ppSpcEpub = NULL;
		*pSpcEpubLen = 0;
		return -1;	
			
	}

	close(fd);

	//return
	*ppSpcEpub = pSpcEpub;
	*pSpcEpubLen = spcEpubLen;
	return 0;
}


static void _freeSpcEpub(char *pSpcEpub)
{
	free(pSpcEpub);
	return;
}


static int _saveSpcEpubToTempFile(char *spcEpub, uint32_t spcEpubLen)
{
	//check input
	if((NULL == spcEpub) || (0 == spcEpubLen))
	{
		printf("_saveSpcEpubToFile():");
		printf("Got invalid inputs.\n");
		return -1;
	}

	//open file
	int fd = -1;
	fd = open(TEMPEPUBFILEPATH, O_WRONLY | O_APPEND);
	if(fd < 0)
	{
		printf("_saveSpcEpubToFile():");
		printf("Failed to open epub.temp.\n");
		return -1;
	}

	//write spcEpub into epub.temp
	int ret = -1;
	ret = write(fd, &spcEpubLen, sizeof(spcEpubLen));
	if(ret != sizeof(spcEpubLen))
	{
		if(-1 == ret)
		{
			perror("_saveSpcEpubToFile():write()");
		}
		else
		{
			printf("_saveSpcEpubToFile():write():");
		}	
		printf("Failed to write length of spc Epub to epub.temp.\n");
		close(fd);
		return -1;
	}

	ret = write(fd, spcEpub, spcEpubLen);
	if(ret != spcEpubLen)
	{
		if(-1 == ret)
		{
			perror("_saveSpcEpubToFile():write()");
		}
		else
		{
			printf("_saveSpcEpubToFile():write():");
		}
		printf("Failed to write spc Epub to epub.temp.\n");
		close(fd);
		return -1;
	}
	
	//close file
	close(fd);
	return 0;
}


static int _mvEpubTempToEpubFile()
{
	return (rename(TEMPEPUBFILEPATH, EPUBFILEPATH));
}


static int _rmEpubFile()
{
	return (remove(EPUBFILEPATH));
}


static int _genTempSatEpubFile(char *uuid, uint32_t uuidLen, char *satEpub, uint32_t satEpubLen)
{
	//check 
	if((NULL == uuid) || (uuidLen != 32) || (NULL == satEpub) || (0 == satEpubLen))
	{
		printf("_genTempSatEpubFile():");
		printf("Got invalid inputs.\n");
		return -1;
	}

	//open file
	int fd = -1;
	fd = open(TEMPSATEPUBFILEPATH, O_CREAT | O_WRONLY, 0777);
	if(fd < 0)
	{
		printf("_genTempSatEpubFile():");
		printf("Failed to open temp.satepub.\n");
		return -1;
	}

	//write uuid
	int ret = -1;
	ret = write(fd, uuid, uuidLen);
	if(ret != uuidLen)
	{
		if(-1 == ret)
		{
			perror("_genTempSatEpubFile()");
		}
		else
		{
			printf("_genTempSatEpubFile():");
		}
		printf("Failed to write uuid into temp.satepub");
		close(fd);
		return -1;
	
	}

	//write length
	ret = write(fd, &satEpubLen, sizeof(satEpubLen));
	if(ret != sizeof(satEpubLen))
	{
		if(-1 == ret)
		{
			perror("_genTempSatEpubFile()");
		}
		else
		{
			printf("_genTempSatEpubFile():");
		}
		printf("Failed to write epub length into temp.satepub");
		close(fd);
		return -1;
	}

	//write epub
	ret = write(fd, satEpub, satEpubLen);
	if(ret != satEpubLen)
	{
		if(-1 == ret)
		{
			perror("_genTempSatEpubFile()");
		}
		else
		{
			printf("_genTempSatEpubFile():");
		}
		printf("Failed to write satEpub into temp.satepub");
		close(fd);
		return -1;
	}

	//close
	close(fd);
	return 0;
}


static int _genUuidSatEpubFile(char *uuid, uint32_t uuidLen)
{
	//check 
	if((NULL == uuid) || (uuidLen != 32))
	{
		printf("_genUuidSatEpubFile():");
		printf("Got invalid inputs.\n");
		return -1;
	}

	//get new path
	void *fileNameBuf = NULL;
	fileNameBuf = malloc(strlen(DOWNLOADDIR) + uuidLen + strlen(SATEPUBFILETAIL)+1);	
	if(NULL == fileNameBuf)
	{
		printf("_genUuidSatEpubFile():");
		printf("Failed to allocate memory for new file name.\n");
		return -1;
	}
	memset(fileNameBuf, '\0', (strlen(DOWNLOADDIR) + uuidLen + strlen(SATEPUBFILETAIL)+1));

	memcpy(fileNameBuf, DOWNLOADDIR, strlen(DOWNLOADDIR));	
	strncat(fileNameBuf, uuid, uuidLen);
	strncat(fileNameBuf, SATEPUBFILETAIL, strlen(SATEPUBFILETAIL));
	
	//rename
	int ret = -1;
	ret = rename(TEMPSATEPUBFILEPATH, fileNameBuf);
	if(-1 == ret)
	{
		perror("_genUuidSatEpubFile()");
		printf("Failed to rename temp.satepub.\n");
		free(fileNameBuf);
		return -1;

	}

	free(fileNameBuf);
	return 0;
}


//
//
int init_checkSpcEpubFile()
{
	int ret = -1;
	ret = _countSpcEpubFile(UPLOADDIR);
	if(1 == ret)
	{
		return 0;
	}
	else if(-1 == ret)
	{
		printf("init_checkSpcEpubFile():");
		printf("Failed to perform searching ops in upload directory.\n");
		return -1;
	}
	else if(0 == ret)
	{
		printf("init_checkSpcEpubFile():");
		printf("No xxxx.spcepub file found in upload directory.\n");
		return -1;
	}
	else if(ret > 1)
	{
		printf("init_checkSpcEpubFile():");
		printf("More than one xxxx.spcepub file found in upload directory.\n");
		return -1;
	}
}


//
//
int init_exchangeEpub()
{
	//get spc epub name
	int ret = -1;
	char *spcEpubFileName = NULL;
	uint32_t spcEpubFileNameLen = 0;
	spcEpubFileName = _findSpcEpubFile(UPLOADDIR, &spcEpubFileNameLen);	
	if((NULL == spcEpubFileName) || (0 == spcEpubFileNameLen))
	{
		printf("init_exchangeEpub():");
		printf("Failed to find uuid.spcepub in upload directory.\n");
		if(NULL != spcEpubFileName)
		{
			free(spcEpubFileName);
		}
		return -1;
	}


	//move upload/uuid.spcepub to temp/spc.epub
	//free name buffer
	ret = _mvSpcEpubFileToTempDir(spcEpubFileName, spcEpubFileNameLen);
	if(-1 == ret)
	{
		printf("init_exchangeEpub():");
		printf("Failed to move upload/uuid.spcepub to node/temp/spc.epub.\n");
		_freeSpcEpubFileName(spcEpubFileName);
		return -1;
	}
	_freeSpcEpubFileName(spcEpubFileName);
	

	//run create new addr cmd
	char *addrRet = NULL;
	uint32_t addrRetLen = 0;
	addrRet = _runCreateNewAddr(&addrRetLen);
	if((NULL == addrRet) || (0 == addrRetLen))
	{
		printf("init_exchangeEpub():");
		printf("Failed to run createnewaddress cmd.\n");
		if(NULL != addrRet)
		{
			free(addrRet);
		}
		return -1;
	}
	

	//parse addrRet
	//free addrRet 
	char *addr = NULL;
	uint32_t addrLen = 0;
	addr = _parseAddrRet(addrRet, addrRetLen, &addrLen);
	if((NULL == addr) || (0 == addrLen))
	{
		printf("init_exchangeEpub():");
		printf("Failed to parse addrRet.\n");
		if(NULL != addr)
		{
			free(addr);
		}
		_freeAddrRet(addrRet);
		return -1;
	}
	_freeAddrRet(addrRet);
	printf("addr:%s\n", addr);


	//run get pubkey cmd
	//free addr
	char *ePubRet = NULL;
	uint32_t ePubRetLen = 0;
	ePubRet = _runGenEpub(addr, addrLen, &ePubRetLen);
	if((NULL == ePubRet) || (0 == ePubRetLen))
	{
		printf("init_exchangeEpub():");
		printf("Failed to run getpubkeys cmd.\n");
		if(NULL != ePubRet)
		{
			free(ePubRet);
		}
		_freeAddr(addr);
		return -1;
	}
	_freeAddr(addr);

	//parse epubRet
	//free epubRet
	char *ePub = NULL;
	uint32_t ePubLen = 0;
	ePub = _parseEpubRet(ePubRet, ePubRetLen, &ePubLen);
	if((NULL == ePub) || (0 == ePubLen))
	{
		printf("init_exchangeEpub():");
		printf("Failed to parse ePubRet.\n");
		if(NULL != ePub)
		{
			free(ePub);
		}
		_freeEpubRet(ePubRet);
		return -1;
	}
	_freeEpubRet(ePubRet);


	//save sat epub to epub.temp
	ret = _saveSatEpubToTempFile(ePub, ePubLen);
	if(-1 == ret)
	{
		printf("init_exchangeEpub():");
		printf("Failed to save sat epub to epub.temp.\n");
		_freeEpub(ePub);
		return -1;
	}


	//parse spc.epub
	char uuid[UUIDLEN+1];
	memset(uuid, '\0', UUIDLEN+1);
	char *spcEpub = NULL;
	uint32_t spcEpubLen = 0;
	ret = _parseSpcEpubFile(uuid, UUIDLEN, &spcEpub, &spcEpubLen);
	if(-1 == ret)
	{
		printf("init_exchangeEpub():");
		printf("Failed to parse spc.epub.\n");
		if(NULL != spcEpub)
		{
			free(spcEpub);
		}
		_freeEpub(ePub);
		return -1;
	}


	//save spc epub to epub.temp
	//free spc epub 
	ret = _saveSpcEpubToTempFile(spcEpub, spcEpubLen);
	if(-1 == ret)
	{
		printf("init_exchangeEpub():");
		printf("Failed to save spc epub to epub.temp.\n");
		free(spcEpub);
		free(ePub);
		return -1;
	}
	_freeSpcEpub(spcEpub);


	//move temp/epub.temp to /node/epub.file
	ret = _mvEpubTempToEpubFile();
	if(-1 == ret)
	{
		printf("init_exchangeEpub():");
		printf("Failed to move epub.temp to epub.file.\n");
		free(ePub);
		return -1;
	}


	//gen temp.satepub
	//free sat epub	
	ret = _genTempSatEpubFile(uuid, UUIDLEN, ePub, ePubLen);
	if(-1 == ret)
	{
		printf("init_exchangeEpub():");
		printf("Failed to generate temp.satepub.\n");
		_freeEpub(ePub);
		return -1;
	}
	_freeEpub(ePub);
	
	//get uuid.satepub
	ret = _genUuidSatEpubFile(uuid, UUIDLEN);
	if(-1 == ret)
	{
		printf("init_exchangeEpub():");
		printf("Failed to move temp.satepub to uuid.satepub.\n");
		return -1;
	}

	printf("init_exchangeEpub():");
	printf("Generated download/%s.satepub, please check.\n", uuid);
	return 0;
}


#if 0
//test main
//for exchange pubkeys
int main()
{
	int ret;
	ret = init_checkSpcEpubFile();
	printf("CHECK RET:%d\n", ret);

	ret = init_exchangeEpub();
	printf("EXCHANGE RET:%d\n", ret);

	return 0;
}
#endif


//cs1
static int _countUuidSpcCs1File(char *dirPath)
{
	//check inputs
	if((NULL == dirPath))
	{
		printf("_countUuidSpcCs1File():");
		printf("Got invalid inputs.\n");
		return -1;
	}
	//printf("CHECKDONE.\n");

	//open dir 
	DIR *dUl = NULL;
	dUl = opendir(dirPath);
	if(NULL == dUl)
	{
		printf("_countUuidSpcCs1File():opendir():");
		printf("Failed to open upload directory.\n");
		return -1;
	}	
	//printf("OPENDIRDONE.\n");
	
	//read dir 
	struct dirent *pD_st = NULL;
	int retCount = 0;

	pD_st = readdir(dUl);
	while(NULL != pD_st)
	{
		if(strncmp(((pD_st->d_name)+strlen(pD_st->d_name)-strlen(SPCCS1FILETAIL)), SPCCS1FILETAIL, strlen(SPCCS1FILETAIL)) == 0)
		{
			retCount++;
		}	
		
		memset(pD_st->d_name, '\0', strlen(pD_st->d_name));	
		pD_st = readdir(dUl);
	}
	//printf("READDIRDONE.\n");	


	//close dir
	if(-1 == closedir(dUl))
	{
		printf("_countUuidSpcCs1File():readdir():closedir()");
		//perror("_countSpcEpubFile():readdir():closedir()");
		printf("Failed to excute closedir().\n");
		return -1;
	}
	 
	//return
	return retCount; 
}


static char *_findUuidSpcCs1File(char *dirPath, uint32_t *retNameLen)
{
	//check inputs
	if((NULL == dirPath) || (NULL == retNameLen))
	{
		printf("_findUuidSpcCs1File():");
		printf("Got invalid inputs.\n");
		*retNameLen = 0;
		return NULL;
	}
	//printf("CHECKDONE.\n");

	//open dir 
	DIR *dUl = NULL;
	dUl = opendir(dirPath);
	if(NULL == dUl)
	{
		perror("_findUuidSpcCs1File():opendir()");
		printf("Failed to open upload directory.\n");
		*retNameLen = 0;
		return NULL;
	}	
	//printf("OPENDIRDONE.\n");
	
	//read dir 
	struct dirent *pD_st = NULL;
	void *pFileName = NULL;

	pD_st = readdir(dUl);
	while(NULL != pD_st)
	{
		if(strncmp(((pD_st->d_name)+strlen(pD_st->d_name)-strlen(SPCCS1FILETAIL)), SPCCS1FILETAIL, strlen(SPCCS1FILETAIL)) == 0)
		{
			pFileName = malloc(strlen(pD_st->d_name)+1);
			if(NULL == pFileName)
			{
				printf("_findUuidSpcCs1File():readdir():malloc():");
				printf("Failed to allocate memory for ret name buffer.\n");

				if(-1 == closedir(dUl))
				{
					perror("_findUuidSpcCs1File():readdir():closedir()");
					printf("Failed to excute closedir().\n");
				}
				
				*retNameLen = 0;
				return NULL;
			}

			memset(pFileName, '\0', strlen(pD_st->d_name)+1);
			memcpy(pFileName, (pD_st->d_name), strlen(pD_st->d_name));
			break;			
		}	
		
		memset(pD_st->d_name, '\0', strlen(pD_st->d_name));	
		pD_st = readdir(dUl);
	}
	//printf("READDIRDONE.\n");	


	//close dir
	if(-1 == closedir(dUl))
	{
		perror("_findUuidSpcCs1File():readdir():closedir()");
		printf("Failed to excute closedir().\n");
		free(pFileName);
		*retNameLen = 0;
		return NULL;
	}
	 
	//return
	if(NULL == pFileName)
	{
		*retNameLen = 0;
		return NULL;
	}

	*retNameLen = strlen(pD_st->d_name);
	return pFileName; 

}


static void _freeUuidSpcCs1FileName(char *pFileName)
{
	free(pFileName);
	return;
}


static int _mvUuidSpcCs1ToSpcCs1(char *fileName, uint32_t fileNameLen)
{
	//check 
	if((NULL == fileName) || (0 == fileNameLen))
	{
		printf("_mvUuidSpcCs1ToSpcCs1():");
		printf("Got invalid inputs.\n");
		return -1;
	}

	//gen old path and new path
	void *pOldPath = NULL;
	pOldPath = malloc(strlen(UPLOADDIR)+fileNameLen+1); 
	if(NULL == pOldPath)
	{	
		printf("_mvUuidSpcCs1ToSpcCs1():");
		//printf("_mvSpcEpubFileToTempDir():");
		printf("Failed to allocate memory for uuid.spccs1 file old path.\n");
		return -1;
	}
	memset(pOldPath, '\0', strlen(UPLOADDIR)+fileNameLen+1);
	memcpy(pOldPath, UPLOADDIR, strlen(UPLOADDIR));
	strncat(pOldPath, fileName, fileNameLen);
	printf("OLDPATH:%s.\n", (char *)pOldPath);

	void *pNewPath = NULL;
	pNewPath = malloc(strlen(SPCCS1FILEPATH)+1); 
	if(NULL == pNewPath)
	{	
		printf("_mvUuidSpcCs1ToSpcCs1():");
		//printf("_mvSpcEpubFileToTempDir():");
		printf("Failed to allocate memory for spc.cs1 file new path.\n");
		free(pOldPath);
		return -1;
	}
	memset(pNewPath, '\0', strlen(SPCCS1FILEPATH)+1);
	memcpy(pNewPath, SPCCS1FILEPATH, strlen(SPCCS1FILEPATH));
	printf("NEWPATH:%s.\n", (char *)pNewPath);

	//move
	if(-1 == rename(pOldPath, pNewPath))
	{
		perror("_mvUuidSpcCs1ToSpcCs1()");
		//perror("_mvSpcEpubFileToTempDir()");
		printf("Failed to move uuid.spccs1 file from upload directory to temp directory.\n");
		free(pOldPath);
		free(pNewPath);
		return -1;
	}
	
	free(pOldPath);
	free(pNewPath);
	return 0;
}


static int _checkSpcCs1()
{
	return (access(SPCCS1FILEPATH, R_OK));
}


static char *_getSpcEpub(uint32_t *pSpcEpubLen)
{
	//check
	if(NULL == pSpcEpubLen)
	{
		printf("_getSpcEpub():");
		printf("Got invalid inputs.\n");
		return NULL;
	}

	//open epub.file 
	int fd = -1;
	fd = open(EPUBFILEPATH, O_RDONLY);
	if(fd < 0)
	{
		printf("_getSpcEpub():");
		printf("Failed to open epub.file.\n");
		*pSpcEpubLen = 0;
		return NULL;
	}


	//read satepub len
	//lseek
	uint32_t length = 0;
	int ret = -1;
	ret = read(fd, &length, sizeof(length));
	if(ret != sizeof(length))
	{
		if(-1 == ret)
		{
			perror("_getSpcEpub()");
		}
		else
		{
			printf("_getSpcEpub():");
		}
		printf("Failed to read length of sat epub.\n");
		close(fd);
		*pSpcEpubLen = 0;
		return NULL;
	}
	
	if((length > 512) || (0 == length))
	{
		printf("_getSpcEpub():");
		printf("Got invalid length of sat epub.\n");
		close(fd);
		*pSpcEpubLen = 0;
		return NULL;
	}
	

	ret = lseek(fd, length+sizeof(length), SEEK_SET);
	if(-1 == ret)
	{
		printf("_getSpcEpub():");
		printf("Failed to jump to the point of length of spc epub.\n");
		close(fd);
		*pSpcEpubLen = 0;
		return NULL;

	}

	//read spcEpub len
	//read spcEpub
	length = 0;
	ret = read(fd, &length, sizeof(length));
	if(ret != sizeof(length))
	{
		if(-1 == ret)
		{
			perror("_getSpcEpub()");
		}
		else
		{
			printf("_getSpcEpub():");
		}
		printf("Failed to read length of spc epub.\n");
		close(fd);
		*pSpcEpubLen = 0;
		return NULL;
		
	}

	if((length > 512) || (0 == length))
	{
		printf("_getSpcEpub():");
		printf("Got invalid length of spc epub.\n");
		close(fd);
		*pSpcEpubLen = 0;
		return NULL;
	}
	
	void *spcEpubBuf = NULL;
	spcEpubBuf = malloc(length);
	if(NULL == spcEpubBuf)
	{
		printf("_getSpcEpub():");
		printf("Failed to allocate memory for spc epub.\n");
		close(fd);
		*pSpcEpubLen = 0;
		return NULL;

	}	

	ret = read(fd, spcEpubBuf, length);
	if(ret != length)
	{
		if(-1 == ret)
		{
			perror("_getSpcEpub()");
		}
		else
		{
			printf("_getSpcEpub():");
		}
		printf("Failed to read spc epub.\n");
		close(fd);
		free(spcEpubBuf);
		*pSpcEpubLen = 0;
		return NULL;
		
	}

	//close
	close(fd);

	//return 
	*pSpcEpubLen = length;
	return spcEpubBuf;
}

/*
static void _freeSpcEpub(char *pSpcEpub)
{
	free(pSpcEpub);
	return;
}
*/

static char *_runEncrypt(char *spcEpub, uint32_t spcEpubLen, uint32_t *pEmsgRetLen)
{
	//check inputs
	if((NULL == spcEpub) || (spcEpubLen == 0) || (NULL == pEmsgRetLen))
	{
		printf("_runEncrypt():");
		printf("Got invalid inputs.\n");
		return NULL;
	}

	//strcat whole cmd 
	void *pCmd = NULL;
	pCmd = malloc(strlen(ENCRYPTCMDHEADER) + spcEpubLen + 1 + strlen(SECRET) + 5);
	if(NULL == pCmd)
	{
		printf("_runEncrypt():");
		printf("Failed to allocate memory for cmd buffer.\n");
		*pEmsgRetLen = 0;
		return NULL;
	}
	memset(pCmd, '\0', (strlen(ENCRYPTCMDHEADER) + spcEpubLen + 1 + strlen(SECRET) + 5));

	memcpy(pCmd, ENCRYPTCMDHEADER, strlen(ENCRYPTCMDHEADER));
	memcpy(pCmd+strlen(ENCRYPTCMDHEADER), spcEpub, spcEpubLen);
	strncat(pCmd, " ", strlen(" "));
	strncat(pCmd, SECRET, strlen(SECRET));

	//perform popen 
	FILE *fRet = NULL;
	int ret = -1;

	fRet = popen(pCmd, "r");
	if(NULL == fRet)
	{
		perror("_runEncrypt():popen()");
		printf("Failed to run encrypt cmd.\n");
		*pEmsgRetLen = 0;
		return NULL;
	}	

	int num = 0;
	int numSum = 0;
	void *buf = NULL;
	void *newBuf = NULL;
	buf = malloc(READSTEP);
	if(NULL == buf)
	{
		printf("_runEncrypt():malloc():");
		printf("Failed to allocate memory for buffer.\n");
		if(-1 == pclose(fRet))
		{
			perror("_runEncrypt():pclose()");
			printf("Failed to close result pipe file.\n");
		}
		*pEmsgRetLen = 0;
		return NULL;
	}
	memset(buf, '\0', READSTEP);
	while(numSum < 1024)
	{
		num = 0;
		num = fread(buf+numSum, 1, READSTEP, fRet);
		if((num < READSTEP) && (ferror(fRet) != 0))
		{
			printf("_runEncrypt():fread():");
			printf("Failed to read data from result pipe file.\n");
			free(buf);
			if(-1 == pclose(fRet))
			{
				perror("_runEncrypt():pclose()");
				printf("Failed to close result pipe file.\n");
			}
			*pEmsgRetLen = 0;
			return NULL;
		}
		else if((num <= READSTEP) && (ferror(fRet) == 0) && (feof(fRet) != 0))
		{
			printf("num <= step, no error, eof.\n");
			numSum += num;
			break;
		}
		else if((num == READSTEP) && (ferror(fRet) == 0) && (feof(fRet) == 0))
		{
			printf("num == step, no error, no eof.\n");
			numSum += num;
			newBuf = realloc(buf, numSum+READSTEP);
			if(NULL == newBuf)
			{
				printf("_runEncrypt():realloc():");
				printf("Failed to realloc for buffer.\n");
				free(buf);
				if(-1 == pclose(fRet))
				{
					perror("_runEncrypt():pclose()");
					printf("Failed to close result pipe file.\n");
				}
				*pEmsgRetLen = 0;
				return NULL;	
			}
			
			memset(newBuf+numSum, '\0', READSTEP);
			buf = newBuf;
			newBuf = NULL;
			continue;
		}

	}		

	if(-1 == pclose(fRet))
	{
		perror("_runEncrypt():pclose()");
		printf("Failed to close result pipe file.\n");
	}

	*pEmsgRetLen = numSum;
	return buf;

}


static void _freeMspcRet(char *pMspcRet)
{
	free(pMspcRet);
	return;
}


static char * _parseMspcRet(char *mSpcRet, uint32_t mSpcRetLen, uint32_t *mSpcLen)
{
	//check input
	if((NULL == mSpcRet) || (0 == mSpcRetLen) || (NULL == mSpcLen))
	{
		printf("_parseMspcRet():");
		printf("Got invalid input.\n");
		return NULL;
	}	

	//copy addrRet
	void *buf = NULL;
	buf = malloc(mSpcRetLen);
	if(NULL == buf)
	{
		printf("_parseMspcRet():");
		printf("Failed to allocate memory for buffer.\n");
		*mSpcLen = 0;
		return NULL;
	}
	memset(buf, '\0', mSpcRetLen);
	memcpy(buf, mSpcRet, mSpcRetLen);

	//delete new line 
	if(((char *)buf)[mSpcRetLen - 1] == '\n')
	{
		printf("_parseMspcRet():");
		printf("Got useless new line.\n");
		((char *)buf)[mSpcRetLen -1] = '\0';
		*mSpcLen = mSpcRetLen - 1;
		return buf;
	}

	//no new line 
	*mSpcLen = mSpcRetLen;
	return buf;

}


static void _freeMspc(char *pMspc)
{
	free(pMspc);
	return;
}


static int _saveMspcToFile(char *mSpc, uint32_t mSpcLen)
{
	//check
	if((NULL == mSpc) || (0 == mSpcLen))
	{
		printf("_saveMspcToFile():");
		printf("Got invalid inputs.\n");
		return -1;
	}

	//create and open mspc.data
	int fd = -1;
	fd = open(MSPCDATAFILEPATH, O_WRONLY | O_CREAT, 0777);
	if(fd < 0)
	{
		printf("_saveMspcToFile():open():");
		printf("Failed to create and open mspc.data.\n");
		return -1;
	}

	//write length of Mspc
	int ret = -1;
	ret = write(fd, &mSpcLen, sizeof(mSpcLen));
	if(sizeof(mSpcLen) != ret)
	{
		if(-1 == ret)
		{
			perror("_saveMspcToFile():write()");
		}
		else
		{
			printf("_saveMspcToFile():write():");
		}
		printf("Failed to write length of Mspc to mspc.data.\n");
		close(fd);
		return -1;
	}


	//write Mspc
	ret = write(fd, mSpc, mSpcLen);
	if(mSpcLen != ret)
	{
		if(-1 == ret)
		{
			perror("_saveMspcToFile():write()");
		}
		else
		{
			printf("_saveMspcToFile():write():");
		}
		printf("Failed to write Mspc to mspc.data.\n");
		close(fd);
		return -1;
	}	

	//close
	close(fd);

	return 0;
}


static int _parseSpcCs1(char *uuidBuf, uint32_t uuidBufLen)
{
	//check input
	if((NULL == uuidBuf) || (uuidBufLen < 32))
	{
		printf("_parseSpcCs1():");
		printf("Got invalid inputs.\n");
		return -1;
	}

	//open file
	int fd = -1;
	fd = open(SPCCS1FILEPATH, O_RDONLY);
	if(fd < 0)
	{
		perror("_parseSpcCs1():open()");
		printf("Failed to open spc.cs1 in temp dir.\n");
		return -1;	
	}
	
	
	//read uuid
	int ret = -1;
	memset(uuidBuf, '\0', UUIDLEN);
	ret = read(fd, uuidBuf, UUIDLEN);
	if(ret != UUIDLEN)
	{
		if(-1 == ret)
		{
			perror("_parseSpcCs1():malloc()");
		}
		else
		{
			printf("_parseSpcCs1():malloc():");
		}
		printf("Failed to read uuid from spc.cs1.\n");
		close(fd);
		return -1;	
			
	}
	
	close(fd);

	//return
	return 0;
}


static void _freeMsatUp(char *pMsatUp)
{
	free(pMsatUp);
	return;
}


static int _genTempSatCs1(char *uuid, uint32_t uuidLen, char *mSpc, uint32_t mSpcLen)
{
	//check
	if((NULL == uuid) || (uuidLen != UUIDLEN) || (NULL == mSpc) || (0 == mSpcLen))
	{
		printf("_genTempSatCs1():");
		printf("Got invalid inputs.\n");
		return -1;
	}


	//create and open
	int fd = -1;
	fd = open(TEMPSATCS1FILEPATH, O_CREAT | O_WRONLY, 0777);
	if(fd < 0)
	{
		printf("_genTempSatCs1():open():");
		printf("Failed to create and open temp.satcs1.\n");
		return -1;

	}


	//write uuid 
	int ret = -1;
	ret = write(fd, uuid, UUIDLEN);
	if(UUIDLEN != ret)
	{
		if(-1 == ret)
		{
			perror("_genTempSatCs1():write()");
		}
		else
		{
			printf("_genTempSatCs1():write():");
		}
		printf("Failed to write uuid into temp.satcs1.\n");
		close(fd);
		return -1;
	}


	//write length 
	uint32_t length = mSpcLen/2;
	ret = write(fd, &length, sizeof(length));
	if(sizeof(length) != ret)
	{
		if(-1 == ret)
		{
			perror("_genTempSatCs1():write()");
		}
		else
		{
			printf("_genTempSatCs1():write():");
		}
		printf("Failed to Mspc_up length into temp.satcs1.\n");
		close(fd);
		return -1;
	}


	//write Mspc_up
	printf("_genTempSatCs1():");
	printf("MspcUpLen:%d.\n", length);

	ret = write(fd, mSpc, length);
	if(length != ret)
	{
		if(-1 == ret)
		{
			perror("_genTempSatCs1():write()");
		}
		else
		{
			printf("_genTempSatCs1():write():");
		}
		printf("Failed to Mspc_up into temp.satcs1.\n");
		close(fd);
		return -1;
	}

	//close
	close(fd);


	return 0;
}


static int _genUuidSatCs1(char *uuid, uint32_t uuidLen)
{
	//check 
	if((NULL == uuid) || (uuidLen != 32))
	{
		printf("_genUuidSatCs1():");
		printf("Got invalid inputs.\n");
		return -1;
	}

	//get new path
	void *fileNameBuf = NULL;
	fileNameBuf = malloc(strlen(DOWNLOADDIR) + uuidLen + strlen(SATCS1FILETAIL)+1);	
	if(NULL == fileNameBuf)
	{
		printf("_genUuidSatCs1():");
		printf("Failed to allocate memory for new file name.\n");
		return -1;
	}
	memset(fileNameBuf, '\0', (strlen(DOWNLOADDIR) + uuidLen + strlen(SATCS1FILETAIL)+1));

	memcpy(fileNameBuf, DOWNLOADDIR, strlen(DOWNLOADDIR));	
	strncat(fileNameBuf, uuid, uuidLen);
	strncat(fileNameBuf, SATCS1FILETAIL, strlen(SATCS1FILETAIL));
	
	//rename
	int ret = -1;
	ret = rename(TEMPSATCS1FILEPATH, fileNameBuf);
	if(-1 == ret)
	{
		perror("_genUuidSatCs1()");
		printf("Failed to rename temp.satcs1.\n");
		free(fileNameBuf);
		return -1;

	}

	free(fileNameBuf);
	return 0;
}


//
//
int init_checkSpcCs1()
{
	int ret = -1;
	ret = _countUuidSpcCs1File(UPLOADDIR);
	if(1 == ret)
	{
		return 0;
	}
	else if(-1 == ret)
	{
		printf("init_checkSpcCs1():");
		printf("Failed to perform searching ops in upload directory.\n");
		return -1;
	}
	else if(0 == ret)
	{
		printf("init_checkSpcCs1():");
		printf("No xxxx.spccs1 file found in upload directory.\n");
		return -1;
	}
	else if(ret > 1)
	{
		printf("init_checkSpcCs1():");
		printf("More than one xxxx.spccs1 file found in upload directory.\n");
		return -1;
	}
}


int init_cs1()
{
	//find uuid.spccs1 in upload dir
	char *retFileName = NULL;
	uint32_t fileNameLen = 0;
	retFileName = _findUuidSpcCs1File(UPLOADDIR, &fileNameLen);
	if((NULL == retFileName) || (0 == fileNameLen))
	{
		printf("init_cs1():");
		printf("No uuid.spccs1 found in upload directory.\n");
		if(NULL != retFileName)
		{
			free(retFileName);
		}
		return -1;
	}

	//move upload/uuid.spccs1 to temp/spc.cs1
	//free retFileName
	int ret = -1;
	ret = _mvUuidSpcCs1ToSpcCs1(retFileName, fileNameLen);
	if(-1 == ret)
	{
		printf("init_cs1():");
		printf("Failed to move upload/uuid.spccs1 to temp/spc.cs1.\n");
		free(retFileName);
		return -1;
	}
	_freeUuidSpcCs1FileName(retFileName);


	//check spc.cs1
	ret = _checkSpcCs1();
	if(-1 == ret)
	{
		printf("init_cs1():_checkSpcCs1():");
		printf("Failed to find spc.cs1 in temp directory.\n");
		return -1;
	}


	//check epub.file
	ret = _checkEpubFile();
	if(-1 == ret)
	{
		printf("init_cs1():_checkEpubFile():");
		printf("Failed to found node/epub.file.\n");
		return -1;
	}



	//get spc epub from epub.file
	char *spcEpub = NULL;
	uint32_t spcEpubLen = 0;
	spcEpub = _getSpcEpub(&spcEpubLen);
	if((NULL == spcEpub) || (0 == spcEpubLen))
	{
		printf("init_cs1():_getSpcEpub():");
		printf("Failed to get spc epub from node/epub.file.\n");
		if(NULL != spcEpub)	
		{
			free(spcEpub);
		}
	
		return -1;
	}


	//run encrypt cmd
	//free spc epub 
	char *eMsgRet = NULL;
	uint32_t eMsgRetLen = 0;
	eMsgRet = _runEncrypt(spcEpub, spcEpubLen, &eMsgRetLen);
	if((NULL == eMsgRet) || (0 == eMsgRetLen) || (eMsgRetLen > 256))
	{
		printf("init_cs1():_runEncrypt():");
		printf("Failed to perform encrypt cmd.\n");
		if(NULL != eMsgRet)
		{
			free(eMsgRet);
		}
		free(spcEpub);
		return -1;
	}
	_freeSpcEpub(spcEpub);
	

	//parse encryptRet, get Mspc
	//free encryptRet
	char *mSpc = NULL;
	uint32_t mSpcLen = 0; 
	mSpc = _parseMspcRet(eMsgRet, eMsgRetLen, &mSpcLen);
	if((NULL == mSpc) || (0 == mSpcLen) || (mSpcLen > 256))
	{
		printf("init_cs1():_parseMspcRet():");
		printf("Failed to parse MspcRet.\n");
		if(NULL != mSpc)
		{
			free(mSpc);
		}
		free(eMsgRet);
		return -1;
	}
	_freeMspcRet(eMsgRet);
	printf("init_cs1():");
	printf("MspcLen:%d.\n", mSpcLen);


	//save  Mspc to file
	ret = _saveMspcToFile(mSpc, mSpcLen);
	if(-1 == ret)
	{
		printf("init_cs1():_saveMspcToFile():");
		printf("Failed to save Mspc to file.\n");
		free(mSpc);
		return -1;
	}


	//parse spc.cs1, get uuid
	char uuid[UUIDLEN + 1];
	memset(uuid, '\0', UUIDLEN+1);
	ret = _parseSpcCs1(uuid, UUIDLEN);
	if(-1 == ret)
	{
		printf("init_cs1():_parseSpcCs1():");
		printf("Failed to parse spc.cs1.\n");
		free(mSpc);
		return -1;
	}


	//gen temp.satcs1
	//free Mspc
	ret = _genTempSatCs1(uuid, UUIDLEN, mSpc, mSpcLen);
	if(-1 == ret)
	{
		printf("init_cs1():_genTempSatCs1():");
		printf("Failed to generate temp.satcs1.\n");
		free(mSpc);
		return -1;
	}
	_freeMspc(mSpc);

	//gen uuid.satcs1
	ret = _genUuidSatCs1(uuid, UUIDLEN);
	if(-1 == ret)
	{
		printf("init_cs1():_genUuidSatCs1():");
		printf("Failed to generate uuid.satcs1.\n");
		return -1;
	}

	return 0;
}


#if 0
//test main
//cs1
int main(void)
{
	if(-1 == init_checkSpcCs1())
		printf("CHECKFAILED\n");

	if(-1 == init_cs1())
		printf("CS1FAILED\n");

	return 0;
}
#endif 


//
//check stage2
static int _countUuidSpcCs2File(char *dirPath)
{
	//check inputs
	if((NULL == dirPath))
	{
		printf("_countUuidSpcCs2File():");
		printf("Got invalid inputs.\n");
		return -1;
	}
	//printf("CHECKDONE.\n");

	//open dir 
	DIR *dUl = NULL;
	dUl = opendir(dirPath);
	if(NULL == dUl)
	{
		printf("_countUuidSpcCs2File():opendir():");
		printf("Failed to open upload directory.\n");
		return -1;
	}	
	//printf("OPENDIRDONE.\n");
	
	//read dir 
	struct dirent *pD_st = NULL;
	int retCount = 0;

	pD_st = readdir(dUl);
	while(NULL != pD_st)
	{
		if(strncmp(((pD_st->d_name)+strlen(pD_st->d_name)-strlen(SPCCS2FILETAIL)), SPCCS2FILETAIL, strlen(SPCCS2FILETAIL)) == 0)
		{
			retCount++;
		}	
		
		memset(pD_st->d_name, '\0', strlen(pD_st->d_name));	
		pD_st = readdir(dUl);
	}
	//printf("READDIRDONE.\n");	


	//close dir
	if(-1 == closedir(dUl))
	{
		printf("_countUuidSpcCs2File():readdir():closedir()");
		//perror("_countSpcEpubFile():readdir():closedir()");
		printf("Failed to excute closedir().\n");
		return -1;
	}
	 
	//return
	return retCount; 
}


static char *_findUuidSpcCs2File(char *dirPath, uint32_t *retNameLen)
{
	//check inputs
	if((NULL == dirPath) || (NULL == retNameLen))
	{
		printf("_findUuidSpcCs2File():");
		printf("Got invalid inputs.\n");
		*retNameLen = 0;
		return NULL;
	}
	//printf("CHECKDONE.\n");

	//open dir 
	DIR *dUl = NULL;
	dUl = opendir(dirPath);
	if(NULL == dUl)
	{
		perror("_findUuidSpcCs2File():opendir()");
		printf("Failed to open upload directory.\n");
		*retNameLen = 0;
		return NULL;
	}	
	//printf("OPENDIRDONE.\n");
	
	//read dir 
	struct dirent *pD_st = NULL;
	void *pFileName = NULL;

	pD_st = readdir(dUl);
	while(NULL != pD_st)
	{
		if(strncmp(((pD_st->d_name)+strlen(pD_st->d_name)-strlen(SPCCS2FILETAIL)), SPCCS2FILETAIL, strlen(SPCCS2FILETAIL)) == 0)
		{
			pFileName = malloc(strlen(pD_st->d_name)+1);
			if(NULL == pFileName)
			{
				printf("_findUuidSpcCs1File():readdir():malloc():");
				printf("Failed to allocate memory for ret name buffer.\n");

				if(-1 == closedir(dUl))
				{
					perror("_findUuidSpcCs1File():readdir():closedir()");
					printf("Failed to excute closedir().\n");
				}
				
				*retNameLen = 0;
				return NULL;
			}

			memset(pFileName, '\0', strlen(pD_st->d_name)+1);
			memcpy(pFileName, (pD_st->d_name), strlen(pD_st->d_name));
			break;			
		}	
		
		memset(pD_st->d_name, '\0', strlen(pD_st->d_name));	
		pD_st = readdir(dUl);
	}
	//printf("READDIRDONE.\n");	


	//close dir
	if(-1 == closedir(dUl))
	{
		perror("_findUuidSpcCs2File():readdir():closedir()");
		printf("Failed to excute closedir().\n");
		free(pFileName);
		*retNameLen = 0;
		return NULL;
	}
	 
	//return
	if(NULL == pFileName)
	{
		*retNameLen = 0;
		return NULL;
	}

	*retNameLen = strlen(pD_st->d_name);
	return pFileName; 

}


static int _mvUuidSpcCs2ToSpcCs2(char *fileName, uint32_t fileNameLen)
{
	//check 
	if((NULL == fileName) || (0 == fileNameLen))
	{
		printf("_mvUuidSpcCs2ToSpcCs2():");
		printf("Got invalid inputs.\n");
		return -1;
	}

	//gen old path and new path
	void *pOldPath = NULL;
	pOldPath = malloc(strlen(UPLOADDIR)+fileNameLen+1); 
	if(NULL == pOldPath)
	{	
		printf("_mvUuidSpcCs2ToSpcCs2():");
		//printf("_mvSpcEpubFileToTempDir():");
		printf("Failed to allocate memory for uuid.spccs1 file old path.\n");
		return -1;
	}
	memset(pOldPath, '\0', strlen(UPLOADDIR)+fileNameLen+1);
	memcpy(pOldPath, UPLOADDIR, strlen(UPLOADDIR));
	strncat(pOldPath, fileName, fileNameLen);
	printf("OLDPATH:%s.\n", (char *)pOldPath);

	void *pNewPath = NULL;
	pNewPath = malloc(strlen(SPCCS2FILEPATH)+1); 
	if(NULL == pNewPath)
	{	
		printf("_mvUuidSpcCs2ToSpcCs2():");
		//printf("_mvSpcEpubFileToTempDir():");
		printf("Failed to allocate memory for spc.cs2 file new path.\n");
		free(pOldPath);
		return -1;
	}
	memset(pNewPath, '\0', strlen(SPCCS2FILEPATH)+1);
	memcpy(pNewPath, SPCCS2FILEPATH, strlen(SPCCS2FILEPATH));
	printf("NEWPATH:%s.\n", (char *)pNewPath);

	//move
	if(-1 == rename(pOldPath, pNewPath))
	{
		perror("_mvUuidSpcCs2ToSpcCs2()");
		//perror("_mvSpcEpubFileToTempDir()");
		printf("Failed to move uuid.spccs2 file from upload directory to temp directory.\n");
		free(pOldPath);
		free(pNewPath);
		return -1;
	}
	
	free(pOldPath);
	free(pNewPath);
	return 0;
}


static void _freeUuidSpcCs2FileName(char *pFileName)
{
	free(pFileName);
	return;
}


static int _checkSpcCs2()
{
	return (access(SPCCS2FILEPATH, R_OK));
}


static char *_getMsat(uint32_t *pMsatLen)
{
	//check 
	if(NULL == pMsatLen)
	{
		printf("_getMsat():");
		printf("Got invalid input.\n");
		return NULL;
	}

	//open spc.cs1
	int cs1Fd = -1;
	cs1Fd = open(SPCCS1FILEPATH, O_RDONLY);
	if(cs1Fd < 0)
	{
		printf("_getMsat():");
		printf("Failed to open spc.cs1.\n");	
		*pMsatLen = 0;
		return NULL;
	}
	
	//get Msat_up length
	int ret = -1;
	uint32_t mSatUpLen = 0;
	ret = lseek(cs1Fd, UUIDLEN, SEEK_SET);
	if(UUIDLEN != ret)
	{
		if(-1 == ret)
		{
			perror("_getMsat():lseek()");
			
		}
		else
		{
			printf("_getMsat():lseek():");
		}
		printf("Failed to jump to the point at the length of MsatUp.\n");
		close(cs1Fd);
		*pMsatLen = 0;
		return NULL;

	}

	ret = read(cs1Fd, &mSatUpLen, sizeof(mSatUpLen));
	if(sizeof(mSatUpLen) != ret)
	{
		if(-1 == ret)
		{
			perror("_getMsat():read()");
		}			
		else
		{
			printf("_getMsat():read():");
		}
		printf("Failed to read length of Msat_up.\n");
		close(cs1Fd);
		*pMsatLen = 0;
		return NULL;
	}

	printf("mSatUpLen:%d\n", mSatUpLen);

	if((0 == mSatUpLen))
	{
		printf("_getMsat():");
		printf("Got invalid Msat_up length.\n");
		close(cs1Fd);
		*pMsatLen = 0;
		return NULL;
	}

	//open spc.cs2
	int cs2Fd = -1;
	cs2Fd = open(SPCCS2FILEPATH, O_RDONLY);
	if(cs2Fd < 0)
	{
		printf("_getMsat():");
		printf("Failed to open spc.cs2.\n");
		close(cs1Fd);	
		*pMsatLen = 0;
		return NULL;
	}
	
	


	//get length of Msat_down
	uint32_t mSatDownLen = 0;
	ret = lseek(cs2Fd, UUIDLEN, SEEK_SET);
	if(UUIDLEN != ret)
	{
		if(-1 == ret)
		{
			perror("_getMsat():lseek()");
			
		}
		else
		{
			printf("_getMsat():lseek():");
		}
		printf("Failed to jump to the point at the length of MsatDown.\n");
		close(cs1Fd);
		close(cs2Fd);
		*pMsatLen = 0;
		return NULL;

	}

	ret = read(cs2Fd, &mSatDownLen, sizeof(mSatDownLen));
	if(sizeof(mSatDownLen) != ret)
	{
		if(-1 == ret)
		{
			perror("_getMsat():read()");
		}			
		else
		{
			printf("_getMsat():read():");
		}
		printf("Failed to read length of Msat_down.\n");
		close(cs2Fd);
		close(cs2Fd);
		*pMsatLen = 0;
		return NULL;
	}

	printf("mSatDownLen:%d\n", mSatDownLen);
	if((0 == mSatDownLen))
	{
		printf("_getMsat():");
		printf("Got invalid Msat_down length.\n");
		close(cs1Fd);
		close(cs2Fd);
		*pMsatLen = 0;
		return NULL;
	}


	//malloc buffer	
	void *buf = NULL;
	buf = malloc(mSatUpLen + mSatDownLen);
	if(NULL == buf)
	{
		printf("_getMsat():malloc():");
		printf("Failed to malloc memory for buffer.\n");
		close(cs1Fd);
		close(cs2Fd);
		*pMsatLen = 0;
		return NULL;
	}
	memset(buf, '\0', mSatUpLen + mSatDownLen);

	//read Msat_up to buffer
	//close spc.cs1
	ret = read(cs1Fd, buf, mSatUpLen);
	if(mSatUpLen != ret)
	{
		if(-1 == ret)
		{
			perror("_getMsat():read()");
		}
		else
		{
			printf("_getMsat():read():");
		}
		printf("Failed to read Msat_up.\n");
		close(cs1Fd);
		close(cs2Fd);
		*pMsatLen = 0;
		return NULL;
	}

	close(cs1Fd);

	//read Msat_down to buffer
	//close spc.cs2
	ret = read(cs2Fd, buf+mSatUpLen, mSatDownLen);
	if(mSatDownLen != ret)
	{
		if(-1 == ret)
		{
			perror("_getMsat():read()");
		}
		else
		{
			printf("_getMsat():read():");
		}
		printf("Failed to read Msat_down.\n");
		close(cs1Fd);
		close(cs2Fd);
		*pMsatLen = 0;
		return NULL;
	}

	close(cs2Fd);

	*pMsatLen = mSatUpLen + mSatDownLen;

	//return
	return buf;
}


static void _freeMsat(char *pMsat)
{
	free(pMsat);
	return;
}


static char *_getSatEpub(uint32_t *pSatEpubLen)
{
	//check
	if(NULL == pSatEpubLen)
	{
		printf("_getSatEpub():");
		printf("Got invalid inputs.\n");
		return NULL;
	}

	//open epub.file 
	int fd = -1;
	fd = open(EPUBFILEPATH, O_RDONLY);
	if(fd < 0)
	{
		printf("_getSatEpub():");
		printf("Failed to open epub.file.\n");
		*pSatEpubLen = 0;
		return NULL;
	}


	//read satepub len
	//lseek
	uint32_t length = 0;
	int ret = -1;
	ret = read(fd, &length, sizeof(length));
	if(ret != sizeof(length))
	{
		if(-1 == ret)
		{
			perror("_getSatEpub()");
		}
		else
		{
			printf("_getSatEpub():");
		}
		printf("Failed to read length of sat epub.\n");
		close(fd);
		*pSatEpubLen = 0;
		return NULL;
	}
	
	if((length > 512) || (0 == length))
	{
		printf("_getSatEpub():");
		printf("Got invalid length of sat epub.\n");
		close(fd);
		*pSatEpubLen = 0;
		return NULL;
	}
	

	//read satEpub
	void *satEpubBuf = NULL;
	satEpubBuf = malloc(length);
	if(NULL == satEpubBuf)
	{
		printf("_getSatEpub():");
		printf("Failed to allocate memory for sat epub.\n");
		close(fd);
		*pSatEpubLen = 0;
		return NULL;

	}	

	ret = read(fd, satEpubBuf, length);
	if(ret != length)
	{
		if(-1 == ret)
		{
			perror("_getSatEpub()");
		}
		else
		{
			printf("_getSatEpub():");
		}
		printf("Failed to read sat epub.\n");
		close(fd);
		free(satEpubBuf);
		*pSatEpubLen = 0;
		return NULL;
		
	}

	//close
	close(fd);

	//return 
	*pSatEpubLen = length;
	return satEpubBuf;
}


static void _freeSatEpub(char *pSatEpub)
{
	free(pSatEpub);
	return;
}


static char *_runDecrypt(char *satEpub, uint32_t satEpubLen, char *mSat, uint32_t mSatLen, uint32_t *pDmsgRetLen)
{
	//check inputs
	if((NULL == satEpub) || (satEpubLen == 0) || (NULL == mSat) || (0 == mSatLen) || (NULL == pDmsgRetLen))
	{
		printf("_runDecrypt():");
		printf("Got invalid inputs.\n");
		return NULL;
	}

	//strcat whole cmd 
	void *pCmd = NULL;
	pCmd = malloc(strlen(DECRYPTCMDHEADER) + satEpubLen + 1 + mSatLen + 1);
	if(NULL == pCmd)
	{
		printf("_runDecrypt():");
		printf("Failed to allocate memory for cmd buffer.\n");
		*pDmsgRetLen = 0;
		return NULL;
	}
	memset(pCmd, '\0', (strlen(DECRYPTCMDHEADER) + satEpubLen + 1 + mSatLen + 1));

	memcpy(pCmd, DECRYPTCMDHEADER, strlen(DECRYPTCMDHEADER));
	memcpy(pCmd+strlen(DECRYPTCMDHEADER), satEpub, satEpubLen);
	strncat(pCmd, " ", strlen(" "));
	strncat(pCmd, mSat, mSatLen);

	printf("pCmd:%s\n", (char *)pCmd);

	//perform popen 
	FILE *fRet = NULL;
	int ret = -1;

	fRet = popen(pCmd, "r");
	if(NULL == fRet)
	{
		perror("_runDecrypt():popen()");
		printf("Failed to run decrypt cmd.\n");
		*pDmsgRetLen = 0;
		return NULL;
	}	

	int num = 0;
	int numSum = 0;
	void *buf = NULL;
	void *newBuf = NULL;
	buf = malloc(READSTEP);
	if(NULL == buf)
	{
		printf("_runDecrypt():malloc():");
		printf("Failed to allocate memory for buffer.\n");
		if(-1 == pclose(fRet))
		{
			perror("_runDecrypt():pclose()");
			printf("Failed to close result pipe file.\n");
		}
		*pDmsgRetLen = 0;
		return NULL;
	}
	memset(buf, '\0', READSTEP);
	while(numSum < 1024)
	{
		num = 0;
		num = fread(buf+numSum, 1, READSTEP, fRet);
		if((num < READSTEP) && (ferror(fRet) != 0))
		{
			printf("_runDecrypt():fread():");
			printf("Failed to read data from result pipe file.\n");
			free(buf);
			if(-1 == pclose(fRet))
			{
				perror("_runDecrypt():pclose()");
				printf("Failed to close result pipe file.\n");
			}
			*pDmsgRetLen = 0;
			return NULL;
		}
		else if((num <= READSTEP) && (ferror(fRet) == 0) && (feof(fRet) != 0))
		{
			printf("num <= step, no error, eof.\n");
			numSum += num;
			break;
		}
		else if((num == READSTEP) && (ferror(fRet) == 0) && (feof(fRet) == 0))
		{
			printf("num == step, no error, no eof.\n");
			numSum += num;
			newBuf = realloc(buf, numSum+READSTEP);
			if(NULL == newBuf)
			{
				printf("_runDecrypt():realloc():");
				printf("Failed to realloc for buffer.\n");
				free(buf);
				if(-1 == pclose(fRet))
				{
					perror("_runDecrypt():pclose()");
					printf("Failed to close result pipe file.\n");
				}
				*pDmsgRetLen = 0;
				return NULL;	
			}
			
			memset(newBuf+numSum, '\0', READSTEP);
			buf = newBuf;
			newBuf = NULL;
			continue;
		}

	}		

	if(-1 == pclose(fRet))
	{
		perror("_runDecrypt():pclose()");
		printf("Failed to close result pipe file.\n");
	}

	*pDmsgRetLen = numSum;
	return buf;

}


static void _freeDmsgRet(char *pDmsgRet)
{
	free(pDmsgRet);
	return;
}


static char * _parseDmsgRet(char *dMsgRet, uint32_t dMsgRetLen, uint32_t *msgLen)
{
	//check input
	if((NULL == dMsgRet) || (0 == dMsgRetLen) || (NULL == msgLen))
	{
		printf("_parseDmsgRet():");
		printf("Got invalid input.\n");
		return NULL;
	}	

	//copy dMsgRet
	void *buf = NULL;
	buf = malloc(dMsgRetLen);
	if(NULL == buf)
	{
		printf("_parseDmsgRet():");
		printf("Failed to allocate memory for buffer.\n");
		*msgLen = 0;
		return NULL;
	}
	memset(buf, '\0', dMsgRetLen);
	memcpy(buf, dMsgRet, dMsgRetLen);

	//delete new line 
	if(((char *)buf)[dMsgRetLen - 1] == '\n')
	{
		printf("_parseDmsgRet():");
		printf("Got useless new line.\n");
		((char *)buf)[dMsgRetLen -1] = '\0';
		*msgLen = dMsgRetLen - 1;
		return buf;
	}

	//no new line 
	*msgLen = dMsgRetLen;
	return buf;

}


static void _freeMsg(char *pMsg)
{
	free(pMsg);
	return;
}


static int _checkMsg(char *msg, uint32_t msgLen)
{
	//check
	if((NULL == msg) || (0 == msgLen))
	{
		printf("_checkMsg():");
		printf("Got invalid inputs.\n");
		return -1;
	}

	//check length
	if(msgLen != strlen(THEWORDS))
	{
		return 1;
	}

	//check words
	if(0 != strncmp(msg, THEWORDS, strlen(THEWORDS)))
	{
		return 1;
	}

	return 0;
}


static int _parseSpcCs2(char *uuidBuf, uint32_t uuidBufLen)
{
	//check input
	if((NULL == uuidBuf) || (uuidBufLen < 32))
	{
		printf("_parseSpcCs2():");
		printf("Got invalid inputs.\n");
		return -1;
	}

	//open file
	int fd = -1;
	fd = open(SPCCS2FILEPATH, O_RDONLY);
	if(fd < 0)
	{
		perror("_parseSpcCs2():open()");
		printf("Failed to open spc.cs2 in temp dir.\n");
		return -1;	
	}
	
	
	//read uuid
	int ret = -1;
	memset(uuidBuf, '\0', UUIDLEN);
	ret = read(fd, uuidBuf, UUIDLEN);
	if(ret != UUIDLEN)
	{
		if(-1 == ret)
		{
			perror("_parseSpcCs2():malloc()");
		}
		else
		{
			printf("_parseSpcCs2():malloc():");
		}
		printf("Failed to read uuid from spc.cs2.\n");
		close(fd);
		return -1;	
			
	}
	
	close(fd);

	//return
	return 0;
}


static char *_getMspc(uint32_t *mSpcLen)
{
	//check 
	if(NULL == mSpcLen)
	{
		printf("_getMspc():");
		printf("Got invalid input.\n");
		return NULL;
	}

	//open file
	int fd = -1;
	fd = open(MSPCDATAFILEPATH, O_RDONLY);
	if(fd < 0)
	{
		printf("_getMspc():");
		printf("Failed to open mspc.data.\n");
		*mSpcLen = 0;
		return NULL;
	}

	
	//read length
	uint32_t length = 0;
	int ret = -1;
	ret = read(fd, &length, sizeof(length));
	if(sizeof(length) != ret)
	{
		if(-1 == ret)
		{
			perror("_getMspc():read()");
		}
		else
		{
			printf("_getMspc():read():");
		}
		printf("Failed to read length of Mspc.\n");
		close(fd);
		*mSpcLen = 0;
		return NULL;
	
	}

	if((length > 1024) || (0 == length))
	{
		printf("_getMspc():");
		printf("Got invalid length of Mspc.\n");
		close(fd);
		*mSpcLen = 0;
		return NULL;
	}


	//malloc buffer
	char *buf = NULL;
	buf = malloc(length);
	if(NULL == buf)
	{
		printf("_getMspc():malloc():");
		printf("Failed to malloc memory for buffer.\n");
		close(fd);
		*mSpcLen = 0;
		return NULL;
	}
	memset(buf, '\0', length);

	//read Mspc
	ret = read(fd, buf, length);
	if(length != ret)
	{
		if(-1 == ret)
		{
			perror("_getMspc():read()");
		}
		else
		{
			printf("_getMspc():read():");
		}
		printf("Failed to read Mspc from mspc.data.\n");
		close(fd);
		*mSpcLen = 0;
		return NULL;
	
	}

	//close
	close(fd);
	*mSpcLen = length;
	return buf;

}


static int _genTempSatCs2(char *uuid, uint32_t uuidLen, char *mSpc, uint32_t mSpcLen)
{
	//check
	if((NULL == uuid) || (uuidLen != UUIDLEN))
	{
		printf("_genTempSatCs2():");
		printf("Got invalid inputs.\n");
		return -1;
	}

	if(((NULL == mSpc) && (mSpcLen != 0)) || ((NULL != mSpc) && (mSpcLen == 0)))
	{
		printf("_genTempSatCs2():");
		printf("Got invalid inputs.\n");
		return -1;
	}



	//create and open
	int fd = -1;
	fd = open(TEMPSATCS2FILEPATH, O_CREAT | O_WRONLY, 0777);
	if(fd < 0)
	{
		printf("_genTempSatCs2():open():");
		printf("Failed to create and open temp.satcs2.\n");
		return -1;

	}


	//write uuid 
	int ret = -1;
	ret = write(fd, uuid, UUIDLEN);
	if(UUIDLEN != ret)
	{
		if(-1 == ret)
		{
			perror("_genTempSatCs2():write()");
		}
		else
		{
			printf("_genTempSatCs2():write():");
		}
		printf("Failed to write uuid into temp.satcs2.\n");
		close(fd);
		return -1;
	}


	//check failed 

	//write length 
	uint32_t length = 0;
	if(0 == mSpcLen)
	{
		length = 0;
	}
	else
	{
		length = mSpcLen - (mSpcLen/2);
	}
	ret = write(fd, &length, sizeof(length));
	if(sizeof(length) != ret)
	{
		if(-1 == ret)
		{
			perror("_genTempSatCs2():write()");
		}
		else
		{
			printf("_genTempSatCs2():write():");
		}
		printf("Failed to Mspc_down length into temp.satcs2.\n");
		close(fd);
		return -1;
	}

	if(0 == mSpcLen)
	{
		close(fd);
		return 0;
	}


	//write Mspc_up
	printf("_genTempSatCs2():");
	printf("MspcDownLen:%d.\n", length);

	ret = write(fd, mSpc+(mSpcLen/2), length);
	if(length != ret)
	{
		if(-1 == ret)
		{
			perror("_genTempSatCs2():write()");
		}
		else
		{
			printf("_genTempSatCs2():write():");
		}
		printf("Failed to Mspc_down into temp.satcs2.\n");
		close(fd);
		return -1;
	}

	//close
	close(fd);

	return 0;
}


static int _genUuidSatCs2(char *uuid, uint32_t uuidLen)
{
	//check 
	if((NULL == uuid) || (uuidLen != 32))
	{
		printf("_genUuidSatCs2():");
		printf("Got invalid inputs.\n");
		return -1;
	}

	//get new path
	void *fileNameBuf = NULL;
	fileNameBuf = malloc(strlen(DOWNLOADDIR) + uuidLen + strlen(SATCS2FILETAIL)+1);	
	if(NULL == fileNameBuf)
	{
		printf("_genUuidSatCs2():");
		printf("Failed to allocate memory for new file name.\n");
		return -1;
	}
	memset(fileNameBuf, '\0', (strlen(DOWNLOADDIR) + uuidLen + strlen(SATCS2FILETAIL)+1));

	memcpy(fileNameBuf, DOWNLOADDIR, strlen(DOWNLOADDIR));	
	strncat(fileNameBuf, uuid, uuidLen);
	strncat(fileNameBuf, SATCS2FILETAIL, strlen(SATCS2FILETAIL));
	
	//rename
	int ret = -1;
	ret = rename(TEMPSATCS2FILEPATH, fileNameBuf);
	if(-1 == ret)
	{
		perror("_genUuidSatCs2()");
		printf("Failed to rename temp.satcs2.\n");
		free(fileNameBuf);
		return -1;

	}

	free(fileNameBuf);
	return 0;
}


//
//
int init_checkSpcCs2()
{
	int ret = -1;
	ret = _countUuidSpcCs2File(UPLOADDIR);
	if(1 == ret)
	{
		return 0;
	}
	else if(-1 == ret)
	{
		printf("init_checkSpcCs2():");
		printf("Failed to perform searching ops in upload directory.\n");
		return -1;
	}
	else if(0 == ret)
	{
		printf("init_checkSpcCs2():");
		printf("No xxxx.spccs2 file found in upload directory.\n");
		return -1;
	}
	else if(ret > 1)
	{
		printf("init_checkSpcCs2():");
		printf("More than one xxxx.spccs2 file found in upload directory.\n");
		return -1;
	}
}

int init_cs2()
{
	//find xxx.spccs2
	char *retFileName = NULL;
	uint32_t retFileNameLen = 0;
	retFileName = _findUuidSpcCs2File(UPLOADDIR, &retFileNameLen);
	if((NULL == retFileName) || (0 == retFileNameLen))
	{
		printf("init_cs2():");
		printf("No xxxx.spccs2 found in upload directory.\n");
		if(retFileName != NULL)
		{
			free(retFileName);
		}
		return -1;
	}

	//move xxx.spccs2 to temp/spc.cs2
	//free retFileName
	int ret = -1;
	ret = _mvUuidSpcCs2ToSpcCs2(retFileName, retFileNameLen);
	if(-1 == ret)
	{
		printf("init_cs2():_mvUuidSpcCs2ToSpcCs2():");
		printf("Failed to move uuid.spccs2 to temp directory.\n");
		free(retFileName);
		return -1;
	}
	_freeUuidSpcCs2FileName(retFileName);


	//check cs1 check cs2
	ret = _checkSpcCs1();
	if(-1 == ret)
	{
		printf("init_cs2():_checkSpcCs1():");
		printf("No spc.cs1 found in temp directory.\n");
		return -1;
	}

	ret = _checkSpcCs2();
	if(-1 == ret)
	{
		printf("init_cs2():_checkSpcCs2():");
		printf("No spc.cs2 found in temp directory.\n");
		return -1;
	}

	//get Msat
	char *mSat = NULL;
	uint32_t mSatLen = 0;
	mSat = _getMsat(&mSatLen);
	if((NULL == mSat) || (0 == mSatLen))
	{
		printf("init_cs2():_getMsat():");
		printf("Failed to generate Msat.\n");
		if(NULL != mSat)
		{
			free(mSat);
		}
		return -1;
	}
	printf("MSAT:%s\n", mSat);

	//get sat epub
	char *satEpub = NULL;
	uint32_t satEpubLen = 0;
	satEpub = _getSatEpub(&satEpubLen);
	if((NULL == satEpub) || (0 == satEpubLen))
	{
		printf("init_cs2():_getSatEpub():");
		printf("Failed to get sat epub in epub.file.\n");
		if(NULL != satEpub)
		{
			free(satEpub);
		}
		free(mSat);
		return -1;
	}

	printf("SATEPUB:%s\n", satEpub);

	//run decrypt cmd
	//free sat epub
	//free Msat
	char *dMsgRet = NULL;
	uint32_t dMsgRetLen = 0;
	dMsgRet = _runDecrypt(satEpub, satEpubLen, mSat, mSatLen, &dMsgRetLen);
	if((NULL == dMsgRet) || (0 == dMsgRetLen))
	{
		printf("init_cs2():_runDecrypt():");
		printf("Failed to run decrypt cmd.\n");
		if(NULL != dMsgRet)
		{
			free(dMsgRet);
		}
		free(satEpub);
		free(mSat);
		return -1;
	}
	_freeSatEpub(satEpub);
	_freeMsat(mSat);
	
	//parse dmsg ret
	//free dmsg ret
	char *msg = NULL;
	uint32_t msgLen = 0;
	msg = _parseDmsgRet(dMsgRet, dMsgRetLen, &msgLen);
	if((NULL == msg) || (0 == msgLen))
	{
		printf("init_cs2():_parseDmsgRet():");
		printf("Failed to parse dmsg ret.\n");
		if(NULL != msg)
		{
			free(msg);
		}
		free(dMsgRet);
		return -1;
	}
	_freeDmsgRet(dMsgRet);
	printf("DMSG:%s\n", msg);

	//check
	//free msg
	int check = -1;
	check = _checkMsg(msg, msgLen);
	if(-1 == check)
	{
		printf("init_cs2():_checkMsg():");
		printf("Failed to compare msg and the words.\n");
		free(msg);
		return -1;
	}
	_freeMsg(msg);


	//parse cs2
	char uuid[UUIDLEN+1];
	memset(uuid, '\0', UUIDLEN+1);
	ret = _parseSpcCs2(uuid, UUIDLEN+1);
	if(-1 == ret)
	{
		printf("init_cs2():_parseSpcCs2():");
		printf("Failed to get uuid from spc.cs2.\n");
		return -1;
	}
	
	if(0 != check)
	{
		printf("init_cs2():");
		printf("Check Failed.\n");
		ret = _genTempSatCs2(uuid, UUIDLEN, NULL, 0);
	}
	else
	{
		//get mspc
		char *mSpc = NULL;
		uint32_t mSpcLen = 0;
		mSpc = _getMspc(&mSpcLen);
		if((NULL == mSpc) || (0 == mSpcLen))
		{
			printf("init_cs2():_getMspc():");
			printf("Failed to get Mspc from mspc.data.\n");
			if(mSpc != NULL)
			{
				free(mSpc);
			}
			return -1;
		}


		//gen temp.spccs2
		//free mspc
		ret = _genTempSatCs2(uuid, UUIDLEN, mSpc, mSpcLen);
		free(mSpc);
	}

	if(-1 == ret)
	{
		printf("init_cs2():_genTempSatCs2():");
		printf("Failed to generate temp.spccs2.\n");
		return -1;
	}


	//gen uuid.spccs2
	ret = _genUuidSatCs2(uuid, UUIDLEN);
	if(-1 == ret)
	{
		printf("init_cs2():_genUuidSatCs2():");
		printf("Failed to generate uuid.satcs2.\n");
		return -1;
	}
	printf("init_cs2():_genUuidSatCs2():");
	printf("Generate download/%s.satcs2.\n", uuid);

	return 0;
}

char *init_getSpcEpub(uint32_t *spcEpubLen)
{
	if(spcEpubLen == NULL)
		return NULL;
	char *ret = NULL;
	ret = _getSpcEpub(spcEpubLen);
	return ret;
}



#if 0
//test main
//check stage 2
int main(void)
{
	int ret = -1;

#if 0
	ret = init_checkSpcEpubFile();
	if(-1 == ret)
	{
		printf("PUBEXCHANGE_CHECK_FAILED\n");
	}

	ret = init_exchangeEpub();
	if(-1 == ret)
	{
		printf("PUBEXCHANGE_EXCHANGE_FAILED\n");
	}
#endif

#if 0
	ret = init_checkSpcCs1();
	if(-1 == ret)
	{
		printf("CS1_CHECK_FAILED\n");
	}

	ret = init_cs1();
	if(-1 == ret)
	{
		printf("CS1_CS1_FAILED\n");
	}
#endif

#if 1
	ret = init_checkSpcCs2();
	if(-1 == ret)
	{
		printf("CS2_CHECK_FAILED\n");
	}

	ret = init_cs2();
	if(-1 == ret)
	{
		printf("CS2_CS2_FAILED.\n");
	}

#endif
	return 0;
}

#endif



