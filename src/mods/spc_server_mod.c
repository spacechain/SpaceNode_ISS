#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <dirent.h>

#include "spc_server_mod.h"


//test the existence of server.list
static int _checkServerList(void)
{
	return access(SERVERLISTPATH, F_OK);	
}


//create server.list and init it
static int _initServerList(void)
{
#if 0
	//check the existence of server.list file
	//but i don't want it to break the struct 
	//functions on same level don't call each other
	//like date with cousin, ew!;
	int ret = -1;
	ret = _checkServerList(); 
	if(ret == 0)
	{
		printf("_initServerList(): ");
		printf("Server.list file exists, no need to create it asshole.\n");
		return -1;
	}	
#endif

	//init info0
	struct server_info info0;
	memset(&info0, '\0', sizeof(struct server_info));
	info0.server_id = 1;
	info0.server_start = 0;
	info0.server_txs = 0;
	info0.server_counter = 1;
	
	//open and create file
	int sFd = open(SERVERLISTPATH, O_WRONLY | O_CREAT, 0777);
	if(sFd < 0)
	{
		perror("_initServerList():open()");
		printf("Failed to open server.list file.\n");
		return -1;
	}
	
	//write info0 into file
	ssize_t nBytes = write(sFd, &info0, sizeof(struct server_info));
	if(nBytes != sizeof(struct server_info))
	{
		perror("_initServerList():write()");
		printf("Failed to write info0 into server.list file.\n");
		close(sFd);
		return -1;
	}

	//close file
	close(sFd);

	return 0;
	
}


static int _getServerInfo(struct server_info *pInfo, uint32_t serverId)
{
	//check params
	if(NULL == pInfo)
	{
		printf("_getServerInfo():");
		printf("Get bad pointer of given buf.\n");
		return -1;
	}


	//open server list file
	int sFd = -1;
	sFd = open(SERVERLISTPATH, O_RDONLY);
	if(sFd < 0)
	{
		perror("_getServerInfo():open()");
		printf("Failed to open server.list file.\n");
		return -1;
	}	

	//move to the position of the file
	int ret = -1;
	ret = lseek(sFd, serverId*sizeof(struct server_info), SEEK_SET);
	if(ret != serverId*sizeof(struct server_info))
	{
		perror("_getServerInfo():lseek()");
		printf("Failed to move to right file position.\n");
		close(sFd);
		return -1;
	}
	ret = -1;
	
	//read info from file
	ret = read(sFd, pInfo, sizeof(struct server_info));
	if(ret != sizeof(struct server_info))
	{
		perror("_getServerInfo():read()");
		printf("Failed to read info from file.\n");
		close(sFd);
		return -1;
	}

	//close file	
	close(sFd);

	return 0;
}

static int _setServerInfo(struct server_info *pInfo, uint8_t isInfo0)
{
	//check pointer
	if(NULL == pInfo)
	{
		printf("_setServerInfo():");
		printf("Get bad pointer.\n");
		return -1;
	}

	//open file
	int sFd = -1;
	sFd = open(SERVERLISTPATH, O_WRONLY);
	if(sFd < 0)
	{
		perror("_setServerInfo():open()");
		printf("Failed to open server.list file.\n");
		return -1;
	}

	//move to right file position 
	int ret = -1;
	int offset = -1;
	if(isInfo0 == 0)
	{
		offset = ((pInfo->server_id))*(sizeof(struct server_info));
	}
	else
	{
		offset = 0;
	}
	
	ret = lseek(sFd, offset, SEEK_SET);
	if(ret != offset)
	{
		perror("_setServerInfo():lseek()");
		printf("Failed to move to pos:%d\n", offset);
		close(sFd);
		return -1;
	}

	//write info into file
	ret = write(sFd, pInfo, sizeof(struct server_info));
	if(ret != sizeof(struct server_info))
	{
		perror("_setServerInfo():write()");
		printf("Failed to write info into server.list file.\n");
		close(sFd);
		return -1;
	}

	//close file
	close(sFd);
	
	return 0;
}

