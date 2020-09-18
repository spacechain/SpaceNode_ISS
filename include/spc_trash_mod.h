#ifndef _SPC_TRASH_MOD_H_
#define _SPC_TRASH_MOD_H_

#define TRASHTEMPDIR "/mnt/data/spc/trash/temp/"
//#define TRASHTEMPDIR "/home/twhite/spc_prepare_mod/spc/trash/temp/"

#define DOWNLOADDIR	"/mnt/data/spc/download/"
//#define DOWNLOADDIR	"/home/twhite/spc_prepare_mod/spc/download/"

#define MVTOTRASHTEMPCMD	"mv /mnt/data/spc/download/* /mnt/data/spc/trash/temp/"
//#define MVTOTRASHTEMPCMD	"mv /home/twhite/spc_prepare_mod/spc/download/* /home/twhite/spc_prepare_mod/spc/trash/temp/"

#define RMTRASHTEMPCMD	"rm -f /mnt/data/spc/trash/temp/*"
//#define RMTRASHTEMPCMD	"rm -f /home/twhite/spc_prepare_mod/spc/trash/temp/*"

#define KEEPLISTFILEPATH	"/mnt/data/spc/trash/keep.list"
//#define KEEPLISTFILEPATH	"/home/twhite/spc_prepare_mod/spc/trash/keep.list"


static void _mvFileToTrashTemp();

//access
static int _checkKeepList();

static int _mvFileToDownloadDir();

static int _cleanTrashTempDir();

//
int trash_keepFile();

#endif
