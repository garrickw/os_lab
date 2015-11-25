#include <fcntl.h>       
#include <unistd.h>  
#include <stdio.h>  
#include <stdlib.h>
#include <dirent.h>        
#include <string.h>  
#include <sys/stat.h>    
#include <sys/types.h>  
#include <limits.h>
#include <math.h>

#define TRUE 1
#define FALSE 0


long int total = 0;

/*record[x] 用来记录2^x  bytes以下的文件数量*/
long int record[28] = {0};

/*从根目录开始统计所有的文件*/
char *root = "/";

void gothrough_record(const char*root)
{
	DIR* dp;
	struct stat statbuf;
	struct dirent *entry;
	int power, i;
	long size;
	char childpath[PATH_MAX+1];

	memset(childpath, 0, sizeof(childpath));

	if( (dp = opendir(root)) == NULL)
	{
		perror("opendir error");
		exit(-127) ;
	}

	while( (entry=readdir(dp)) != NULL)
	{

		sprintf(childpath, "%s/%s", root,entry->d_name);
		if( entry->d_type & DT_DIR)
		{
			if( strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)      //ignore  "."  and ".."
				continue;
			gothrough_record(childpath);
		}
		else  //if(S_ISREG(statbuf.st_mode))       /*not a directory, record its size*/
		{
			++total;
			lstat(childpath, &statbuf);
			size = statbuf.st_size;
			power =  (size == 0 ? 0 : ceil(log2(size)));    
			for(i=power; i<28; i++)
				record[i] += 1;
		}
	}
 
	closedir(dp);    
}


int main(int argc, char const *argv[])
{
	gothrough_record(argv[1]);
	printf("total: %ld\n",  total);
	int i;
	for(i=0; i<28; i++)
		printf("%d kb:  %ld\n", (int)pow(2,i), record[i]);
	return 0;
}