static int _makeDir(char *pPath)
{
	//add '/' tail
	int len = -1;
	len = strlen(pPath);
	char *pathBuf = NULL;
	pathBuf = malloc(len + 50);
	if(pathBuf == NULL)
	{
		printf("_makeDir():");
		printf("Failed to allocate buffer for path.\n");
		return -1;
	}
	memset(pathBuf, '\0', len+50);
	memcpy(pathBuf, pPath, strlen(pPath));

	if('/' != pathBuf[len - 1])
	{
		strcat(pathBuf, "/");
		len++;
	}

	//make dir for every level
	int i;
	for(i = 1; i < len; i++)
	{
		if('/' == pathBuf[i])
		{
			pathBuf[i] = '\0';
			if(access(pathBuf, F_OK) != 0)
			{
				if(mkdir(pathBuf, 0777) == -1)
				{
					perror("_makeDir():mkdir()");
					printf("Failed to make directory.\n");
					return -1;
				}
			}
			pathBuf[i] = '/'; 
		}
	}
	
	return 0;
}

static int _makeFileDir(void)
{
	if((access(TXFILEDIR, F_OK) == 0) && (access(RESFILEDIR, F_OK) == 0))
	{
		return 0;
	}

	printf("DEBUG:_MAKEFILEDIR() AFTER CHECK\n");

	int ret = -1;
	ret = _makeDir(TXFILEDIR); 
	if(ret != 0)
	{
		printf("_makeFileDir():_makeDir():");
		printf("Failed to make tx files directory.\n");	
		return -1;
	}	
	
	printf("DEBUG:MAKEFILEDIR AFTER TX\n");

	ret = _makeDir(RESFILEDIR); 
	if(ret != 0)
	{
		printf("_makeFileDir():_makeDir():");
		printf("Failed to make res files directory.\n");	
		return -1;
	}	

	return 0;
}


static int _makeServerDir(uint32_t serverId)
{
	if(serverId <= 0)
	{
		printf("_makeServerDir():");
		printf("Got wrong id.\n");
		return -1;
	}

	char *pPath = NULL;	
	
	//make server wallet dir
	//gen full path 
	pPath = malloc(strlen(WALLETDIRHEADER)+IDLENGTHMAX+10);
	if(NULL == pPath)
	{
		perror("_makeServerDir()");
		printf("Failed to malloc mem for path buf.\n");
		return -1;
	}
	
	memset(pPath, '\0', strlen(WALLETDIRHEADER)+IDLENGTHMAX+10);
	memcpy(pPath, WALLETDIRHEADER, strlen(WALLETDIRHEADER));
	sprintf(pPath+strlen(WALLETDIRHEADER), "%d", serverId);

	//make dir for server wallet
	int ret = -1;
	ret = _makeDir(pPath);
	if(ret < 0)
	{	
		free(pPath);
		printf("_makeServerDir():");
		printf("Failed to make dir for server:%d.\n", serverId);
		return -1;
	}
	free(pPath);

	//make server certificate file dir 
	//gen full path 
	pPath = malloc(strlen(CERTIDIRHEADER)+IDLENGTHMAX+10);
	if(NULL == pPath)
	{
		perror("_makeServerDir()");
		printf("Failed to malloc mem for path buf.\n");
		return -1;
	}
	
	memset(pPath, '\0', strlen(CERTIDIRHEADER)+IDLENGTHMAX+10);
	memcpy(pPath, CERTIDIRHEADER, strlen(CERTIDIRHEADER));
	sprintf(pPath+strlen(CERTIDIRHEADER), "%d", serverId);

	//make dir for server wallet
	ret = -1;
	ret = _makeDir(pPath);
	if(ret < 0)
	{	
		free(pPath);
		printf("_makeServerDir():");
		printf("Failed to make dir for server:%d.\n", serverId);
		return -1;
	}
	free(pPath);

	return 0;
}


static void _printServerInfo(struct server_info *pInfo)
{
	//check pointer
	if(NULL == pInfo)
	{
		printf("_printfServerInfo():");
		printf("Get bad pointer.\n");
		return;
	}

	//print all the things
	printf("server_id: %d\n", pInfo->server_id);
	printf("server_start: %d\n", pInfo->server_start);
	printf("server_txs: %ld\n", pInfo->server_txs);
	printf("server_counter: %d\n", pInfo->server_counter);
	
	return;
}


