#ifndef _SPC_SECRET_MOD_H_
#define _SPC_SECRET_MOD_H_
#include <stdio.h>
#include <string.h>

#define CMDHEADER	"python3.6 /mnt/data/Electrum-3.3.8_spc_190812/run_electrum encrypt "
//#define CMDHEADER	"python3.6 /home/twhite/spc_prepare_mod/Electrum-3.3.8_190812/run_electrum encrypt "

#define SECRETTEMPDIR	"/mnt/data/spc/secret/temp/"
//#define SECRETTEMPDIR	"/home/twhite/spc_prepare_mod/spc/secret/temp/"

#define DOWNLOADDIR	"/mnt/data/spc/download/"
//#define DOWNLOADDIR	"/home/twhite/spc_prepare_mod/spc/download/"

#define TEMPSECFILEPATH	"/mnt/data/spc/secret/temp/temp.sec"
//#define TEMPSECFILEPATH	"/home/twhite/spc_prepare_mod/spc/secret/temp/temp.sec"

#define UUIDLEN	32

//
int _encryptSecret(char *pubkey1Buf, uint32_t pubkey1Len, \
		   	char *pubkey2Buf, uint32_t pubkey2Len, \
				char *secretBuf, uint32_t secretLen, \
					char **retBuf, uint32_t *retLen);


//
void _freeDmsg(char *pEncryptMsg);

static int _writeDmsgToTempFile(char *uuidBuf, uint32_t uuidBufLen, uint32_t serverId, char *dMsg, uint32_t dMsgLen);

static int _checkTempSec();

static int _mvTempSecToDownloadDir(char *uuidBuf, uint32_t uuidBufLen, uint32_t serverId);

int secret_genSecFile(char *pubkey1Buf, uint32_t pubkey1Len, \
		      char *pubkey2Buf, uint32_t pubkey2Len, \
		      char *secretBuf, uint32_t secretLen, \
		      char *uuidBuf, uint32_t uuidBufLen, \
	              uint32_t serverId);



#endif
