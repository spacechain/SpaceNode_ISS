#ifndef SPC_SEED_OPS_H
#define SPC_SEED_OSP_H

#define XPUBLEN		111

//
#define MAKEMAX			10	
#define CMDBUFLEN		200
#define SEEDPATHLEN		150
#define XPUBFILEPATHLEN		100
#define SEEDFILEPATHLEN		100
#define JSONBUFLEN		1024	
#define XKEYBUFLEN		130
#define XPUBSAVELEN		130
#define XPUBLEN			111	
#define SECRETLEN		16
#define IDLENGTHMAX		12

#define UUIDLEN			32


#define XPUBFILEDIR		"/mnt/data/spc/certifile/"
//#define XPUBFILEDIR		"/home/twhite/spc_prepare_mod/spc/certifile/"

#define WALLETFILEDIR		"/mnt/data/spc/wallet/"
//#define WALLETFILEDIR		"/home/twhite/spc_prepare_mod/spc/wallet/"

#define TMPWALLETFILEPATH	"/mnt/data/spc/wallet/wallet.tmp"
//#define TMPWALLETFILEPATH	"/home/twhite/spc_prepare_mod/spc/wallet/wallet.tmp"

#define JSONTMPFILEPATH		"/mnt/data/Electrum-3.3.8_spc_190812/json.tmp"
//#define JSONTMPFILEPATH		"/home/twhite/spc_prepare_mod/spc/wallet/json.tmp"

#define MAKESEEDCMDHEAD		"python3.6 /mnt/data/Electrum-3.3.8_spc_190812/run_electrum createspc --segwit -w /mnt/data/spc/wallet/"
//#define MAKESEEDCMDHEAD		"python3.6 /home/twhite/spc_prepare_mod/Electrum-3.3.8_190812/run_electrum createspc --segwit -w /home/twhite/spc_prepare_mod/spc/wallet/"

#define DOWNLOADDIR	"/mnt/data/spc/download/"
//#define DOWNLOADDIR	"/home/twhite/spc_prepare_mod/spc/download/"



static void _genMakeSeedCmd(char *dest, int len, uint32_t serverId, uint16_t index);

static int _saveXpubToFile(uint32_t serverId, uint16_t fileNo, uint16_t index, char *uuidBuf);

static int _checkStart(uint32_t serverId, uint16_t start);

static int _mvToDownloadDir(uint32_t serverId, uint16_t fileNo, char *uuidBuf);

int seed_makeSeed(uint32_t serverId, uint16_t start, char *uuidBuf, uint32_t uuidLen);

static void _getSeedPath(char *path, int len, uint32_t serverId, uint16_t index);

int seed_genWalletFile(uint32_t serverId, uint16_t index, const char *xpub2, const char *xpub3);

void seed_removeSeed(uint32_t serverId, uint16_t start);



#endif 
