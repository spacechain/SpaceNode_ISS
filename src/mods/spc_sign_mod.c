#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <net/if.h>
#include <error.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#include <stdint.h>

#include "cJSON.h"
#include "spc_sign_mod.h"

const char *walletJsonTmpStr = 
"{\n\t\"seed_version\":\t18,\n\t\"spent_outpoints\":\t{},\n\t\"wallet_type\":\t\"2of3\",\n\t\"x1/\":\t{\n\t\t\"pw_hash_version\":\t1,\n\t\t\"type\":\t\"bip32\",\n\t\t\"xprv\":\t\"Zprv\",\n\t\t\"xpub\":\t\"Zpub\"\n\t},\n\t\"x2/\":\t{\n\t\t\"pw_hash_version\":\t1,\n\t\t\"type\":\"bip32\",\n\t\t\"xprv\":\tnull,\n\t\t\"xpub\":\"Zpub\"},\n\t\"x3/\":\t{\n\t\t\"pw_hash_version\":\t1,\n\t\t\"type\":\t\"bip32\",\n\t\t\"xprv\":\tnull,\n\t\t\"xpub\":\t\"Zpub\"\n\t}\n}";



//
unsigned char spcCharToHex(char cIn)
{
	if((cIn >= '0') && (cIn <= '9'))
	{
		return (cIn - '0');
	}else if((cIn >= 'a') && (cIn <= 'f'))
	{
		return (cIn - 'a' + 10);
	}else if((cIn >= 'A') && (cIn <= 'F'))
	{
		return (cIn - 'A' + 10);
	}else
	{
		return -1;
	}
}


//
char spcHexToChar(unsigned char ucIn)
{
	if((ucIn >= 0) && (ucIn <= 9))
	{
		return (ucIn + '0');
	}else if((ucIn >= 10) && (ucIn <= 15))
	{
		return (ucIn - 10 + 'a');
	}else
	{
		return (-1);
	}
}


//
int spcStringToHex(unsigned char *ucOutData, char *cString, int iLen)
{

	printf("spcStringToHex:");
	printf("SourceStirng:%s\n", cString);
	char *cData = cString;
	int iInIndex = 0;
	int iOutIndex = 0;
	
	for(iInIndex = 0; iInIndex < iLen; iInIndex++)
	{
		if((iInIndex % 2) == 0)
		{
			//odd
			*(ucOutData + iOutIndex) |= (spcCharToHex(*(cData + iInIndex)) << 4);
		}else
		{
			//even
			*(ucOutData + iOutIndex) |= (spcCharToHex(*(cData + iInIndex)));
			iOutIndex++;
		}
	}	
	return (iInIndex);
}


//
int spcHexToString(char *cOutString, unsigned char *ucInData, int iLen, u_int16_t bIs)
{
	unsigned char *ucData	= ucInData;
	int iInIndex = 0;
	int iOutIndex = 0;
	
	for(iInIndex = 0; iInIndex < iLen; iInIndex++)
	{	
		*(cOutString + iOutIndex) = spcHexToChar(((*(ucData + iInIndex)) & 0xF0) >> 4);
		iOutIndex++;
		*(cOutString + iOutIndex) = spcHexToChar((*(ucData + iInIndex)) & 0x0F);
		iOutIndex++;
	}

	if(bIs) 
	{
		iOutIndex--;
	}
	*(cOutString + iOutIndex) = '\0';

	return (iOutIndex);
}


