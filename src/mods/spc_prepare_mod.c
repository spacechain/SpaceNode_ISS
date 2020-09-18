#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "spc_prepare_mod.h"

#define BUFFERSIZE	512
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




static int _getCheckFileName(char *dataFileName, uint32_t dataFileNameLen, char *checkFileNameBuf, uint32_t checkFileNameBufLen)
{
	//check
	if((NULL == dataFileName) || (37 != dataFileNameLen) || (NULL == checkFileNameBuf) || (41 > checkFileNameBufLen))
	{
		printf("CHECKFILENAME:%s\n", dataFileName);
		printf("CHECKFILENAMELEN:%d\n", dataFileNameLen);
		printf("_getCheckFileName():");
		printf("Got invalid inputs.\n");
		return -1;
	}

	//get dataFileName
	char *nameBuf = NULL;
	nameBuf = malloc(37+10);
	if(NULL == nameBuf)
	{
		printf("_getCheckFileName():malloc():");
		printf("Failed to allocate memory for buffer.\n");
		return -1;
	}
	memset(nameBuf, '\0', 47);
	
	strncat(nameBuf, dataFileName, dataFileNameLen);

	uint32_t retLen = 0;
	//change tail of data file
	if(strncmp(nameBuf+dataFileNameLen-strlen(DATAFILETAIL), DATAFILETAIL, strlen(DATAFILETAIL)) == 0)
	{
		memcpy(nameBuf+dataFileNameLen-strlen(DATAFILETAIL), CHECKFILETAIL, strlen(CHECKFILETAIL));
		nameBuf[dataFileNameLen+(strlen(CHECKFILETAIL) - strlen(DATAFILETAIL))] = '\0'; 
		retLen = strlen(nameBuf);
	}
	else
	{
		printf("_getCheckFileName():");
		printf("Got invalid data file name.\n");
		free(nameBuf);
		return -1;
		
	}

	//copy to ret buf
	memset(checkFileNameBuf, '\0', checkFileNameBufLen);
	memcpy(checkFileNameBuf, nameBuf, retLen);
	free(nameBuf);

	return 0;
}


static int _getMd5(char *checkFileName, uint32_t checkFileNameLen, char *md5Buf, uint32_t md5BufLen)
{
	//check
	if((NULL == checkFileName) || (41 != checkFileNameLen) || (NULL == md5Buf) || (md5BufLen < 32))
	{
		printf("%d\n", checkFileNameLen);
		printf("_getMd5():");
		printf("Got invalid inputs.\n");
		return -1;
	}	

	//get check file path
	char *checkFilePath = NULL;
	checkFilePath = malloc(strlen(UPLOADDIR)+checkFileNameLen+1);
	if(NULL == checkFilePath)
	{
		printf("_getMd5():");
		printf("Failed to allocate memory for buffer.\n");
		return -1;
	}
	memset(checkFilePath, '\0', strlen(UPLOADDIR)+checkFileNameLen+1);

	strncat(checkFilePath, UPLOADDIR, strlen(UPLOADDIR));
	strncat(checkFilePath, checkFileName, checkFileNameLen);

	//access
	printf("%s\n", checkFilePath);
	if(access(checkFilePath, R_OK) != 0)
	{
		printf("_getMd5():");
		printf("Check file check failed.\n");
		free(checkFilePath);
		return -1;
	}

	//open
	int fd = -1;
	fd = open(checkFilePath, O_RDONLY);
	if(fd < 0)
	{
		perror("_getMd5()");
		printf("Failed to open check file.\n");
		free(checkFilePath);
		return -1;
	}
	free(checkFilePath);

	//read md5
	int ret = -1;
	char md5[33];
	memset(md5, '\0', 33); 
	ret = lseek(fd, 1+4+4, SEEK_SET);
	if(ret != 1+4+4)
	{
		perror("_getMd5()");
		printf("Failed to jump to head of checksum.\n");
		close(fd);
		return -1;
	}
	
	unsigned char head;
	ret = read(fd, &head, sizeof(head));
	if(ret != sizeof(head))
	{
		perror("_getMd5()");
		printf("Failed to read head of checksum.\n");
		close(fd);
		return -1;
	}

	if(head != 0xba)
	{
		perror("_getMd5()");
		printf("Got invalid head of checksum.\n");
		close(fd);
		return -1;
	}

	uint32_t length = 0;
	ret = read(fd, &length, sizeof(length));
	if(ret != sizeof(length))
	{
		perror("_getMd5()");
		printf("Failed to read length of checksum.\n");
		close(fd);
		return -1;
	}

	if(32 != length)
	{
		perror("_getMd5()");
		printf("Got invalid length of checksum.\n");
		close(fd);
		return -1;
	}

	ret = read(fd, md5, 32);
	if(ret != 32)
	{
		perror("_getMd5()");
		printf("Failed to read checksum.\n");
		close(fd);
		return -1;
	}

	//close
	close(fd);
	memcpy(md5Buf, md5, 32);
	printf("DEBUG:END OF GETMD5\n");
	
	return 0;
}

