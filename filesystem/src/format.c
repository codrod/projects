#include <unistd.h>
#include <sys/stat.h>

#include "fs.h"

int main(int argc, char* *argv)
{
	FILE *disk = NULL;
	struct stat info;
	uint8_t first_byte = 0xC0, null_byte = 0;
	uint64_t table_size = 0, num_blocks = 0, i = 1;

	if(argc != 3)
	{
		printf("usage: format [num_of_blocks] [disk_file]\n");
		return 0;
	}

	if(!(disk = fopen(argv[2], "r+")))
	{
		perror("ERROR(1): the disk cant be opened ");
		return 1;
	}

	if((num_blocks = strtoll(argv[1], NULL, 10)) < 100)
	{
		perror("ERROR(2): number of blocks is too small < 100");
		return 2;
	}

	//calculate size of allocation table in bytes
	table_size = roundup(num_blocks, 8);

	//adjust 'num_blocks' to account for the size of the allocation table itself
	num_blocks = num_blocks - roundup(table_size + 8, DISK_BLOCK_SIZE);
	table_size = roundup(num_blocks, 8);

	char block[DISK_BLOCK_SIZE];
	memset(block, 0, DISK_BLOCK_SIZE);

	printf("num_blocks = %llu disk-blocks\ntable_size = %llu bytes\n", (unsigned long long int)num_blocks, (unsigned long long int)table_size);

	//this records the table size measured in disk-blocks
	fwrite(&num_blocks, 8, 1, disk);
	//the first two-disk blocks are always used by the root directory
	fwrite(&first_byte, 1, 1, disk);

	//create the allocation-table
	for(; i < table_size; i++)
		fwrite(&null_byte, 1, 1, disk);

	//move to the end of the allocation-table
	fseek(disk, roundup(table_size, DISK_BLOCK_SIZE) * DISK_BLOCK_SIZE, SEEK_SET);

	printf("root = %llu-th byte\n", (unsigned long long int)(roundup(table_size, DISK_BLOCK_SIZE) * DISK_BLOCK_SIZE));

	for(int j = 0; j < num_blocks; j++)
		fwrite(block, DISK_BLOCK_SIZE, 1, disk);

	//empty internal buffer
	fflush(disk);

	return 0;
}
