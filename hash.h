#ifndef HASH_H
#define HASH_H

#include <stdbool.h>

/*
 * O tamanho da Tabela Hash deve ser um número primo para minimizar colisões.
 * OBS: Segundo o documento, a melhor implementação deve justificar a 
 * dimensão da tabela considerando o fator de carga (load factor).
 * Você poderá alterar este valor posteriormente para os testes de 1.000, 
 * 10.000 e 100.000 registros.
 */
#define HASH_SIZE 100003 

/*
 * Estrutura do nó para tratar colisões via Encadeamento Externo.
 * O ID possui 11 caracteres (8 letras + 3 números) + 1 caractere nulo (\0).
 */
typedef struct Node {
    char id[12];
    struct Node* next;
} Node;

/* Estrutura principal da Tabela Hash */
typedef struct {
    Node** table; // Vetor de ponteiros para os nós
} HashTable;

// Inicializa a tabela hash
HashTable* create_hash_table();

// Função Hash manual
unsigned int hash_function(const char* str);

// Operação de Inserção
bool insert_hash(HashTable* ht, const char* id);

// Operação de Busca
bool search_hash(HashTable* ht, const char* id);

// Libera a memória alocada
void free_hash_table(HashTable* ht);

#endif