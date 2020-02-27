#ifndef _SPC_PREPARE_MOD_
#define _SPC_PREPARE_MOD_

#define UPLOADDIR	"/mnt/data/spc/upload/"
//#define UPLOADDIR	"/home/twhite/spc_prepare_mod/spc/upload/"

#define PREPARETEMPDIR	"/mnt/data/spc/prepare/temp/"
//#define PREPARETEMPDIR	"/home/twhite/spc_prepare_mod/spc/prepare/temp/"

#define KEEPLISTFILEPATH	"/mnt/data/spc/trash/keep.list"
//#define KEEPLISTFILEPATH	"/home/twhite/spc_prepare_mod/spc/trash/keep.list"

#define TXDIR	"/mnt/data/spc/transaction/txfiles/"
//#define TXDIR	"/home/twhite/spc_prepare_mod/spc/transaction/txfiles/"

#define SERVEREPUBDIR	"/mnt/data/spc/server/epub/"   
//#define SERVEREPUBDIR	"/home/twhite/spc_prepare_mod/spc/server/epub/"   

#define DATAFILETAIL	".data"
#define CHECKFILETAIL	".checksum"

#define TEMPDATAFILEPATH	"/mnt/data/spc/prepare/temp/temp.data"
//#define TEMPDATAFILEPATH	"/home/twhite/spc_prepare_mod/spc/prepare/temp/temp.data"



#define TXFILENAMELEN	29

#define CMDFILENAMELEN	41

static int _readDataToFile(int fd, uint32_t start, uint32_t length, char *filePath);

static int _countDataFile(char *dirPath);

static char *_findDataFile(char *dirPath, uint32_t *retFileNameLen);

static void _freeDataFileName(char *pName);

//check md5
#define CHECKMD5CMDHEADER	"python3.6 /mnt/data/Electrum-3.3.8_spc_190812/run_electrum verifymessagespc "
//#define CHECKMD5CMDHEADER	"python3.6 /home/twhite/spc_prepare_mod/Electrum-3.3.8_190812/run_electrum verifymessagespc "

#define RMCHECKFILECMD	"rm -f /mnt/data/spc/upload/*.checksum"
//#define RMCHECKFILECMD	"rm -f /home/twhite/spc_prepare_mod/spc/upload/*.checksum"

#define EPUBFILEPATH	"/mnt/data/spc/node/epub.file"
//#define EPUBFILEPATH	"/home/twhite/spc_prepare_mod/spc/node/epub.file"

static int _getCheckFileName(char *dataFileName, uint32_t dataFileNameLen, char *checkFileName, uint32_t checkFileNameLen);

static int _getMd5(char *checkFileName, uint32_t checkFileNameLen, char *md5Buf, uint32_t md5BufLen);

static int _genMd5(char *dataFileName, uint32_t dataFileNameLen, char *md5Buf, uint32_t md5BufLen);

static char *_getSig(char *checkFileName, uint32_t checkFileNameLen, uint32_t *sigLen);

static void _freeSig(char *pSig);

static int _checkSig(char *spcEpub, uint32_t spcEpubLen, char *sig, uint32_t sigLen, char *md5, uint32_t md5Len);

static void _rmCheckFile();
//

static int _mvDataFileToTempDir(char *fileName, uint32_t fileNameLen);

static int _checkTempDataInTempDir();

//check checksum


//check file_no
static int _getFileNo(int fd, uint32_t *fileNo);

//check temp.data head and tail
static int _checkHeadAndTail(int fd);

//generate keep.list
static int _genKeepList(int fd);

//generate tx files
static int _genTxFiles(int fd);

//generate user epub file
static int _genCmdFiles(int fd);





int prepare_checkDataFile();

int prepare_processDataFile();









#endif
