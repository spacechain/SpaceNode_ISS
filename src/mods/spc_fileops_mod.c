#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>

#include "spc_fileops_mod.h"
//#include "spc_log_mod.h"


#define TXFILETAIL	".tx"
#define RESFILETAIL	".res"
#define TXFILEIDLEN	5
#define TXFILETIMELEN	20
#define TXFILENAMELEN	29

int _getTxFileNum(const char *path, uint32_t *num, int isList)
{
	//open dir
	DIR *dir = NULL;
	dir = opendir(path);
	if(NULL == dir)
	{
		printf("_getTxFileNum():opendir():Failed to open directory of given path,%s\n", strerror(errno));
		return -1;
	}

	//while read 
	struct dirent *pDe;
	uint32_t count = 0;
	//int index = -1;
	
	if(isList)
	{
		printf("Tx file list:\n");
	}	

	while((pDe = readdir(dir)) != NULL)
	{
		if((pDe->d_type == DT_REG) && (strncmp(TXFILETAIL, \
			(pDe->d_name)+(strlen(pDe->d_name) - 3), \
				strlen(TXFILETAIL)) == 0) && ((strlen(pDe->d_name) - 3) != 0))
		{
			if(isList)
			{
				printf("%s\n", pDe->d_name);
			}
			//spc_log(INFO, "%s\n", pDe->d_name);
			count++;
		}
	}
	
	//return num
	//spc_log(INFO, "_getTxFileNum():count = %d\n", count);
	*num = count;
	
	//close dir stream
	int ret = -1;
	ret = closedir(dir);
	if(ret != 0)
	{
		//printf("_getTxFileNum():closedir():Failed to close directory of given path,%s\n", strerror(errno));
		//spc_log(ERROR, "_getTxFileNum():closedir():Failed to close directory of given path,%s\n", strerror(errno));
		return -1;
	}	

	return 0;
}


//
int fileops_getTxFileNum(const char *txDir, uint32_t *pNum)
{
	//check
	if((NULL == txDir) || (access(txDir, F_OK) != 0) || (NULL == pNum))
	{
		printf("fileops_getTxFileNum(): Got invalid input.\n");		
		//spc_log(ERROR, "fileops_getTxFileNum(): Got invalid input.\n");		
		return -1;
	}

	return _getTxFileNum(txDir, pNum, 0);
}


//remove
int _rmTxFile(const char *path)
{
	//check tail null and so on
	if((NULL == path) || (access(path, F_OK) != 0) || (strncmp(TXFILETAIL, path+(strlen(path)-strlen(TXFILETAIL)), strlen(TXFILETAIL)) != 0))
	{
		printf("_removeTxFile():");
		printf("Got invalid input.\n");
		return -1;
	}

	//remove tx file path given
	return remove(path);
}


//get next tx file path
int _getNextTxFile(const char *txPath, char *path, uint32_t len)
{
	//check input
	if((NULL == txPath) || (access(txPath, F_OK) != 0) || (NULL == path) || (len < TXFILENAMELEN + strlen(txPath) + 5))
	{
		printf("_getNextTxFile():");
		printf("Get invalid input.\n");
		return -1;
	}

	

	//get minimal time_t in the tx file directory
	DIR *dir = NULL;
	dir = opendir(txPath);
	if(NULL == dir)
	{
		perror("_getNextTxFile():opendir()");
		printf("Failed to open tx file dir of given path.\n");
		return -1;
	}
	

	struct dirent *pDe = NULL;
	char fileName[TXFILENAMELEN + 10] = {'\0'};
	char cFileTime[TXFILETIMELEN+1] = {'\0'};
	long long timeMin = 0;
	long long  timeCur = 0;
	int timeIndex = 0;
	while((pDe = readdir(dir)) != NULL)
	{
		if((pDe->d_type == DT_REG) && (strncmp(TXFILETAIL, \
			(pDe->d_name)+(strlen(pDe->d_name) - 3), \
				strlen(TXFILETAIL)) == 0) && ((strlen(pDe->d_name) - 3) != 0))
		{
			memcpy(cFileTime, (pDe->d_name) + TXFILEIDLEN + 1, TXFILETIMELEN);
			for(timeIndex = 0; timeIndex < TXFILETIMELEN; timeIndex++)
			{
				printf("%c", cFileTime[timeIndex]);
			}
			printf("\n");
			timeCur = atoll(cFileTime);
			//printf("\ntimeMin = %lld\n", timeMin);
			//printf("\ntimeCur = %lld\n", timeCur);
			if(timeMin == 0)
			{
				timeMin = timeCur;
				memcpy(fileName, pDe->d_name, strlen(pDe->d_name));
				
			}
			else
			{
				if(timeCur < timeMin)
				{
					memset(fileName, '\0', TXFILENAMELEN+10);
					memcpy(fileName, pDe->d_name, strlen(pDe->d_name));
					//printf("fileName:%s\n", fileName);
					timeMin = timeCur;
				}
			}
		
		}		
	}

	int resRet = -1;
	if(strlen(fileName) != 0)
	{
		//return path
		memset(path, '\0', len);
		strncat(path, txPath, strlen(txPath));
		strncat(path, fileName, TXFILENAMELEN);
		resRet = 0;
	}
	else
	{
		memset(path, '\0', len);
		resRet = -1;
	}	

	//close dir
	int dirRet = -1;
	dirRet = closedir(dir);
	if(dirRet == -1)
	{
		perror("_getNextTxFile():closedir()");
		printf("Failed to close directory.\n");
	}

	if((dirRet == -1) || (resRet == -1))
	{
		return -1;
	}
	else
	{
		return 0;
	}

}


