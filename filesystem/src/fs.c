#include "fs.h"

size_t fs_fread(const void *ptr, size_t size, size_t n, file_t *file)
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
		fseek(file->disk->file, file->blocks[i] + pos + 10, SEEK_SET);
		fread(buf, 1, num, file->disk->file);
		memcpy((char*)ptr + offset, buf, num);

		//Measure how much we have read so far
		nbytes -= num;
		offset += num;
	}

	file->pos += size * n;

	//read all of the data requested
	return size * n;
}

size_t fs_fwrite(const void *ptr, size_t size, size_t n, file_t *file)
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
			if(!(file->blocks[i] = diskalloc(file->disk, TYPE_FILE)))
				return 0;

			//If this is not the first disk-block
			if(i)
			{
				//link the last disk-block to new disk-block
				fseek(file->disk->file, file->blocks[i - 1], SEEK_SET);
				fwrite(&file->blocks[i], 1, 8, file->disk->file);
			}
			else
			{
				//link the first disk-block to the directory entry of the file
				fseek(file->disk->file, file->entry + 1, SEEK_SET);
				fwrite(&file->blocks[i], 1, 8, file->disk->file);
			}

			//initialize the disk-block's header
			fseek(file->disk->file, file->blocks[i], SEEK_SET);
			fwrite(&zero, 1, 8, file->disk->file);
			pos = 0;

			//terminate the disk-block array
			file->blocks[i + 1] = 0;
		}
		else
		{
			//seek to the disk-block and read the size of the block
			fseek(file->disk->file, file->blocks[i] + 8, SEEK_SET);
		}

		//set 'num' equal to number of remaining bytes in the
		//current disk-block
		if(nbytes < DISK_BLOCK_SIZE - 10 - pos) num = nbytes;
		else num = DISK_BLOCK_SIZE - 10 - pos;

		//update the disk-block's header with the newsize of the disk-block
		newsiz = pos + num;
		fwrite(&newsiz, 1, 2, file->disk->file);

		//seek to the current pos in the block and write
		fseek(file->disk->file, pos, SEEK_CUR);
		fwrite((char*)ptr + offset, 1, num, file->disk->file);

		//update the offset and nbytes to reflect the amount
		//of bytes written so far
		offset += num;
		nbytes -= num;
	}

	//update the file position
	file->pos += size * n;

	return size * n;
}

file_t* fs_fopen(disk_t *disk, char *path)
{
	dir_t *dirp = NULL;
	file_t *file = malloc(sizeof(file_t));
	dirent_t entry;
	size_t i = 0;
	uint64_t link = 0;
	void *tmp = NULL;

	file->disk = disk;

	//get the parent directory of 'path'
	getpdir(disk, path, &dirp);

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
			fseek(disk->file, link, SEEK_SET);
			fread(&link, 1, 8, disk->file);
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

int fs_fclose(file_t *file)
{
	free(file->blocks);
	free(file);

	return 1;
}

//This function creates a new directory referenced by 'path'
int fs_dcreate(disk_t *disk, char *path)
{
	size_t j = 0;
	uint64_t zero = 0;
	dir_t *dirp = NULL;
	dirent_t entry;

	//get the parent directory
	getpdir(disk, path, &dirp);

	//If the directory dosent exist
	if(!findent(dirp, strrchr(path, '/') + 1, &entry))
	{
		//create a new directory entry (to go in the parent directory
		entry.type = TYPE_DIR;
		entry.link = diskalloc(disk, TYPE_DIR);
		entry.size = 0;
		entry.modtime = (uint64_t)time(NULL);
		strcpy(entry.name, strrchr(path, '/') + 1);

		//add the new directory's entry to the parent directory
		addentry(dirp, &entry);

		//initalize the directory-block's header
		fseek(disk->file, entry.link, SEEK_SET);
		fwrite(&zero, 1, 8, disk->file);
		fwrite(&dirp->add, 1, 8, disk->file);

		//initialize the entire directory to zero
		for(j = 16; j < 1024; j++)
			fwrite(&zero, 1, 1, disk->file);
	}
	else return 0; //already exists

	return 1;
}

//This function initializes disk
disk_t* fs_init(char *path)
{
	disk_t *disk = NULL;

	if(!(disk = malloc(sizeof(disk_t))))
		return NULL;

	//open the "disk"
	if(!(disk->file = fopen(path, "r+")))
		return NULL;

	//read the number of disk-blocks
	fread(&disk->nblocks, 8, 1, disk->file);

	if(disk->nblocks <= 0)
		return NULL;

	//allocate space for the allocation table
	if(!(disk->table = malloc(roundup(disk->nblocks, 8))))
		return NULL;

	//read the allocation table into memory
	fread(disk->table, 1, roundup(disk->nblocks, 8), disk->file);

	//calculate the disk-address of the root directory
	disk->root = roundup(roundup(disk->nblocks, 8), DISK_BLOCK_SIZE) * DISK_BLOCK_SIZE;

	return disk;
}
