#ifndef SPC_TRANSMIT_LAYER_H
#define SPC_TRANSMIT_LAYER_H
#include <stdio.h>

#define SIGNXPUBLEN		107
#define IDLENGTHMAX		12	
#define MAKEMAX			10	
#define CMDBUFLEN		200
#define SEEDPATHLEN		150
#define XPUBFILEPATHLEN		100
#define SEEDFILEPATHLEN		100
#define JSONBUFLEN		1024	
#define XKEYBUFLEN		130
#define XPUBSAVELEN		130

#define XPUBFILEDIR		"/mnt/data/spc/certifile/"
//#define XPUBFILEDIR		"/home/twhite/spc_prepare_mod/spc/certifile/"

#define WALLETFILEDIR		"/mnt/data/spc/wallet/"
//#define WALLETFILEDIR		"/home/twhite/spc_prepare_mod/spc/wallet/"

#define TMPWALLETFILEPATH	"/mnt/data/spc/wallet/wallet.tmp"
//#define TMPWALLETFILEPATH	"/home/twhite/spc_prepare_mod/spc/wallet/wallet.tmp"

#define JSONTMPFILEPATH		"/mnt/data/Electrum-3.3.8_spc_190812/json.tmp"
//#define JSONTMPFILEPATH		"/home/twhite/spc_prepare_mod/spc/Electrum-3.3.8_190812/json.tmp"

#define MAKESEEDCMDHEAD		"python3.6 /mnt/data/Electrum-3.3.8_spc_190812/run_electrum createspc --segwit -w /mnt/data/spc/wallet/"
//#define MAKESEEDCMDHEAD		"python3.6 /home/twhite/spc_prepare_mod/Electrum-3.3.8_190812/run_electrum createspc --segwit -w /home/twhite/spc_prepare_mod/spc/wallet/"

#define SIGNCMDHEAD		"python3.6 /mnt/data/Electrum-3.3.8_spc_190812/run_electrum -w /mnt/data/spc/wallet/wallet.tmp signtransaction "
//#define SIGNCMDHEAD		"python3.6 /home/twhite/Electrum-3.3.8_190812/run_electrum -w /home/twhite/spc_prepare_mod/spc/wallet/wallet.tmp signtransaction "

#define XPUBHEADER		"Zpub"


typedef struct {
	uint8_t 	ucStartFlag;
	uint16_t 	ucDataLong;
	uint8_t		ucData[1];
} __attribute__((packed)) ProtocolDataHead;



int sign_signTransaction(unsigned char *inBuf, uint32_t inLen, unsigned char *outBuf, uint32_t outLen, uint32_t *pDataLen, uint32_t serverId);



#endif
