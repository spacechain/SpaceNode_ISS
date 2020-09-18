#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#include "spc_secret_mod.h"

#define READSTEP		50	
#define ENCRYPTEDMSGMAX		512	 

static char *_genEncryptCmd(char *pubkeyBuf, uint32_t pubkeyLen, char *msgBuf, uint32_t msgLen, uint32_t *cmdLen)
{
	void *pCmd = NULL;
	size_t cmdBufLen = strlen(CMDHEADER) + 1 + pubkeyLen + 1 + msgLen + 1 + 10;
	pCmd = malloc(cmdBufLen);
	if(NULL == pCmd)
	{
		perror("_encrypt()");
		printf("Failed to malloc buf for cmd.\n");
		*cmdLen = 0;
		return NULL;
	}

	memset(pCmd, '\0', cmdBufLen);

	strncat(pCmd, CMDHEADER, strlen(CMDHEADER));	
	strncat(pCmd, " ", 1);
	
	strncat(pCmd, pubkeyBuf, pubkeyLen);
	strncat(pCmd, " ", 1);

	strncat(pCmd, msgBuf, msgLen);
	((char *)pCmd)[cmdBufLen - 10 - 1] = '\0'; 

	*cmdLen = cmdBufLen - 10;
	return pCmd;
}


static void  _freeEncryptCmd(char *pEncryptCmd)
{
	free(pEncryptCmd);
	return;
}


static char *_encryptMsg(char *cmdBuf, uint32_t cmdLen, uint32_t *pRetLen)
{
	FILE *retStream = NULL;
	retStream = popen(cmdBuf, "r");
	if(NULL == retStream)
	{
		perror("_encryptMsg():popen()");
		printf("Failed to execute popen.\n");
		*pRetLen = 0;
		return NULL;
	}	

	
	int getNum = 0;
	int sumNum = 0;
	

//getNum = 0 eof
//0 < getNum <= 50 eof
//get = 50 no eof

	void *pRet = NULL;
	while(sumNum < ENCRYPTEDMSGMAX)
	{
		pRet = realloc(pRet, sumNum + READSTEP);
		if(NULL == pRet)
		{
			perror("_encryptMsg():realloc()");
			printf("Failed to malloc buffer for read result stream.\n");
			free(retStream);
			*pRetLen = 0;
			return NULL;
		}
		memset(pRet + sumNum, '\0', READSTEP);
		
		getNum = fread(pRet + sumNum, 1, READSTEP, retStream);
		
		if((getNum <= READSTEP) && (feof(retStream) != 0) && (ferror(retStream) == 0))
		{
			printf("_encryptMsg():get less step, eof.\n");
			sumNum += getNum;
			break;
		}
		else if((READSTEP == getNum) && (0 == feof(retStream)) && (ferror(retStream) == 0))
		{
			printf("_encryptMsg():get one step, not eof.\n");
			sumNum += getNum;
		}
		else if((getNum < READSTEP) && (ferror(retStream) != 0))
		{
			perror("_encryptMsg():fread()");
			printf("Error occured when fread result stream.\n");
			free(pRet);
			fclose(retStream);
			*pRetLen = 0;
			return NULL;
		}
	}

	if(sumNum > ENCRYPTEDMSGMAX)
	{
		printf("_encryptMsg():fread():");
		printf("Error occured when fread result stream.\n");
		free(pRet);
		fclose(retStream);
		*pRetLen = 0;
		return NULL;
	}

	fclose(retStream);

	((char *)pRet)[sumNum-1] = '\0'; 
	*pRetLen = sumNum - 1;
	return pRet;
}

static void  _freeEncryptMsg(char *pEncryptMsg)
{
	free(pEncryptMsg);
	return;
}

//generate encrypt command
//popen encrypt command 
//read result from stream
//get result length
//copy result to caller's buffer

/*
static int _decrypt(char *pRet, uint32_t lRet, \
				char *pMass, uint32_t lMass, \
					char *pPri, uint32_t lPri);
*/

