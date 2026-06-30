#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// Função para validar ID no formato: 8 letras + 3 números
bool validar_id(const char* id) {
    if (id == NULL) {
        return false;
    }
    
    int comprimento = strlen(id);
    
    // Verifica se o comprimento está entre 1 e 8 caracteres
    if (comprimento == 0 || comprimento > 8) {
        return false;
    }
    

    
    return true;
}
// Função para limpar o buffer de entrada
void limpar_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

/* Estrutura principal da Tabela Hash */
typedef struct {
    Node** table; // Vetor de ponteiros para os nós
} HashTable;

// Inicializa a tabela hash
// Cria e inicializa a Tabela Hash
HashTable* create_hash_table() {
    HashTable* ht = (HashTable*)malloc(sizeof(HashTable));
    if (ht == NULL) {
        printf("Erro ao alocar memoria para a tabela hash.\n");
        return NULL;
    }

    // Aloca o vetor de ponteiros e inicializa todos com NULL (calloc)
    ht->table = (Node**)calloc(HASH_SIZE, sizeof(Node*));
    if (ht->table == NULL) {
        printf("Erro ao alocar memoria para os nós da tabela hash.\n");
        free(ht);
        return NULL;
    }

    return ht;
}

// Função Hash manual
// Implementação manual da Função Hash
unsigned int hash_function(const char* str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    
    return hash % HASH_SIZE;
}
// Operação de Busca
// Operação de busca
bool search_hash(HashTable* ht, const char* id) {
    // 1. Calcular o índice
    unsigned int index = hash_function(id);

    // 2. Acessar a lista encadeada no índice correspondente
    Node* current = ht->table[index];

    // 3. Percorrer a lista procurando pelo ID
    while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
            return true; // Encontrou
        }
        current = current->next;
    }

    return false; // Não encontrou
}
// Operação de Inserção
// Operação de inserção com encadeamento externo
bool insert_hash(HashTable* ht, const char* id) {
    // 1. Verificar se o usuário já existe para não inserir duplicados
    if (search_hash(ht, id)) {
        return false; // Usuário já está na tabela
    }

    // 2. Calcular o índice usando a função hash
    unsigned int index = hash_function(id);

    // 3. Criar o novo nó
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) {
        printf("Erro ao alocar memoria para o novo usuario.\n");
        return false;
    }
   
    // Copiar a string para o nó
    strncpy(new_node->id, id, sizeof(new_node->id) - 1);
    new_node->id[11] = '\0'; // Garantir terminação da string

    // 4. Inserir no início da lista encadeada (tratamento de colisão)
    new_node->next = ht->table[index];
    ht->table[index] = new_node;

    return true; // Inserido com sucesso
}



// Libera a memória alocada
// Libera a memória para evitar memory leaks (boa prática em C)
void free_hash_table(HashTable* ht) {
    if (ht == NULL) return;

    for (int i = 0; i < HASH_SIZE; i++) {
        Node* current = ht->table[i];
        while (current != NULL) {
            Node* temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(ht->table);
    free(ht);
}
#endif

