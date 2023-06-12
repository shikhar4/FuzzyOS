#ifndef _FILESYS_H
#define _FILESYS_H

#include "types.h"
#include "multiboot.h"
#include "lib.h"
#include "system_call.h"


#define BOOT_BLOCK_RESERVED     52
#define DENTRY_RESERVED         24
#define MAX_DENTRIES            63
#define MAX_DATA_BLOCKS         1023
#define BLOCK_SIZE              4096 // 4KB
#define DENTRY_SIZE             64
#define NUM_DENTRIES            64
#define MAX_FILE_NAME_LEN       32
#define DIR                     1
#define FILE                    2


typedef struct dentry_t {
    int8_t fileName[MAX_FILE_NAME_LEN]; // 32B
    int32_t fileType; // 4B
    uint32_t inodeNum; // 4B
    // 24B reserved
    uint8_t reserved[DENTRY_RESERVED]; // Reserved bytes
} dentry_t;

typedef struct bootBlock_t {
    uint32_t numDentries;
    uint32_t n; // num inodes
    uint32_t d; // num data blocks
    // 53B reserved
    uint8_t reserved[BOOT_BLOCK_RESERVED]; // Reserved bytes
    dentry_t dentries[MAX_DENTRIES]; // 64B dir. entries
} bootBlock_t;

typedef struct inode_t {
    uint32_t length; // in B
    uint32_t dataBlocks[MAX_DATA_BLOCKS]; // in B, max 1023 data block nums
} inode_t;


// Initialize file system: boot block and data block based on mod address
int32_t init_filesys(uint32_t* mod);

// Finds the dentry with name fname and fills dentry pointer with its information
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);

// Finds the dentry with given index and fills dentry pointer with its information
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);

// Fills the given buffer with "length" bytes from file pointed to from "inode" starting at "offset" bytes
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

// Returns the length in bytes of the currently opened file
int32_t getFileLen(int32_t fd);

// Returns file length of file pointed to by dentry
int32_t get_file_len_by_dentry(dentry_t dentry);



// opens file with given filename; return 0 on success, -1 on failure
extern int32_t file_open (int32_t fd, const uint8_t* filename);

// closes file; return 0 on success, -1 on failure
extern int32_t file_close (int32_t fd);

// Reads nbytes from fd into buf; returns number of bytes read
extern int32_t file_read (int32_t fd, void* buf, int32_t nbytes);

// Currently does nothing; return -1
extern int32_t file_write (int32_t fd, const void* buf, int32_t nbytes);


// opens directory with given dirname; return 0 on success, -1 on failure
extern int32_t dir_open (int fd, const uint8_t* dirname);

// closes directory with given directory; return 0 on success, -1 on failure
extern int32_t dir_close (int32_t fd);

// Reads a filename from directory into buf; return 0 on success, -1 on failure
extern int32_t dir_read (int32_t fd, void* buf, int32_t nbytes);

// Currently does nothing; return -1
extern int32_t dir_write (int32_t fd, const void* buf, int32_t nbytes);


#endif /* _FILESYS_H */

