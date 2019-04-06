#include "fs.h"

int main()
{
	file_t *file = NULL;
	char str[256], buf[256];
	int i = 0, j = 0;

	disk_t *disk = fs_init("examples/disk");

	fs_dcreate(disk, "/dir");
	fs_dcreate(disk, "/dir/sub");

	file = fs_fopen(disk, "/dir/sub/file.txt");
	printf("opened /dir/sub/file.txt\n");

	printf("Enter a string: ");
	fgets(str, 256, stdin);

	fs_fwrite(str, 1, strlen(str) + 1, file);
	printf("wrote string to /dir/sub/file.txt\n");

	fs_fclose(file);
	printf("closed /dir/sub/file.txt\n");

	file = fs_fopen(disk, "/dir/sub/file.txt");
	printf("opened /dir/sub/file.txt again\n");

	fs_fread(str, 1, strlen(str) + 1, file);
	printf("read back: \"%s\"\n", str);

	return 0;
}