//
int server_getNewInfo(struct server_info *pInfo)
{
	//check input
	if(NULL == pInfo)
	{
		printf("server_getNewInfo():");
		printf("Get bad server_info address.\n");
		return -1;
	}
	
	//check and create server.file
	int ret = -1;
	ret = _checkServerList();
	if(-1 == ret)
	{
		printf("Server_getNewInfo():");
		printf("server.list file check: no.\n");
		printf("initing server.list file.\n");
		ret = _initServerList();
		if(0 == ret)
		{
			printf("Init server.list file done.\n");
		}
		else
		{
			printf("Failed to init server.list.\n");
		}
	}

	//get info0
	struct server_info info0;
	memset(&info0, '\0', sizeof(struct server_info));

	ret = -1;
	ret = _getServerInfo(&info0, 0);
	if(0 != ret)
	{
		printf("server_getNewInfo():_getServerInfo():");
		printf("Failed to get info0.\n");
		return -1;
	}

	//copy info0 to given buf
	memcpy(pInfo, &info0, sizeof(struct server_info));

	//update info0
	(info0.server_id)++;
	
	//set info0
	ret = _setServerInfo(&info0, 1);
	if(ret != 0)
	{
		printf("server_getNewInfo():_setServerInfo():");
		printf("Failed to set new info0 back.\n");
		return -1;
	}

	return 0;
}


//
int server_initNewServer(struct server_info *pInfo) 
{
	//check input
	if(NULL == pInfo)
	{
		printf("server_initNewServer():");
		printf("Get wrong server_info struct pointer.\n");
		return -1;
	}

	printf("DEBUG:BEFORE MAKE DIR\n");
	
	//make server dir
	int ret = -1;
	uint32_t serverId = 0;
	serverId = pInfo->server_id;

	printf("DEBUG: 1\n");	
	ret = _makeFileDir();
	if(0 != ret)
	{
		printf("server_initNewServer():_makeFileDir():");
		printf("Failed to create file directory.\n");
		return -1;
	}

	printf("DEBUG: 2\n");	

	ret = _makeServerDir(serverId);
	if(0 != ret)
	{
		printf("server_initNewServer():_makeServerDir():");
		printf("Failed to create server directory.\n");
		return -1;
	}

	printf("DEBUG:MAKE DIR DONE\n");

	//regist server info	
	ret = _setServerInfo(pInfo, 0);
	if(-1 == ret)
	{
		printf("server_initNewServer():_setServerInfo():");
		printf("Failed to regist server info into list.\n");
		return -1;
	}

	return 0;
}

static int _validateId(uint32_t serverId)
{
	struct server_info info0;
	_getServerInfo(&info0, 0);
	if((serverId > 0) && (serverId < info0.server_id))
	{
		return 0;
	}
	else
	{
		return -1;
	}	
}

int server_getServerInfo(struct server_info *pInfo, uint32_t serverId)
{
	//check null,id valid or not
	int ret = -1;
	ret = _validateId(serverId);
	if((0 != ret) || (NULL == pInfo))
	{
		printf("server_getServerInfo():");
		printf("Got invalid server id.\n");
		return -1;
	}

	//get info
	ret = -1;
	ret = _getServerInfo(pInfo, serverId);
	if(0 != ret)
	{
		printf("server_getServerInfo():_getServerInfo():");
		printf("Failed to get server_info struct from server list file.\n ");
		return -1;
	}

	return 0;
}

int server_setServerInfo(struct server_info *pInfo)
{
	//check null,serverId,secret,txs_num
	if((NULL == pInfo) || ((_validateId(pInfo->server_id)) != 0))
	{
		printf("server_setServerInfo():");
		printf("Get invalid input.\n");
		return -1;	
	}

	//set info into file
	int ret = -1;
	ret = _setServerInfo(pInfo, 0);
	if(ret != 0)
	{
		printf("server_setServerInfo():_setServerInfo():");
		printf("Failed to write server_info struct into server.list.\n");
		return -1;
	}

	return 0;
}

