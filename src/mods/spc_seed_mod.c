#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

//
#include "spc_seed_mod.h"
#include "cJSON.h"


//
static void _genMakeSeedCmd(char *dest, int len, uint32_t serverId, uint16_t index)
{
	int destIndex = 0;	
	//clear buf
	memset(dest, '\0', len);

	//strcat cmd head
	strncat(dest, MAKESEEDCMDHEAD, strlen(MAKESEEDCMDHEAD));
	destIndex += strlen(MAKESEEDCMDHEAD);
	
	//strcat server id
	char idBuf[IDLENGTHMAX] = {'\0'};
	sprintf(idBuf, "%d", serverId);
	strncat(idBuf, "/", strlen("/"));
	strncat(dest, idBuf, strlen(idBuf));
	destIndex += strlen(idBuf);

	//strcat index
	sprintf(dest+destIndex, "%d", index);

	//print cmd
	printf("_genMakeSeedCmd():");
	printf("one cmd done:%s\n", dest);

	return;
}


//
static int _saveXpubToFile(uint32_t serverId, uint16_t fileNo, uint16_t index, char *uuid)
{
	cJSON *root;
	cJSON *keystore;
	void * jsonBuf;
	//char * xpubStr;

	FILE *sFd, *xFd;
	int len;
	size_t nBytes;

	//1
	if((fileNo >= 0) && ((fileNo % MAKEMAX) == 0) && (index >= fileNo) && (index < (fileNo+MAKEMAX)))
	{
	} 
	else
	{
		printf("_saveXpubToFile(): ");
		printf("Get a bad index.\n");
		return -1;
	}

	//2	
	char seedFilePath[SEEDFILEPATHLEN] = {'\0'};
	//write wallet dir
	strncat(seedFilePath, WALLETFILEDIR, strlen(WALLETFILEDIR));

	//write id
	char idBuf[IDLENGTHMAX] = {"\0"};
	sprintf(idBuf, "%d", serverId);
	strncat(idBuf, "/", strlen("/"));
	strncat(seedFilePath, idBuf, strlen(idBuf));

	//write index
	char indexBuf[IDLENGTHMAX] = {'\0'};
	sprintf(indexBuf, "%d", index);
	strncat(seedFilePath, indexBuf, strlen(indexBuf));

	////////////////
	printf("gen seed path:%s\n", seedFilePath);
	
	//open file
	sFd = fopen(seedFilePath, "r");
	if(NULL == sFd)
	{
		perror("_saveXpubToFile():fopen()");
		printf("Failed to open source seed file.\n");
		return -2;
	}

	fseek(sFd, 0, SEEK_END);
	len = ftell(sFd);	
	fseek(sFd, 0, SEEK_SET);
	jsonBuf = malloc(len+1);
	memset(jsonBuf, '\0', len+1);
	
	nBytes = fread(jsonBuf, 1, len, sFd);
	if(nBytes < 0)
	{
		perror("_saveXpubToFile():fread()");
		printf("Can not read json data in seed file.\n");
		free(jsonBuf);
		fclose(sFd);
		return -2;
	}

	fclose(sFd);

	//3
	root = cJSON_Parse(jsonBuf);
	if(NULL == root)
	{
		perror("_saveXpubToFile():cJSON_Parse()");
		printf("Failed to parse source seed file.\n");
		free(jsonBuf);
		return -3;
		
	}

	//keystore = cJSON_GetObjectItem(root, "keystore");
	keystore = cJSON_GetObjectItem(root, "x1/");

	char xpubSaveBuf[XPUBSAVELEN] = {'\0'};
	memset(xpubSaveBuf, '\0', XPUBSAVELEN);
	memcpy(xpubSaveBuf, &index, sizeof(uint16_t));

#if 0
	nBytes = sprintf(xpubSaveBuf, "%03d", index);
	if(nBytes != 3)
	{
		perror("_spcSlaveSaveXpubToFile():sprintf()");
		printf("Failed to write index to buffer.\n");
		return -3;
	}
#endif
	uint32_t xpubLen = XPUBLEN; 	
	memcpy(xpubSaveBuf+sizeof(uint16_t), &xpubLen, sizeof(uint32_t));
	nBytes = sprintf(xpubSaveBuf+sizeof(uint16_t)+sizeof(uint32_t), "%s", (cJSON_GetObjectItem(keystore, "xpub")->valuestring));
	//nBytes = sprintf(xpubSaveBuf+3, "%s", (cJSON_GetObjectItem(keystore, "xpub")->valuestring));
	if(nBytes < 0)
	{
		perror("_saveXpubToFile():sprintf()");
		printf("Failed to write xpub to buffer.\n");
		cJSON_Delete(root);
		return -3;
	}
	printf("_saveXpubToFile():");
	printf("GOTXPUBLEN:%d\n", (int)strlen((cJSON_GetObjectItem(keystore, "xpub")->valuestring)));

	cJSON_Delete(root);

	//4
	char xpubFilePath[XPUBFILEPATHLEN] = {'\0'};
	memset(xpubFilePath, '\0', XPUBFILEPATHLEN);
	//write header
	strncat(xpubFilePath, XPUBFILEDIR, strlen(XPUBFILEDIR));
	
	//write id
	strncat(xpubFilePath, idBuf, strlen(idBuf));
	sprintf(xpubFilePath+strlen(xpubFilePath), "%05d-", serverId);

	//write fileNo
	char fileNoBuf[IDLENGTHMAX] = {'\0'};
	memset(fileNoBuf, '\0', IDLENGTHMAX);
	sprintf(fileNoBuf, "%05d.ce", fileNo);
	strncat(xpubFilePath, fileNoBuf, strlen(fileNoBuf));

	int wRet = 0;
	printf("%s\n", xpubFilePath);
	if(access(xpubFilePath, F_OK) == -1)
	{
		printf("DEBUG\n");
		xFd = fopen(xpubFilePath, "w+");
		if(NULL == xFd)
		{
			perror("_saveXpubToFile():fopen()");	
			printf("Failed to open xpub file.\n");
			return -4;
		}
	
		wRet = fwrite(uuid, 1, UUIDLEN, xFd);
		if(wRet < UUIDLEN)
		{
			printf("_saveXpubToFile():fwrite():");
			printf("Failed to write uuid to file.\n");
			fclose(xFd);
			return -4;
		}	
		wRet = fwrite(&serverId, 1, sizeof(uint32_t), xFd);
		if(wRet < sizeof(uint32_t))
		{
			printf("_saveXpubToFile():fwrite():");
			printf("Failed to write serverId to file.\n");
			fclose(xFd);
			return -4;
		}	
		fseek(xFd, sizeof(uint32_t)+UUIDLEN, SEEK_SET);
	}
	else
	{
		xFd = fopen(xpubFilePath, "a");
		if(NULL == xFd)
		{
			perror("_saveXpubToFile():fopen()");	
			printf("Failed to open xpub file.\n");
			return -4;
		}
	}


	//nBytes = fwrite(xpubSaveBuf, 1, XPUBSAVELEN, xFd);
	nBytes = fwrite(xpubSaveBuf, 1, XPUBLEN+sizeof(uint16_t)+sizeof(uint32_t), xFd);
	//if(nBytes != XPUBSAVELEN)
	if(nBytes != (XPUBLEN+sizeof(uint16_t)+sizeof(uint32_t)))
	{
		perror("_saveXpubToFile():fwrite()");
		printf("Failed to write xpub to file.\n");
		fclose(xFd);
		return -4;
	}	
	
	fclose(xFd);

	return 0;
}


