/*
Programa focado em chamadas de CPU. Tenta adivinhar os 5 primeiros dígitos de Pi, chutando randomicamente números entre 0 e 1.000.000.

@Autores:
Gustavo B H Azevedo - 1321442
Maria Carolina Santos - 1312063
*/

//Senha.
#define PASS 31417

//Tamanho da senha
#define SIZE 23

// Funcao principal de trabalho. 42 indica conclusao com sucesso.
int main (void){
	int i;
	srand(5000);
	
	do{
		do{
			i = rand();
		}while(0<i && i>1000000);	
	}while(i!=PASS);
	 
	return 42;
}
