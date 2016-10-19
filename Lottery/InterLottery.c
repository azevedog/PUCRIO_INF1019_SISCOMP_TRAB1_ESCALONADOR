/*
Interpretador de nome de programas

@Autores:
Gustavo B H Azevedo - 1321442
Maria Carolina Santos - 1312063
*/

#include <signal.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include "contractLottery.h"

//Identificador area de memoria do PID deste processo
int ipcid_PID_INT;

//Identificador area de memoria do PID do processo escalonador
int ipcid_PID_ESC;

//Identificador area de memoria compartilhada (NOME do programa)
int ipcid_TRANSF_NAME;

//Identificador area de memoria compartilhada (tickets)
int ipcid_TRANSF_TICK;

//Indica se a memoria compartilhada esta ocupada ou se o programa pode escrever nela
int canWrite = 0;

//Id deste processo
int* pID_INT;

//Id do processo escalonador
int* pID_ESC = NULL;

//area de memoria compartilhada (nome do programa)
char* transfAREA_NAME;

//area de memoria compartilhada (tickets)
int* transfAREA_TICK;

//Arquivo de entrada com os nomes dos programas a serem executados
FILE* inputFILE;

//Rotina de limpeza para encerramento
void exitRoutine(){
	shmdt(transfAREA_NAME);
	shmctl(ipcid_TRANSF_NAME, IPC_RMID, 0);
	shmdt(transfAREA_TICK);
	shmctl(ipcid_TRANSF_TICK, IPC_RMID, 0);
	shmdt(pID_INT);
	shmctl(ipcid_PID_INT, IPC_RMID, 0);
	shmdt(pID_ESC);
	shmctl(ipcid_PID_ESC, IPC_RMID, 0);
	fclose(inputFILE);
	exit(42);
}

//Aguarda sinal de dados lidos para liberar recursos. 42 indica conclusao com sucesso
void sigHandler(int sinal){
	exitRoutine();
}

//Aguarda sinal de dados lidos para liberar recursos. 42 indica conclusao com sucesso
void usr2Handler(int sinal){
	canWrite=0;
}

// Cadastramento do tratador de sinal para liberar recursos
void registerHandler(){
	void (*p) (int);
	p = signal (SIGUSR2, usr2Handler);
	if(p == SIG_ERR){
		printf("Erro de registro SIGUSR2\n");
		exit(-1);
	}
	p = signal (SIGSEGV, sigHandler);
	if(p == SIG_ERR){
		printf("Erro de registro SIGSEGV\n");
		exit(-1);
	}
	p = signal (SIGQUIT, sigHandler);
	if(p == SIG_ERR){
		printf("Erro de registro SIGQUIT\n");
		exit(-1);
	}
	p = signal (SIGTERM, sigHandler);
	if(p == SIG_ERR){
		printf("Erro de registro SIGTERM\n");
		exit(-1);
	}
}

//Zera a area de transferência para evitar lixo.
void resetTransfTick(){
	int i;
	for(i=0;i<MAX_TICK; i++){
		transfAREA_TICK[i]=0;
	}
}

// Funcao principal de trabalho. 42 indica conclusao com sucesso
int main (void){

	//Cadastramento do PID em memoria
	ipcid_PID_INT = shmget(INT_KEY, sizeof(int), IPC_CREAT|IPC_EXCL|S_IRUSR|S_IWUSR);
	pID_INT = (int*) shmat(ipcid_PID_INT, 0, 0); 
	*pID_INT = getpid();
	
	// Cadastramento do tratador de sinal para liberar recursos
	registerHandler();

	//Abertura do arquivo de entrada para leitura
	if ((inputFILE = fopen ("./entrada.txt", "r")) == NULL){
		printf("Erro ao abrir o arquivo de inputFILE!\n");
		exit(-1);
	}

	ipcid_PID_ESC = shmget(ESC_KEY, sizeof(int), S_IRUSR|S_IWUSR);
	pID_ESC = (int*) shmat(ipcid_PID_ESC, 0, 0);
	

	//Criacao da infra compartilhada

	ipcid_TRANSF_NAME = shmget(TRANSF_NAME_KEY, sizeof(char)*MAX_, IPC_CREAT|IPC_EXCL|S_IRUSR|S_IWUSR);
	transfAREA_NAME = (char*) shmat(ipcid_TRANSF_NAME, 0, 0);

	ipcid_TRANSF_TICK = shmget(TRANSF_TICK_KEY, sizeof(int)*MAX_TICK, IPC_CREAT|IPC_EXCL|S_IRUSR|S_IWUSR);
	transfAREA_TICK = (int*) shmat(ipcid_TRANSF_TICK, 0, 0);
	
	//Interpretacao do entrada.txt para a area compartilhada
	while(1){
		if(!canWrite){
			resetTransfTick();
			int i=0;
			char current;
			while(1){
				current = fgetc(inputFILE);
				if(current == '{') break;
				if(current == EOF){
					exitRoutine();
				}
				transfAREA_NAME[i]=current;
				i++;
			}
			transfAREA_NAME[i]='\0';

			i=0;
			int j=0, aux;
			char buffer[2];
			buffer[0] = ' ';
			buffer[1] = ' ';
			while((current = fgetc(inputFILE))!='\n'){
				if(j==2 || current==',' || current=='}'){
					sscanf(buffer, "%d", &aux);
					transfAREA_TICK[i++] = aux;
					j=0;
				}else {
					buffer[j++]=current;
				}
			}

			if(current=='\n'){
				canWrite=1;
				kill(*pID_ESC, SIGUSR1);
			}
		}
	}
	
	return 42;
}


