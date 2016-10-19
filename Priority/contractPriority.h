/*
Contrato de comunicacao

@Autores:
Gustavo B H Azevedo - 1321442
Maria Carolina Santos - 1312063
*/

// Numero maximo de programas de entrada e tamanho maximo do nome dos programas de entrada
#define MAX_ 13

// Chave acordada entre Interpretador e escalonador para chave do PID do Escalonador
key_t ESC_KEY = 3111;

// Chave acordada entre Interpretador e escalonador para chave do PID do Interpretador
key_t INT_KEY = 3112;

// Chave acordada entre Interpretador e escalonador para dados de transferencia (NOME)
key_t TRANSF_NAME_KEY = 3113;

// Chave acordada entre Interpretador e escalonador para dados de transferencia (prioridade)
key_t TRANSF_PRIO_KEY = 3114;
