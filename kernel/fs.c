#include "fs.h"
#include "vga.h"

#define FS_MAX_DIRENTS (FS_BLOCK_SIZE / sizeof(fs_dirent_t))

/* The entire "disk" — a fixed-size byte array in BSS */
static uint8_t ramdisk[FS_TOTAL_BLOCKS * FS_BLOCK_SIZE];

static fs_superblock_t *sb;
static fs_dirent_t      *directory;
static fs_inode_t       *inodes;
static uint8_t           *block_bitmap;

static void k_strcpy(char *dst, const char *src, size_t max) {
    size_t i = 0;
    while (src[i] && i < max - 1) { dst[i] = src[i]; i++; }
    dst[i] = 0;
}

static int k_strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) { a++; b++; }
    return (uint8_t)*a - (uint8_t)*b;
}

static uint8_t *block_ptr(uint32_t block_num) {
    return ramdisk + (block_num * FS_BLOCK_SIZE);
}

void fs_init(void) {
    sb           = (fs_superblock_t *)block_ptr(0);
    directory    = (fs_dirent_t *)block_ptr(1);
    inodes       = (fs_inode_t *)block_ptr(2);
    block_bitmap = block_ptr(3);

    sb->magic        = FS_MAGIC;
    sb->total_blocks = FS_TOTAL_BLOCKS;
    sb->total_inodes = FS_MAX_INODES;
    sb->free_blocks  = FS_TOTAL_BLOCKS - FS_DATA_START;
    sb->free_inodes  = FS_MAX_INODES;

    for (uint32_t i = 0; i < FS_MAX_DIRENTS; i++) {
        directory[i].inode = -1;
        directory[i].name[0] = '\0';
    }

    for (uint32_t i = 0; i < FS_MAX_INODES; i++) {
        inodes[i].used = 0;
        inodes[i].size = 0;
        for (int b = 0; b < FS_DIRECT_BLOCKS; b++) inodes[i].direct[b] = 0;
    }

    for (uint32_t i = 0; i < FS_TOTAL_BLOCKS / 8; i++) block_bitmap[i] = 0;
}

static int find_free_inode(void) {
    for (int i = 0; i < FS_MAX_INODES; i++) {
        if (!inodes[i].used) return i;
    }
    return -1;
}

static int find_dirent_by_name(const char *name) {
    for (uint32_t i = 0; i < FS_MAX_DIRENTS; i++) {
        if (directory[i].inode != -1 && k_strcmp(directory[i].name, name) == 0) {
            return (int)i;
        }
    }
    return -1;
}

static int find_free_dirent(void) {
    for (uint32_t i = 0; i < FS_MAX_DIRENTS; i++) {
        if (directory[i].inode == -1) return (int)i;
    }
    return -1;
}

static int block_bitmap_test(uint32_t block) {
    return block_bitmap[block / 8] & (1 << (block % 8));
}
static void block_bitmap_set(uint32_t block) {
    block_bitmap[block / 8] |= (1 << (block % 8));
}
static void block_bitmap_clear(uint32_t block) {
    block_bitmap[block / 8] &= ~(1 << (block % 8));
}

static int32_t alloc_data_block(void) {
    for (uint32_t b = FS_DATA_START; b < FS_TOTAL_BLOCKS; b++) {
        if (!block_bitmap_test(b)) {
            block_bitmap_set(b);
            sb->free_blocks--;
            return (int32_t)b;
        }
    }
    return -1;
}

static void free_data_block(uint32_t b) {
    if (block_bitmap_test(b)) {
        block_bitmap_clear(b);
        sb->free_blocks++;
    }
}

int fs_create(const char *name) {
    if (find_dirent_by_name(name) != -1) return -1; /* already exists */

    int inode_idx = find_free_inode();
    if (inode_idx == -1) return -1;

    int dirent_idx = find_free_dirent();
    if (dirent_idx == -1) return -1;

    inodes[inode_idx].used = 1;
    inodes[inode_idx].size = 0;
    for (int b = 0; b < FS_DIRECT_BLOCKS; b++) inodes[inode_idx].direct[b] = 0;

    k_strcpy(directory[dirent_idx].name, name, FS_MAX_FILENAME);
    directory[dirent_idx].inode = inode_idx;

    sb->free_inodes--;

    return inode_idx;
}

