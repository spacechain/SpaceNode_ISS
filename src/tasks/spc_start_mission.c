#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>


#include "spc_server_mod.h"
#include "spc_parse_mod.h"
#include "spc_crypt_mod.h"
#include "spc_sign_mod.h"
#include "spc_seed_mod.h"
#include "spc_hotp_mod.h"
#include "spc_fileops_mod.h"

#include "spc_start_mission.h"

//#define TXFILEDIR	"/mnt/data/spc/transaction/txfiles/"
//#define TXFILEDIR	"/home/twhite/spc_prepare_mod/spc/transaction/txfiles/"
//#define RESFILEDIR	"/mnt/data/spc/transaction/resfiles/"
//#define RESFILEDIR	"/home/twhite/spc_prepare_mod/spc/transaction/resfiles/"

#define RAWHOTPLEN	6
#define HOTPLEN		16	
#define SECRETLEN	16	
#define CHECKBLOCKLEN	16	
#define HOTPTIMES	5

#define FILEPATHLEN	100

int start_sign_tx_file(const char *txFilePath, const char *resFilePath)
{
	if(access(txFilePath, F_OK) != 0)
	{
		printf("start_sign_tx_file():");
		printf("Got invalid tx file path.\n");
		return -1;
	}
	
	uint32_t serverId = 0;
	unsigned char checkBlock[CHECKBLOCKLEN] = {'\0'};
	int ret = -1;
	ret = parse_getCheckBlock(txFilePath, &serverId, checkBlock, CHECKBLOCKLEN);
	if(ret != 0)
	{
		printf("start_sign_tx_file():");
		printf("Failed to get check block data.\n");
		return -1;
	}

	struct server_info info;
	memset(&info, '\0', sizeof(struct server_info));
	ret = server_getServerInfo(&info, serverId);
	if(ret != 0)
	{
		printf("start_sign_tx_file():server_getServerInfo():");
		printf("Failed to get server info.\n");
		return -1;
	}

	unsigned char hotp[HOTPLEN+1] = {'\0'};
	unsigned char secret[SECRETLEN+1] = {'\0'};
	unsigned char hotp_array[HOTPTIMES*(HOTPLEN+1)] = {'\0'};
	printf("COUNTER IN LIST:%d\n", info.server_counter);
	memcpy(secret, info.server_secret, SECRETLEN);
	
	int secIndex;
#if 0
	printf("GOT SECRET:");
	for(secIndex = 0; secIndex < 16; secIndex++)
	{
		printf("%c", secret[secIndex]);
	}
	printf("\n");
#endif

	ret = hotp_genNHotp(secret, info.server_counter, HOTPTIMES, hotp_array, (HOTPLEN+1)*HOTPTIMES);
	if(ret < 0)
	{
		printf("start_sign_tx_file():hotp_genNHOTP():");
		printf("Failed to generate hotp array.\n");
		return -1;
	}

#if 1
//debug print
	int rowIndex = -1;
	int colIndex = -1;
	for(rowIndex = 0; rowIndex < HOTPTIMES; rowIndex++)
	{
		for(colIndex = 0; colIndex < HOTPLEN; colIndex++)
		{
			printf("%c", hotp_array[rowIndex*(HOTPLEN+1) + colIndex]);
		}
		printf("\n");
	}

#endif

	uint32_t deServerId = 0;
	unsigned char deCheckBlock[CHECKBLOCKLEN] = {'\0'};
	uint32_t counterIndex = 0;
	
	for(counterIndex = 0; counterIndex < HOTPTIMES; counterIndex++)
	{
		ret = crypt_decryptData(checkBlock, CHECKBLOCKLEN, deCheckBlock, CHECKBLOCKLEN, hotp_array+counterIndex*(HOTPLEN+1), HOTPLEN+1);
		if(ret != 0)
		{
			printf("start_sign_tx_file():crypt_decryptData():");
			printf("Failed to decrypt check block data.\n");
			return -1;
		}

		memcpy(&deServerId, deCheckBlock, sizeof(uint32_t));
		printf("deServerId:%d\n", deServerId);
		if(deServerId == serverId)
		{
			memcpy(hotp, hotp_array+counterIndex*(HOTPLEN+1), HOTPLEN);
			info.server_counter += (counterIndex+1);
			printf("\nCOUNTER NOW:%d\n", info.server_counter);
			break;
		}
		else
		{
			continue;
		}

	}
	
	extern struct next_tx_info nextTx; 
	extern struct tx_file_info curFile;
	
	if(counterIndex == HOTPTIMES)
	{
		printf("Wrong HOTP passwd, stop ops.\n");
		ret = parse_parseTxFile((char *)txFilePath);		
		if(ret != 0)
		{
			printf("start_sign_tx_file():parse_parseTxFile():");
			printf("Failed to parse tx file.\n");
			return -1;
		}

		while(1)
		{
			if((curFile.txs_remain == 0) && (nextTx.aes_offset == 0) && (nextTx.aes_length == 0))
			{
				printf("All void Tx result written.\n");
				break;
			}
			
			ret = parse_writeToResFile(curFile.server_id, NULL, 0, (char *)txFilePath, (char *)resFilePath);
			if(ret != 0)
			{
				printf("start_sign_tx_file():parse_writeResFile():");
				printf("Failed to write No.%d tx result(hotp check failed) to result file.", (curFile.txs_num + 1 - curFile.txs_remain));
				return -1;
			}		
			
		}

		return 0;

	}


	ret = parse_parseTxFile((char *)txFilePath);
	if(ret != 0)
	{
		printf("start_sign_tx_file():parse_parseTxFile():");
		printf("Failed to parse tx file.\n");
		return -1;
	}

	unsigned char *pAesTxBuf = NULL;
	unsigned char *pRawTxBuf = NULL;
	unsigned char *pResTxBuf = NULL;
	unsigned char *pTempTxBuf = NULL;

	int flag = 0;
	while(1)
	{
		if((curFile.txs_remain == 0) && (nextTx.aes_offset == 0) && (nextTx.aes_length == 0))
		{
			printf("start_sign_tx_file():parse_parseTxFile():");
			printf("All Tx signed.\n");
			break;
		}
	
		pAesTxBuf = malloc(nextTx.aes_length + 100);
		if(NULL == pAesTxBuf)
		{
			perror("start_sign_tx_file():malloc()");
			printf("Failed to malloc buf for aes tx data.\n");
			return -1;
		}
		memset(pAesTxBuf, '\0', nextTx.aes_length + 100);
	
		pRawTxBuf = malloc(nextTx.aes_length + 100);
		if(NULL == pRawTxBuf)
		{
			perror("start_sign_tx_file():malloc()");
			printf("Failed to malloc buf for raw tx data.\n");
			free(pAesTxBuf);
			return -1;
		}
		memset(pRawTxBuf, '\0', nextTx.aes_length + 100);
	
		ret = parse_getNextTx((char *)txFilePath, pAesTxBuf, nextTx.aes_length + 10);
		if(ret != 0)
		{
			printf("start_sign_tx_file():parse_getNextTx():");
			printf("Failed to get No.%d tx aes data.\n", (curFile.txs_num + 1 - curFile.txs_remain));
			free(pAesTxBuf);
			free(pRawTxBuf);
			return -1;
		}

		ret = crypt_decryptData(pAesTxBuf, nextTx.aes_length, pRawTxBuf, nextTx.aes_length, hotp, 17);
		if(ret != 0)
		{
			printf("start_sign_tx_file():crypt_decryptData():");
			printf("Failed to decrypt No.%d tx aes data.\n", (curFile.txs_num + 1 - curFile.txs_remain));
			free(pAesTxBuf);
			free(pRawTxBuf);
			return -1;
		}

		int rawIndex = 0;
		printf("start_sign_tx_file():parse_getNextTx():");
		printf("No.%d raw data :\n", (curFile.txs_num + 1 - curFile.txs_remain));
		printf("aes_length:%d\n", nextTx.aes_length);
		for(rawIndex = 0; rawIndex < nextTx.aes_length; rawIndex++)
		{
			printf("%02x", pRawTxBuf[rawIndex]);

		}
		printf("\n");
	
		free(pAesTxBuf);

		pResTxBuf = malloc(nextTx.aes_length + 100);
		if(NULL == pResTxBuf)
		{
			perror("start_sign_tx_file():malloc()");
			printf("Failed to malloc buf for res tx data.\n");
			free(pAesTxBuf);
			free(pRawTxBuf);
			return -1;
		}
		memset(pResTxBuf, '\0', nextTx.aes_length + 100);


		uint32_t resDataLen = 0;
		ret = sign_signTransaction(pRawTxBuf, nextTx.aes_length, pResTxBuf, nextTx.aes_length+100, &resDataLen, curFile.server_id);
		if(ret != 0)
		{
			printf("start_sign_tx_file():sign_signTransaction():");
			printf("Failed to sign No.%d tx.\n", (curFile.txs_num + 1 - curFile.txs_remain));
			resDataLen = 0;	
			flag = 1;
		}

		free(pRawTxBuf);
		
		int i;
		printf("BEFORE PARSE_WRITETORESFILE:\n");
		for(i = 0; i < resDataLen; i ++)
		{
			printf("%02x", pResTxBuf[i]);
		} 
		printf("\n");
		
		if(flag)
		{
			pTempTxBuf = NULL;			
		}
		else
		{
			pTempTxBuf = pResTxBuf;
		}

		ret = parse_writeToResFile(curFile.server_id, pTempTxBuf, resDataLen, (char *)txFilePath, (char *)resFilePath);
		if(ret != 0)
		{
			printf("_sign_tx_file():parse_writeToResFile():");
			printf("Failed to write No.%d tx result to result file.", (curFile.txs_num + 1 - curFile.txs_remain));
			free(pResTxBuf);
			return -1;
		}		
		free(pResTxBuf);

	}

#if 0
	ret = server_setServerInfo(&info);
	if(ret != 0)
	{
		printf("_sign_tx_file():server_setServerInfo():");
		printf("Failed to update server_counter");
		return -1;
	}	

#endif	
	return 0;
}