static uint32_t _serverNum(void)
{
	struct server_info info0;
	_getServerInfo(&info0, 0);
	uint32_t id = info0.server_id;
	return (id-1);
}

void server_showServerList(void)
{
	//printf all the shit
	uint32_t serverNum = _serverNum();
	
	struct server_info info;
	uint32_t i = 0;
	for(i = 1; i <= serverNum; i++)
	{
		_getServerInfo(&info, i);
		_printServerInfo(&info);
	}	
	
}


static char *_getCmdFileName(char *dirPath, uint32_t *retNameLen)
{
	//check inputs
	if((NULL == dirPath) || (NULL == retNameLen))
	{
		printf("_getCmdFileName():");
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
		perror("_getCmdFileName():opendir()");
		printf("Failed to open upload directory.\n");
		*retNameLen = 0;
		return NULL;
	}	
	//printf("OPENDIRDONE.\n");
	
	//read dir 
	struct dirent *pD_st = NULL;

	pD_st = readdir(dUl);
	char *retFileName = NULL;
	while(NULL != pD_st)
	{
		if(strncmp(((pD_st->d_name)+strlen(pD_st->d_name)-strlen(USEREPUBFILETAIL)), USEREPUBFILETAIL, strlen(USEREPUBFILETAIL)) == 0)
		{
			retFileName = malloc(strlen(pD_st->d_name) + 1);
			if(NULL == retFileName)
			{
				if(-1 == closedir(dUl))
				{
					perror("_getCmdFileName():readdir():closedir()");
					printf("Failed to excute closedir().\n");
				}
				*retNameLen = 0;
				return NULL;
			}

			memset(retFileName, '\0', strlen(pD_st->d_name) + 1);
			memcpy(retFileName, (pD_st->d_name), strlen(pD_st->d_name));
			break;

		}	
		
		memset(pD_st->d_name, '\0', strlen(pD_st->d_name));
		pD_st = readdir(dUl);
	}
	//printf("READDIRDONE.\n");	


	//close dir
	if(-1 == closedir(dUl))
	{
		perror("_getCmdFileName():readdir():closedir()");
		printf("Failed to excute closedir().\n");
		*retNameLen = 0;
		return NULL;
	}
	 
	//return
	*retNameLen = strlen(pD_st->d_name);
	return retFileName; 
}


static void _freeRetFileName(char *pName)
{
	free(pName);
	return;
}


static int _mvCmdFileToTempDir(char *fileName, uint32_t fileNameLen)
{
	//check 
	if((NULL == fileName) || (0 == fileNameLen))
	{
		printf("_mvCmdFileToTempDir():");
		printf("Got invalid inputs.\n");
		return -1;
	}

	//gen old path and new path
	void *pOldPath = NULL;
	pOldPath = malloc(strlen(EPUBDIR)+fileNameLen); 
	if(NULL == pOldPath)
	{	
		printf("_mvCmdFileToTempDir():");
		printf("Failed to allocate memory for cmd file old path.\n");
		return -1;
	}
	memset(pOldPath, '\0', strlen(EPUBDIR)+fileNameLen);
	memcpy(pOldPath, EPUBDIR, strlen(EPUBDIR));
	strncat(pOldPath, fileName, fileNameLen);
	printf("OLDPATH:%s.\n", (char *)pOldPath);

	void *pNewPath = NULL;
	pNewPath = malloc(strlen(USEREPUBFILEPATH)+1); 
	if(NULL == pNewPath)
	{	
		printf("_mvCmdFileToTempDir():");
		printf("Failed to allocate memory for cmd file new path.\n");
		free(pOldPath);
		return -1;
	}
	memset(pNewPath, '\0', strlen(USEREPUBFILEPATH)+1);
	memcpy(pNewPath, USEREPUBFILEPATH, strlen(USEREPUBFILEPATH));
	printf("NEWPATH:%s.\n", (char *)pNewPath);

	//move
	if(-1 == rename(pOldPath, pNewPath))
	{
		perror("_mvCmdFileToTempDir()");
		printf("Failed to move cmd file from upload directory to temp directory.\n");
		free(pOldPath);
		free(pNewPath);
		return -1;
	}
	
	free(pOldPath);
	free(pNewPath);
	return 0;
}

