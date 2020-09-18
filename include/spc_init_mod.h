#ifndef _SPC_INIT_MOD_H_
#define _SPC_INIT_MOD_H_
#include <stdint.h>

//stage change
#define STAGEFILEPATH	"/mnt/data/spc/node/node.stage"
//#define STAGEFILEPATH	"/home/twhtie/spc_prepare_mod/spc/node/node.stage"


//stage value
//1	ready to exchange pubkey
//2	ready to perform check stage 1
//3	ready to perform chack stage 2
//4	initiated 

#define INITSTAGEVALUE	1

#define TEMPEPUBFILEPATH	"/mnt/data/spc/node/temp/epub.temp"
//#define TEMPEPUBFILEPATH	"/home/twhite/spc_prepare_mod/spc/node/temp/epub.temp"

#define EPUBFILEPATH	"/mnt/data/spc/node/epub.file"
//#define EPUBFILEPATH	"/home/twhite/spc_prepare_mod/spc/node/epub.file"

#define CREATENEWADDRCMD	"python3.6 /mnt/data/Electrum-3.3.8_spc_190812/run_electrum createnewaddress -w /mnt/data/spc/node/node.ewallet"
//#define CREATENEWADDRCMD	"python3.6 /home/twhite/spc_prepare_mod/Electrum-3.3.8_190812/run_electrum createnewaddress -w /home/twhite/spc_prepare_mod/spc/node/node.ewallet"

#define GETPUBKEYSCMDHEADER	"python3.6 /mnt/data/Electrum-3.3.8_spc_190812/run_electrum getpubkeys -w /mnt/data/spc/node/node.ewallet "	
//#define GETPUBKEYSCMDHEADER	"python3.6 /home/twhite/spc_init_mod/Electrum-3.3.8_190812/run_electrum getpubkeys -w /home/twhite/spc_prepare_mod/spc/node/node.ewallet "	

#define SPCEPUBFILETAIL	".spcepub"

#define UPLOADDIR	"/mnt/data/spc/upload/"
//#define UPLOADDIR	"/home/twhite/spc_prepare_mod/spc/upload/"

#define DOWNLOADDIR	"/mnt/data/spc/download/"
//#define DOWNLOADDIR	"/home/twhite/spc_prepare_mod/spc/download/"

#define NODETEMPDIR	"/mnt/data/spc/node/temp/"
//#define NODETEMPDIR	"/home/twhite/spc_prepare_mod/spc/node/temp/"

#define SPCEPUBFILETEMPPATH	"/mnt/data/spc/node/temp/spc.epub"
//#define SPCEPUBFILETEMPPATH	"/home/twhite/spc_prepare_mod/spc/node/temp/spc.epub"

#define UUIDLEN	32	

#define TEMPSATEPUBFILEPATH	"/mnt/data/spc/node/temp/temp.satepub"
//#define TEMPSATEPUBFILEPATH	"/home/twhite/spc_prepare_mod/spc/node/temp/temp.satepub"

#define SATEPUBFILETAIL	".satepub"



//stage change
static int _checkStage(void);

static int _validStage(uint32_t stage);

static int _initStage(void);

static int _setStage(uint32_t stage);

static int _getStage(uint32_t *pStage);

static int _removeStage(void);

static int _pushStage(uint32_t stagePrv, uint32_t *stageNext);

//
int init_checkStage(void);

//
int init_initStage(void);

//
int init_getStage(uint32_t *pStage);

//
int init_pushStage(void);


//exchagnge pubkey

//sat epub
static int _checkEpubFile();

static char *_runCreateNewAddr(uint32_t *pRetLen);

static void _freeAddrRet(char *pAddrRet);

static char *_parseAddrRet(char *addrRet, uint32_t addrRetLen, uint32_t *pAddrLen);

static void _freeAddr(char *pAddr);

static char *_runGenEpub(char *addr, uint32_t addrLen, uint32_t *pEpubRetLen);

static void _freeEpubRet(char *pEpubRet);

static char *_parseEpubRet(char *ePubRet, uint32_t ePubRetLen, uint32_t *pEpubLen);

static void _freeEpub(char *pEpub);

static int _saveSatEpubToTempFile(char *satEpubBuf, uint32_t satEpubLen);

//spc epub
static char *_findSpcEpubFile(char *dirPath, uint32_t *retNameLen);

static void _freeSpcEpubFileName(char *pName);

static int _countSpcEpubFile(char *dirPath);

static int _mvSpcEpubFileToTempDir(char *fileName, uint32_t fileNameLen);

static int _checkSpcEpubFileInTempDir(void);

static int _parseSpcEpubFile(char *uuidBuf, uint32_t uuidBufLen, char **ppSpcEpub, uint32_t *pSpcEpubLen);

static void _freeSpcEpub(char *pSpcEpub);

//epub.file
static int _saveSpcEpubToTempFile(char *spcEpubBuf, uint32_t spcEpubLen);

static int _mvEpubTempToEpubFile(void);

static int _rmEpubFile(void);

//gen xxxx.satepub
static int _genTempSatEpubFile(char *uuid, uint32_t uuidLen, char *satEpub, uint32_t satEpubLen);

static int _genUuidSatEpubFile(char *uuid, uint32_t uuidLen);

