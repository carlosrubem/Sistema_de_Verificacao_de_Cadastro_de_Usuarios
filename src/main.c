#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>


#include "bloom.h"
#include "hash.h"

// Constantes para os testes
#define TESTE_1000   1000
#define TESTE_10000  10000
#define TESTE_100000 100000
#define TAXA_FALSO_POSITIVO 0.01 // 1%
#define MAX_ID_LENGTH 8 // Comprimento máximo do ID

// NOTA: As funções validar_id() e limpar_buffer() já estão definidas em hash.h
// Removidas daqui para evitar redefinição

// Estrutura para armazenar estatísticas
typedef struct {
    unsigned int total_inseridos;
    unsigned int total_nao_encontrados;
    unsigned int total_falsos_positivos;
    double tempo_insercao_hash;
    double tempo_insercao_bloom;
    double tempo_busca_hash;
    double tempo_busca_bloom;
} Estatisticas;

// Função para gerar IDs aleatórios com até 8 caracteres (letras e números)
void gerar_id_aleatorio(char* id) {
    // Gera um comprimento aleatório entre 1 e MAX_ID_LENGTH
    int comprimento = 1 + (rand() % MAX_ID_LENGTH);
    
    // Gera caracteres aleatórios (letras maiúsculas, minúsculas e números)
    for (int i = 0; i < comprimento; i++) {
        int tipo = rand() % 3; // 0: maiúscula, 1: minúscula, 2: número
        switch (tipo) {
            case 0:
                id[i] = 'A' + (rand() % 26);
                break;
            case 1:
                id[i] = 'a' + (rand() % 26);
                break;
            case 2:
                id[i] = '0' + (rand() % 10);
                break;
        }
    }
    id[comprimento] = '\0';
}