static int _genMd5(char *dataFileName, uint32_t dataFileNameLen, char *md5Buf, uint32_t md5BufLen)
{
	//check
	if((NULL == dataFileName) || (37 != dataFileNameLen) || (NULL == md5Buf) || (md5BufLen < 32))
	{
		printf("_genMd5():");
		printf("Got invalid inputs.\n");
		return -1;
	}	

	//get data file path
	char *dataFilePath = NULL;
	dataFilePath = malloc(strlen(UPLOADDIR)+dataFileNameLen+1);
	if(NULL == dataFilePath)
	{
		printf("_genMd5():");
		printf("Failed to allocate memory for buffer.\n");
		return -1;
	}
	memset(dataFilePath, '\0', strlen(UPLOADDIR)+dataFileNameLen+1);

	strncat(dataFilePath, UPLOADDIR, strlen(UPLOADDIR));
	strncat(dataFilePath, dataFileName, dataFileNameLen);

	//access
	if(access(dataFilePath, R_OK) != 0)
	{
		printf("_genMd5():");
		printf("Data file check failed.\n");
		free(dataFilePath);
		return -1;
	}

	//gen cmd
	char *md5Cmd = NULL;
	md5Cmd = malloc(strlen("md5sum ") + strlen(dataFilePath) + 5);
	if(NULL == md5Cmd)
	{
		printf("_genMd5():");
		printf("Failed to allocate buffer for md5sum cmd.\n");
		free(dataFilePath);
		return -1;
	}
	memset(md5Cmd, '\0', strlen("md5sum ") + strlen(dataFilePath) + 5);
	
	strncat(md5Cmd, "md5sum ", strlen("md5sum "));
	strncat(md5Cmd, dataFilePath, strlen(dataFilePath));
	free(dataFilePath);

	//popen
	FILE *pipe = NULL;
	pipe = popen(md5Cmd, "r");
	if(NULL == pipe)
	{
		printf("_genMd5():");
		printf("Failed to execute md5sum cmd.\n");
		free(md5Cmd);
		return -1;
	}
	free(md5Cmd);

	char md5[33];
	memset(md5, '\0', 33);
	int ret = 0;
	ret = fread(md5, 1, 32, pipe);
	if(ret != 32)
	{
		printf("_genMd5():");
		printf("Failed to execute md5sum cmd.\n");
		pclose(pipe);
		return -1;
	}
	
	memset(md5Buf, '\0', md5BufLen);
	memcpy(md5Buf, md5, 32);

	pclose(pipe);

	return 0;
}


static char *_getSig(char *checkFileName, uint32_t checkFileNameLen, uint32_t *sigLen)
{
	//check
	if((NULL == checkFileName) || (41 != checkFileNameLen) || (NULL == sigLen))
	{
		printf("_getSig():");
		printf("Got invalid inputs.\n");
		return NULL;
	}	

	//get check file path
	char *checkFilePath = NULL;
	checkFilePath = malloc(strlen(UPLOADDIR)+checkFileNameLen+1);
	if(NULL == checkFilePath)
	{
		printf("_getSig():");
		printf("Failed to allocate memory for buffer.\n");
		return NULL;
	}
	memset(checkFilePath, '\0', strlen(UPLOADDIR)+checkFileNameLen+1);

	strncat(checkFilePath, UPLOADDIR, strlen(UPLOADDIR));
	strncat(checkFilePath, checkFileName, checkFileNameLen);

	//access
	if(access(checkFilePath, R_OK) != 0)
	{
		printf("_getSig():");
		printf("Check file check failed.\n");
		free(checkFilePath);
		return NULL;
	}

	//open
	int fd = -1;
	fd = open(checkFilePath, O_RDONLY);
	if(fd < 0)
	{
		perror("_getSig()");
		printf("Failed to open check file.\n");
		free(checkFilePath);
		return NULL;
	}
	free(checkFilePath);

	//read sig
	int ret = -1;
	ret = lseek(fd, 1+4+4+1+4+32, SEEK_SET);
	if(ret != 1+4+4+1+4+32)
	{
		perror("_getSig()");
		printf("Failed to jump to head of sig.\n");
		close(fd);
		return NULL;
	}
	
	unsigned char head;
	ret = read(fd, &head, sizeof(head));
	if(ret != sizeof(head))
	{
		perror("_getSig()");
		printf("Failed to read head of checksum.\n");
		close(fd);
		return NULL;
	}

	if(head != 0xca)
	{
		perror("_getSig()");
		printf("Got invalid head of sig.\n");
		close(fd);
		return NULL;
	}

	uint32_t length = 0;
	ret = read(fd, &length, sizeof(length));
	if(ret != sizeof(length))
	{
		perror("_getSig()");
		printf("Failed to read length of sig.\n");
		close(fd);
		return NULL;
	}

	if((0 == length) || (length > 1024))
	{
		perror("_getSig()");
		printf("Got invalid length of sig.\n");
		close(fd);
		return NULL;
	}

	//malloc for sig
	char *sigBuf = NULL;
	sigBuf = malloc(length);
	if(NULL == sigBuf)
	{
		perror("_getSig()");
		printf("Failed to allocate memory for  sig.\n");
		close(fd);
		return NULL;
	}
	memset(sigBuf, '\0', length+1);
	
	ret = read(fd, sigBuf, length);
	if(ret != length)
	{
		perror("_getSig()");
		printf("Failed to read sig.\n");
		close(fd);
		return NULL;
	}

	//close
	close(fd);

	//return
	*sigLen = length;
	return sigBuf;


}

static void _freeSig(char *pSig)
{
	free(pSig);
	return;
}


