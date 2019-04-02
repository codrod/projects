#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "./fs.h"

disk_t MY_DISK;

int getdir(dir_t* *ret);
int todir(char *buf, dir_t *dir);
int pathspn(char *path, char *str);
int findent(dir_t *dir, char *name, dirent_t *entry);
int addentry(dir_t *dirp, dirent_t *entry);
uint64_t diskalloc(int n);
int getpdir(char *path, dir_t* *pdir);

int main()
{
	my_FILE *file = NULL;
	char str[256], buf[256];
	int i = 0, j = 0;
	
	my_init("/home/tux/test/disk");
	
	my_dcreate("/dir");
	my_dcreate("/dir/sub");
	
	file = my_fopen("/dir/sub/file.txt");
	printf("opened /dir/sub/file.txt\n");
	
	printf("Enter a string: ");
	fgets(str, 256, stdin);
	
	my_fwrite(str, 1, strlen(str) + 1, file);
	printf("wrote string to /dir/sub/file.txt\n");
	
	my_fclose(file);
	printf("closed /dir/sub/file.txt\n");
	
	file = my_fopen("/dir/sub/file.txt");
	printf("opened /dir/sub/file.txt again\n");
	
	my_fread(str, 1, strlen(str) + 1, file);
	printf("read back: \"%s\"\n", str);
	
	return 0;
}

size_t my_fread(const void *ptr, size_t size, size_t n, my_FILE *file)
{
	size_t i = file->pos / (DISK_BLOCK_SIZE - 10),
	pos = file->pos - i * (DISK_BLOCK_SIZE - 10); //jump to the correct block
	uint16_t num = 0;
	int64_t nbytes = size * n, offset = 0;
	char buf[DISK_BLOCK_SIZE];
	
	//while we still have more bytes to read
	for(; nbytes > 0; i++)
	{
		//End of the file so return amount read
		if(!file->blocks[i])
		{
			file->pos += size * n - nbytes;
			
			return size * n - nbytes;
		}
		
		if(nbytes < DISK_BLOCK_SIZE - 10 - pos) num = nbytes;
		else num = DISK_BLOCK_SIZE - 10 - pos;
		
		//read data into memory
		fseek(MY_DISK.file, file->blocks[i] + pos + 10, SEEK_SET);
		fread(buf, 1, num, MY_DISK.file);
		memcpy((char*)ptr + offset, buf, num);
		
		//Measure how much we have read so far
		nbytes -= num;
		offset += num;
	}
	
	file->pos += size * n;
	
	//read all of the data requested
	return size * n;
}

size_t my_fwrite(const void *ptr, size_t size, size_t n, my_FILE *file)
{
	size_t i = file->pos / (DISK_BLOCK_SIZE - 10), offset = 0,
	pos = file->pos - i * (DISK_BLOCK_SIZE - 10);
	
	int64_t nbytes = size * n;
	uint16_t newsiz = 0, num = 0;
	uint64_t zero = 0;
	void *tmp = NULL;
	
	for(; nbytes > 0; i++)
	{
		//expand the file->blocks array if necessary
		if(i + 1 >= file->blocks_size)
		{
			if(!(tmp = realloc(file->blocks, file->blocks_size * 16)))
				return 0;
			
			file->blocks = tmp;
			file->blocks_size *= 2;
		}
		
		//if there are no more disk-blocks
		if(!file->blocks[i])
		{
			//allocate a new disk-block
			if(!(file->blocks[i] = diskalloc(TYPE_FILE)))
				return 0;
			
			//If this is not the first disk-block
			if(i)
			{
				//link the last disk-block to new disk-block
				fseek(MY_DISK.file, file->blocks[i - 1], SEEK_SET);
				fwrite(&file->blocks[i], 1, 8, MY_DISK.file);
			}
			else
			{
				//link the first disk-block to the directory entry of the file
				fseek(MY_DISK.file, file->entry + 1, SEEK_SET);
				fwrite(&file->blocks[i], 1, 8, MY_DISK.file);
			}
			
			//initialize the disk-block's header
			fseek(MY_DISK.file, file->blocks[i], SEEK_SET);
			fwrite(&zero, 1, 8, MY_DISK.file);
			pos = 0;
			
			//terminate the disk-block array
			file->blocks[i + 1] = 0;
		}
		else
		{
			//seek to the disk-block and read the size of the block
			fseek(MY_DISK.file, file->blocks[i] + 8, SEEK_SET);
		}
	
		//set 'num' equal to number of remaining bytes in the
		//current disk-block
		if(nbytes < DISK_BLOCK_SIZE - 10 - pos) num = nbytes;
		else num = DISK_BLOCK_SIZE - 10 - pos;

		//update the disk-block's header with the newsize of the disk-block	
		newsiz = pos + num;
		fwrite(&newsiz, 1, 2, MY_DISK.file);
		
		//seek to the current pos in the block and write
		fseek(MY_DISK.file, pos, SEEK_CUR);
		fwrite((char*)ptr + offset, 1, num, MY_DISK.file);
		
		//update the offset and nbytes to reflect the amount
		//of bytes written so far
		offset += num;
		nbytes -= num;
	}
	
	//update the file position
	file->pos += size * n;
	
	return size * n;
}