static int _parseUserEpubFile(char *uuidBuf, uint32_t uuidBufLen, uint32_t *serverId, char **userEpub, uint32_t *userEpubLen)
{
	//check
	if((NULL == uuidBuf) || (uuidBufLen < 32) || (NULL == serverId) || (NULL == userEpub) || (NULL == userEpubLen))
	{
		printf("_parseUserEpubFile():");
		printf("Got invalid inputs.\n");
		*serverId = 0;
		*userEpub = NULL;
		*userEpubLen = 0;
		return -1;
	}

	//open user.epub
	int fd = -1;
	fd = open(USEREPUBFILEPATH, O_CREAT | O_RDONLY, 0777);
	if(fd < 0)
	{
		perror("_parseUserEpubFile():open()");
		printf("Failed to create and open user.epub.\n");
		*serverId = 0;
		*userEpub = NULL;
		*userEpubLen = 0;
		return -1;
		
	}

	//read uuid
	int ret = -1;
	memset(uuidBuf, '\0', UUIDLEN);
	ret = read(fd, uuidBuf, UUIDLEN);
	if(UUIDLEN != ret)
	{
		if(-1 == ret)
		{
			perror("_parseUserEpubFile():read()");
		}
		else
		{
			printf("_parseUserEpubFile():read():");
		}
		printf("Failed to read uuid from user.epub.\n");
		close(fd);
		*serverId = 0;
		*userEpub = NULL;
		*userEpubLen = 0;
		return -1;

	}

	//read serverId
	uint32_t getServerId = 0;
	ret = read(fd, &getServerId, sizeof(getServerId));
	if(sizeof(getServerId) != ret)
	{
		if(-1 == ret)
		{
			perror("_parseUserEpubFile()");
		}
		else
		{
			printf("_parseUserEpubFile():");
		}
		printf("Failed to read serverId from user.epub.\n");
		close(fd);
		*serverId = 0;
		*userEpub = NULL;
		*userEpubLen = 0;
		return -1;
		
	}
	if(getServerId > 50000)
	{
		printf("_parseUserEpubFile():");
		printf("Failed to read serverId from user.epub.\n");
		close(fd);
		*serverId = 0;
		*userEpub = NULL;
		*userEpubLen = 0;
		return -1;

	}
	*serverId = getServerId;

	//read user epub len
	uint32_t ePubLen = 0;
	ret = read(fd, &ePubLen, sizeof(ePubLen));
	if(sizeof(ePubLen) != ret)
	{
		if(-1 == ret)
		{
			perror("_parseUserEpubFile()");
		}
		else
		{
			printf("_parseUserEpubFile():");
		}
		printf("Failed to read length of user epub from user.epub.\n");
		close(fd);
		*serverId = 0;
		*userEpub = NULL;
		*userEpubLen = 0;
		return -1;
		
	}
	if(ePubLen > 1024)
	{
		printf("_parseUserEpubFile():");
		printf("Failed to read length of user epub from user.epub.\n");
		close(fd);
		*serverId = 0;
		*userEpub = NULL;
		*userEpubLen = 0;
		return -1;

	}
	*userEpubLen  = ePubLen;

	if(0 == ePubLen)
	{
		*userEpub = NULL;	
		return 0;
	}
	
	//read user epub
	char *buf = NULL;
	buf = malloc(ePubLen+1);
	if(NULL == buf)
	{
		printf("_parseUserEpubFile():");
		printf("Failed to allocate memory for buffer.\n");
		close(fd);
		*serverId = 0;
		*userEpub = NULL;
		*userEpubLen = 0;
		return -1;
	}

	memset(buf, '\0', ePubLen+1);
	
	ret = read(fd, buf, ePubLen);
	if(ePubLen != ret)
	{
		if(-1 == ret)
		{
			perror("_parseUserEpubFile()");
		}
		else
		{
			printf("_parseUserEpubFile():");
		}
		printf("Failed to read user epub from user.epub.\n");
		free(buf);
		close(fd);
		*serverId = 0;
		*userEpub = NULL;
		*userEpubLen = 0;
		return -1;
		
	}
	*userEpub = buf;	

	return 0;
}