static int _checkSig(char *spcEpub, uint32_t spcEpubLen, char *sig, uint32_t sigLen, char *md5, uint32_t md5Len)
{
	//check input
	if((NULL == spcEpub) || (0 == spcEpubLen) || (NULL == sig) || (0 == sigLen) || (NULL == md5) || (0 == md5Len))
	{
		printf("_checkSig():");
		printf("Got invalid inputs.\n");
		return -1;
	}

	//get cmd
	char *cmdBuf = NULL;
	cmdBuf = malloc(strlen(CHECKMD5CMDHEADER) + spcEpubLen + sigLen + md5Len + 10);
	if(NULL == cmdBuf)
	{
		printf("_checkSig():malloc():");
		printf("Failed to allocate memory for cmd.\n");
		return -1;
	}
	memset(cmdBuf, '\0',(strlen(CHECKMD5CMDHEADER) + spcEpubLen + sigLen + md5Len + 10));

	//strcat header
	strncat(cmdBuf, CHECKMD5CMDHEADER, strlen(CHECKMD5CMDHEADER));

	//strcat spc epub
	strncat(cmdBuf, spcEpub, spcEpubLen);

	//strcat sig 
	strncat(cmdBuf, " ", 1);
	strncat(cmdBuf, sig, sigLen);

	//strcat md5
	strncat(cmdBuf, " ", 1);
	strncat(cmdBuf, md5, md5Len);	

	//popen
	FILE *pipe = NULL;
	pipe = popen(cmdBuf, "r");
	if(NULL == pipe)	
	{
		printf("_checkSig():");
		printf("Failed to execute check cmd.\n");
		free(cmdBuf);
		return -1;
	}
	free(cmdBuf);

	//read
	char retStr[5];
	memset(retStr, '\0', 5);
	int ret = 0;
	ret = fread(retStr, 1, 4, pipe);
	if(ret != 4)
	{
		printf("_checkSig():fread():");
		printf("Failed to read result.\n");
		pclose(pipe);
		return -1;
	}
	pclose(pipe);	

	//check
	if(strncmp(retStr, "true", 4) == 0)
	{
		return 0;
	}
	else
	{
		return -1;	
	} 

}


static void _rmCheckFile()
{
	system(RMCHECKFILECMD);
	return;
}


static int _readDataToFile(int fd, uint32_t start, uint32_t length, char *filePath)
{
	//check inputs
	if((fd < 0) || (start > 50*1024*1024) || (length > 50*1024*1024) || (NULL == filePath))
	{
		printf("_readDataToFile():");
		printf("Got invalid inputs.\n");
		return -1;
	}

	//lseek to start
	int ret = -1;
	ret = lseek(fd, start, SEEK_SET);
	if(ret != start)	
	{
		printf("_readDataToFile():");
		printf("Failed to jump to the start.\n");
		return -1;
	}
	
	//create and opne file 
	int newFd = -1;
	printf("%s\n", filePath);	
	newFd = open(filePath, O_CREAT | O_WRONLY | O_TRUNC, 0777);	
	if(newFd < 0)
	{
		perror("_readDataToFile()");
		printf("Failed to open new file.\n");
		return -1;
	}

	//malloc buf
	void *buf = NULL;
	buf = malloc(BUFFERSIZE);
	if(NULL == buf)
	{
		printf("_readDataToFile():");
		printf("Failed to allocate buffer.\n");
		close(newFd);
		remove(filePath);
		return -1;
	}
	memset(buf, '\0', BUFFERSIZE);
	
	//read and write
	int rRet = -1;
	int wRet = -1;
	uint32_t sum = length;
	ret = lseek(fd, start, SEEK_SET);
	int rewind = 0;	
	while((rRet = read(fd, buf, BUFFERSIZE)) != 0)
	{
		if(rRet < 0)
		{
			if(errno == EINTR)
			{
				continue;
			}
			else
			{	
				free(buf);
				close(newFd);
				remove(filePath);
				return -1;
			}
		}
	
		if(sum >= rRet)
		{
			sum = sum - rRet;
		}
		else
		{
			rewind = (rRet - sum);
			rewind = -rewind;
			rRet = sum;
			sum = sum - rRet;
			lseek(fd, rewind, SEEK_CUR);
		}
	
		wRet = write(newFd, buf, rRet);
		if(wRet < 0)
		{
			if(errno == EINTR)
			{
				continue;
			}
			else
			{
				free(buf);
				close(newFd);
				remove(filePath);
				return -1;
			}
		}
		memset(buf, '\0', BUFFERSIZE);
		if(sum == 0)
		{
			break;
		}		
	}

	free(buf);
	close(newFd);
	return 0;
}


