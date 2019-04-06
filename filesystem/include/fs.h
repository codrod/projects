#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>

#define DISK_BLOCK_SIZE 512
#define DIR_BLOCK_SIZE (DISK_BLOCK_SIZE * 2)

#define TYPE_NULL 0
#define TYPE_FILE 1
#define TYPE_DIR 2

#define roundup(dividend, divisor) ((dividend) / (divisor) + ((dividend) % (divisor) ? 1 : 0))

typedef struct disk
{
	FILE *file; //FILE* for the "disk"

	uint8_t *table; //allocation table
	uint64_t nblocks; //number of disk-blocks in the disk

	uint64_t root; //disk-address of the root directory
}
disk_t;

typedef struct file_t
{
	disk_t *disk;
	uint64_t pos; //offset from beginning of file as measured in bytes

	uint64_t *blocks; //null (0) terminated array of disk-addresses
	uint64_t blocks_size; //size of array of disk-address

	uint64_t entry; //disk-address of file’s directory-entry (meta-data)
}
file_t;

typedef struct dirent_t
{
	uint64_t add; //disk-address of the entry

	uint8_t type; //type of entry TYPE_*
	uint64_t link; //disk-address of first disk-block
	uint64_t size; //size of the entry in bytes
	uint64_t modtime; //modification time stamp

	char name[100]; //entry name
}
dirent_t;

typedef struct dir
{
	disk_t *disk;
	uint64_t add; //disk-address of the directory

	uint64_t link; //disk-address of next directory-fragment
	uint64_t parent; //disk-address of parent directory
	uint8_t nentries; //number of entries in the directory

	dirent_t entries[8]; //directory entries

	struct dir *next; //next directory-fragment in memory
}
dir_t;

//initializes MY_DISK so the other I/O functions can be used
disk_t* fs_init(char *path);

file_t* fs_fopen(disk_t *disk, char *path);
int fs_fclose(file_t *file);

size_t fs_fwrite(const void *ptr, size_t size, size_t nmemb, file_t *stream);
size_t fs_fread(const void *ptr, size_t size, size_t n, file_t *file);

//creates a directory
int fs_dcreate(disk_t *disk, char *path);

int getdir(disk_t *disk, dir_t* *ret);
int todir(char *buf, dir_t *dir);
int pathspn(char *path, char *str);
int findent(dir_t *dir, char *name, dirent_t *entry);
int addentry(dir_t *dirp, dirent_t *entry);
uint64_t diskalloc(disk_t *disk, int n);
int getpdir(disk_t *disk, char *path, dir_t* *pdir);