//
static int _checkStart(uint32_t serverId, uint16_t start)
{
	//size_t nBytes = 0;

	char xpubFilePath[XPUBFILEPATHLEN] = {'\0'};
	//write header
	strncat(xpubFilePath, XPUBFILEDIR, strlen(XPUBFILEDIR));
		
	//write id
	char idBuf[IDLENGTHMAX] = {'\0'};
	sprintf(idBuf, "%d", serverId);
	strncat(idBuf, "/", strlen("/"));
	strncat(xpubFilePath, idBuf, strlen(idBuf));

	//write start 
	char startBuf[IDLENGTHMAX] = {'\0'};
	sprintf(startBuf, "%d", start);
	strncat(xpubFilePath, startBuf, strlen(startBuf));	

	if(access(xpubFilePath, F_OK) == 0)
	{
		return -1;
	}	
	else
	{
		return 0;
	}

}


static int _mvToDownloadDir(uint32_t serverId, uint16_t fileNo, char *uuidBuf)
{
	//gen path to sat pub path
	char idBuf[IDLENGTHMAX] = {"\0"};
	memset(idBuf, '\0', IDLENGTHMAX);
	sprintf(idBuf, "%d", serverId);
	strncat(idBuf, "/", strlen("/"));
	char xpubFilePath[XPUBFILEPATHLEN];
	memset(xpubFilePath, '\0', XPUBFILEPATHLEN);
	//write header
	strncat(xpubFilePath, XPUBFILEDIR, strlen(XPUBFILEDIR));
	
	//write id
	strncat(xpubFilePath, idBuf, strlen(idBuf));
	sprintf(xpubFilePath+strlen(xpubFilePath), "%05d-", serverId);

	//write fileNo
	char fileNoBuf[IDLENGTHMAX] = {'\0'};
	memset(fileNoBuf, '\0', IDLENGTHMAX);
	sprintf(fileNoBuf, "%05d.ce", fileNo);
	strncat(xpubFilePath, fileNoBuf, strlen(fileNoBuf));

	//gen new path
	char newPath[XPUBFILEPATHLEN];
	memset(newPath, '\0', XPUBFILEPATHLEN);
	strncat(newPath, DOWNLOADDIR, strlen(DOWNLOADDIR));
	sprintf(newPath + strlen(DOWNLOADDIR), "%05d-", serverId);
	strncat(newPath, uuidBuf, UUIDLEN);
	strncat(newPath, ".satpub", strlen(".satpub"));

	//rename
	int ret = -1;
	ret = rename(xpubFilePath, newPath);
	if(-1 == ret)
	{
		perror("_mvToDownloadDir()");
		printf("Failed to move sat pub file to downlaod directory.\n");
		return -1;
	}

	return 0;

}