//
int spcProcessSigResult(unsigned char *ucOutData, uint32_t sOutLen, uint32_t *pDataLen, char *cJsonStr, size_t stLen)
{
	uint16_t sHexLong;
	int sRet;
	int iIndex;
	cJSON *json=NULL, *json_hex=NULL;

	json = cJSON_Parse(cJsonStr);
	//
	if (NULL == json)
	{
		printf("spcProcessSigResult:");
		printf("Failed to parse Json data\n");
		return -1;
	}else
	{
		printf("spcProcessSigResult:");
		printf("get the json root\n");
		cJSON *json_complete = cJSON_GetObjectItem(json, "complete");
	
		if(cJSON_IsTrue(json_complete))
		{
			printf("spcProcessSigResult:");
			printf("Signed the transaction successfully.\n");
			json_hex = cJSON_GetObjectItem(json, "hex");
			printf("spcProcessSigResult:");
			printf("ResultString:%s\n", json_hex->valuestring);
			printf("Len of result string:%ld\n", strlen((char *)(json_hex->valuestring)));			
			sRet = spcStringToHex(ucOutData + 3, (char *)(json_hex->valuestring), strlen((char *)(json_hex->valuestring)));
			
			printf("spcProcessSigResult:");
			printf("sRet = %d\n", sRet);
		}else
		{
			printf("spcProcessSigResult:");
			printf("Failed to sign transation.\n");			
			memset(ucOutData, '\0', sOutLen);
			cJSON_Delete(json);
			return -1;
			
		}		

		iIndex = 0;
		ucOutData[iIndex++] = 0xc1;
		sHexLong = (sRet / 2) + (sRet % 2);
		//1+2+sHexLong+1
		*pDataLen = 1+ 2 + sHexLong + 1;
		if(sRet % 2)		
		{
			sHexLong |= 0x8000;
		}

		memcpy(&ucOutData[iIndex], &sHexLong, sizeof(sHexLong));		
		iIndex = iIndex + 2 + (sHexLong & 0x7FFF);
		ucOutData[iIndex] = 0x1c;

		printf("spcProcessSigResult:");
		printf("sHexLong = %d, sizeof(sHexLong) = %ld\n", sHexLong,sizeof(sHexLong));

		cJSON_Delete(json);
		return 0;
	}

	
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
	printf("_getSeedPath():");
	printf("Get one path:%s, strlen:%d\n", path, (int)strlen(path));	
	return;
}


//
int sign_genWalletFile(uint32_t serverId, uint16_t index, const char *xpub2, const char *xpub3)
{
	cJSON *root;
	cJSON *keystore, *x1, *x2, *x3;
	void *seedTmpBuf, *walletTmpBuf;

	FILE *seedFd, *jsonTmpFd, *walletTmpFd;
	int len;
	size_t nBytes;

	char seedNameBuf[SEEDPATHLEN] = {'\0'};

	//0	
	_getSeedPath(seedNameBuf, SEEDPATHLEN, serverId, index);	

	seedFd = fopen(seedNameBuf, "r");
	if(NULL == seedFd)
	{
		perror("sign_genWalletFile():fopen()");
		printf("Failed to open seed file.\n");
		return -1;
	}

	printf("DEBUG:AFTER FOPEN\n");
	fseek(seedFd, 0, SEEK_END);
	len = ftell(seedFd);
	printf("DEBUG:FTELLLEN:%d\n", len);	
	fseek(seedFd, 0, SEEK_SET);
	seedTmpBuf = malloc(len+1);
	if(NULL == seedTmpBuf)
	{
		perror("sign_genWalletFile():malloc()");
		printf("Failed to malloc space seed json data.\n");
		fclose(seedFd);
		return -1;
	}
	memset(seedTmpBuf, '\0', len+1);
	printf("DEBUG:AFTER MALLOC\n");	

	nBytes = fread(seedTmpBuf, 1, len, seedFd);
	if(nBytes != len)
	{
		perror("sign_genWalletFile():fread()");
		printf("Failed to read seed file.\n");
		free(seedTmpBuf);
		fclose(seedFd);
		return -2;
	}
	printf("DEBUG:AFTER FREAD\n");

	fclose(seedFd);

	//1
	char xpub1[XKEYBUFLEN] = {'\0'};
	char xprv1[XKEYBUFLEN] = {'\0'};

	root = cJSON_Parse(seedTmpBuf);

	if(NULL == root)
	{
		perror("sign_genWalletFile():cJSON_Parse()");
		printf("Failed to parse seed json date\n");
		free(seedTmpBuf);
		return -3;
		
	}

	//keystore = cJSON_GetObjectItem(root, "keystore");
	keystore = cJSON_GetObjectItem(root, "x1/");
	sprintf(xpub1, "%s", (cJSON_GetObjectItem(keystore, "xpub")->valuestring));
	sprintf(xprv1, "%s", (cJSON_GetObjectItem(keystore, "xprv")->valuestring));

	cJSON_Delete(root);
	free(seedTmpBuf);

	//2
#if 0
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
#endif

	root = NULL;
	root = cJSON_Parse(walletJsonTmpStr);

	if(NULL == root)
	{
		perror("sign_genWalletFile():cJSON_Parse()");
		printf("Failed to parse seed json date\n");
		return -2;
		
	}
	
	x1 = cJSON_GetObjectItem(root, "x1/");
	//cJSON_GetObjectItem(x1, "xpub")->valuestring = (char *)xpub1;
	cJSON_DeleteItemFromObject(x1, "xpub");
	cJSON_AddStringToObject(x1, "xpub", xpub1);
	
	//cJSON_GetObjectItem(x1, "xprv")->valuestring = (char *)xprv1;
	cJSON_DeleteItemFromObject(x1, "xprv");
	cJSON_AddStringToObject(x1, "xprv", xprv1);
	

	x2 = cJSON_GetObjectItem(root, "x2/");
	//cJSON_GetObjectItem(x2, "xpub")->valuestring = (char *)xpub2;
	cJSON_DeleteItemFromObject(x2, "xpub");
	cJSON_AddStringToObject(x2, "xpub", xpub2);
	

	x3 = cJSON_GetObjectItem(root, "x3/");
	//cJSON_GetObjectItem(x3, "xpub")->valuestring = (char *)xpub3;
	cJSON_DeleteItemFromObject(x3, "xpub");
	cJSON_AddStringToObject(x3, "xpub", xpub3);
	
	walletTmpBuf = cJSON_Print(root);

	cJSON_Delete(root);

	//3	
	walletTmpFd = fopen(TMPWALLETFILEPATH, "w");
	if(NULL == walletTmpFd)
	{
		perror("sign_genWalletFile():fopen()");
		printf("Failed to open tmp.wallet\n");
		free(walletTmpBuf);
		return -1;
	}

	nBytes = fprintf(walletTmpFd, "%s", (char *)walletTmpBuf);
	if(nBytes != strlen(walletTmpBuf))
	{
		perror("sign_genWalletFile():fprintf()");
		printf("Failed to write tmp.wallet\n");
		free(walletTmpBuf);
		fclose(walletTmpFd);
		return -1;
	}

	fclose(walletTmpFd);

	free(walletTmpBuf);

	return 0;
}