void spc_start_mission(void)
{
	//check tx file dir and res file dir
	int ret = -1;
#if 1
	if(access(TXFILEDIR, F_OK) != 0)
	{
		printf("Failed to detect tx files dir, please init server first.\n");
		return;
	}

	if(access(RESFILEDIR, F_OK) != 0)
	{
		printf("Failed to detect res files dir, please init server first.\n");
		return;
	}
#endif
	while(1)
	{
		//get tx file path
		char txFilePath[FILEPATHLEN] = {'\0'};	
		memset(txFilePath, '\0', FILEPATHLEN);
		char resFilePath[FILEPATHLEN] = {'\0'};
		memset(resFilePath, '\0', FILEPATHLEN);
		char downloadFilePath[FILEPATHLEN] = {'\0'};	
		memset(downloadFilePath, '\0', FILEPATHLEN);
		ret = fileops_getNextTxFile(TXFILEDIR, txFilePath, FILEPATHLEN);
		if(ret != 0)
		{
			printf("All tx file done.\n");
			break;
		}
		
		printf("\nNEXT TX FILE PATH:%s\n", txFilePath);

		//gen res file path
		ret = fileops_genResFilePath(RESFILEDIR, txFilePath, resFilePath, FILEPATHLEN);
		if(ret != 0)
		{
			printf("Failed to generate res file path.\n");
			return;
		}

		//gen download file path
		ret = fileops_genDownloadFilePath(DOWNLOADFILEDIR, txFilePath, downloadFilePath, FILEPATHLEN);
		if(ret != 0)
		{
			printf("Failed to generate download file path.\n");
			return;
		}

		//sign tx file
		ret = start_sign_tx_file(txFilePath, resFilePath);
		if(ret != 0)
		{
			printf("\n");
			printf("/////////////////////////////////\n");
			printf("Failed to sign tx file:\n");
			printf("%s\n", txFilePath);
			printf("Move to next tx file now.\n");
			printf("/////////////////////////////////\n");
			printf("\n");
			if(access(resFilePath, F_OK) == 0)
			{
				if(rename(resFilePath, downloadFilePath) != 0)
				{
					printf("Failed to move res file to download directory.\n");
				}

			}


			//rename error file
		#if 1
			//rm tx file
			ret = fileops_rmTxFile(txFilePath);
			if(ret != 0)
			{
				printf("Failed to remove tx file.\n");
				//return; 
			}
		#endif
			continue;
		}
		else if(ret == 0)
		{
			printf("\n");
			printf("/////////////////////////////////\n");
			printf("Signed tx file:\n");
			printf("%s\n", txFilePath);
			printf("Move to next tx file now.\n");
			printf("/////////////////////////////////\n");
			printf("\n");
			if(access(resFilePath, F_OK) == 0)
			{
				if(rename(resFilePath, downloadFilePath) != 0)
				{
					printf("Failed to move res file to download directory.\n");
				}

			}
		#if 1
			//rm tx file
			ret = fileops_rmTxFile(txFilePath);
			if(ret != 0)
			{
				printf("Failed to remove tx file.\n");
				//return; 
			}
		#endif
			continue;
		}

		//loop
	}	
	return;
}



#if 0
int main(int argc, char **argv)
{
	spc_start_mission();
	return 0;
}
#endif