//
int seed_makeSeed(uint32_t serverId, uint16_t start, char *uuidBuf, uint32_t uuidBufLen)
{
	uint16_t index = 0;
	int ret = -1;

	if((start < 0) || ((start % MAKEMAX) != 0))
	{
		printf("seed_makeSeed():");
		printf("Get a bad start.\n");
		return -1;
	}

#if 0
	ret = _checkStart(serverId, start);
	if(ret < 0)
	{
		printf("seed_makeSeed():_checkStart():");
		printf("xpub file existed.\n");
		return -1;
	}	
#endif

	if((NULL == uuidBuf) || (uuidBufLen < UUIDLEN))
	{
		printf("seed_makeSeed():");
		printf("invalid uuid buf.\n");
		return -1;
	}

	//cmd buf
	char makeSeedCmdBuf[CMDBUFLEN] = {'\0'};

	for(index = start; index < start+MAKEMAX; index++)
	{
		//call genMakeSeedCmd
		_genMakeSeedCmd(makeSeedCmdBuf, CMDBUFLEN, serverId, index);	
		
		//system cmd
		system(makeSeedCmdBuf);

		//return ack
		//after redesign

		ret = _saveXpubToFile(serverId, start, index, uuidBuf);
		if(ret < 0)
		{
			printf("seed_makeSeed():_saveXpubToFile():");
			printf("Failed to save Xpub No.%d to file.\n", index);
		}

		printf("seed_makeSeed():");
		printf("Generate seed No.%d\n", index);	

	}

	ret = _mvToDownloadDir(serverId, start, uuidBuf);
	if(-1 == ret)
	{
		printf("seed_makeSeed():");
		printf("Failed to move sat pub file to download directory.\n");
		return -1;
	}

	return 0;
}