//
int _encryptSecret(char *pubkey1Buf, uint32_t pubkey1Len, \
			char *pubkey2Buf, uint32_t pubkey2Len, \
				char *secretBuf, uint32_t secretLen, \
					char **retBuf, uint32_t *retLen)
{
	char *pCmd1 = NULL;
	uint32_t cmd1Len = 0;
	pCmd1 = _genEncryptCmd(pubkey1Buf, pubkey1Len, secretBuf, secretLen, &cmd1Len);
	if((NULL == pCmd1) || (0 == cmd1Len))
	{
		printf("_encryptSecret():_genEncryptCmd():");
		printf("Failed to generate first encrypt command.\n");
		return -1;
	}

	char *pRet1 = NULL;
	uint32_t ret1Len = 0;
	pRet1 = _encryptMsg(pCmd1, cmd1Len, &ret1Len);
	if((NULL == pRet1) || (0 == ret1Len))
	{
		printf("_encryptSecret():_encryptMsg():");
		printf("Failed to encrypt message with first pubkey.\n");
		_freeEncryptCmd(pCmd1);
		return -1;
	}
	_freeEncryptCmd(pCmd1);

	char *pCmd2 = NULL;
	uint32_t cmd2Len = 0;
	pCmd2 = _genEncryptCmd(pubkey2Buf, pubkey2Len, pRet1, ret1Len, &cmd2Len);
	if((NULL == pCmd2) || (0 == cmd2Len))
	{
		printf("_encryptSecret():_genEncryptCmd():");
		printf("Failed to generate second encrypt command.\n");
		_freeEncryptMsg(pRet1);
		return -1;
	}
	_freeEncryptMsg(pRet1);

	char *pRet2 = NULL;
	uint32_t ret2Len = 0;
	pRet2 = _encryptMsg(pCmd2, cmd2Len, &ret2Len);
	//printf("HERE?\n");
	if((NULL == pRet2) || (0 == ret2Len))
	{
		printf("_encryptSecret():_encryptMsg():");
		printf("Failed to encrypt message with second pubkey.\n");
		_freeEncryptCmd(pCmd2);
		return -1;
	}
	//printf("HERE?\n");
	_freeEncryptCmd(pCmd2);
	//printf("HERE?\n");
	
	*retBuf = pRet2;
	//printf("HERE?\n");
	printf("%d\n", ret2Len);
	*retLen = ret2Len;
	//printf("HERE?\n");
	return 0;
}


void _freeDmsg(char *pEncryptMsg)
{
	free(pEncryptMsg);
	return;
}


static int _writeDmsgToTempFile(char *uuidBuf, uint32_t uuidBufLen, uint32_t serverId, char *dMsg, uint32_t dMsgLen)
{
	//check 
	if((NULL == uuidBuf) || (uuidBufLen < UUIDLEN) || (0 == serverId) || (NULL == dMsg) || (0 == dMsgLen))
	{
		printf("_writeDmsgToTempFile():");
		printf("GOt invalid inputs.\n");
		return -1;
	}

	//open
	int fd = -1;
	fd = open(TEMPSECFILEPATH, O_CREAT | O_TRUNC | O_WRONLY, 0777);
	if(fd < 0)
	{
		perror("_writeDmsgToTempFile():open():");
		printf("Failed to create and open temp/temp.sec.\n");
		return -1;
	}


	//write 
	int ret = -1;
	ret = write(fd, uuidBuf, UUIDLEN);
	if(UUIDLEN != ret)
	{
		if(-1 == ret)
		{
			perror("_writeDmsgToTempFile():write():");
		}
		else
		{
			printf("_writeDmsgToTempFile():write():");
		}
		printf("Failed to write uuid to temp.sec.\n");
		close(fd);
		return -1;
	}

	ret = write(fd, &serverId, sizeof(serverId));
	if(sizeof(serverId) != ret)
	{
		if(-1 == ret)
		{
			perror("_writeDmsgToTempFile():write():");
		}
		else
		{
			printf("_writeDmsgToTempFile():write():");
		}
		printf("Failed to write serverId to temp.sec.\n");
		close(fd);
		return -1;
	}

	ret = write(fd, &dMsgLen, sizeof(dMsgLen));
	if(sizeof(dMsgLen) != ret)
	{
		if(-1 == ret)
		{
			perror("_writeDmsgToTempFile():write():");
		}
		else
		{
			printf("_writeDmsgToTempFile():write():");
		}
		printf("Failed to write dmsg length to temp.sec.\n");
		close(fd);
		return -1;
	}

	ret = write(fd, dMsg, dMsgLen);
	if(dMsgLen != ret)
	{
		if(-1 == ret)
		{
			perror("_writeDmsgToTempFile():write():");
		}
		else
		{
			printf("_writeDmsgToTempFile():write():");
		}
		printf("Failed to write dMsg to temp.sec.\n");
		close(fd);
		return -1;
	}

	//close	
	close(fd);

	return 0;
}