static void _freeUserEpub(char *userEpub)
{
	free(userEpub);
	return;
}

static int _countCmdFile(char *dirPath)
{
	//check inputs
	if((NULL == dirPath))
	{
		printf("_countCmdFile():");
		printf("Got invalid inputs.\n");
		return -1;
	}
	//printf("CHECKDONE.\n");

	//open dir 
	DIR *dUl = NULL;
	dUl = opendir(dirPath);
	if(NULL == dUl)
	{
		perror("_countCmdFile():opendir()");
		printf("Failed to open /server/epub/.\n");
		return -1;
	}	
	//printf("OPENDIRDONE.\n");
	
	//read dir 
	struct dirent *pD_st = NULL;
	int retCount = 0;

	pD_st = readdir(dUl);
	while(NULL != pD_st)
	{
		if(strncmp(((pD_st->d_name)+strlen(pD_st->d_name)-strlen(USEREPUBFILETAIL)), USEREPUBFILETAIL, strlen(USEREPUBFILETAIL)) == 0)
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
		perror("_countCmdFile():closedir()");
		printf("Failed to excute closedir().\n");
		return -1;
	}
	 
	//return
	return retCount; 

}




int server_countCmdFile()
{
	int ret = -1;
	ret = _countCmdFile(EPUBDIR);
	if(-1 == ret)
		return 0;
	else
		return ret;
}



int server_parseCmdFile(char *uuidBuf, uint32_t uuidBufLen, uint32_t *serverId, char **userEpub, uint32_t *userEpubLen)
{
	//get cmd file name
	char *retFileName = NULL;
	uint32_t retFileNameLen = 0;
	retFileName = _getCmdFileName(EPUBDIR, &retFileNameLen);
	if(NULL == retFileName)
	{
		printf("server_parseCmdFile():");
		printf("No xxxx.userepub file found in server/epub.\n");
		return -1;
	}


	//move cmd file to temp/user.epub
	//free retFileName
	int ret = -1;	
	ret = _mvCmdFileToTempDir(retFileName, retFileNameLen);
	if(-1 == ret)
	{
		printf("server_parseCmdFile():");
		printf("Failed to move epub/xxx.userepub to temp/user.epub.\n");
		free(retFileName);
		return -1;
	}
	_freeRetFileName(retFileName);

	//parse user.epub
	ret = _parseUserEpubFile(uuidBuf, uuidBufLen, serverId, userEpub, userEpubLen);
	if(-1 == ret)
	{
		printf("server_parseCmdFile():");
		printf("Failed to parse user.epub.\n");
		return -1;

	}

	return 0;
}









#if 0
//test main
int main(int argc, char **argv)
{
	int ret = -1;
	ret = _checkServerList();
	if(ret == -1)
	{
		printf("There is no list file.\n");
		_initServerList();
	}

	struct server_info info;
	server_getNewInfo(&info);
	server_initNewServer(&info);
	server_showServerList();

	printf("\n");

	server_getServerInfo(&info, info.server_id);
	(info.server_counter)++;
	server_setServerInfo(&info);
	server_showServerList();

	
	return 0;
}

#endif

#if 0
int main()
{
	int ret = -1;
	int i = 0;
	char uuid[UUIDLEN+1];
	memset(uuid, '\0', UUIDLEN+1);
	uint32_t serverId;
	char *userEpub = NULL;
	uint32_t userEpubLen = 0;
	ret = server_countCmdFile();
	if(ret > 0)
	{
		for(i = 0; i < ret; i++)
		{
			ret = server_parseCmdFile(uuid, UUIDLEN, &serverId, &userEpub, &userEpubLen);
			if(-1 == ret)
			{
				continue;
			}
			else
			{
				printf("UUID:%s\n", uuid);
				printf("SERVERID:%d\n", serverId);
				printf("USEREPUB:%s\n", userEpub);
				printf("USEREPUBLEN:%d\n", userEpubLen);
			}

		}
	}
	

	return 0; 
}





#endif