//
int spcRecvDataToCmdString(char *ucDest, uint32_t iLen, unsigned char *ucSrc, uint32_t serverId)
{
	int ret;
	int iIndex = 0;
	uint16_t index = 0;
	char xpub2Buf[XPUBSAVELEN] = {'\0'};
	memset(xpub2Buf, '\0', XPUBSAVELEN);
	char xpub3Buf[XPUBSAVELEN] = {'\0'};
	memset(xpub3Buf, '\0', XPUBSAVELEN);
	
	ProtocolDataHead *pData = (ProtocolDataHead *)ucSrc;
	memset(ucDest, '\0', iLen);
	
	if(pData->ucStartFlag == 0xba)
	{
		//get index
		uint16_t *ptr = (uint16_t *)&(pData->ucData[0]); 
		memcpy(&index, ptr, sizeof(index));
		printf("spcRecvDataToCmdString():");
		printf("index:%d\n", index);
			
		pData = (ProtocolDataHead *)((unsigned char *)pData + 1 + sizeof(uint16_t) + sizeof(uint16_t)); 
		printf("index:%d\n", index);

	}
	else
	{
		printf("spcRecvDataToCmdString():");
		printf("Failed to find header 0xba.\n");
		return -1;
	}	

	if(pData->ucStartFlag == 0xca)
	{
		uint16_t pub2Len = 0;
		memcpy(&pub2Len, &(pData->ucDataLong), sizeof(uint32_t));
		memcpy(xpub2Buf, &(pData->ucData[0]), pub2Len);
		printf("spcRecvDataToCmdString():");
		printf("Xpub2:%s\n", xpub2Buf);
		pData = (ProtocolDataHead *)((unsigned char*)pData + 1 + sizeof(uint16_t) + (pData->ucDataLong));
		printf("index:%d\n", index);
	
	}
	else
	{
		printf("spcRecvDataToCmdString():");
		printf("Failed to find header 0xca.\n");
		return -1;
	}

	if(pData->ucStartFlag == 0xda)
	{
		uint16_t pub3Len = 0;
		memcpy(&pub3Len, &(pData->ucDataLong), sizeof(uint32_t));
		memcpy(xpub3Buf, &(pData->ucData[0]), pub3Len);
		printf("spcRecvDataToCmdString():");
		printf("Xpub3:%s\n", xpub3Buf);
		pData = (ProtocolDataHead *)(((unsigned char *)pData) + 1 + sizeof(uint16_t) + (pData->ucDataLong));
		printf("index:%d\n", index);
	
	}
	else
	{
		printf("spcRecvDataToCmdString():");
		printf("Failed to find header 0xda.\n");
		return -1;
	}

	printf("DEBUG:INDEX:%d\n", index);
	ret = sign_genWalletFile(serverId, index, xpub2Buf, xpub3Buf);
	if(ret < 0)
	{
		printf("spcRecvDataToCmdString():");
		printf("Failed to generate wallet file\n");
		printf("ret = %d\n", ret);
		return -1;
	}

	if(pData->ucStartFlag == 0xea)
	{

		printf("0xEA ucHexLong = %d\n", (pData->ucDataLong) & 0x7fff);
		//get hex
		//generate cmd
		//python3.6 /mnt/data/Electrum-3.3.3/run_electurm --testnet -w /mnt/data/Electrum-3.3.3/tmp.wallet signtransaction hex
		memcpy(ucDest + iIndex, SIGNCMDHEAD, strlen(SIGNCMDHEAD));
		iIndex += strlen(SIGNCMDHEAD);

		printf("spcRecvDataToCmdString():");
		printf("CMD before adding Hex part:");
		int ii;
		for(ii = 0; ii < iLen; ii++)
		{
			printf("%c", ucDest[ii]);
		}
		printf("\n");
		
		ret = spcHexToString(ucDest + iIndex, &(pData->ucData[0]), (pData->ucDataLong) & 0x7FFF, (pData->ucDataLong) & 0x8000);
		iIndex += ret;
		ucDest[iIndex] = '\0';
		printf("spcRecvDataToCmdString():");
		printf("ret from Hex to String = %d\n", ret);

	}
	else
	{
		printf("spcRecvDataToCmdString:");
		printf("Failed to find header 0xea.\n");
		return -1;
		
	}
	
	return 0;

}



			
int sign_signTransaction(unsigned char *inBuf, uint32_t inLen, unsigned char *outBuf, uint32_t outLen, uint32_t *pDataLen,uint32_t serverId)
{
	ProtocolDataHead *data = (ProtocolDataHead *)inBuf;
				
	if((data->ucStartFlag == 0xa1) && (data->ucData[(data->ucDataLong) - 1] == 0x1a))
	{
		FILE	*sigResult = NULL;
		size_t 	 sSigDataNum;
		char 	*cmdBuf = NULL;
					
		printf("\n\n");
		printf("sign_signTransaction():");
		printf("rawTxDataLen = %d\n", (data->ucDataLong));
		printf("\n\n");
		
		cmdBuf = malloc(inLen*2);
		if(NULL == cmdBuf)
		{
			printf("sign_signTransaction():malloc()");
			printf("Failed to malloc buf for generate sign cmd.\n");
			return -1;
		}
		
		int ret = -1;
		ret = spcRecvDataToCmdString(cmdBuf, inLen*2, data->ucData, serverId);
		if(ret != 0)
		{
			printf("sign_signTransaction():RecvDataToCmdString():");
			printf("Failed to generate sign cmd and wallet file.\n");
			return -1;
		}		
		
		printf("\n\n");
		printf("sign_signtTransaction():");
		printf("CMD : ");
		for(int ii = 0; ii < inLen*2; ii++)
		{
			printf("%c", cmdBuf[ii]);
		}

		printf("\n");
		printf("\n\n");
					
		//printf("CMD:%s\n", (const char *)cmdBuf);	
		printf("line max: %ld\n", sysconf(_SC_LINE_MAX));
		printf("shell max: %ld\n", sysconf(_SC_ARG_MAX));
		system("pwd");
		printf("\n\n");


		sigResult = popen((const char *)cmdBuf, "r");
		if(NULL == sigResult)
		{
			printf("spcProcessRecvData():");
			printf("Filed to execute popen.\n");
			free(cmdBuf);
			return -1;
		}	

		memset(cmdBuf, '\0', inLen*2);
					
		sSigDataNum = fread(cmdBuf, sizeof(char), inLen*2, sigResult);

		printf("\n\n");					
		printf("spcProcessRecvData():");
		printf("sSigDataNum = %ld\n", sSigDataNum);
		printf("\n\n");					

		pclose(sigResult);

		if(spcProcessSigResult(outBuf, outLen, pDataLen, (char *)cmdBuf, sSigDataNum) < 0)
		{
			printf("spcProcessRecvData():");
			printf("Get bad result from spcProcessSigResult().\n");
			free(cmdBuf);
			memset(outBuf, '\0', outLen);
			*pDataLen = 0;
			return -1;
		}							
		
		free(cmdBuf);
					
		printf("\n\n");					
		{
			int iIndex;
			for(iIndex = 0; iIndex < outLen; iIndex++)
			{
				printf("%02x", outBuf[iIndex]);
			}
			printf("\n");

		}
		printf("\n\n");					

	}else
	{
		printf("sign_signTransaction():");
		printf("Get wrong transaction data.\n");
		return -1;
	}
	
	return 0;

}



