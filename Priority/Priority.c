/*
Escalonador com politica de loteria

@Autores:
Gustavo B H Azevedo - 1321442
Maria Carolina Santos - 1312063
*/
#include <time.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include "contractPriority.h"

//Identificador area de memoria do PID deste processo
int ipcid_PID_ESC;

//Identificador area de memoria do PID do processo interpretador
int ipcid_PID_INT;

//Identificador area de memoria compartilhada (NOME do programa)
int ipcid_TRANSF_NAME;

//Identificador area de memoria compartilhada (prioridade)
int ipcid_TRANSF_PRIO;

//Proximo indice vazio.
int nextEmptyIndex = 0;

//Indica se chegou um processo com mais prioridade
int changed = 0;

//pID sendo executado no momento
int currentPID = -1;

//Prioridade do PID do momento
int currentPRIO = 8;

//Mapa de pID para prioridade
int pIDPrioMap[MAX_][2];

//Id deste processo
int* pID_ESC;

//Id do processo escalonador
int* pID_INT = NULL;

//area de memoria compartilhada (prioridade)
int* transfAREA_PRIO = NULL;

//Area de memoria compartilhada (nome do programa)
char* transfAREA_NAME = NULL;

//Vetor de caracteres do nome do programa (memoria local)
char localAREA_NAME[MAX_];

//Arquivo de saída para escrita de stdout
FILE* outputFILE;

//Inicializa o vetor de pIDs para Tickets.
void startPIDS(){
	int i;
	for (i=0; i<MAX_; i++){
		pIDPrioMap[i][0]=-1;
	}
}

//Rotina de limpeza para encerramento
void exitRoutine(){
	shmdt(transfAREA_NAME);
	shmctl(ipcid_TRANSF_NAME, IPC_RMID, 0);
	shmdt(transfAREA_PRIO);
	shmctl(ipcid_TRANSF_PRIO, IPC_RMID, 0);
	shmdt(pID_INT);
	shmctl(ipcid_PID_INT, IPC_RMID, 0);
	shmdt(pID_ESC);
	shmctl(ipcid_PID_ESC, IPC_RMID, 0);
	fclose(outputFILE);
}

//Aguarda sinal de dados escritos iniciar escalonamento
void usr1Handler(int sinal){
	if(transfAREA_NAME==NULL){
		ipcid_TRANSF_NAME = shmget(TRANSF_NAME_KEY, sizeof(char)*MAX_, S_IRUSR|S_IWUSR);
		transfAREA_NAME = (char*) shmat(ipcid_TRANSF_NAME, 0, 0);
	}

	if(transfAREA_PRIO==NULL){
		ipcid_TRANSF_PRIO = shmget(TRANSF_PRIO_KEY, sizeof(int), S_IRUSR|S_IWUSR);
		transfAREA_PRIO = (int*) shmat(ipcid_TRANSF_PRIO, 0, 0);
	}

	int i;
	for(i=0;i<MAX_; i++){
		localAREA_NAME[i]=transfAREA_NAME[i];
	}

	int f = fork();
	if(f!=0){
		kill(f, SIGSTOP);
		printf("ESC: Recebido (%s), pid do novo processo: %d\n", localAREA_NAME, f);
		
		pIDPrioMap[nextEmptyIndex][0] = f;
		pIDPrioMap[nextEmptyIndex][1] = *transfAREA_PRIO;
		nextEmptyIndex++;
		if(currentPRIO>*transfAREA_PRIO){
			changed=1;
		}
		
		if(pID_INT==NULL){
			ipcid_PID_INT = shmget(INT_KEY, sizeof(int), S_IRUSR|S_IWUSR);
			pID_INT = (int*) shmat(ipcid_PID_INT, 0, 0);
		}
		
		kill(*pID_INT, SIGUSR2);
				
	}else{
		char* s[] = {localAREA_NAME, NULL};
		execve(localAREA_NAME, s ,0);
	}


	
}

void sigHandler(int sinal){
	printf("TOMEI EXIT %d\n", sinal);
	exitRoutine();
	exit(42);
}

// Cadastramento do tratador de sinal para liberar recursos
void registerHandler(){
	void (*p) (int);
	p = signal (SIGUSR1, usr1Handler);
	if(p == SIG_ERR){
		printf("Erro de registro SIGUSR1\n");
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

// Recupera o pid de maior prioridade (menor número)
void getNextPid(){
	int index, i, pid = -1, min = 8;
	while(pid==-1){
		for (i=0; i<nextEmptyIndex; i++){
			pid = pIDPrioMap[i][0];
			if(pid==-1) continue;
		
			if(pIDPrioMap[i][1]<min){
				min = pIDPrioMap[i][1];
				index = i;
			}
		}
		pid = pIDPrioMap[index][0];
	}
	currentPID = pid;
	currentPRIO = min;
}

//Remove o pid concluido do mapa
void removePid(int pid){
	int i;
	for (i=0; i<nextEmptyIndex; i++){
		if (pIDPrioMap[i][0]==pid){
			pIDPrioMap[i][0] = -1;
		}
	}
}

// Funcao principal de trabalho. 42 indica conclusao com sucesso
int main (void){
	
	//Cadastramento do PID em memoria
	ipcid_PID_ESC = shmget(ESC_KEY, sizeof(int), IPC_CREAT|IPC_EXCL|S_IRUSR|S_IWUSR);
	pID_ESC = (int*) shmat(ipcid_PID_ESC, 0, 0); 
	*pID_ESC = getpid();
	
	// Cadastramento do tratador de sinal para liberar recursos
	registerHandler();

	//Inicializa mapa de pid para tickets
	startPIDS();
	
	/*if ((outputFILE = fopen ("./saida.txt", "a+")) == NULL){
		printf("Erro ao abrir o arquivo de outputFILE!\n");
		exit(-1);
	}*/
	
	while(1){
		if(nextEmptyIndex>0){
			if(changed){
				printf("ESC: Pid (%d) changed - ENTER!\n", currentPID);
				changed=0;
				if(currentPID!=-1) kill(currentPID, SIGSTOP);
				getNextPid();
				kill(currentPID, SIGCONT);
				printf("ESC: Pid (%d) changed - EXIT!\n", currentPID);
			}else{
				int status;
				pid_t result = waitpid(currentPID, &status, WNOHANG);
				if (result != 0){
					printf("ESC: Pid (%d) concluido!\n", currentPID);
					removePid(currentPID);
					getNextPid();
					kill(currentPID, SIGCONT);
					printf("ESC: Pid (%d) Executando!\n", currentPID);
				}
			}
		}
	}
	return 42;
}