//
static void _getSeedPath(char *path, int len, uint32_t serverId, uint16_t index)
{
	//clear buf
	memset(path, '\0', len);

	//strcat dir path
	strncat(path, WALLETFILEDIR, strlen(WALLETFILEDIR));

	//strcat id path
	char idBuf[IDLENGTHMAX] = {'\0'};
	sprintf(idBuf, "%d", serverId);
	strncat(idBuf, "/", strlen("/"));
	strncat(path, idBuf, strlen(idBuf));
	
	//strcat index
	char indexBuf[IDLENGTHMAX] = {'\0'};
	sprintf(indexBuf, "%d", index);
	strncat(path, indexBuf, strlen(indexBuf));

	///////////////////////print path
	printf("_spcSlaveGetSeedPath():");
	printf("Get one path:%s\n", path);
	
	return;
}

#if 0
//
int seed_genWalletFile(uint32_t serverId, uint16_t index, const char *xpub2, const char *xpub3)
{
	cJSON *root;
	cJSON *keystore, *x1, *x2, *x3;
	void *tmpDataBuf, *jsonTmpBuf;

	FILE *sFd, *jsonTmpFd, *tmpFd;
	int len;
	size_t nBytes;

	char seedNameBuf[SEEDPATHLEN] = {'\0'};

	//0	
	_getSeedPath(seedNameBuf, SEEDPATHLEN, serverId, index);	

	sFd = fopen(seedNameBuf, "r");
	if(NULL == sFd)
	{
		perror("seed_genWalletFile():fopen()");
		printf("Failed to open seed file.\n");
		return -1;
	}

	fseek(sFd, 0, SEEK_END);
	len = ftell(sFd);	
	fseek(sFd, 0, SEEK_SET);
	tmpDataBuf = malloc(JSONBUFLEN);
	memset(tmpDataBuf, '\0', JSONBUFLEN);
	
	nBytes = fread(tmpDataBuf, 1, len, sFd);
	if(nBytes != len)
	{
		perror("seed_genWalletFile():fread()");
		printf("Failed to read seed file.\n");
		free(tmpDataBuf);
		fclose(sFd);
		return -2;
	}

	fclose(sFd);

	//1
	char xpub1[XKEYBUFLEN] = {'\0'};
	char xprv1[XKEYBUFLEN] = {'\0'};

	root = cJSON_Parse(tmpDataBuf);

	if(NULL == root)
	{
		perror("seed_genWalletFile():cJSON_Parse()");
		printf("Failed to parse seed json date\n");
		free(tmpDataBuf);
		return -3;
		
	}

	//keystore = cJSON_GetObjectItem(root, "keystore");
	keystore = cJSON_GetObjectItem(root, "x1/");
	sprintf(xpub1, "%s", (cJSON_GetObjectItem(keystore, "xpub")->valuestring));
	sprintf(xprv1, "%s", (cJSON_GetObjectItem(keystore, "xprv")->valuestring));

	cJSON_Delete(root);

	//2
	jsonTmpFd = fopen(JSONTMPFILEPATH, "r");
	if(NULL == jsonTmpFd)
	{
		perror("seed_genWalletFile():fopen()");
		printf("Failed to open json tmp file.\n");
		return -1;
	}

	fseek(jsonTmpFd, 0, SEEK_END);
	len = ftell(jsonTmpFd);	
	fseek(jsonTmpFd, 0, SEEK_SET);
	jsonTmpBuf = malloc(JSONBUFLEN);
	memset(jsonTmpBuf, '\0', JSONBUFLEN);
	
	nBytes = fread(jsonTmpBuf, 1, len, jsonTmpFd);
	if(nBytes != len)
	{
		perror("seed_genWalletFile():fread()");
		printf("Failed to read json tmp file.\n");
		free(jsonTmpBuf);
		fclose(jsonTmpFd);
		return -2;
	}
	fclose(jsonTmpFd);

	root = NULL;
	root = cJSON_Parse(jsonTmpBuf);

	if(NULL == root)
	{
		perror("seed_genWalletFile():cJSON_Parse()");
		printf("Failed to parse seed json date\n");
		free(jsonTmpBuf);
		free(tmpDataBuf);
		return -2;
		
	}
	
	x1 = cJSON_GetObjectItem(root, "x1/");
	cJSON_GetObjectItem(x1, "xpub")->valuestring = (char *)xpub1;
	cJSON_GetObjectItem(x1, "xprv")->valuestring = (char *)xprv1;

	x2 = cJSON_GetObjectItem(root, "x2/");
	cJSON_GetObjectItem(x2, "xpub")->valuestring = (char *)xpub2;

	x3 = cJSON_GetObjectItem(root, "x3/");
	cJSON_GetObjectItem(x3, "xpub")->valuestring = (char *)xpub3;

	memset(tmpDataBuf, '\0', JSONBUFLEN);
	
	tmpDataBuf = cJSON_Print(root);

	free(jsonTmpBuf);
	
	//cJSON_Delete(root);

	//3	
	tmpFd = fopen(TMPWALLETFILEPATH, "w");
	if(NULL == tmpFd)
	{
		perror("seed_genWalletFile():fopen()");
		printf("Failed to open tmp.wallet\n");
		free(tmpDataBuf);
		return -1;
	}

	nBytes = fprintf(tmpFd, "%s", (char *)tmpDataBuf);
	if(nBytes < 0)
	{
		perror("seed_genWalletFile():fprintf()");
		printf("Failed to write tmp.wallet\n");
		free(tmpDataBuf);
		fclose(tmpFd);
		return -1;
	}

	fclose(tmpFd);

	free(tmpDataBuf);

	return 0;
}
#endif

