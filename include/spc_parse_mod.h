#ifndef _SPC_PARSE_MOD_H_
#define _SPC_PARSE_MOD_H_

#define CHECKBLOCKLEN	16
#define TXSNUMMAX	10
#define TXIDLEN		32

//data struct
struct tx_file_info
{
	uint32_t server_id;
	uint32_t txs_num;
	uint32_t txs_remain;
};

struct next_tx_info
{
	char tx_id[TXIDLEN];
	uint32_t aes_offset;
	uint32_t aes_length;
};

//util
static int _getCheckBlock(const char *path, uint32_t *pServerId, unsigned char *dest);

//mod
int parse_getCheckBlock(const char *path ,uint32_t *pServerId, unsigned char *dest, int len);

int parse_parseTxFile(char *path);

int parse_getNextTx(char *path, unsigned char *buf, uint32_t len);

int parse_writeToResFile(uint32_t serverId, const unsigned char *resBuf, uint32_t len, char *txPath, char *resPath);

#endif