//
int fileops_getNextTxFile(const char *txDirPath, char *txFilePath, uint32_t len)
{
	return _getNextTxFile(txDirPath, txFilePath, len);
} 


//
int fileops_genResFilePath(const char *resDirPath, const char *txFilePath, char *resFilePath, uint32_t len)
{
	//check
	if((NULL == resDirPath) || (access(resDirPath, F_OK) != 0) || \
		(NULL == txFilePath) || (access(txFilePath, F_OK) != 0) || \
			(len < TXFILENAMELEN + strlen(resDirPath) + 5))
	{
		printf("fileops_genResFilePath():");
		printf("Got invalid inputs.\n");
		return 1;
	}

	//get tx file name 
	char txFileName[TXFILENAMELEN+1] = {'\0'};
	memcpy(txFileName, txFilePath + strlen(txFilePath) - TXFILENAMELEN, TXFILENAMELEN);
	if(strncmp(TXFILETAIL, txFileName + strlen(txFileName) - strlen(TXFILETAIL), strlen(TXFILETAIL)) != 0)
	{
		printf("fileops_genResFilePath():strcmp():");
		printf("Failed to get tx file name: %s", txFileName);
		return -1;
	}
	
	uint32_t i;
	for(i = strlen(txFileName) - strlen(TXFILETAIL); i < strlen(txFileName); i++)
	{
		txFileName[i] = '\0';
	}

	
	//replace .tx with .res
	strncat(txFileName, RESFILETAIL, strlen(RESFILETAIL));
	
	//strcat after res file dir 
	//if res dir last != '\'
	memset(resFilePath, '\0', len);
	strncat(resFilePath, resDirPath, strlen(resDirPath));

	if(resDirPath[strlen(resDirPath) - 1] != '/')
		strncat(resFilePath, "/", strlen("/"));

	strncat(resFilePath, txFileName, strlen(txFileName));

	return 0;
}

//
int fileops_genDownloadFilePath(const char *downloadDirPath, const char *txFilePath, char *downloadFilePath, uint32_t len)
{
	//check
	if((NULL == downloadDirPath) || (access(downloadDirPath, F_OK) != 0) || \
		(NULL == txFilePath) || (access(txFilePath, F_OK) != 0) || \
			(len < TXFILENAMELEN + strlen(downloadDirPath) + 5))
	{
		printf("fileops_genDownloadFilePath():");
		printf("Got invalid inputs.\n");
		return 1;
	}

	//get tx file name 
	char txFileName[TXFILENAMELEN+1] = {'\0'};
	memset(txFileName, '\0', TXFILENAMELEN+1);
	memcpy(txFileName, txFilePath + strlen(txFilePath) - TXFILENAMELEN, TXFILENAMELEN);
	if(strncmp(TXFILETAIL, txFileName + strlen(txFileName) - strlen(TXFILETAIL), strlen(TXFILETAIL)) != 0)
	{
		printf("fileops_genDownloadFilePath():strcmp():");
		printf("Failed to get tx file name: %s", txFileName);
		return -1;
	}
	
	uint32_t i;
	for(i = strlen(txFileName) - strlen(TXFILETAIL); i < strlen(txFileName); i++)
	{
		txFileName[i] = '\0';
	}

	
	//replace .tx with .res
	strncat(txFileName, RESFILETAIL, strlen(RESFILETAIL));
	
	//strcat after res file dir 
	//if res dir last != '\'
	memset(downloadFilePath, '\0', len);
	strncat(downloadFilePath, downloadDirPath, strlen(downloadDirPath));

	if(downloadDirPath[strlen(downloadDirPath) - 1] != '/')
		strncat(downloadFilePath, "/", strlen("/"));

	strncat(downloadFilePath, txFileName, strlen(txFileName));

	return 0;
}

//
int fileops_rmTxFile(const char *path)
{
	return _rmTxFile(path);
}


#if 0
//test main
int main(int argc, char **argv)
{
	system("touch 00001-00000000000000000009.tx");		
	system("touch 00005-00000000000000000007.tx");		
	system("touch 00009-00000000000000000015.tx");		
	system("touch 00007-00000000000000000009.tx");		
	system("touch 00008-00000000000000000009.tx");		

	uint32_t num;
	//char name[30] = {'\0'};
	char path[40] = {'\0'};
	char resPath[40] = {'\0'};
	_getTxFileNum("./", &num);
	printf("count: %d\n", num);
	fileops_getNextTxFile("./", path, 40);
	printf("%s\n", path);
	//strcat(path, "./");
	//strcat(path, name);
	//fileops_rmTxFile(path);
	fileops_genResFilePath("./", path, resPath, 40);
	printf("%s\n", resPath);

	return -1;
}

#endif
