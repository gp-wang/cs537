#ifndef __MFS_h__
#define __MFS_h__

#define MFS_DIRECTORY    (0)
#define MFS_REGULAR_FILE (1)

#define MFS_BLOCK_SIZE   (4096)
#define MFS_BYTE_STEP_SIZE   (16384)
#define NUM_INODE_POINTERS   (14)
#define TOTAL_NUM_INODES (4096)
#define IMAP_PIECE_SIZE   (16)		
#define NUM_IMAP TOTAL_NUM_INODES / IMAP_PIECE_SIZE /* gw: 256 */
#define LEN_NAME 60
//#define NUM_DIR_ENTRIES 14
//#define NUM_DIR_ENTRIES 64
#define NUM_DIR_ENTRIES 64


enum MFS_REQ {
  REQ_INIT,
  REQ_LOOKUP,
  REQ_STAT,
  REQ_WRITE,
  REQ_READ,
  REQ_CREAT,
  REQ_UNLINK,
  REQ_RESPONSE,
  REQ_SHUTDOWN
};

typedef struct __MFS_Stat_t {
    int type;   // MFS_DIRECTORY or MFS_REGULAR
    int size;   // bytes
    // note: no permissions, access times, etc.
} MFS_Stat_t;


typedef struct __MFS_DirEnt_t {
    char name[LEN_NAME];  
	int  inum;      	/* inume is the index of each inode, not address */
} MFS_DirEnt_t;


typedef struct __UDP_Packet {
	enum MFS_REQ request;

	int inum;
	int block;
	int type;

	char name[LEN_NAME];
	char buffer[MFS_BLOCK_SIZE];
	MFS_Stat_t stat;
} UDP_Packet;


typedef struct __MFS_Inode_t{
	int size;
	int type;
	int data[NUM_INODE_POINTERS]; /* pointer to data blocks (direct) */
} MFS_Inode_t;

/* gw: strictly, this is just one piece of imap, total 256 pieces */
typedef struct __MFS_Imap_t{
	int inodes[IMAP_PIECE_SIZE]; /* gw: 16 */
} MFS_Imap_t;

typedef struct __MFS_CR_t{
	int inode_count;
	int end_log;
	int imap[NUM_IMAP];	/* 256, NUM_IMAP = TOTAL_NUM_INODES / IMAP_PIECE_SIZE */
} MFS_CR_t;

/* gw: data block for dir */
typedef struct __MFS_DirDataBlock_t {
  MFS_DirEnt_t entries[NUM_DIR_ENTRIES];
} MFS_DirDataBlock_t;



int MFS_Init(char *hostname, int port);
int MFS_Lookup(int pinum, char *name);
int MFS_Stat(int inum, MFS_Stat_t *m);
int MFS_Write(int inum, char *buffer, int block);
int MFS_Read(int inum, char *buffer, int block);
int MFS_Creat(int pinum, int type, char *name);
int MFS_Unlink(int pinum, char *name);

#endif 