static char *_findDataFile(char *dirPath, uint32_t *retNameLen)
{
	//check inputs
	if((NULL == dirPath) || (NULL == retNameLen))
	{
		printf("_findDataFile():");
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
		perror("_findDataFile():opendir()");
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
		if(strncmp(((pD_st->d_name)+strlen(pD_st->d_name)-strlen(DATAFILETAIL)), DATAFILETAIL, strlen(DATAFILETAIL)) == 0)
		{
			pFileName = malloc(strlen(pD_st->d_name)+1);
			if(NULL == pFileName)
			{
				printf("_findDataFile():readdir():malloc():");
				printf("Failed to allocate memory for ret name buffer.\n");

				if(-1 == closedir(dUl))
				{
					perror("_findDataFile():readdir():closedir()");
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
		perror("_findDataFile():readdir():closedir()");
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


static void _freeDataFileName(char *pName)
{
	free(pName);
	return;
}


static int _countDataFile(char *dirPath)
{
	//check inputs
	if((NULL == dirPath))
	{
		printf("_countDataFile():");
		printf("Got invalid inputs.\n");
		return -1;
	}

	//open dir 
	DIR *dUl = NULL;
	dUl = opendir(dirPath);
	if(NULL == dUl)
	{
		perror("_countDataFile():opendir()");
		printf("Failed to open upload directory.\n");
		return -1;
	}	
	
	//read dir 
	struct dirent *pD_st = NULL;
	int retCount = 0;

	pD_st = readdir(dUl);
	while(NULL != pD_st)
	{
		if(strncmp(((pD_st->d_name)+strlen(pD_st->d_name)-strlen(DATAFILETAIL)), DATAFILETAIL, strlen(DATAFILETAIL)) == 0)
		{
			retCount++;
		}	
		memset(pD_st->d_name, '\0', strlen(pD_st->d_name));	
		pD_st = readdir(dUl);
	}

	//close dir
	if(-1 == closedir(dUl))
	{
		perror("_countDataFile():readdir():closedir()");
		printf("Failed to excute closedir().\n");
		return -1;
	}
	 
	//return
	return retCount; 

}


static int _mvDataFileToTempDir(char *fileName, uint32_t fileNameLen)
{

	//check 
	if((NULL == fileName) || (0 == fileNameLen))
	{
		printf("_mvDataFileToTempDir():");
		printf("Got invalid inputs.\n");
		return -1;
	}

	//gen old path and new path
#if 1
	void *pOldPath = NULL;
	pOldPath = malloc(strlen(UPLOADDIR)+fileNameLen); 
	if(NULL == pOldPath)
	{	
		printf("_mvDataFileToTempDir():");
		printf("Failed to allocate memory for data file old path.\n");
		return -1;
	}
#endif
	//char pOldPath[strlen(UPLOADDIR)+fileNameLen] = {'\0'};
	memset(pOldPath, '\0', strlen(UPLOADDIR)+fileNameLen);
	memcpy(pOldPath, UPLOADDIR, strlen(UPLOADDIR));
	strncat(pOldPath, fileName, fileNameLen);
	printf("PREPARE MOD:\n");
	printf("src dir: %s.\n", (char *)pOldPath);

	void *pNewPath = NULL;
	pNewPath = malloc(strlen(TEMPDATAFILEPATH)+1); 
	if(NULL == pNewPath)
	{	
		printf("_mvDataFileToTempDir():");
		printf("Failed to allocate memory for data file new path.\n");
		free(pOldPath);
		return -1;
	}
	memset(pNewPath, '\0', strlen(TEMPDATAFILEPATH)+1);
	memcpy(pNewPath, TEMPDATAFILEPATH, strlen(TEMPDATAFILEPATH));
	printf("PREPARE MOD:\n");
	printf("dest dir: %s.\n", (char *)pNewPath);

	//move
	if(-1 == rename(pOldPath, pNewPath))
	{
		perror("_mvDataFileToTempDir()");
		printf("Failed to move data file from upload directory to temp directory.\n");
		free(pOldPath);
		free(pNewPath);
		return -1;
	}
	
	free(pOldPath);
	free(pNewPath);
	return 0;
}


static int _checkTempDataInTempDir()
{
	return (access(TEMPDATAFILEPATH, R_OK));	
}


//check sum chcek

//after open
//check head and tail
static int _checkHeadAndTail(int fd)
{
	printf("PREPARE MOD: ready to check data file.\n");

	//check 
	if(fd < 0)
	{
		printf("_checkHeadAndTail():");
		printf("Got invalid fd.\n");
		return -1;
	}
	
	//lseek to start
	int ret = -1;
	ret = lseek(fd, 0, SEEK_SET);
	if(0 != ret)
	{
		if(-1 == ret)
		{
			perror("_checkHeadAndTail()");
		}
		else
		{
			printf("_checkHeadAndTail():");
		}
		printf("Failed to jump to the start of data file.\n");
		return -1;
	}
	
	//check head
	unsigned char head = 0;
	ret = read(fd, &head, sizeof(head));
	if(sizeof(head) != ret)
	{
		if(-1 == ret)
		{
			perror("_checkHeadAndTail()");
		}
		else
		{
			printf("_checkHeadAndTail():");
		}

		printf("Failed to read head from data file.\n");
		return -1;
	}

	if(0xa1 != head)
	{
		printf("_checkHeadAndTail():");
		printf("Invalid head in data file.\n");
		return -1;
	}


	//lseek to length
	ret = lseek(fd, 1+sizeof(uint32_t), SEEK_SET);
	if((1+sizeof(uint32_t)) != ret)
	{
		if(-1 == ret)
		{
			perror("_checkHeadAndTail()");
		}
		else
		{
			printf("_checkHeadAndFile():");
		}
		printf("Failed to jump to point of data length.\n");
		return -1;
	}
	

	//get data length
	uint32_t length = 0;
	ret = read(fd, &length, sizeof(length));
	if(sizeof(length) != ret)
	{
		if(-1 == ret)
		{
			perror("_checkHeadAndTail()");
		}
		else
		{
			printf("_checkHeadAndTail():");
		}
		printf("Failed to read length from data file.\n");
		return -1;
	}

	if((0 == length) || (length > 10*1024*1024))
	{
		printf("_checkHeadAndTail():");
		printf("Got invalid data length from data file.\n");
		return -1;		
	}

	//lseek to tail
	ret = lseek(fd, 1+sizeof(uint32_t)+sizeof(uint32_t)+length-1, SEEK_SET);
	if((1+sizeof(uint32_t)+sizeof(uint32_t)+length-1) != ret)
	{
		printf("_checkHeadAndTail():");
		printf("Failed to jump to the point of tail.\n");
		return -1;
	}


	//read tail
	//check tail
	unsigned char tail = 0;
	ret = read(fd, &tail, sizeof(tail));
	if(sizeof(tail) != ret)
	{
		printf("_checkHeadAndTail():");
		printf("Failed to read tail from data file.\n");
		return -1;
	}

	if(tail != 0x1a)
	{
		printf("_checkHeadAndTail():");
		printf("Invalid tail in data file.\n");
		return -1;
	}

	printf("PREPARE MOD: data file check pass.\n");
	return 0;

}


//file_no
//get
static int _getFileNo(int fd, uint32_t *fileNo)
{
	//check
	if((fd < 0) || (NULL == fileNo))
	{
		printf("_getFileNo():");
		printf("Got invalid inputs.\n");
		return -1;
	}

	//lseek
	int ret = -1;
	ret = lseek(fd, 0, SEEK_SET);
	if(0 != ret)
	{
		if(-1 == ret)
		{
			perror("_getFileNo():lseek()");
		}
		else
		{
			printf("_getFileNo():lseek()");
		}
		printf("Failed to reset pos in file.\n");
		*fileNo = 0;
		return -1;
	}

	//read file_no
	ret = lseek(fd, 1, SEEK_SET);
	if(1 != ret)
	{
		if(-1 == ret)
		{
			perror("_getFileNo():lseek()");
		}
		else
		{
			printf("_getFileNo():lseek():");
		}
		printf("Failed to jump to pos of file_no.\n");
		*fileNo = 0;
		return -1;
	}
	uint32_t getFileNo = 0;
	ret = read(fd ,&getFileNo, sizeof(getFileNo));
	if(sizeof(getFileNo) != ret)
	{
		if(-1 == ret)
		{
			perror("_getFileNo():read()");
		}
		else
		{
			printf("_getFileNo():read():");
		}
		printf("Failed to read fileNo in data file.\n");
		*fileNo = 0;
		return -1;
	}

	if(0 == getFileNo)
	{
		printf("_getFileNo():");
		printf("Got invalid value of fileNo.\n");
		*fileNo = 0;
		return -1;
	}

	//return
	*fileNo = getFileNo;
	return 0;

}


//check

//update



//gen keep.list
static int _genKeepList(int fd)
{
	//check
	if(fd < 0)
	{
		printf("_genKeepList():");
		printf("Got invalid fd.\n");
		return -1;
	}

	//lseek
	int ret = -1;
	int offset = 1+sizeof(uint32_t)+sizeof(uint32_t);	
	ret = lseek(fd, offset, SEEK_SET);
	if(offset != ret)
	{
		printf("_genKeepList():lseek():");
		printf("Failed to jump to the point at keep files data.\n");
		return -1;
	}

	//check head
	unsigned char head = 0;
	ret = read(fd, &head, sizeof(head));
	if(sizeof(head) != ret)
	{
		if(-1 == ret)
		{
			perror("_genKeepList():read()");
		}
		else
		{
			printf("_genKeepList():read():");
		}
		printf("Failed to read head of keep file list data.\n");
		return -1;
	}
	if(0xba != head)
	{
		printf("_genKeepList():");
		printf("Got wrong head of keep file data.\n");		
		return -1;
	}
	offset += sizeof(head);

	//read num
	uint32_t num = 0;
	ret = read(fd, &num, sizeof(num));
	if(sizeof(num) != ret)
	{
		if(-1 == ret)
		{
			perror("_genKeepList():read()");
		}
		else
		{
			printf("_genKeepList():read():");
		}
		printf("Failed to read num of keep filef.\n");
		return -1;
	}

	if(num > (50*1024*1024/37))
	{
		printf("_genKeepList():");
		printf("Got invalid num of keep file.\n");
		return -1;
	}

	printf("_genKeepList():");
	printf("Keep file num:%d\n", num);

	if(0 == num)
	{
		return 0;
	}
	offset += sizeof(num);

	//read length
	uint32_t length = 0;
	ret = read(fd, &length, sizeof(length));
	if(sizeof(length) != ret)
	{
		if(-1 != ret)
		{
			perror("_genKeepList():read()");
		}
		else
		{
			printf("_genKeepList():read():");
		}
		printf("Failed to read length of keep file list data.\n");
		return -1;
	}

	if((length > (50*1024*1024)) || (0 == length))
	{
		printf("_genKeepList():");
		printf("Got invalid value of length of keep file list data.\n");
		return -1;
	}
	offset += sizeof(length);
	
	//read data to file
	//filehead(1) + file_no(4) + length(4) + head(1) + num(4) + keeplength(4)
	ret = _readDataToFile(fd, offset, length, KEEPLISTFILEPATH);
	if(-1 == ret)
	{
		printf("_genKeepList():");
		printf("Failed to write data to keep.list.\n");
		return -1;
	}
		
	return 0;
}

struct data_file_st
{
	unsigned char head;
	uint32_t num;
	uint32_t length;
}__attribute__((__packed__));

struct tx_file_st
{
	char name[TXFILENAMELEN];
	uint32_t length;
}__attribute__((__packed__));

struct cmd_file_st
{
	char name[CMDFILENAMELEN];
	uint32_t length;
}__attribute__((__packed__));

//gen tx files
static int _genTxFiles(int fd)
{
	//check 
	if(fd < 0)
	{
		printf("_genTxFiles():");
		printf("Got invalid fd.\n");
		return -1;
	}
	
	int ret = -1;
	//lseek to start of file
	ret = lseek(fd, 0, SEEK_SET);
	if(ret != 0)
	{
		if(-1 == ret)
		{
			perror("_genTxFiles():lseek()");	
		}
		else
		{
			printf("_genTxFiles():lseek():");
		}
		printf("Failed to jump to the start of data file.\n");
		return -1;
	}	


	//read file data struct
	struct data_file_st dataFile;
	memset(&dataFile, '\0', sizeof(dataFile));
	ret = read(fd, &dataFile, sizeof(dataFile));
	if(sizeof(dataFile) != ret)
	{
		if(-1 == ret)
		{
			perror("_genTxFiles():read()");	
		}
		else
		{
			printf("_genTxFiles():read():");
		}
		printf("Failed to read file struct of data file.\n");
		return -1;
	}

	//check head
	if(((dataFile.head) != 0xa1) || ((dataFile.num) == 0) || ((dataFile.length) > 50*1024*1024))
	{
		printf("_genTxFile():");
		printf("Got invalid struct of data file.\n");
		return -1;
	}

	//read keep list data struct
	memset(&dataFile, '\0', sizeof(dataFile));
	ret = read(fd, &dataFile, sizeof(dataFile));
	if(sizeof(dataFile) != ret)
	{
		if(-1 == ret)
		{
			perror("_genTxFiles():read()");	
		}
		else
		{
			printf("_genTxFiles():read():");
		}
		printf("Failed to read struct of keep list data.\n");
		return -1;
	}

	//check head
	//check num
	//check length
	if(((dataFile.head) != 0xba) || ((dataFile.length) > 50*1024*1024))
	{
		printf("_genTxFile():");
		printf("Got invalid struct of keep list data.\n");
		return -1;
	}

	//jump to the head of tx files
	ret = lseek(fd, dataFile.length, SEEK_CUR);
	if(ret != (sizeof(struct data_file_st)*2 + dataFile.length))
	{
		if(-1 == ret)
		{
			perror("_genTxFiles():lseek()");	
		}
		else
		{
			printf("_genTxFiles():lseek():");
		}
		printf("Failed to jump to the head of tx file.\n");
		return -1;
	}	
	uint32_t start = ret; 

	

	//read tx files data struct 
	memset(&dataFile, '\0', sizeof(dataFile));
	ret = read(fd, &dataFile, sizeof(dataFile));
	if(sizeof(dataFile) != ret)
	{
		if(-1 == ret)
		{
			perror("_genTxFiles():read()");	
		}
		else
		{
			printf("_genTxFiles():read():");
		}
		printf("Failed to read struct of tx file data.\n");
		return -1;
	}


	//check head
	//check num
	//check length
	if(((dataFile.head) != 0xca) || ((dataFile.length) > 50*1024*1024) || ((dataFile.num == 0) && (dataFile.length != 0)) || ((dataFile.num != 0) && (dataFile.length == 0)))
	{
		printf("_genTxFile():");
		printf("Got invalid struct of keep list data.\n");
		return -1;
	}

	start += ret;

	printf("_genTxFiles():");
	printf("Tx files num:%d\n", dataFile.num);
	
	if(dataFile.num == 0)
	{
		return 0;
	}

	//loop
	//read name
	//check name
	//gen path
	//read length
	//gen tx files
	int i = 0;
	uint32_t numSum = dataFile.num;
	uint32_t readSum = dataFile.length;
	struct tx_file_st txFile;

	//path buffer
	char * pathBuf = NULL;
	pathBuf = malloc(strlen(TXDIR)+TXFILENAMELEN+1);
	if(NULL == pathBuf)
	{
		printf("_genTxFile():");
		printf("Failed to allcate buffer.\n");
		return -1;
	}
	memset(pathBuf, '\0', strlen(TXDIR)+TXFILENAMELEN+1);
	memcpy(pathBuf, TXDIR, strlen(TXDIR));

	while((numSum > 0) && (readSum > 0))
	{
		memset(&txFile, '\0', sizeof(txFile));
		ret = read(fd, &txFile, sizeof(txFile));
		if(sizeof(txFile) != ret)
		{
			if(-1 == ret)
			{
				perror("_genTxFiles():read()");	
			}
			else
			{
				printf("_genTxFiles():read():");
			}
			printf("Failed to read struct of one tx file.\n");
			free(pathBuf);
			return -1;
		}

		printf("txFile.length:%d\n", txFile.length);
		if(txFile.length > 50*1024*1024)
		{
			printf("_genTxFile():");
			printf("Got invalid length.\n");
			free(pathBuf);
			return -1;
		}

		start += ret;
		printf("DEBUG:START:%d\n", start);
		memcpy(pathBuf+strlen(TXDIR), (&txFile)->name, TXFILENAMELEN);

		printf("DEBUG:FD OFFSET:%d\n", (int)lseek(fd, 0, SEEK_CUR));
		ret = _readDataToFile(fd, start, txFile.length, pathBuf);
		if(-1 == ret)
		{
			printf("_genTxFile():");
			printf("Failed to generate one tx file data.\n");
			free(pathBuf);
			return -1;
		}
		printf("DEBUG:FD OFFSET:%d\n", (int)lseek(fd, 0, SEEK_CUR));
		
		start += txFile.length;
		printf("DEBUG:START:%d\n", start);
		numSum = numSum - 1;
		readSum = readSum - sizeof(txFile) - txFile.length;

	}

	free(pathBuf);
	return 0;
}


//gen cmd file
static int _genCmdFiles(int fd)
{
	//check 
	if(fd < 0)
	{
		printf("_genCmdFiles():");
		printf("Got invalid fd.\n");
		return -1;
	}
	
	int ret = -1;
	//lseek to start of file
	ret = lseek(fd, 0, SEEK_SET);
	if(ret != 0)
	{
		if(-1 == ret)
		{
			perror("_genCmdFiles():lseek()");	
		}
		else
		{
			printf("_genCmdFiles():lseek():");
		}
		printf("Failed to jump to the start of data file.\n");
		return -1;
	}	


	//read file data struct
	struct data_file_st dataFile;
	memset(&dataFile, '\0', sizeof(dataFile));
	ret = read(fd, &dataFile, sizeof(dataFile));
	if(sizeof(dataFile) != ret)
	{
		if(-1 == ret)
		{
			perror("_genCmdFiles():read()");	
		}
		else
		{
			printf("_genCmdFiles():read():");
		}
		printf("Failed to read file struct of data file.\n");
		return -1;
	}

	//check head
	if(((dataFile.head) != 0xa1) || ((dataFile.num) == 0) || ((dataFile.length) > 50*1024*1024))
	{
		printf("_genCmdFile():");
		printf("Got invalid struct of data file.\n");
		return -1;
	}

	//read keep list data struct
	memset(&dataFile, '\0', sizeof(dataFile));
	ret = read(fd, &dataFile, sizeof(dataFile));
	if(sizeof(dataFile) != ret)
	{
		if(-1 == ret)
		{
			perror("_genCmdFiles():read()");	
		}
		else
		{
			printf("_genCmdFiles():read():");
		}
		printf("Failed to read struct of keep list data.\n");
		return -1;
	}

	//check head
	//check num
	//check length
	if(((dataFile.head) != 0xba) || ((dataFile.length) > 50*1024*1024) || ((dataFile.num == 0) && (dataFile.length != 0)) || ((dataFile.num != 0) && (dataFile.length == 0)))
	{
		printf("_genTxFile():");
		printf("Got invalid struct of keep list data.\n");
		return -1;
	}

	//jump to the head of tx files
	ret = lseek(fd, dataFile.length, SEEK_CUR);
	if(ret != (sizeof(struct data_file_st)*2 + dataFile.length))
	{
		if(-1 == ret)
		{
			perror("_genCmdFiles():lseek()");	
		}
		else
		{
			printf("_genCmdFiles():lseek():");
		}
		printf("Failed to jump to the head of tx file.\n");
		return -1;
	}	
	uint32_t start = ret; 

	//read tx files data struct 
	memset(&dataFile, '\0', sizeof(dataFile));
	ret = read(fd, &dataFile, sizeof(dataFile));
	if(sizeof(dataFile) != ret)
	{
		if(-1 == ret)
		{
			perror("_genTxFiles():read()");	
		}
		else
		{
			printf("_genTxFiles():read():");
		}
		printf("Failed to read struct of tx file data.\n");
		return -1;
	}


	//check head
	//check num
	//check length
	if(((dataFile.head) != 0xca) || ((dataFile.length) > 50*1024*1024) || ((dataFile.num == 0) && (dataFile.length != 0)) || ((dataFile.num != 0) && (dataFile.length == 0)))
	{
		printf("_genTxFile():");
		printf("Got invalid struct of tx file data.\n");
		return -1;
	}

	start += ret;

	//jump to the head of cmd files
	ret = lseek(fd, dataFile.length, SEEK_CUR);
	if(ret != (start+(dataFile.length)))
	{
		if(-1 == ret)
		{
			perror("_genCmdFiles():lseek()");	
		}
		else
		{
			printf("_genCmdFiles():lseek():");
		}
		printf("Failed to jump to the head of tx file.\n");
		return -1;
	}	

	start = ret;

	//read cmd files data struct 
	memset(&dataFile, '\0', sizeof(dataFile));
	ret = read(fd, &dataFile, sizeof(dataFile));
	if(sizeof(dataFile) != ret)
	{
		if(-1 == ret)
		{
			perror("_genCmdFiles():read()");	
		}
		else
		{
			printf("_genCmdFiles():read():");
		}
		printf("Failed to read struct of cmd file data.\n");
		return -1;
	}

	//check head
	//check num
	//check length
	if(((dataFile.head) != 0xda) || ((dataFile.length) > 50*1024*1024) || ((dataFile.num == 0) && (dataFile.length != 0)) || ((dataFile.num != 0) && (dataFile.length == 0)))
	{
		printf("HEAD:%02x\n", dataFile.head);
		printf("LENGTH:%d\n", dataFile.length);
		printf("NUM:%d\n", dataFile.num);
		printf("_genCmdFile():");
		printf("Got invalid struct of cmd file data.\n");
		return -1;
	}

	printf("_genCmdFiles():");
	printf("Cmd files num:%d\n", dataFile.num);
	
	if(dataFile.num == 0)
	{
		return 0;
	}

	start += ret;

	//loop
	//read name
	//check name
	//gen path
	//read length
	//gen tx files
	int i = 0;
	uint32_t numSum = dataFile.num;
	uint32_t readSum = dataFile.length;
	struct cmd_file_st cmdFile;

	//path buffer
	char * pathBuf = NULL;
	pathBuf = malloc(strlen(SERVEREPUBDIR)+CMDFILENAMELEN+1);
	if(NULL == pathBuf)
	{
		printf("_genCmdFile():");
		printf("Failed to allcate buffer.\n");
		return -1;
	}
	memset(pathBuf, '\0', strlen(SERVEREPUBDIR)+CMDFILENAMELEN+1);
	memcpy(pathBuf, SERVEREPUBDIR, strlen(SERVEREPUBDIR));

	while((numSum > 0) && (readSum > 0))
	{
		memset(&cmdFile, '\0', sizeof(cmdFile));
		ret = read(fd, &cmdFile, sizeof(cmdFile));
		if(sizeof(cmdFile) != ret)
		{
			if(-1 == ret)
			{
				perror("_genCmdFiles():read()");	
			}
			else
			{
				printf("_genCmdFiles():read():");
			}
			printf("Failed to read struct of one cmd file.\n");
			return -1;
		}
		//printf("DEBUF:CMDFILE.NAME:%s\n", cmdFile.name);
		printf("DEBUG:CMDFILE.LENGTH:%d\n", cmdFile.length);
		if(cmdFile.length > 50*1024*1024)
		{
			printf("_genCmdFile():");
			printf("Got invalid length.\n");
			return -1;
		}

		start += ret;
		memcpy(pathBuf+strlen(SERVEREPUBDIR), (&cmdFile)->name, CMDFILENAMELEN);

		ret = _readDataToFile(fd, start, cmdFile.length, pathBuf);
		if(-1 == ret)
		{
			printf("_genCmdFile():");
			printf("Failed to create one cmd file.\n");
			return -1;
		}

		start += cmdFile.length;
		numSum = numSum - 1;
		readSum = readSum - sizeof(cmdFile) - cmdFile.length;

	}

	return 0;
}


int prepare_checkDataFile()
{
	int ret = -1;
	ret = _countDataFile(UPLOADDIR);
	if(1 == ret)
	{
		return 0;
	}
	else if(-1 == ret)
	{
		printf("prepare_checkDataFile():");
		printf("Failed to perform searching ops in upload directory.\n");
		return -1;
	}
	else if(0 == ret)
	{
		printf("prepare_checkDataFile():");
		printf("No xxxx.data file found in upload directory.\n");
		return -1;
	}
	else if(ret > 1)
	{
		printf("prepare_checkDataFile():");
		printf("More than one xxxx.data file found in upload directory.\n");
		return -1;
	}
}


int prepare_processDataFile()
{
	//find xxx.data
	int ret = -1;
	char *retFileName = NULL;
	uint32_t retFileNameLen = 0;
	retFileName = _findDataFile(UPLOADDIR, &retFileNameLen);
	if((NULL == retFileName) || (0 == retFileNameLen))
	{
		printf("prepare_precessDataFile():");
		printf("No data file found in upload directory.\n");
		if(NULL != retFileName)
		{
			free(retFileName);
		}
		return -1;
	}

	printf("DATAFILENAME:%s\n", retFileName);

	//check check file
	//get check file name
	char checkFileName[42];
	memset(checkFileName, '\0', 42);
	ret = _getCheckFileName(retFileName, strlen(retFileName), checkFileName, 41);
	if(-1 == ret)
	{
		printf("prepare_processDataFile():");
		printf("Failed to get check file name.\n");
		free(retFileName);
		return -1;
	}
#if 1
	//get md5
	char md5[33];
	memset(md5, '\0', 33);
	printf("%s\n", checkFileName);
	ret = _getMd5(checkFileName, strlen(checkFileName), md5, 32);
	if(-1 == ret)
	{
		printf("prepare_processDataFile():");
		printf("Failed to get md5.\n");
		free(retFileName);
		return -1;
	}
	printf("DEBUG:AFTER GET MD5\n");

	//gen md5
	char md5Gen[33];
	memset(md5Gen, '\0', 33);
	ret = _genMd5(retFileName, strlen(retFileName), md5Gen, 32);
	if(-1 == ret)
	{
		printf("prepare_processDataFile():");
		printf("Failed to generate md5.\n");
		free(retFileName);
		return -1;
	}
	printf("DEBUG:AFTER GEN MD5\n");

	//check md5
	if(strncmp(md5, md5Gen, 32) != 0)
	{
		printf("prepare_processDataFile():");
		printf("Md5 check failed.\n");
		free(retFileName);
		return -1;
	}

	//get sig
	char *sig = NULL;
	uint32_t sigLen = 0;
	sig = _getSig(checkFileName, strlen(checkFileName), &sigLen);
	if((NULL == sig) || (0 == sigLen))
	{
		printf("prepare_processDataFile():");
		printf("Failed to get sig.\n");
		free(retFileName);
		return -1;
	}

	//get spc epub
	//error free(sig)
	char *spcEpub = NULL;
	uint32_t spcEpubLen = 0;
	spcEpub = _getSpcEpub(&spcEpubLen);
	if((NULL == spcEpub) || (0 == spcEpubLen))
	{
		printf("prepare_processDataFile():");
		printf("Failed to get spc epub.\n");
		free(retFileName);
		free(sig);
		return -1;
	}

	//check sig
	//free sig
	ret = _checkSig(spcEpub, spcEpubLen, sig, sigLen, md5, 32);
	if(ret == -1)
	{
		printf("prepare_processDataFile():");
		printf("Failed to get spc epub.\n");
		free(retFileName);
		free(sig);
		free(spcEpub);
		return -1;
		
	}

	free(sig);
	free(spcEpub);
	printf("prepare_precessDataFile():");
	printf("Checksum pass.\n");

#endif

	//mv xxx.data to temp.data
	//free retFileName

	printf("retFileNameLen:%d\n", retFileNameLen);
	ret = _mvDataFileToTempDir(retFileName, retFileNameLen);
	if(-1 == ret)
	{
		printf("prepare_processDataFile():");
		printf("Failed to move xxx.data to prepare/temp/temp.data\n");
		free(retFileName);
		return -1;
	}
	_freeDataFileName(retFileName);

	ret = _checkTempDataInTempDir();
	if(-1 == ret)
	{
		printf("prepare_processDataFile():");
		printf("No temp.data found in prepare/temp directory.\n");
		return -1;
	}

	//open temp.data
	int fd = -1;
	fd = open(TEMPDATAFILEPATH, O_RDONLY);
	if(fd < 0)
	{
		printf("prepare_processDataFile():");
		printf("Failed to open prepare/temp/temp.data.\n");
		return -1;
	}

	//get file_no
	//check file_no
	uint32_t file_no = 0;
	ret = _getFileNo(fd, &file_no);
	if(-1 == ret)
	{
		printf("prepare_processDataFile():");
		printf("Failed to get file_no in temp.data.\n");
		close(fd);
		return -1;
		
	}
	printf("GET FILE_NO:%d\n", file_no);

	//check head and tail
	ret = _checkHeadAndTail(fd);
	if(-1 == ret)
	{
		printf("prepare_processDataFile():");
		printf("Head and tail check failed.\n");
		close(fd);
		return -1;
		
	}


	//generate Keep.List file
	ret = _genKeepList(fd);
	if(-1 == ret)
	{
		printf("prepare_processDataFile():");
		printf("Failed to generate /trash/keep.list.\n");
		close(fd);
		return -1;
		
	}

		
	//generate tx files
	ret = _genTxFiles(fd);
	if(-1 == ret)
	{
		printf("prepare_processDataFile():");
		printf("Failed to generate tx files in /transaction/txfiles.\n");
		close(fd);
		return -1;
		
	}

	
	//generate cmd files
	ret = _genCmdFiles(fd);
	if(-1 == ret)
	{
		printf("prepare_processDataFile():");
		printf("Failed to generate cmd files in /server/epub/.\n");
		close(fd);
		return -1;
		
	}



	//close temp.data
	close(fd);

	return 0;
}

#if  0
int main(void)
{
	int ret = -1;
	ret = prepare_checkDataFile();
	if(-1 == ret)
		printf("CHECKFAILED.\n");

	ret = prepare_processDataFile();
	if(-1 == ret)
		printf("PROCESSFAILED.\n");


	return 0;
}

#endif
