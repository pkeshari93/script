/*****************************************************************************
 * Name : Prakhar Keshari
 * Username : pkeshari
 * Course : CS3411 Fall 2017
 * Description : Program 4, 
 				shell like implementation that executes a program and captures
 				data with use of pipes
 ******************************************************************************
 */

#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
 #include <sys/wait.h>

struct stat dir_stat;
int i;
int pipe1[2]; //pipe that reads from stdin in script and pipes data to application program 
int pipe2[2];
int pipe3[2];
int fdin, fdout, fderr;

/*
 *Function : concatenates two strings
 *Return   : returns the resulted string result
 *Args	   : s1: string 1 to be concatenated
 		   : s2: string 2 to be concatenated
*/ 	
char* concat(const char *s1, const char *s2 ){
    char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the null-terminator
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

/*
 *Function : creates directory and log file
 *Return   : void
 *Args	   : argc and argv
*/ 	
void dircheck(int argc, char* argv[]){

	char path0[strlen(argv[argc-1])];
	char path1[strlen(argv[argc-1])];
	char path2[strlen(argv[argc-1])];
	//check if directory exists, else create
	if(lstat(argv[argc-1], &dir_stat) == 0){
		if(!(dir_stat.st_mode & S_IFDIR)){
			perror("Directory error");
			exit(-1);
		}
	}
	else{
		if(mkdir(argv[argc-1], 0777) != 0){
			perror("mkdir error");
			exit(-1);
		}

		strcpy(path0, argv[argc-1]);
		strcat(path0, "/0");
		fdin = open(path0, O_WRONLY | O_CREAT);
		if(fdin < 0) printf("error\n\n");

		strcpy(path1, argv[argc-1]);
		strcat(path1, "/1");
		fdout = open(path1, O_WRONLY | O_CREAT);
		if(fdout < 0) printf("error\n\n");

		strcpy(path2, argv[argc-1]);
		strcat(path2, "/2");
		fderr = open(path2, O_WRONLY | O_CREAT);
		if(fderr < 0) printf("error\n\n");
	}
}

int main(int argc, char* argv[]){

	fd_set rfds;
	// struct timeval tv = {1, 0};
	int retval;
	char buf[1024];
	// char bufout[1024];
	// char buferr[1024];
	int readval = 0;
	// int readvalerr = 0;
	// int readvalin = 0;

	//store program arguments into an array
	// char *args_array[argc - 1];
	// for (i = 0; i < argc - 2; i ++){
	// 	//------------------------------------------------------------------------ask if this is right
	// 	args_array[i] = argv[i+1];
	// 	// printf("args_array[%d] = %s\n", i, args_array[i]);
	// }

	char *args_array[argc - 2];
	for (i = 1; i < argc - 1; i ++)
    	args_array[i-1] = argv[i];
	
	args_array[argc-1] = NULL;

	for (i = 0; i < argc - 2; i ++){
		// args_array[i] = argv[i+1];
		printf("args_array[%d] = %s\n", i, args_array[i]);
	}

	
	//create pipes
	pipe(pipe1);
	pipe(pipe2);
	pipe(pipe3);

	//child to setup pipes and exec the program 
	if(fork() == 0){

		close(pipe2[0]);
		close(pipe3[0]);
		//making child connect to stdin
		close(pipe1[1]);
		close(0);
		dup2(pipe1[0], 0);
		close(pipe1[0]);


		//redirecting childs stdout
		close(1);
		dup2(pipe2[1], 1);
		close(pipe2[1]);

		//redirecting childs stderr
		close(2);
		dup2(pipe3[1], 2);
		close(pipe3[1]);

		//exec an application program 
		if(execv(argv[1], args_array) < 0){
			perror("execerr");
		}
		exit(0);
	}

	else{

		dircheck(argc, argv);

		close(pipe2[1]);
		close(pipe3[1]);

    	FD_ZERO(&rfds);
    	FD_SET(0, &rfds);
    	FD_SET(pipe1[0], &rfds);
    	FD_SET(pipe2[0], &rfds);
    	FD_SET(pipe3[0], &rfds);
    	/* Wait up to five seconds. */
    	// tv.tv_sec = 5;

    	while(1){

    		//resetting the fd sets
    		FD_ZERO(&rfds);
    		FD_SET(0, &rfds);
    		FD_SET(pipe1[0], &rfds);
    		FD_SET(pipe2[0], &rfds);
    		FD_SET(pipe3[0], &rfds);


    		retval = select(pipe3[0] + 1, &rfds, NULL, NULL, NULL);


    		if(FD_ISSET(0, &rfds)){
    			printf("Entered the while for writing into file 0\n");
    			while((readval = read(0, buf, sizeof(buf))) > 0){
    				write(fdin, buf, readval);
    				write(pipe1[1], buf, readval);
    			}
    		}

    		if(FD_ISSET(pipe2[0], &rfds)){
    			while((readval = read(pipe2[0], buf, sizeof(buf))) > 0){
    				
    				write(fdout, buf, readval);
    				write(1, buf, readval);
    			}
    		}

    		if(FD_ISSET(pipe3[0], &rfds)){
    			while((readval = read(pipe2[0], buf, sizeof(buf))) > 0){
    				
    				write(fderr, buf, readval);
    				write(2, buf, readval);
    			}
    		}
    		wait(NULL);
		    close(pipe1[1]);
			close(pipe2[0]);
			close(pipe3[0]);
			FD_CLR(pipe2[0], &rfds);
			FD_CLR(pipe3[0], &rfds);
			FD_CLR(pipe1[1], &rfds);
			break;
		}

	}
	close(fdin);
	close(fdout);
	close(fderr);
	return 0;
}