my_FILE* my_fopen(char *path)
{
	dir_t *dirp = NULL;
	my_FILE *file = malloc(sizeof(my_FILE));
	dirent_t entry;
	size_t i = 0;
	uint64_t link = 0;
	void *tmp = NULL;
	
	//get the parent directory of 'path'
	getpdir(path, &dirp);
	
	file->pos = 0;
	file->blocks = calloc(10, 8);
	file->blocks_size = 10;
	
	//If the file already exists
	if(findent(dirp, strrchr(path, '/') + 1, &entry))
	{
		file->entry = entry.add;
		file->blocks[0] = entry.link;
		link = entry.link;
	
		//find the disk-addresses of all the blocks and
		//store them in memory
		for(i = 1; link; i++)
		{
			fseek(MY_DISK.file, link, SEEK_SET);
			fread(&link, 1, 8, MY_DISK.file);
			file->blocks[i] = link;
			
			//expand the array if necessary
			if(i + 1 >= file->blocks_size)
			{
				if(!(tmp = realloc(file->blocks, file->blocks_size * 2)))
					return 0;
				
				file->blocks = tmp;
				file->blocks_size *= 2;
			}
		}
	}
	else
	{
		//create new directory entry
		entry.type = TYPE_FILE;
		entry.link = 0;
		entry.size = 0;
		entry.modtime = (uint64_t)time(NULL);
		strcpy(entry.name, strrchr(path, '/') + 1);
		
		//add the new entry to the directory
		addentry(dirp, &entry);
		
		file->entry = entry.add;
	}
	
	return file;
}

int my_fclose(my_FILE *file)
{
	free(file->blocks);
	free(file);
	
	return 1;
}

//This function creates a new directory referenced by 'path'
int my_dcreate(char *path)
{
	size_t j = 0;
	uint64_t zero = 0;
	dir_t *dirp = NULL;
	dirent_t entry;
	
	//get the parent directory
	getpdir(path, &dirp);
	
	//If the directory dosent exist
	if(!findent(dirp, strrchr(path, '/') + 1, &entry))
	{
		//create a new directory entry (to go in the parent directory
		entry.type = TYPE_DIR;
		entry.link = diskalloc(TYPE_DIR);
		entry.size = 0;
		entry.modtime = (uint64_t)time(NULL);
		strcpy(entry.name, strrchr(path, '/') + 1);
		
		//add the new directory's entry to the parent directory
		addentry(dirp, &entry);
		
		//initalize the directory-block's header
		fseek(MY_DISK.file, entry.link, SEEK_SET);
		fwrite(&zero, 1, 8, MY_DISK.file);
		fwrite(&dirp->add, 1, 8, MY_DISK.file);
		
		//initialize the entire directory to zero
		for(j = 16; j < 1024; j++)
			fwrite(&zero, 1, 1, MY_DISK.file);
	}
	else return 0; //already exists
	
	return 1;
}

//This function reads the parent directory
//of 'path' from the disk and loads it into memory
int getpdir(char *path, dir_t* *pdir)
{
	dir_t *dirp;
	dirent_t entry;
	uint64_t link = MY_DISK.root;
	size_t offset = 1;
	
	while(1)
	{
		//get the current directory
		fseek(MY_DISK.file, link, SEEK_SET);
		getdir(&dirp);
		
		//if last directory then stop
		if(!strchr(&path[offset], '/'))
			break;
		
		//if the next directory isnt in the current directory
		//then error
		if(!findent(dirp, &path[offset], &entry))
			return 0;
		
		//advance to the next directory
		offset += pathspn(&path[offset], entry.name) + 1;
		link = entry.link;
	}
	
	//set the pdir to the current directory
	*pdir = dirp;
	
	return 1;
}

//This function reads the directory at the current disk-address.
int getdir(dir_t* *ret)
{
	char buf[DIR_BLOCK_SIZE];
	dir_t *start = NULL, *dir = NULL;
	
	if(!(start = malloc(sizeof(dir_t))))
		return 0;
	
	//read the current directory-fragment into memory
	//and convert it to a dir_t
	dir = start;
	dir->add = ftell(MY_DISK.file);	
	fread(buf, 1, DIR_BLOCK_SIZE, MY_DISK.file);
	todir(buf, dir);
	
	dir->link = 0;
	
	//while more directory-fragments
	for(; dir->link; dir = dir->next)
	{
		//link new directory-fragment to last directory-fragment
		dir->next = malloc(sizeof(dir_t));
		
		//read the current directory-fragment into memory
		//and conver it to a dir_t
		dir->add = ftell(MY_DISK.file);
		fread(buf, 1, DIR_BLOCK_SIZE, MY_DISK.file);
		todir(buf, dir->next);
	}
	
	//return the first directory-fragment
	*ret = start;
	
	return 1;
}

