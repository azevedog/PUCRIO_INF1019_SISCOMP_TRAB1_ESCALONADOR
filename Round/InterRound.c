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
#include "contractRound.h"

//Identificador area de memoria do PID deste processo
int ipcid_PID_INT;

//Identificador area de memoria do PID do processo escalonador
int ipcid_PID_ESC;

//Identificador area de memoria compartilhada para o nome do programa
int ipcid_TRANSF;

//Id deste processo
int* pID_INT;

//Id do processo escalonador
int* pID_ESC = NULL;

//Vetor de caracteres (nome do programa)
char* transfAREA;

//Arquivo de entrada com os nomes dos programas a serem executados
FILE* inputFILE;

//Indica que a memoria compartilhada esta ocupada e o programa deve aguardar
int canWrite = 0;

//Rotina de limpeza para encerramento
void exitRoutine(){
	shmdt(transfAREA);
	shmctl(ipcid_TRANSF, IPC_RMID, 0);
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
		printf("Erro ao abrir o arquivo de INPUT!\n");
		exit(-1);
	}

	ipcid_PID_ESC = shmget(ESC_KEY, sizeof(int), S_IRUSR|S_IWUSR);
	pID_ESC = (int*) shmat(ipcid_PID_ESC, 0, 0);
	

	//Criacao da infra compartilhada
	ipcid_TRANSF = shmget(TRANSF_KEY, sizeof(char*)*MAX_, IPC_CREAT|IPC_EXCL|S_IRUSR|S_IWUSR);
	transfAREA = (char*) shmat(ipcid_TRANSF, 0, 0); 
	
	//Interpretacao do entrada.txt para a area compartilhada
	char current;
	int i=0;
	while(1){
		if(!canWrite){
			current = fgetc(inputFILE);
			if(current == EOF){
				exitRoutine();
			}else{
				transfAREA[i] = current;
				if(current=='\n'){
					transfAREA[i] = '\0';
					canWrite=1;
					i=0;
					kill(*pID_ESC, SIGUSR1);
				}else{
					i++;
				}
			}
		}
	}
	
	return 42;
}


