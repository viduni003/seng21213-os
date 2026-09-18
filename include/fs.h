#ifndef FS_H
#define FS_H

#include "types.h"

#define FS_BLOCK_SIZE     4096
#define FS_TOTAL_BLOCKS   256
#define FS_MAGIC          0x53454E47
#define FS_MAX_INODES     32
#define FS_MAX_FILENAME   28
#define FS_DIRECT_BLOCKS  8
#define FS_DATA_START     4

typedef struct {
    uint32_t magic;
    uint32_t total_blocks;
    uint32_t total_inodes;
    uint32_t free_blocks;
    uint32_t free_inodes;
} fs_superblock_t;

typedef struct {
    uint8_t  used;
    uint32_t size;
    uint32_t direct[FS_DIRECT_BLOCKS];
} fs_inode_t;

typedef struct {
    char     name[FS_MAX_FILENAME];
    int32_t  inode;
} fs_dirent_t;

void fs_init(void);
int  fs_create(const char *name);
int  fs_unlink(const char *name);
int  fs_write(const char *name, const char *data, uint32_t len);
int  fs_read(const char *name, char *buf, uint32_t buf_size);
void fs_list(void);

#endif