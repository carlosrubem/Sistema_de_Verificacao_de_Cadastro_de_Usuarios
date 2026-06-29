/*#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "bloom.h"
*/
#ifndef BLOOM_H
#define BLOOM_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Constantes internas do algoritmo FNV-1a de 64 bits.
 * Duas sementes distintas geram hash_1 e hash_2, que são combinados
 * via double hashing para simular as k funções sem k implementações.
 */
#define FNV_PRIMO      UINT64_C(1099511628211)
#define FNV_SEMENTE_1  UINT64_C(14695981039346656037)
#define FNV_SEMENTE_2  UINT64_C(2166136261)

/* Macros de acesso ao vetor de bits com operações bitwise puras */
#define BIT_ATIVAR(vetor, pos)    ((vetor)[(pos) >> 3] |= (uint8_t)(1u << ((pos) & 7u)))
#define BIT_VERIFICAR(vetor, pos) ((vetor)[(pos) >> 3] &  (uint8_t)(1u << ((pos) & 7u)))

// Implementação interna do hash FNV-1a com semente configurável
static uint64_t calcular_fnv1a(const char* id, uint64_t semente) {
    uint64_t hash = semente;

    while (*id) {
        hash ^= (uint8_t)(*id++);
        hash *= FNV_PRIMO;
    }
    return hash;
}




/*
 * Comprimento do identificador de usuário no sistema.
 * Formato: [8 letras][3 números] = 11 caracteres úteis + '\0'.
 * Valor alinhado com o campo char id[12] da Tabela Hash.
 */
#define BLOOM_ID_LEN 12

/*
 * Estrutura principal do Filtro de Bloom.
 * O tamanho do vetor de bits (total_bits) e o número de funções hash
 * (num_hashes) são calculados automaticamente por create_bloom_filter()
 * com base no número de elementos esperados (n) e na taxa de falso
 * positivo desejada (p).
 *
 * OBS: Segundo o documento, a melhor implementação deve justificar a
 * dimensão do filtro e a quantidade de funções hash com base em análise
 * quantitativa. Os cálculos seguem as fórmulas de Bloom (1970).
 */
typedef struct {
    uint8_t* vetor_bits;      // Vetor de bits alocado dinamicamente
    uint64_t total_bits;      // Tamanho do filtro em bits (m da fórmula)
    unsigned int num_hashes;  // Número de funções hash simuladas (k da fórmula)
    unsigned int n_elementos; // Número esperado de elementos a inserir
} BloomFilter;

// Cria e inicializa o Filtro de Bloom com dimensionamento automático
BloomFilter* create_bloom_filter(unsigned int n, double p) {
    // 1. Validar os parâmetros de entrada
    if (n == 0 || p <= 0.0 || p >= 1.0) {
        printf("Erro: parametros invalidos para o filtro de bloom (n=%u, p=%.4f).\n", n, p);
        return NULL;
    }

    BloomFilter* bf = (BloomFilter*)malloc(sizeof(BloomFilter));
    if (bf == NULL) {
        printf("Erro ao alocar memoria para o filtro de bloom.\n");
        return NULL;
    }

    // 2. Calcular o tamanho ideal do vetor de bits em bits
    //    Fórmula: m = -(n * ln(p)) / (ln(2))^2
    double ln2 = log(2.0);
    double m_calculado = -((double)n * log(p)) / (ln2 * ln2);
    bf->total_bits = (uint64_t)ceil(m_calculado);

    // 3. Calcular o número ideal de funções hash a simular
    //    Fórmula: k = (m / n) * ln(2)
    double k_calculado = ((double)bf->total_bits / (double)n) * ln2;
    bf->num_hashes = (unsigned int)(k_calculado < 1.0 ? 1 : round(k_calculado));

    bf->n_elementos = n;

    // 4. Alocar e zerar o vetor de bits (ceil(m / 8) bytes no total)
    size_t bytes_necessarios = (size_t)((bf->total_bits + 7ULL) >> 3);
    bf->vetor_bits = (uint8_t*)calloc(bytes_necessarios, sizeof(uint8_t));
    if (bf->vetor_bits == NULL) {
        printf("Erro ao alocar memoria para o vetor de bits do bloom.\n");
        free(bf);
        return NULL;
    }

    return bf;
}

// Insere um ID de usuário no filtro
// Operação de inserção no Filtro de Bloom
void insert_bloom(BloomFilter* bf, const char* id) {
    if (bf == NULL || id == NULL) return;

    // 1. Gerar dois hashes base com sementes distintas (double hashing)
    uint64_t hash_1 = calcular_fnv1a(id, FNV_SEMENTE_1);
    uint64_t hash_2 = calcular_fnv1a(id, FNV_SEMENTE_2) | 1ULL; // força valor ímpar

    // 2. Ativar os k bits via: indice = (hash_1 + i * hash_2) % total_bits
    for (unsigned int i = 0; i < bf->num_hashes; i++) {
        uint64_t indice = (hash_1 + (uint64_t)i * hash_2) % bf->total_bits;
        BIT_ATIVAR(bf->vetor_bits, indice);
    }
}

// Consulta se um ID possivelmente existe no filtro
// Operação de consulta no Filtro de Bloom
bool search_bloom(BloomFilter* bf, const char* id) {
    if (bf == NULL || id == NULL) return false;

    // 1. Gerar os mesmos dois hashes base usados na inserção
    uint64_t hash_1 = calcular_fnv1a(id, FNV_SEMENTE_1);
    uint64_t hash_2 = calcular_fnv1a(id, FNV_SEMENTE_2) | 1ULL;

    // 2. Verificar se todos os k bits correspondentes estão ativos
    for (unsigned int i = 0; i < bf->num_hashes; i++) {
        uint64_t indice = (hash_1 + (uint64_t)i * hash_2) % bf->total_bits;
        if (!BIT_VERIFICAR(bf->vetor_bits, indice)) {
            return false; // Bit apagado: definitivamente não existe
        }
    }

    return true; // Todos os bits ativos: possivelmente existe
}

// Libera a memória alocada pelo filtro
// Libera a memória alocada para evitar memory leaks (boa prática em C)
void free_bloom_filter(BloomFilter* bf) {
    if (bf == NULL) return;

    free(bf->vetor_bits);
    free(bf);
}

#endif
