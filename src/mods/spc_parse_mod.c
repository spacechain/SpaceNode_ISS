#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "spc_parse_mod.h"


struct tx_file_info curFile;

struct next_tx_info nextTx;


//
static int _getCheckBlock(const char *path, uint32_t *pServerId, unsigned char *dest)
{
	//open file
	int fd = -1;
	fd = open(path, O_RDONLY);
	if(fd < 0)
	{
		perror("_getCheckBlock():open()");
		printf("Failed to open tx file.\n");
		return -1;
	}

	//get id
	int ret = -1;
	ret = read(fd, pServerId, sizeof(uint32_t));
	if(ret != sizeof(uint32_t))
	{
		perror("_getCheckBlock():read()");
		printf("Failed to read server id from tx file.\n");
		close(fd);
		return -1;
	}

	//get check
	ret = read(fd, dest, CHECKBLOCKLEN);
	if(ret != CHECKBLOCKLEN)
	{
		perror("_getCheckfBlock():read()");
		printf("Failed to read check block from tx file.\n");
		close(fd);
		return -1;
	}

	//close
	close(fd);
	
	return 0;
}

int parse_getCheckBlock(const char *path, uint32_t *pServerId, unsigned char *dest, int len)
{
	if((NULL == path) || (access(path, F_OK) != 0) || (NULL == pServerId) || (NULL == dest) || (len < 16))
	{
		printf("parse_getCheckBlock():");
		printf("Get bad input.\n");
		return -1;
	}

	int ret = -1;
	ret = _getCheckBlock(path, pServerId, dest);
	if(ret != 0)
	{
		printf("parse_getCheckBlock():_getCheckBlock():");
		printf("Failed to get check block data.\n");
		return -1;
	}	

	return 0;
}

static int _parseInit(char *path)
{
	//clear
	memset(&curFile, '\0', sizeof(struct tx_file_info));
	memset(&nextTx, '\0', sizeof(struct next_tx_info));

	//get tx_file
	int fd = -1;
	fd = open(path, O_RDONLY);
	if(fd < 0)
	{
		perror("_parseInit():open()");
		printf("Failed to open tx file.\n");
		return -1;
	}
	
	int ret = -1;
	ret = read(fd, &(curFile.server_id), sizeof(uint32_t));
	if(ret != sizeof(uint32_t))
	{
		perror("_parseInit():read()");
		printf("Failed to read server id from file.\n");
		close(fd);
		return -1;
	}

	ret = lseek(fd, sizeof(uint32_t)+CHECKBLOCKLEN, SEEK_SET);
	if(ret != (sizeof(uint32_t)+CHECKBLOCKLEN))
	{
		perror("_parseInit():lseek()");
		printf("Failed to move to txs_num pos in file.\n");
		close(fd);
		return -1;
	}

	ret = read(fd, &(curFile.txs_num), sizeof(uint32_t));
	if((ret != sizeof(uint32_t)) || (curFile.txs_num > TXSNUMMAX))
	{
		printf("DEBUG:READRET:%d\n", ret);
		printf("DEBUG:TXS_NUM:%d\n", curFile.txs_num);
		perror("_parseInit():read()");
		printf("Failed to read tx_num from tx file.\n");
		close(fd);
		return -1;
	}
	
	curFile.txs_remain = curFile.txs_num;
	
	ret = lseek(fd, sizeof(uint32_t)+CHECKBLOCKLEN+sizeof(uint32_t)+sizeof(uint32_t), SEEK_SET);
	if(ret != (sizeof(uint32_t)+CHECKBLOCKLEN+sizeof(uint32_t)+sizeof(uint32_t)))
	{
		perror("_parseInit():lseek()");
		printf("Failed to move to tx_id pos in file.\n");
		close(fd);
		return -1;
	}


	//get next_tx
	ret = read(fd, &(nextTx.tx_id), TXIDLEN);
	if(ret != TXIDLEN)
	{
		perror("_parseInit():read()");
		printf("Failed to read tx_id from tx file.\n");
		close(fd);
		return -1;
	}	

	uint32_t offset = sizeof(uint32_t) + CHECKBLOCKLEN + sizeof(uint32_t) + sizeof(uint32_t) + TXIDLEN + sizeof(uint32_t);
	nextTx.aes_offset = offset;
	
	ret = read(fd, &(nextTx.aes_length), sizeof(uint32_t));
	if(ret != sizeof(uint32_t))
	{
		perror("_parseInit():read()");
		printf("Failed to read aes_length from tx file.\n");
		close(fd);
		return -1;
	}
	
	close(fd);
	return 0;
}


