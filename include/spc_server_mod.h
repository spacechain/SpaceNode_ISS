#ifndef _SPC_SERVER_MOD_H_
#define _SPC_SERVER_MOD_H_

#include <stdio.h>
#include <string.h>
#include <stdint.h>

//
#define IDLENGTHMAX	12

//path define
#define SERVERLISTPATH		"/mnt/data/spc/server/server.list"
//#define SERVERLISTPATH		"/home/twhite/spc_prepare_mod/spc/server/server.list"
#define WALLETDIRHEADER		"/mnt/data/spc/wallet/"
//#define WALLETDIRHEADER		"/home/twhite/spc_prepare_mod/spc/wallet/"
#define CERTIDIRHEADER		"/mnt/data/spc/certifile/"
//#define CERTIDIRHEADER		"/home/twhite/spc_prepare_mod/spc/certifile/"
#define TXFILEDIR		"/mnt/data/spc/transaction/txfiles/"
//#define TXFILEDIR		"/home/twhite/spc_prepare_mod/spc/transaction/txfiles/"
#define RESFILEDIR		"/mnt/data/spc/transaction/resfiles/"
//#define RESFILEDIR		"/home/twhite/spc_prepare_mod/spc/transaction/resfiles/"


//server info struct in server list file
struct server_info 	
{
	uint32_t server_id;
	uint32_t server_start;
	uint64_t server_txs;
	char server_secret[16];
	uint32_t server_counter;
};


//util
//
//if exists, return 0;
//else, return -1;
static int _checkServerList(void);

//
//if succeed, return 0;
//if failed, return -1;
static int _initServerList(void);

//
//if succeed, return 0;
//else, return -1;
static int _getServerInfo(struct server_info *pInfo, uint32_t serverId);

//
//if succeed, return 0;
//else, return -1;
static int _setServerInfo(struct server_info *pInfo, uint8_t isInfo0);

//
//
//
static int _makeServerDir(uint32_t server_id);

//
//
//
static void _printServerInfo(struct server_info *pInfo);


//server interface
//
//if succeed, return 0;
//else, return -1;
int server_getNewInfo(struct server_info *pInfo);

//
//
//
int server_initNewServer(struct server_info *pInfo);
//make dir
//set server info

//
//
//
int server_getServerInfo(struct server_info *pInfo, uint32_t serverId);

//
//
//
int server_setServerInfo(struct server_info *pInfo);

//
//
//
void server_showServerList(void);

#define UUIDLEN	32

#define USEREPUBFILETAIL	".userepub"

#define EPUBDIR	"/mnt/data/spc/server/epub/"
//#define EPUBDIR	"/home/twhite/spc_prepare_mod/spc/server/epub/"

#define USEREPUBFILEPATH	"/mnt/data/spc/server/temp/user.epub"
//#define USEREPUBFILEPATH	"/home/twhite/spc_prepare_mod/spc/server/temp/user.epub"

static int _countCmdFile(char *dirPath);

static char *_getCmdFileName(char *dirPath, uint32_t *retNameLen);

static void _freeRetFileName(char *pName);


static int _mvCmdFileToTempDir(char *fileName, uint32_t fileNameLen);


static int _parseUserEpubFile(char *uuidBuf, uint32_t uuidBufLen, uint32_t *serverId, char **userEpub, uint32_t *userEpubLen);


static void _freeUserEpub(char *userEpub);

int server_countCmdFile();

int server_parseCmdFile(char *uuidBuf, uint32_t uuidBufLen, uint32_t *serverId, char **userEpub, uint32_t *userEpubLen);


#endif
