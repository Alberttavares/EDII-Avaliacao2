/* ==============================================
|     Trabalho de Estrutura de Dados II      |
----------------------------------------------
| Arquivo: Bplus.h                           |
----------------------------------------------
|                 Alunos                     |
----------------------------------------------
| Gabriel Vargas de Melo                     |
| Isaac Gabriel Covre Silva                  |
| Albert Rocha Tavares                       |
| Victor Rodrigues Silva                     |
============================================== 
*/

#ifndef BPLUS_H
#define BPLUS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* --- Funções de retorno para manipulação de tipos genéricos em disco --- */
typedef int (*FuncaoComparar)(const void *chave1, const void *chave2);
typedef size_t (*FuncaoTamanho)(const void *dado);
typedef void (*FuncaoEscrever)(const void *dado, FILE *fluxo);
typedef void *(*FuncaoLer)(FILE *fluxo);

/* --- Estrutura Opaca da Árvore B+ --- */
typedef struct BPlusTree BPlusTree;

/**
 * @brief Cria ou abre uma Árvore B+ em disco.
 * * @param nome_arquivo Nome do arquivo binário.
 * @param ordem Ordem da árvore B+.
 * @param comparar Função para comparar as chaves.
 * @param tamanho_chave Função que retorna o tamanho da chave em bytes.
 * @param tamanho_valor Função que retorna o tamanho do registro em bytes.
 * @param escrever_chave Função para serializar a chave no disco.
 * @param ler_chave Função para desserializar a chave do disco.
 * @param escrever_valor Função para serializar o registro no disco.
 * @param ler_valor Função para desserializar o registro do disco.
 * @return BPlusTree* Ponteiro para o controlador da árvore.
 */
BPlusTree* arvore_bmais_criar(const char *nome_arquivo, int ordem,
                              FuncaoComparar comparar,
                              FuncaoTamanho tamanho_chave, FuncaoTamanho tamanho_valor,
                              FuncaoEscrever escrever_chave, FuncaoLer ler_chave,
                              FuncaoEscrever escrever_valor, FuncaoLer ler_valor);

/**
 * @brief Insere um par chave-valor na árvore em disco.
 * * @return 1 se sucesso, 0 se duplicado ou erro.
 */
int arvore_bmais_inserir(BPlusTree *arvore, void *chave, void *valor);

/**
 * @brief Busca um registro pela chave.
 * * @return Ponteiro alocado para o valor (deve ser liberado pelo usuário), ou NULL se não achar.
 */
void* arvore_bmais_buscar(BPlusTree *arvore, void *chave);

/**
 * @brief Remove um registro a partir da chave.
 * * @return 1 se sucesso, 0 se não encontrado.
 */
int arvore_bmais_remover(BPlusTree *arvore, void *chave);

/**
 * @brief Lista registros contidos em um intervalo aberto (chave_a, chave_b).
 */
void arvore_bmais_buscar_intervalo(BPlusTree *arvore, void *chave_a, void *chave_b, void (*imprimir_func)(void *valor));

/**
 * @brief Imprime a estrutura hierárquica do índice para depuração.
 */
void arvore_bmais_imprimir_estrutura(BPlusTree *arvore, void (*imprimir_chave)(void *chave));

/**
 * @brief Fecha os arquivos e libera o controlador da memória RAM.
 */
void arvore_bmais_destruir(BPlusTree *arvore);

#endif /* BPLUS_H */