//
void spcSlaveRemoveSeed(uint32_t serverId, uint16_t start)
{
	//
	if((start<0) || ((start % MAKEMAX) != 0))
	{
		printf("seed_removeSeed():");
		printf("Get a bad start.\n");
		return;
	}

	int index = 0;
	int ret = -1;	

	//new buf
	char path[SEEDPATHLEN] = {'\0'};

	for(index = start; index < start+10; index++)
	{
		//call
		_getSeedPath(path, SEEDPATHLEN, serverId, index);
		
		//remove
		ret = remove(path);	
		if(ret < 0)
		{
			printf("seed_removeSeed():");
			printf("Failed to remove seed No.%d\n", index);	
			continue;
		}

		//return index ack 
		//after redesign

		printf("seed_removeSeed():");
		printf("Remove seed No.%d\n", index);	
	}
	
	return;
}

#if 0
//test main
int main(int argc, char *argv[])
{
	char *xpub2 = "xpub2hahahaha";
	char *xpub3 = "xpub3hahahaha";
	char *uuid = "12345678901234567890123456789012";

	if(argc != 3)
	{
		printf("Usage: only need id and start param.\n");
		return -1;
	}

	uint32_t serverId = 0;
	uint16_t start = 0;

	serverId = atoi(argv[1]);
	start = atoi(argv[2]);

	printf("main(): Get a server id: %d\n", serverId);
	printf("main(): Get a start: %d\n", start);

	seed_makeSeed(serverId, start, uuid, 32);

	//seed_genWalletFile(serverId, start, xpub2, xpub3);

	return 0;
}
#endif
