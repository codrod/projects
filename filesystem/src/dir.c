#include "fs.h"

//This function reads the parent directory
//of 'path' from the disk and loads it into memory
int getpdir(disk_t *disk, char *path, dir_t* *pdir)
{
	dir_t *dirp;
	dirent_t entry;
	uint64_t link = disk->root;
	size_t offset = 1;

	while(1)
	{
		//get the current directory
		fseek(disk->file, link, SEEK_SET);
		getdir(disk, &dirp);

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
int getdir(disk_t *disk, dir_t* *ret)
{
	char buf[DIR_BLOCK_SIZE];
	dir_t *start = NULL, *dir = NULL;

	if(!(start = malloc(sizeof(dir_t))))
		return 0;

	start->disk = disk;

	//read the current directory-fragment into memory
	//and convert it to a dir_t
	dir = start;
	dir->add = ftell(disk->file);
	fread(buf, 1, DIR_BLOCK_SIZE, disk->file);

	todir(buf, dir);

	dir->link = 0;

	//while more directory-fragments
	for(; dir->link; dir = dir->next)
	{
		//link new directory-fragment to last directory-fragment
		dir->next = malloc(sizeof(dir_t));

		//read the current directory-fragment into memory
		//and conver it to a dir_t
		dir->add = ftell(disk->file);
		fread(buf, 1, DIR_BLOCK_SIZE, disk->file);
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
	fseek(dirp->disk->file, dirp->entries[i].add, SEEK_SET);

	//write 'entry' to disk
	entry->add = dirp->entries[i].add;
	fwrite(&entry->type, 1, 1, dirp->disk->file);
	fwrite(&entry->link, 1, 8, dirp->disk->file);
	fwrite(&entry->size, 1, 8, dirp->disk->file);
	fwrite(&entry->modtime, 1, 8, dirp->disk->file);
	fwrite(entry->name, 1, strlen(entry->name), dirp->disk->file);

	return 1;
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

//This function allocates a file-block or a directory-block
//and returns its disk-address
uint64_t diskalloc(disk_t *disk, int type)
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

	for(; i < disk->nblocks; i++)
	{
		//if available disk-block
		if(!(disk->table[j] & mask))
		{
			//allocate block
			disk->table[j] |= mask;

			//update allocation table
			loc = ftell(disk->file);
				fseek(disk->file, 8, SEEK_SET);
				fwrite(disk->table, 1, roundup(disk->nblocks, 8), disk->file);
			fseek(disk->file, loc, SEEK_SET);

			//return disk-address
			return disk->root + i * DISK_BLOCK_SIZE;
		}

		//increment through the table
		if(!((i + 1) % pace)) j++, mask = init;
		else mask>>=1;
	}

	return 0;
}
