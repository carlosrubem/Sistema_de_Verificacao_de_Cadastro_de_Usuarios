#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "bloom.h"

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
BloomFilter* create_bloom_filter(unsigned int n, double p);

// Insere um ID de usuário no filtro
void insert_bloom(BloomFilter* bf, const char* id);

// Consulta se um ID possivelmente existe no filtro
bool search_bloom(BloomFilter* bf, const char* id);

// Libera a memória alocada pelo filtro
void free_bloom_filter(BloomFilter* bf);