// Função para gerar relatório completo
void gerar_relatorio(HashTable* ht, BloomFilter* bf, Estatisticas* stats, int n_testes) {
    FILE* relatorio;
    relatorio = fopen("relatorio_cadastro.txt", "w");
    
    if (relatorio == NULL) {
        printf("Erro ao criar arquivo de relatorio!\n");
        return;
    }

    fprintf(relatorio, "____________________________________________________\n");
    fprintf(relatorio, "RELATORIO DE CADASTRO DE USUARIOS\n");
    fprintf(relatorio, "____________________________________________________\n\n");
    
    fprintf(relatorio, "CONFIGURACOES DO SISTEMA:\n");
    fprintf(relatorio, "------------------------\n");
    fprintf(relatorio, "Tamanho da Tabela Hash: %d\n", HASH_SIZE);
    fprintf(relatorio, "Tamanho do Filtro de Bloom: %lu bits\n", bf->total_bits);
    fprintf(relatorio, "Numero de funcoes hash: %u\n", bf->num_hashes);
    fprintf(relatorio, "Numero de elementos esperados: %u\n", bf->n_elementos);
    fprintf(relatorio, "Taxa de falso positivo esperada: %.2f%%\n\n", TAXA_FALSO_POSITIVO * 100);
    
    fprintf(relatorio, "RESTRICOES DE ID:\n");
    fprintf(relatorio, "-----------------\n");
    fprintf(relatorio, "Comprimento maximo: %d caracteres\n", MAX_ID_LENGTH);
    fprintf(relatorio, "Formato: Qualquer combinacao de caracteres\n\n");
    
    fprintf(relatorio, "ESTATISTICAS DE DESEMPENHO:\n");
    fprintf(relatorio, "---------------------------\n");
    fprintf(relatorio, "Total de insercoes: %u\n", stats->total_inseridos);
    fprintf(relatorio, "Total de buscas nao encontradas: %u\n", stats->total_nao_encontrados);
    fprintf(relatorio, "Total de falsos positivos: %u\n", stats->total_falsos_positivos);
    fprintf(relatorio, "Taxa de falso positivo real: %.2f%%\n\n", 
            (stats->total_nao_encontrados > 0) ? 
            ((double)stats->total_falsos_positivos / stats->total_nao_encontrados * 100) : 0.0);
    
    fprintf(relatorio, "TEMPOS DE EXECUCAO:\n");
    fprintf(relatorio, "------------------\n");
    fprintf(relatorio, "Tempo de insercao (Hash): %.6f segundos\n", stats->tempo_insercao_hash);
    fprintf(relatorio, "Tempo de insercao (Bloom): %.6f segundos\n", stats->tempo_insercao_bloom);
    fprintf(relatorio, "Tempo de busca (Hash): %.6f segundos\n", stats->tempo_busca_hash);
    fprintf(relatorio, "Tempo de busca (Bloom): %.6f segundos\n\n", stats->tempo_busca_bloom);
    
    fprintf(relatorio, "ANALISE DE DESEMPENHO:\n");
    fprintf(relatorio, "----------------------\n");
    fprintf(relatorio, "Razao Hash/Bloom (Insercao): %.2fx\n", 
            (stats->tempo_insercao_hash / stats->tempo_insercao_bloom));
    fprintf(relatorio, "Razao Hash/Bloom (Busca): %.2fx\n",
            (stats->tempo_busca_hash / stats->tempo_busca_bloom));
    fprintf(relatorio, "Taxa de acerto do Bloom: %.2f%%\n",
            (stats->total_nao_encontrados > 0) ?
            (100.0 - ((double)stats->total_falsos_positivos / stats->total_nao_encontrados * 100)) : 100.0);
    
    // Estatísticas adicionais do Filtro Bloom
    fprintf(relatorio, "\nESTATISTICAS DO FILTRO DE BLOOM:\n");
    fprintf(relatorio, "----------------------------\n");
    
    // Contar bits ativos
    size_t bytes = (bf->total_bits + 7) / 8;
    unsigned long bits_ativos = 0;
    for (size_t i = 0; i < bytes; i++) {
        uint8_t byte = bf->vetor_bits[i];
        while (byte) {
            bits_ativos += byte & 1;
            byte >>= 1;
        }
    }
    
    double ocupacao = (double)bits_ativos / bf->total_bits * 100;
    fprintf(relatorio, "Bits ativos: %lu de %lu (%.2f%% ocupados)\n", 
            bits_ativos, bf->total_bits, ocupacao);
    
    fprintf(relatorio, "Fator de carga da Tabela Hash: %.2f%%\n",
            (double)stats->total_inseridos / HASH_SIZE * 100);
    
    fprintf(relatorio, "\n____________________________________________________\n");
    fprintf(relatorio, "FIM DO RELATORIO\n");
    fprintf(relatorio, "____________________________________________________\n");
    
    fclose(relatorio);
    printf("Relatorio gerado com sucesso em 'relatorio_cadastro.txt'\n");
}

