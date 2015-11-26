#include <fcntl.h>       
#include <unistd.h>  
#include <stdio.h>  
#include <stdlib.h>
#include <dirent.h>        
#include <string.h>  
#include <sys/stat.h>    
#include <sys/types.h>  
#include <limits.h>


#define TRUE 1
#define FALSE 0


int ToCheckName = FALSE;
int ToCheckMtime = FALSE;
int ToCheckCtime = FALSE;
int ToAvoidDir = FALSE;

char  thefilename[NAME_MAX+1];
char avoiddir[NAME_MAX+1] ;
int cdays, mdays;


int mystrcmp(char*pattern, char*str)
{
	if(strchr( pattern, '*'))
	{
		int prefix, suffix;
		char *token, *remainstr;
		remainstr = str;
		prefix=FALSE;
		suffix=FALSE;

		if(pattern[0] != '*')
			prefix = TRUE;
		if(pattern[strlen(pattern)-1] != '*')
			suffix = TRUE;

		token = strtok(pattern,"*");
		if(prefix)                                              //ensure the prefix is same
			if(strncmp(str,token,strlen(token)) != 0)
				return FALSE;

		while(token)
		{				
			remainstr = strstr(remainstr,token);
			if(remainstr == NULL)               //can't find 
				return FALSE;
			remainstr += strlen(token);
			token = strtok(NULL,"*");	

			if( token == NULL && suffix)                           //this is  the last token, so have  to check the  suffix if  has
				if( strlen(remainstr ) != 0 )
					return FALSE;
		}
		return TRUE;
	}
	else 
	{
		int len1, len2, i;
		len1 = strlen(pattern);
		len2 = strlen(str);
		if(len1 != len2)
			return FALSE;
		for (i=0; i<len1; i++)
		{
			if(pattern[i] == str[i]  ||  pattern[i] == '?')
				continue;
			return FALSE;
		}
		return TRUE;
	}
}



int dealwithThefile(char* filename, struct stat *statbuf)
{
	int isTheFile = TRUE;
	int secondOfAday =  3600*24; 
	if(ToCheckName)                             //use the parameter -name
		if( mystrcmp(thefilename,filename) ==0 )
			isTheFile = FALSE;
	if(ToCheckCtime)                        //check the create time , actually , it's the status changetime , As i known, there is no way to find the exactly created time of a file
	{
		int thedays;
		thedays =  (time(0) - statbuf->st_ctime)  / secondOfAday;
		if( (cdays>0  && thedays<=cdays) || (cdays<0 && thedays>cdays) )           
			isTheFile = FALSE;
	}
	if(ToCheckMtime)                     //check the modifytime of the file
	{
		int thedays;
		thedays = (time(0) - statbuf->st_mtime) / secondOfAday ;
		if( (mdays>0 && thedays<=mdays) || (mdays<0 && thedays>-mdays) )
			isTheFile = FALSE;
	}
	return isTheFile;
}
void find(char *root)
{
	DIR* dp;
	struct stat statbuf;
	struct dirent *entry;
	long size;
	char *pathbuf;
	char *ptr;

	size = pathconf(".",_PC_PATH_MAX);
	if( ( pathbuf = (char*)malloc((size_t)size)) != NULL)
		ptr = getcwd(pathbuf, (size_t)size);    


	if( (dp = opendir(root)) == NULL)
	{
		perror("opendir error");
		return ;
	}
	chdir(root);
	while( (entry=readdir(dp)) != NULL)
	{
		if( lstat(entry->d_name,&statbuf) == -1)
		{
			perror("lstat error!");
			return;
		}

		if(S_ISDIR(statbuf.st_mode))                    //deal with the directory
		{
			if( strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)      //ignore  "."  and ".."
				continue;
			if(ToAvoidDir && strcmp(avoiddir, entry->d_name) == 0)      //don't go to find in this directory
				continue;
			find(entry->d_name);
		}
		else                                                                //deal with the not directory
			if( dealwithThefile(entry->d_name, &statbuf) )

				printf("%s/%s\n", ptr,entry->d_name);
	}

	chdir("..");
	closedir(dp);
	free(ptr);
}

int main(int argc, char const *argv[])
{
	int i;
	if( argc < 2) 
	{
		printf("wrong usage!!\n");
		return 1;
	}


	/*analyse the parameters*/
	char root[NAME_MAX+1];
	strcpy(root,argv[1]);

	for (i = 2; i < argc; i+=2)       
	{
		if( strcmp("-name",argv[i])==0)
		{
			ToCheckName = TRUE;   
			strcpy(thefilename,argv[i+1]);
		}
		else if( strcmp("-prune", argv[i]) == 0)
		{
		   ToAvoidDir = TRUE;  
		   strcpy(avoiddir, argv[i+1]);
		}
		else if( strcmp("-ctime", argv[i]) == 0)
		{
			ToCheckCtime = TRUE; 
			cdays = atoi(argv[i+1]);
		}
		else if( strcmp("-mtime", argv[i]) == 0)
		{
			ToCheckMtime = TRUE; 
			mdays = atoi(argv[i+1]);
		}

	}

	find(root);
	return 0;
}