int fs_unlink(const char *name) {
    int dirent_idx = find_dirent_by_name(name);
    if (dirent_idx == -1) return -1;

    int inode_idx = directory[dirent_idx].inode;
    fs_inode_t *ino = &inodes[inode_idx];

    for (int b = 0; b < FS_DIRECT_BLOCKS; b++) {
        if (ino->direct[b] != 0) {
            free_data_block(ino->direct[b]);
            ino->direct[b] = 0;
        }
    }
    ino->used = 0;
    ino->size = 0;
    sb->free_inodes++;

    directory[dirent_idx].inode = -1;
    directory[dirent_idx].name[0] = '\0';

    return 0;
}

int fs_write(const char *name, const char *data, uint32_t len) {
    int dirent_idx = find_dirent_by_name(name);
    if (dirent_idx == -1) return -1;

    int inode_idx = directory[dirent_idx].inode;
    fs_inode_t *ino = &inodes[inode_idx];

    uint32_t max_size = FS_DIRECT_BLOCKS * FS_BLOCK_SIZE;
    if (len > max_size) len = max_size;

    uint32_t blocks_needed = (len + FS_BLOCK_SIZE - 1) / FS_BLOCK_SIZE;
    if (blocks_needed == 0) blocks_needed = 1;

    for (uint32_t b = 0; b < blocks_needed; b++) {
        if (ino->direct[b] == 0) {
            int32_t blk = alloc_data_block();
            if (blk == -1) return -1; /* out of space */
            ino->direct[b] = (uint32_t)blk;
        }
    }

    uint32_t remaining = len;
    const char *src = data;
    for (uint32_t b = 0; b < blocks_needed; b++) {
        uint8_t *dst = block_ptr(ino->direct[b]);
        uint32_t chunk = remaining < FS_BLOCK_SIZE ? remaining : FS_BLOCK_SIZE;
        for (uint32_t i = 0; i < chunk; i++) dst[i] = (uint8_t)src[i];
        src += chunk;
        remaining -= chunk;
    }

    ino->size = len;
    return (int)len;
}

int fs_read(const char *name, char *buf, uint32_t buf_size) {
    int dirent_idx = find_dirent_by_name(name);
    if (dirent_idx == -1) return -1;

    int inode_idx = directory[dirent_idx].inode;
    fs_inode_t *ino = &inodes[inode_idx];

    uint32_t to_read = ino->size < buf_size ? ino->size : buf_size;
    uint32_t remaining = to_read;
    char *dst = buf;

    uint32_t blocks_needed = (to_read + FS_BLOCK_SIZE - 1) / FS_BLOCK_SIZE;

    for (uint32_t b = 0; b < blocks_needed; b++) {
        if (ino->direct[b] == 0) break;
        uint8_t *src = block_ptr(ino->direct[b]);
        uint32_t chunk = remaining < FS_BLOCK_SIZE ? remaining : FS_BLOCK_SIZE;
        for (uint32_t i = 0; i < chunk; i++) dst[i] = (char)src[i];
        dst += chunk;
        remaining -= chunk;
    }

    return (int)to_read;
}

void fs_list(void) {
    vga_puts_color("\n  Files on RAM disk\n", VGA_YELLOW, VGA_BLACK);
    vga_puts("  -----------------------------------------------\n");
    int count = 0;
    for (uint32_t i = 0; i < FS_MAX_DIRENTS; i++) {
        if (directory[i].inode != -1) {
            fs_inode_t *ino = &inodes[directory[i].inode];
            vga_puts("  ");
            vga_puts(directory[i].name);
            vga_puts("  (");
            vga_printf("%d", ino->size);
            vga_puts(" bytes)\n");
            count++;
        }
    }
    if (count == 0) {
        vga_puts("  (no files)\n");
    }
    vga_puts("\n");
}