//
int parse_parseTxFile(char *path)
{
	//check
	if((NULL == path) || (access(path, F_OK) != 0))
	{
		perror("parse_parseTxFile()");
		printf("Get invalid input.\n");
		return -1;
	}

	int ret = -1;
	ret = _parseInit(path);
	if(ret != 0)
	{
		printf("parse_parseTxFile():_parseInit()");
		printf("Failed to excute parse init.\n");
		return -1;
	}

	return 0;
}

static int _getAesData(char *path, unsigned char *buf, uint32_t len)
{
	if((NULL == buf) || (nextTx.aes_length > len))
	{
		printf("_getAesData():");
		printf("Get invalid input.\n");
		return -1;
	}

	int fd = -1;
	fd = open(path, O_RDONLY);
	if(fd < 0)
	{
		perror("_getAesData():open()");
		printf("Failed to open tx file.\n");
		return -1;
	}

	int ret = -1;
	ret = lseek(fd, nextTx.aes_offset, SEEK_SET);
	if(ret != nextTx.aes_offset)
	{
		perror("_getAesData():lseek()");
		printf("Failed to move to aes data pos in tx file.\n");
		close(fd);
		return -1;
	}
	
	ret = read(fd, buf, nextTx.aes_length);
	if(ret != nextTx.aes_length)
	{
		perror("_getAesData():read()");
		printf("Failed to read aes data from tx file.\n");
		close(fd);
		return -1;
	}
	
	close(fd);
	return 0;
}

static int _moveToNextTx(char *path)
{
	//open file 
	int fd = -1;
	fd = open(path, O_RDONLY);
	if(fd < 0)
	{
		perror("_moveToNextTx():open()");
		printf("Failed to open Tx file.\n");
		return -1;
	}	

	//read tx_id of next transaction
	int ret = -1;
	uint32_t offset = (nextTx.aes_offset) + (nextTx.aes_length);
	ret = lseek(fd, offset, SEEK_SET);
	if(ret != offset)
	{
		perror("_moveToNextTx():lseek()");
		printf("Failed to move to tx_id pos of next transaction.\n");
		close(fd);
		return -1;
	}
	ret = read(fd, &(nextTx.tx_id), TXIDLEN);
	if(ret != TXIDLEN)
	{
		perror("_moveToNextTx():read()");
		printf("Failed to read tx id from tx file.\n");
		close(fd);
		return -1;
	}

	offset += TXIDLEN;	

	//read length of next transaction
	ret = lseek(fd, offset, SEEK_SET);
	if(ret != offset)
	{
		perror("_moveToNextTx():lseek()");
		printf("Failed to move to length pos of next transaction.\n");
		close(fd);
		return -1;
	}

	nextTx.aes_length = 0;
	ret = read(fd, &(nextTx.aes_length), sizeof(uint32_t));
	if(ret != sizeof(uint32_t))
	{
		perror("_moveToNextTx():read()");
		printf("Failed to read length of next transaction.\n");
		close(fd);
		return -1;
	}
	
	offset += sizeof(uint32_t);

	close(fd);

	nextTx.aes_offset = offset;

	return 0;
}


