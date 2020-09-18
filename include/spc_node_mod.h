#ifndef _SPC_NODE_MOD_H_
#define _SPC_NODE_MOD_H_

#define EWALLETFILEPATH	"/mnt/data/spc/node/node.ewallet"
//#define EWALLETFILEPATH	"/home/twhtie/spc_init_mod/node/node.ewallet"

#define CREATECMD	"python3.6 /mnt/data/Electrum-3.3.8_spc_190812/run_electrum create --segwit -w /mnt/data/spc/node/node.ewallet"
//#define CREATECMD	"python3.6 /home/twhite/spc_init_mod/Electrum-3.3.8_190812/run_electrum create --segwit -w /home/twhite/spc_init_mod/node/node.ewallet"

static int _checkEwallet(void);
 
static int _rmEwallet(void);

static void _runCreate(void);

int node_checkEwallet();

int node_rmEwallet();

int node_createEwallet();

#endif