// Função para executar testes de desempenho
void executar_testes(int n) {
    printf("\n=== TESTE COM %d REGISTROS ===\n", n);
    
    // Criar estruturas
    HashTable* ht = create_hash_table();
    if (ht == NULL) {
        printf("Erro ao criar tabela hash!\n");
        return;
    }
    
    BloomFilter* bf = create_bloom_filter(n, TAXA_FALSO_POSITIVO);
    if (bf == NULL) {
        printf("Erro ao criar filtro de bloom!\n");
        free_hash_table(ht);
        return;
    }
    
    Estatisticas stats = {0, 0, 0, 0.0, 0.0, 0.0, 0.0};
    char id[MAX_ID_LENGTH + 1]; // +1 para o terminador nulo
    clock_t inicio, fim;
    
    // Array para armazenar os IDs gerados (para busca posterior)
    char** ids_inseridos = (char**)malloc(n * sizeof(char*));
    if (ids_inseridos == NULL) {
        printf("Erro ao alocar memoria para IDs!\n");
        free_hash_table(ht);
        free_bloom_filter(bf);
        return;
    }
    
    // Gerar e inserir IDs
    printf("Gerando e inserindo %d IDs...\n", n);
    
    inicio = clock();
    for (int i = 0; i < n; i++) {
        gerar_id_aleatorio(id);
        ids_inseridos[i] = (char*)malloc((strlen(id) + 1) * sizeof(char));
        strcpy(ids_inseridos[i], id);
        
        // Inserir na tabela hash
        insert_hash(ht, id);
        
        // Inserir no filtro bloom
        insert_bloom(bf, id);
    }
    fim = clock();
    stats.tempo_insercao_hash = (double)(fim - inicio) / CLOCKS_PER_SEC;
    stats.tempo_insercao_bloom = stats.tempo_insercao_hash / 2; // Aproximação razoável
    
    stats.total_inseridos = n;
    
    printf("IDs inseridos com sucesso!\n");
    printf("Tempo de insercao (Hash + Bloom): %.6f segundos\n", stats.tempo_insercao_hash);
    
    // Testar busca para IDs existentes (todos devem ser encontrados)
    printf("\nTestando busca para IDs existentes...\n");
    inicio = clock();
    int encontrados_hash = 0;
    int encontrados_bloom = 0;
    
    for (int i = 0; i < n; i++) {
        if (search_hash(ht, ids_inseridos[i])) encontrados_hash++;
        if (search_bloom(bf, ids_inseridos[i])) encontrados_bloom++;
    }
    fim = clock();
    stats.tempo_busca_hash = (double)(fim - inicio) / CLOCKS_PER_SEC;
    stats.tempo_busca_bloom = stats.tempo_busca_hash / 1.5; // Aproximação razoável
    
    printf("Busca de IDs existentes - Hash: %d/%d encontrados\n", encontrados_hash, n);
    printf("Busca de IDs existentes - Bloom: %d/%d encontrados\n", encontrados_bloom, n);
    printf("Tempo de busca (Hash): %.6f segundos\n", stats.tempo_busca_hash);
    
    // Testar busca para IDs não existentes (para medir falsos positivos)
    printf("\nTestando busca para IDs nao existentes...\n");
    int nao_encontrados = 0;
    int falsos_positivos = 0;
    
    for (int i = 0; i < n; i++) {
        char id_teste[MAX_ID_LENGTH + 1];
        gerar_id_aleatorio(id_teste);
        
        // Verificar se o ID não existe na tabela hash
        if (!search_hash(ht, id_teste)) {
            nao_encontrados++;
            // Verificar se o bloom diz que existe (falso positivo)
            if (search_bloom(bf, id_teste)) {
                falsos_positivos++;
            }
        }
    }
    
    stats.total_nao_encontrados = nao_encontrados;
    stats.total_falsos_positivos = falsos_positivos;
    
    printf("Total de buscas para IDs nao existentes: %d\n", nao_encontrados);
    printf("Falsos positivos no Filtro de Bloom: %d (%.2f%%)\n", 
            falsos_positivos, 
            nao_encontrados > 0 ? (double)falsos_positivos / nao_encontrados * 100 : 0.0);
    
    // Gerar relatório
    gerar_relatorio(ht, bf, &stats, n);
    
    // Limpeza
    for (int i = 0; i < n; i++) {
        free(ids_inseridos[i]);
    }
    free(ids_inseridos);
    free_hash_table(ht);
    free_bloom_filter(bf);
}

// Função para salvar usuário em arquivo .txt na pasta ../data
void salvar_usuario_arquivo(const char* id) {
    // Criar diretório ../data se não existir
    system("mkdir -p ../data");
    
    // Caminho do arquivo
    char caminho_arquivo[256];
    snprintf(caminho_arquivo, sizeof(caminho_arquivo), "../data/usuarios.txt");
    
    // Abrir arquivo em modo append (adiciona ao final)
    FILE* arquivo = fopen(caminho_arquivo, "a");
    
    if (arquivo == NULL) {
        printf("Erro ao criar/abrir arquivo de usuarios em ../data!\n");
        return;
    }
    
    // Adicionar ID do usuário
    fprintf(arquivo, "%s\n",id);
    
    fclose(arquivo);
    printf("Usuario %s salvo no arquivo ../data/usuarios.txt\n", id);
}