int parse_getNextTx(char *path, unsigned char *buf, uint32_t len)
{
	if(curFile.txs_remain == 0)
	{
		printf("parse_getNextTx():");
		printf("No more transaction to be gotten.\n");
		return -1;
	}

	//check
	if((NULL == path) || (access(path, F_OK) != 0) || (NULL == buf))
	{
		printf("parse_getNextTx():");
		printf("Get invalid input.\n");
		return -1;
	}
	
	//get aes data
	int ret = -1;
	ret = _getAesData(path, buf, len);
	if(ret != 0)
	{
		printf("parse_getNextTx():_getAesData():");
		printf("Failed to get aes data from file.\n");
		return -1;
	}	

	return 0;	
}

//
int parse_writeToResFile(uint32_t serverId, const unsigned char *resBuf, uint32_t len, char *txPath, char *resPath)
{
	//check input
	if(len != 0)
	{ 
		if((NULL == resBuf) || (NULL == txPath) || (access(txPath, F_OK) != 0) || (NULL == resPath))
		{
			printf("parse_writeToResFile():");
			printf("Get invalid inputs.\n");
			return -1;
		}
	}
	else
	{
		if((NULL == txPath) || (access(txPath, F_OK) != 0)  || (NULL == resPath))
		{
			printf("parse_writeToResFile():");
			printf("Get invalid inputs.\n");
			return -1;
		}
	}
	
	//if access()
	int fd = -1;
	int ret = -1;
	uint64_t length = 0;
	uint32_t tx_res_num = 0;
	if(access(resPath, F_OK) == -1)
	{
		//creat
		fd = open(resPath, O_RDWR | O_CREAT, 0666);
		if(fd < 0)
		{
			perror("parse_writeToResFile():open()");
			printf("Failed to open result file.\n");
			return -1;
		}
	
		ret = write(fd, &serverId, sizeof(uint32_t));
		if(ret != sizeof(uint32_t))
		{
			perror("parse_writeToResFile():write()");
			printf("Failed to write server id to result file.\n");
			close(fd);
			return -1;
		}

		ret = write(fd, &length, sizeof(uint64_t));
		if(ret != sizeof(uint64_t))
		{
			perror("parse_writeToResFile():write()");
			printf("Failed to write length to result file.\n");
			close(fd);
			return -1;
		}
	
		ret = write(fd, &tx_res_num, sizeof(uint32_t));
		if(ret != sizeof(uint32_t))
		{
			perror("parse_writeToResFile():write()");
			printf("Failed to write tx_res_num to result file.\n");
			close(fd);
			return -1;
		}		
	
	}
	else if(access(resPath, F_OK) == 0)
	{
		fd = open(resPath, O_RDWR | O_APPEND);
		if(fd < 0)
		{
			perror("parse_writeToResFile():open()");
			printf("Failed to open result file.\n");
			return -1;
		}
	}
	
	//write file
	//write id
	ret = write(fd, &(nextTx.tx_id), TXIDLEN);
	if(ret != TXIDLEN)
	{
		perror("parse_writeToResFile():write()");
		printf("Failed to write tx id into result file.\n");
		close(fd);
		return -1;
	}

	//write len
	ret = write(fd, &len, sizeof(uint32_t));
	if(ret != sizeof(uint32_t))
	{
		perror("parse_writeToResFile():write()");
		printf("Failed to write tx_res_length into result file.\n");
		close(fd);
		return -1;
	}

	//write res data
	if(len != 0)
	{
		ret = write(fd, resBuf, len);
		if(ret != len)
		{
			perror("parse_writeToResFile():write()");
			printf("Failed to write result data to result file.\n");
			close(fd);
			return -1;
		}	
	}
	close(fd);

	fd = -1;
	fd = open(resPath, O_RDWR);
	if(fd < 0)
	{
		perror("parse_writeToResFile():open()");
		printf("Failed to open resFile for update length and txs_num data.\n");
		return -1;
	}

	uint32_t updateOffset = 0;
	updateOffset = sizeof(uint32_t);
	lseek(fd, updateOffset, SEEK_SET);
	ret = read(fd, &length, sizeof(uint64_t));
	if(ret != sizeof(uint64_t))
	{
		perror("parse_writeToResFile():read()");
		printf("Failed to read length data from result file.\n");
		close(fd);
		return -1;
	}

	length =length + len + TXIDLEN + sizeof(uint32_t);
	lseek(fd, updateOffset, SEEK_SET);
	ret = write(fd, &length, sizeof(uint64_t));
	if(ret != sizeof(uint64_t))
	{
		perror("parse_writeToResFile():write()");
		printf("Failed to write length data into result file.\n");
		close(fd);
		return -1;
	}
	
	updateOffset += sizeof(uint64_t);
	lseek(fd, updateOffset, SEEK_SET);
	ret = read(fd, &tx_res_num, sizeof(uint32_t));
	if(ret != sizeof(uint32_t))
	{
		perror("parse_writeToResFile():read()");
		printf("Failed to read tx_res_num data from result file.\n");
		close(fd);
		return -1;
	}
	tx_res_num ++;

	lseek(fd, updateOffset, SEEK_SET);
	ret = write(fd, &tx_res_num, sizeof(uint32_t));
	if(ret != sizeof(uint32_t))
	{
		perror("parse_writeToResFile():write()");
		printf("Failed to write tx_res_num data into result file.\n");
		close(fd);
		return -1;
	}
	
	close(fd);

	//update
	//update static 
	curFile.txs_remain += -1;

	if(curFile.txs_remain == 0)
	{
		memset(nextTx.tx_id, '\0', TXIDLEN);
		nextTx.aes_offset = 0;
		nextTx.aes_length = 0;
	}
	else if(curFile.txs_remain > 0)
	{
		//update next tx info 
		ret = _moveToNextTx(txPath);
		if(ret != 0)
		{
			printf("parse_writeTofResFile():_moveToNextTx()");
			printf("Failed to update nextTx.\n ");
			return -1;
		}
	}
	else
	{
		printf("parse_writeToResFile():");
		printf("Got invalid remain value.\n");
		return -1;
	}
	return 0;
}


