#ifndef _SPC_FILEOPS_MOD_H_
#define _SPC_FILEOPS_MOD_H_

#include "spc_fileops_mod.h"

#define DOWNLOADFILEDIR	"/mnt/data/spc/download/"
//#define DOWNLOADFILEDIR	"/home/twhite/spc_prepare_mod/spc/download/"


int fileops_getNextTxFile(const char *txPath, char *path, uint32_t len);

int fileops_genResFilePath(const char *resDirPath, const char *txFilePath, char *resFilePath, uint32_t len);

int fileops_genDownloadFilePath(const char *downloadDirPath, const char *txFilePath, char *downloadFilePath, uint32_t len);

int fileops_rmTxFile(const char *path);





#endif