//
//
int init_checkSpcEpubFile(void);

int init_exchangeEpub(void);




//check stage 1
#define SPCCS1FILETAIL	".spccs1"

#define SPCCS1FILEPATH	"/mnt/data/spc/node/temp/spc.cs1"
//#define SPCCS1FILEPATH	"/home/twhite/spc_init_node/node/temp/spc.cs1"

#define ENCRYPTCMDHEADER	"python3.6 /mnt/data/Electrum-3.3.8_spc_190812/run_electrum encrypt "
//#define ENCRYPTCMDHEADER	"python3.6 /home/twhite/spc_init_node/Electrum-3.3.8_190812/run_electrum encrypt "

#define SECRET	"issnode"

#define MSPCDATAFILEPATH	"/mnt/data/spc/node/mspc.data"
//#define MSPCDATAFILEPATH	"/home/twhite/spc_init_node/node/mspc.data"

#define TEMPSATCS1FILEPATH	"/mnt/data/spc/node/temp/temp.satcs1"
//#define TEMPSATCS1FILEPATH	"/home/twhite/spc_init_node/node/temp/temp.satcs1"

#define SATCS1FILETAIL	".satcs1"

static int _countUuidSpcCs1File(char *dirPath);

static char *_findUuidSpcCs1File(char *dirPath, uint32_t *pFileNameLen);

static void _freeUuidSpcCs1FileName(char *pFileName);

static int _mvUuidSpcCs1ToSpcCs1(char *fileName, uint32_t fileNameLen);

static int _checkSpcCs1();

static char *_getSpcEpub(uint32_t *pSpcEpubLen);

char * init_getSpcEpub(uint32_t *pSpcEpubLen);

static void _freeSpcEpub(char *pSpcEpub);

static char *_runEncrypt(char *spcEpub, uint32_t spcEpubLen, uint32_t *pEmsgRetLen);

static void _freeMspcRet(char *pMspcRet);

static char *_parseMspcRet(char *mSpcRet, uint32_t mSpcRetLen, uint32_t *mSpcLen);

static void _freeMspc(char *pMspc);

static int _saveMspcToFile(char *mSpc, uint32_t mSpcLen);//M.spc

static int _parseSpcCs1(char *uuidBuf, uint32_t uuidBufLen);//get uuid and Msat_up

static int _genTempSatCs1(char *uuid, uint32_t uuidLen, char *mSpc, uint32_t mSpcLen);//temp/temp.satcs1

static int _genUuidSatCs1(char *uuid, uint32_t uuidLen);

//
//
int init_checkSpcCs1();

int init_cs1();



//check stage 2
#define SPCCS2FILETAIL	".spccs2"

#define SPCCS2FILEPATH	"/mnt/data/spc/node/temp/spc.cs2"
//#define SPCCS2FILEPATH	"/home/twhite/spc_init_node/node/temp/spc.cs2"

#define DECRYPTCMDHEADER	"python3.6 /mnt/data/Electrum-3.3.8_spc_190812/run_electrum -w /mnt/data/spc/node/node.ewallet decrypt "
//#define DECRYPTCMDHEADER	"python3.6 /home/twhite/spc_init_node/Electrum-3.3.8_190812/run_electrum -w /home/twhite/spc_init_node/node/node.ewallet  decrypt "

#define THEWORDS	"spc"
//#define THEWORDS	"spc1"

#define TEMPSATCS2FILEPATH	"/mnt/data/spc/node/temp/temp.satcs2"
//#define TEMPSATCS2FILEPATH	"/home/twhite/spc_init_node/node/temp/temp.satcs2"

#define SATCS2FILETAIL	".satcs2"

static int _countUuidSpcCs2File(char *dirPath);

static char *_findUuidSpcCs2File(char *dirPath, uint32_t *pFileNameLen);

static void _freeUuidSpcCs2FileName(char *pFileName);

static int _mvUuidSpcCs2ToSpcCs2(char *fileName, uint32_t fileNameLen);

static int _checkSpcCs2();

//get Msat(spc.cs1 spc.cs2)
static char *_getMsat(uint32_t *pMsatLen);

static void _freeMsat(char *pMsat);

static char *_getSatEpub(uint32_t *pSatEpubLen);

static void _freeSatEpub(char *pSatEpub);

static char *_runDecrypt(char *satEpub, uint32_t satEpubLen, char *mSat, uint32_t mSatLen, uint32_t *pDmsgRetLen);

static void _freeDmsgRet(char *pDmsgRet);

static char *_parseDmsgRet(char *dMsgRet, uint32_t dMsgRetLen, uint32_t *msgLen);

static void _freeMsg(char *pMsg);

static int _checkMsg(char *msg, uint32_t msgLen);

static int _parseSpcCs2(char *uuidBuf, uint32_t uuidBufLen);//get uuid and Msat_up

static char *_getMspc(uint32_t *mSpcLen);

static int _genTempSatCs2(char *uuid, uint32_t uuidLen, char *mSpc, uint32_t mSpcLen);//temp/temp.satcs2 if mSpc == NULL

static int _genUuidSatCs2(char *uuid, uint32_t uuidLen);

//
//
int init_checkSpcCs2();

int init_cs2();
























#endif 