//This functions adds a directory-entry to a directory
int addentry(dir_t *dirp, dirent_t *entry)
{
	size_t i = 0;
	
	//find the first empty directory-entry
	for(i = 0; dirp->entries[i].type != TYPE_NULL; i++)
	{
		if(i == 7)
		{
			i = 0;
		
			if(dirp->next) dirp = dirp->next;
			else return 0;
		}
	}

	//seek to the empty directory-entry
	fseek(MY_DISK.file, dirp->entries[i].add, SEEK_SET);

	//write 'entry' to disk
	entry->add = dirp->entries[i].add;
	fwrite(&entry->type, 1, 1, MY_DISK.file);
	fwrite(&entry->link, 1, 8, MY_DISK.file);
	fwrite(&entry->size, 1, 8, MY_DISK.file); 
	fwrite(&entry->modtime, 1, 8, MY_DISK.file);
	fwrite(entry->name, 1, strlen(entry->name), MY_DISK.file);
	
	return 1;
}

//This function allocates a file-block or a directory-block
//and returns its disk-address
uint64_t diskalloc(int type)
{
	size_t i = 0, j = 0;
	uint8_t pace = 0, init = 0, mask = 0, zero = 0;
	uint64_t loc = 0;
	
	//If we are allocating a directory-block
	if(type == TYPE_DIR)
	{
		//allocate two disk-blocks
		init = mask = 0xC0;
		pace = 7;
	}
	else
	{
		//allocate one disk-block
		init = mask = 0x80;
		pace = 8;
	}
	
	for(; i < MY_DISK.nblocks; i++)
	{
		//if available disk-block
		if(!(MY_DISK.table[j] & mask))
		{
			//allocate block
			MY_DISK.table[j] |= mask;
			
			//update allocation table
			loc = ftell(MY_DISK.file);
				fseek(MY_DISK.file, 8, SEEK_SET);
				fwrite(MY_DISK.table, 1, roundup(MY_DISK.nblocks, 8), MY_DISK.file);
			fseek(MY_DISK.file, loc, SEEK_SET);
			
			//return disk-address
			return MY_DISK.root + i * DISK_BLOCK_SIZE;
		}
		
		//increment through the table
		if(!((i + 1) % pace)) j++, mask = init;
		else mask>>=1;
	}
	
	return 0;
}

//this function converts a raw directory into a dir_t
int todir(char *buf, dir_t *dir)
{
	size_t i = 0, offset = 0;

	//segment the buffer
	memcpy(&dir->link, buf, 8);
	memcpy(&dir->parent, &buf[8], 8);
	memcpy(&dir->nentries, &buf[16], 1);
	
	//initialize the directory-entries
	for(i = 0, offset = 17; i < 8; i++, offset += 100)
	{
		//printf("%i\n", i);
		dir->entries[i].add = dir->add + offset;
		memcpy(&dir->entries[i].type, &buf[offset], 1);
		memcpy(&dir->entries[i].link, &buf[offset += 1], 8);
		memcpy(&dir->entries[i].size, &buf[offset += 8], 8);
		memcpy(&dir->entries[i].modtime, &buf[offset += 8], 8);
		memcpy(&dir->entries[i].name, &buf[offset += 8], 100);
	}
	
	dir->next = NULL;
	
	return 1;
}

//This function finds an entry in a directory
int findent(dir_t *dir, char *name, dirent_t *entry)
{
	size_t i = 0;
	
	for(; dir->entries[i].type == TYPE_NULL || !pathspn(name, dir->entries[i].name); i++)
	{
		//printf("i = %i\n", i);
		if(i == 7)
		{
			if(!dir->next) return 0;
			else dir = dir->next;
		}
	}
	
	*entry = dir->entries[i];
	
	return 1;
}

int freedir(dir_t *dir)
{
	dir_t *tmp = NULL;
	
	do
	{
		tmp = dir->next;
		free(dir);
		dir = tmp;
	}
	while(dir);
	
	return 1;
}

//This function compares the two strings up until a '/' or '\0'
//it also returns the length of the common segment
int pathspn(char *path, char *str)
{
	size_t i = 0;
	
	for(; path[i] && path[i] != '/'; i++)
		if(path[i] != str[i])
			return 0;
			
	return i;
}

//This function initializes MY_DISK
int my_init(char *path)
{
	//open the "disk"
	if(!(MY_DISK.file = fopen(path, "r+")))
		return 1;
	
	//read the number of disk-blocks
	fread(&MY_DISK.nblocks, 8, 1, MY_DISK.file);
	
	if(MY_DISK.nblocks <= 0) return 1;
	
	//allocate space for the allocation table
	if(!(MY_DISK.table = malloc(roundup(MY_DISK.nblocks, 8))))
		return 1;
	
	//read the allocation table into memory
	fread(MY_DISK.table, 1, roundup(MY_DISK.nblocks, 8), MY_DISK.file);
	
	//calculate the disk-address of the root directory
	MY_DISK.root = roundup(roundup(MY_DISK.nblocks, 8), DISK_BLOCK_SIZE) * DISK_BLOCK_SIZE;
	
	return 1;
}