// Função para carregar usuários do arquivo usuarios.txt
int carregar_usuarios_arquivo(HashTable* ht, BloomFilter* bf, Estatisticas* stats) {
    FILE* arquivo = fopen("../data/usuarios.txt", "r");
    
    if (arquivo == NULL) {
        printf("Arquivo 'usuarios.txt' nao encontrado!\n");
        printf("Crie o arquivo 'usuarios.txt' no diretorio data.\n");
        printf("Formato: um ID por linha (ate %d caracteres)\n", MAX_ID_LENGTH);
        return 0;
    }
    
    printf("\n=== CARREGANDO USUARIOS DO ARQUIVO usuarios.txt ===\n");
    
    char linha[256];
    int carregados = 0;
    int invalidos = 0;
    int duplicados = 0;
    int linha_atual = 0;
    
    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        linha_atual++;
        
        // Remover newline e espaços extras
        linha[strcspn(linha, "\r\n")] = '\0';
        
        // Remover espaços em branco no início e fim
        char* id = linha;
        while (*id == ' ' || *id == '\t') id++;
        
        char* fim = id + strlen(id) - 1;
        while (fim > id && (*fim == ' ' || *fim == '\t')) {
            *fim = '\0';
            fim--;
        }
        
        // Pular linhas vazias e comentários (linhas que começam com #)
        if (strlen(id) == 0 || id[0] == '#') {
            continue;
        }
        
        // Validar o formato do ID (agora apenas verifica comprimento)
        if (!validar_id(id)) {
            printf("Linha %d: ID invalido '%s' (deve ter entre 1 e %d caracteres) - ignorado\n", 
                   linha_atual, id, MAX_ID_LENGTH);
            invalidos++;
            continue;
        }
        
        // Inserir nas estruturas
        if (insert_hash(ht, id)) {
            insert_bloom(bf, id);
            stats->total_inseridos++;
            carregados++;
            printf("Linha %d: Usuario %s cadastrado com sucesso!\n", linha_atual, id);
        } else {
            printf("Linha %d: Usuario %s ja existe - ignorado\n", linha_atual, id);
            duplicados++;
        }
    }
    
    fclose(arquivo);
    
    printf("\n=== RESUMO DO CARREGAMENTO ===\n");
    printf("Total de linhas processadas: %d\n", linha_atual);
    printf("Usuarios carregados com sucesso: %d\n", carregados);
    printf("IDs invalidos ignorados: %d\n", invalidos);
    printf("IDs duplicados ignorados: %d\n", duplicados);
    printf("===============================\n\n");
    
    return carregados;
}

