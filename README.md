# Sistema de Verificação de Cadastro de Usuários
## Descrição do Projeto

Sistema desenvolvido em C para armazenamento e consulta eficiente de cadastros de usuários, utilizando duas estruturas complementares:

- **Tabela Hash**: armazenamento e recuperação exata dos elementos
- **Filtro de Bloom**: estrutura probabilística para acelerar consultas de existência

O sistema permite verificar rapidamente se um usuário está cadastrado, reduzindo acessos desnecessários à tabela hash principal através do filtro de bloom.

## Requisitos de Compilação

### Compilação Padrão

```bash
gcc main.c -o executavel
```
### Compilação com biblioteca matemática

Dependendo da versão do GCC, pode ser necessário incluir a biblioteca `math.h`:

```bash
gcc main.c -o executavel -lm
```

## Como Executar

```bash
./executavel
```

**Observação:** Cada vez que o programa é executado, o sistema inicia do zero, sem dados persistidos, exceto em Executar Testes de Desempenho, os quais são valores fixos.

## Estrutura do Menu

Ao executar o programa, o seguinte menu será exibido:

```
____________________________________________________
        SISTEMA DE CADASTRO DE USUÁRIOS
____________________________________________________
1. Inserir Usuário
2. Verificar se Usuário está cadastrado
3. Gerar Relatório de Estatísticas
4. Executar Testes de Desempenho
5. Sair
____________________________________________________
Escolha uma opção: 
```

Para navegar, digite o número inteiro correspondente à opção desejada.

## Funcionalidades

### 1. Inserir Usuário
Cadastra um novo usuário utilizando um identificador único.
- **Exemplo:** `INSERIR joao123`
- O usuário é inserido tanto na Tabela Hash quanto no Filtro de Bloom
- Quando cadastrado com sucesso, o registro do usuário é salvo em /data/usuario.txt

### 2. Verificar Usuário
Consulta se um usuário existe no sistema seguindo o fluxo:

1. **Consultar o Filtro de Bloom**
2. Se o filtro indicar "definitivamente não existe" → retorna imediatamente
3. Se indicar "possivelmente existe" → consulta a Tabela Hash
4. Informa o resultado final

- **Exemplo:** 
  - `CONSULTAR matheusw123` → Usuário encontrado
  - `CONSULTAR anapijam777` → Usuário inexistente

### 3. Relatório de Estatísticas
Exibe métricas detalhadas do sistema:
- Quantidade de elementos armazenados
- Quantidade de consultas realizadas
- Consultas evitadas pelo Filtro de Bloom
- Número de falsos positivos
- Taxa percentual de falsos positivos
- Tempo médio de consulta

### 4. Testes de Desempenho
Realiza testes automatizados com diferentes volumes de dados:

- **1.000 registros**
- **10.000 registros**
- **100.000 registros**

Para cada cenário, são medidos:
- Tempo sem Bloom (busca direta na Tabela Hash)
- Tempo com Bloom (busca utilizando Filtro de Bloom)
- Taxa de falsos positivos
