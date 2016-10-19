/*
Programa focado em chamadas de I/O copia o arquivo de entrada para o arquivo de saída.

@Autores:
Gustavo B H Azevedo - 1321442
Maria Carolina Santos - 1312063
*/
#include<stdio.h>
#include <stdlib.h>

// Funcao principal de trabalho. 42 indica conclusao com sucesso.
int main (void){
	//FILE* output;
	FILE* input;
	int c;

	/*if ((output = fopen ("./saida.txt", "a+")) == NULL){
		printf("Erro ao abrir o arquivo de output!\n");
		exit(-1);
	}*/

	if ((input = fopen ("./entrada.txt", "r")) == NULL){
		printf("Erro ao abrir o arquivo de input!\n");
		exit(-1);
	}

	//fprintf(output, "ioBound2.c started\n");
	printf("ioBound2.c started\n");
	sleep(2);

	while((c = fgetc(input)) != EOF){
			//fprintf(output, "%c",c);
			printf( "%c",c);
	}

	//fprintf(output, "ioBound2.c finished\n");
	printf("ioBound2.c finished\n");

	fclose(input);
	//fclose(output);

	return 42;
}
		


	