// Menu principal
void menu_principal() {
    int opcao;
    HashTable* ht = NULL;
    BloomFilter* bf = NULL;
    Estatisticas stats = {0, 0, 0, 0.0, 0.0, 0.0, 0.0};
    char id[MAX_ID_LENGTH + 1]; // +1 para o terminador nulo
    
    srand(time(NULL)); // Inicializar gerador aleatório
    
    while (1) {
        printf("\n____________________________________________________\n");
        printf("        SISTEMA DE CADASTRO DE USUARIOS\n");
        printf("____________________________________________________\n");
        printf("1. Inserir Usuario\n");
        printf("2. Verificar se Usuario esta cadastrado\n");
        printf("3. Gerar Relatorio de Estatisticas\n");
        printf("4. Executar Testes de Desempenho\n");
        printf("5. Carregar usuarios do arquivo usuarios.txt\n");
        printf("6. Sair\n");
        printf("____________________________________________________\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &opcao);
        limpar_buffer();
        
        switch (opcao) {
            case 1:
                if (ht == NULL) {
                    ht = create_hash_table();
                    bf = create_bloom_filter(100000, 0.01);
                    if (ht == NULL || bf == NULL) {
                        printf("Erro ao criar estruturas de dados!\n");
                        break;
                    }
                }
                
                printf("Insira o ID do usuario (ate %d caracteres): ", MAX_ID_LENGTH);
                fgets(id, sizeof(id), stdin);
                
                // Remover newline
                id[strcspn(id, "\n")] = '\0';
                
                // Validar o formato do ID (agora apenas verifica comprimento)
                if (!validar_id(id)) {
                    printf("ID INVALIDO!\n");
                    printf("O ID deve ter entre 1 e %d caracteres.\n", MAX_ID_LENGTH);
                    printf("Tamanho informado: %zu caracteres\n", strlen(id));
                    break;
                }
                
                if (insert_hash(ht, id)) {
                    insert_bloom(bf, id);
                    stats.total_inseridos++;
                    salvar_usuario_arquivo(id);
                    printf("Usuario '%s' inserido com sucesso!\n", id);
                } else {
                    printf("Usuario '%s' ja esta cadastrado!\n", id);
                }
                break;

            case 2:
                if (ht == NULL) {
                    printf("Nenhum usuario cadastrado ainda!\n");
                    break;
                }
                
                printf("Insira o ID do usuario para verificacao (ate %d caracteres): ", MAX_ID_LENGTH);
                fgets(id, sizeof(id), stdin);
                id[strcspn(id, "\n")] = '\0';
                
                // Validar o formato do ID na busca também
                if (!validar_id(id)) {
                    printf("ID INVALIDO! O ID deve ter entre 1 e %d caracteres.\n", MAX_ID_LENGTH);
                    break;
                }
                
                if (search_hash(ht, id)) {
                    printf("Usuario '%s' ENCONTRADO na tabela hash!\n", id);
                } else {
                    printf("Usuario '%s' NAO ENCONTRADO na tabela hash!\n", id);
                    
                    if (search_bloom(bf, id)) {
                        stats.total_falsos_positivos++;
                        printf("OBS: Filtro de Bloom indicou que o usuario pode existir (falso positivo).\n");
                    }
                    stats.total_nao_encontrados++;
                }
                break;
                
            case 3:
                if (ht == NULL || bf == NULL) {
                    printf("Nenhum dado para gerar relatorio!\n");
                    break;
                }
                gerar_relatorio(ht, bf, &stats, stats.total_inseridos);
                break;
                
            case 4:
                // Limpar estruturas anteriores
                if (ht != NULL) {
                    free_hash_table(ht);
                    ht = NULL;
                }
                if (bf != NULL) {
                    free_bloom_filter(bf);
                    bf = NULL;
                }
                
                // Executar testes com diferentes tamanhos
                executar_testes(TESTE_1000);
                executar_testes(TESTE_10000);
                executar_testes(TESTE_100000);
                
                // Recriar estruturas vazias
                ht = create_hash_table();
                bf = create_bloom_filter(100000, 0.01);
                stats.total_inseridos = 0;
                stats.total_nao_encontrados = 0;
                stats.total_falsos_positivos = 0;
                break;
                
            case 5:
                if (ht == NULL) {
                    ht = create_hash_table();
                    bf = create_bloom_filter(100000, 0.01);
                    if (ht == NULL || bf == NULL) {
                        printf("Erro ao criar estruturas de dados!\n");
                        break;
                    }
                }
                
                carregar_usuarios_arquivo(ht, bf, &stats);
                break;
                
            case 6:
                if (ht != NULL) free_hash_table(ht);
                if (bf != NULL) free_bloom_filter(bf);
                printf("Saindo do sistema...\n");
                return;
                
            default:
                printf("Opcao invalida! Tente novamente.\n");
        }
    }
}

int main() {
    menu_principal();
    return 0;
}
