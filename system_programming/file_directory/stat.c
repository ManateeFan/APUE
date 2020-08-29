#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char* argv[])
{
	struct stat testStat;
	// lstat 不会穿透链接，stat会穿透链接得到最终的文件本质属性
	int res = stat(argv[1], &testStat);
	if(res ==-1)
	{
		perror("stat error");
		exit(1);
	}
	
	printf("File size:%lld\n", testStat.st_size);
	printf("Is regular file? : %d",S_ISREG(testStat.st_mode));
	exit(0);
}