#if 0
//test main
int main(int argc, char **argv)
{
	uint32_t serverId = 0;
	unsigned char buf[17] = {'\0'};

	unsigned char aesBuf[7000] = {'\0'};

		
	parse_getCheckBlock("./tx.file", &serverId, buf, 17);
	printf("server id : %d\n", serverId);
	printf("checkblock:\n");
	int i;
	for(i = 0; i < 17; i++)
	{
		printf("%02x", buf[i]);
	}
	printf("\n");

	parse_parseTxFile("./tx.file");

	printf("tx_file_info:\n");
	printf("server_id: %d\n", curFile.server_id);
	printf("txs_num: %d\n", curFile.txs_num);
	printf("txs_remain: %d\n", curFile.txs_remain);

	printf("next_tx_info:\n");
	printf("tx_id:");
	for(i = 0; i < 32; i++)
	{
		printf("%c", nextTx.tx_id[i]);
	}
	printf("\n");

	printf("aes_offset: %d\n", nextTx.aes_offset);
	printf("aes_length: %d\n", nextTx.aes_length);

	int j = 0;
	while(parse_getNextTx("./tx.file", aesBuf, 7000) == 0)
	{
	#if 0
		for(i = 0, j = 1; i < 7000; i++, j++)
		{
			printf("%02x", aesBuf[i]);
			if((j % 16) == 0)
				printf("\n");
		}
		printf("\n");
	#endif
		parse_writeToResFile(aesBuf, 1500, "./tx.file", "./res.file");
		
		printf("tx_file_info:\n");
		printf("server_id: %d\n", curFile.server_id);
		printf("txs_num: %d\n", curFile.txs_num);
		printf("txs_remain: %d\n", curFile.txs_remain);

		printf("next_tx_info:\n");
		printf("tx_id:");
		for(i = 0; i < 32; i++)
		{
			printf("%c", nextTx.tx_id[i]);
		}
		printf("\n");

		printf("aes_offset: %d\n", nextTx.aes_offset);
		printf("aes_length: %d\n", nextTx.aes_length);

	
	}

	return 0;
}

#endif