static int _checkTempSec()
{
	return(access(TEMPSECFILEPATH, R_OK));
}


static int _mvTempSecToDownloadDir(char *uuidBuf, uint32_t uuidBufLen, uint32_t serverId)
{
	//check
	if((NULL == uuidBuf) || (uuidBufLen < UUIDLEN) || (0 == serverId))
	{
		printf("_mvTempSecToDownloadDir():");
		printf("Got invalid inputs.\n");
		return -1;
	}

	//gen path
	char *pathBuf = NULL;
	pathBuf = malloc(strlen(DOWNLOADDIR)+5+1+UUIDLEN+4+1);
	if(NULL == pathBuf)
	{
		printf("_writeDmsgToTempFile():malloc():");
		printf("Failed to allocate memory for buffer.\n");
		return -1;
	}
	memset(pathBuf, '\0', (strlen(DOWNLOADDIR)+5+1+UUIDLEN+4+1));

	strncat(pathBuf, DOWNLOADDIR, strlen(DOWNLOADDIR));
	sprintf(pathBuf+strlen(DOWNLOADDIR), "%05d-", serverId);
	memcpy(pathBuf+strlen(DOWNLOADDIR)+6, uuidBuf, UUIDLEN);
	strncat(pathBuf, ".sec", strlen(".sec"));
	printf("%s\n", pathBuf);


	//rename
	int ret = -1;
	ret = rename(TEMPSECFILEPATH, pathBuf);
	if(-1 == ret)
	{
		perror("_writeDmsgToTempFile():rename()");
		printf("Failed to move temp.sec to downlaod directory.\n");
		free(pathBuf);
		return -1;
	}
	free(pathBuf);

	return 0;
}


int secret_genSecFile(char *pubkey1, uint32_t pubkey1Len, \
	              char *pubkey2, uint32_t pubkey2Len, \
	              char *secretBuf, uint32_t secretBufLen, \
		      char *uuidBuf, uint32_t uuidBufLen, \
		      uint32_t serverId)
{
	int ret = -1;
	char *retBuf = NULL;
	uint32_t retLen = 0;
	ret = _encryptSecret(pubkey1, pubkey1Len, \
			     pubkey2, pubkey2Len, \
			     secretBuf, secretBufLen, \
			     &retBuf, &retLen);
	if(-1 == ret)
	{
		printf("secret_genSecFile():");
		printf("Failed to encrypt secret.\n");
		if(NULL != retBuf)
		{
			free(retBuf);
		}
		return -1;
	}
	printf("%s\n", retBuf);
	printf("%d\n", retLen);

	ret = _writeDmsgToTempFile(uuidBuf, uuidBufLen, serverId, retBuf, retLen);
	if(-1 == ret)
	{
		printf("secret_genSecFile():");
		printf("Failed to encrypted secret to temp.sec.\n");
		free(retBuf);
		return -1;
	}
	_freeDmsg(retBuf);

	ret = _checkTempSec();
	if(-1 == ret)
	{
		printf("secret_genSecFile():");
		printf("Temp.sec not found.\n");
		return -1;
	}

	ret = _mvTempSecToDownloadDir(uuidBuf, uuidBufLen, serverId);
	if(-1 == ret)
	{
		printf("secret_genSecFile():");
		printf("Failed to move temp.sec to ID-UUID.sec.\n");
		return -1;
	}

	return 0;
}





#if 0
int main(int argc, char **argv)
{
	//char *msg = "hellohellohello1";
	char *msg = "hellohellohellohellohellohellohellohello";
	char *pub = "028164015822084ab45e336f2627bc3aad23c1487e4f5663c094c5b357159d1d73";
	char *uuid = "12345678901234567890123456789012";
	
	int ret = -1;
	ret = secret_genSecFile(pub, strlen(pub), pub, strlen(pub), msg, strlen(msg), uuid, strlen(uuid), 1);
	printf("RET:%d\n", ret);


	return 0;
}

#endif
