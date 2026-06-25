#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"

// Implementação manual da Função Hash
// Utiliza o clássico algoritmo djb2, que é excelente para strings.
unsigned int hash_function(const char* str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    
    return hash % HASH_SIZE;
}

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