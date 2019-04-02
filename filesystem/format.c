#include <unistd.h>
#include <sys/stat.h>

#include "./fs.h"

int main(int argc, char* *argv)
{
	FILE *disk = NULL;
	struct stat info;
	uint8_t first_byte = 0xC0, null_byte = 0;
	uint64_t table_size = 0, num_blocks = 0, i = 1;
	
	if(argc != 2)
	{
		printf("This program is used to format the \"disk\". It takes only one argument: the path to the file representing the \"disk\"\n");
		return 0;
	}
	
	if(!(disk = fopen(argv[1], "r+")))
	{
		perror("ERROR(1): the disk cant be opened ");
		return 1;
	}
	
	if(stat(argv[1], &info) == -1)
	{
		perror("ERROR(2): cant stat the disk ");
		return 2;
	}
	
	//find number of disk-blocks
	num_blocks = info.st_size / DISK_BLOCK_SIZE;
	//calculate size of allocation table in bytes
	table_size = roundup(num_blocks, 8);
	
	//adjust 'num_blocks' to account for the size of the allocation table itself
	num_blocks = num_blocks - roundup(table_size + 8, DISK_BLOCK_SIZE);
	table_size = roundup(num_blocks, 8);
	
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
	
	//Create the root directory - just make sure everything is zero
	for(i = 0; i < DISK_BLOCK_SIZE * 2; i++)
		fwrite(&null_byte, 1, 1, disk);
	
	//empty internal buffer
	fflush(disk);
	
	return 0;
}


