#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <sys/file.h>
#include <signal.h>
#include <semaphore.h>

int shmid;//Segmentid for mutex

int wordCount(char *buff);

void masterExit(int sig){
	switch(sig){
		case SIGALRM:
			printf("\n2 minutes has passed. The program will now terminate.\n");
			break;
		case SIGINT:
			printf("\nUser has exited using ctrl+c\n");
			break;
	}
	//detach shared mem
	shmctl(shmid, IPC_RMID, NULL);
	sem_unlink("proj3sem");	
	kill(0,SIGKILL);


}

int main(int argc, char *argv[]){
	signal(SIGALRM, masterExit);
	alarm(120);
	signal(SIGINT,masterExit);
	char *buff;//Will write file to this and count number of words from here
	int n = 0;//Store max number of children
	int s = 0;//Max number in system at any given time
	char *i = NULL; //Inputfile
	//Get command line arguments	
	int c;	
	while((c=getopt(argc, argv, "n:s:i:h"))!= EOF){
		switch(c){
			case 'h':
				printf("\n\n-n: Total number of processes(Default of 4)");
				printf("\n-s: Total number of processes in machine at any given time(Default of 2)");
				printf("\n-i: Specify default inputfile(Default is input.txt)\n\n");
				exit(0);
				break;
			case 'n':
				n = atoi(optarg);
				break;
			case 's':
				s = atoi(optarg);
				if(s > 20){
					printf("Maximum number of child processes running on machine is 20. Setting s to 20.");
					s = 20;
				}
				break;
			case 'i':
				i = optarg;
				break;
		}
	}
	//open inputfile
	FILE *fp;
	fp = fopen(i,"r");
	if(fp == NULL){
		fprintf(stderr, "%s: Error in %s: ", argv[0] , i);
		perror("");
		exit(0);
	}

	//Create output files if dont exist
	FILE *fp1 = fopen("palin.out","w");
	FILE *fp2 = fopen("nopalin.out","w");
		
	//write file to buffer
	fscanf(fp,"%m[^EOF]",&buff);

	//Store words in string array
	int count = wordCount(buff);
	
	//set defailt files
	if(i == NULL){
		i = "input.txt";
		
	}
	if(n == 0){
		n = count;
	}
	if(s == 0){
		s = 2;
	}

	if(count != 0){
		//mylist count = # of words
		char (*mylist)[count];
	
		//create shared memory for string array
		key_t mem_key = ftok(".",'a');
		shmid = shmget(mem_key, count* sizeof(char), IPC_CREAT | 0666);
		if(shmid < 0){
			printf("Error in shmget");
			exit(1);
		}		
		//attach shared mem
		mylist = shmat(shmid, (void *)0,0);
		if((intptr_t)mylist == -1){
			printf("\n Error in shmat \n");
			exit(1);
		}
		//read words from file
		char delims[4] = {' ','\n','\t','\0'};	
		int i;
		strcpy(mylist[0],strtok(buff,delims));
		for(i = 1; i < count; i++){
			strcpy(mylist[i],strtok(NULL,delims));
		}
		//Create semaphore
		sem_t* sem;
		sem = sem_open("proj3sem", O_CREAT, 0644, 1);
		if(sem == SEM_FAILED){
			fprintf(stderr,"%s: Error in sem_open",argv[0]);
			exit(0);
		}
		int status;
		int active = 1;
		int k = 0;
		pid_t pids[n],wpid;
		while(k < count){//Until n has been met or all words processed
			if(active < s){
				pids[k] = fork();
				if(pids[k] == 0){
					char num[10];
					char num2[10];
					snprintf(num, 10, "%d", k);
					snprintf(num2, 10, "%d", count);
					execl("./palin",num,num2,NULL);
					exit(0);
				}	
				active++;	
				k = k + 5;
				n--;
				//if active proccesses hit s, then find first active child and wait for it
				if(active == s){
					//loop through pid array to find first running child
					int m;
					for(m = 0; m < n; m++){
						pid_t tempId = waitpid(pids[m], &status, WNOHANG);
						if(tempId == 0){
							waitpid(tempId, &status, 0);
							active--;
							break;
						}
					}
				}
						
				//Wait for all childern to finish if all words
				//are passed or n is met
				if(k >= count){
					while((wpid = wait(&status)) > 0);
					break;
				}
			}
		}
		printf("All finished!\n");
		//detach shared mem
		shmdt((void*)mylist);
		//remove shared mem
		shmctl(shmid, IPC_RMID, NULL);
		sem_unlink("proj3sem");
	}else{
		fprintf(stderr, "\n%s: ERROR: Inputfile contains no words\n",argv[0]);
	}
	
}

int wordCount(char *buff){
	int count = 0, i , length;
	if(buff == NULL) return count;
	length = strlen(buff);
	for(i = 0; i <= length; i++){
		if(buff[i] == ' ' && (buff[i-1] != '\t' && buff[i-1] != '\n' && buff[i-1] != ' ')
			|| (buff[i] == '\t' && (buff[i-1] != ' ' && buff[i-1] != '\n' && buff[i-1] != '\t' && (i >= 1)))
			|| (buff[i] == '\n' && (buff[i-1] != ' ' && buff[i-1] != '\t' && buff[i-1] != '\n' && (i >= 1)))
			|| (buff[i] == '\0' && (buff[i-1] != ' ' && buff[i-1] != '\t' && buff[i-1] != '\n' && (i >= 1)))){

			count++;
		}
	}
	return count;
}
