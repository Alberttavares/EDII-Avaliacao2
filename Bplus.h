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

/* --- Callbacks para manipulação de tipos genéricos em disco --- */
typedef int (*CompareFunc)(const void *key1, const void *key2);
typedef size_t (*SizeFunc)(const void *data);
typedef void (*WriteFunc)(const void *data, FILE *stream);
typedef void *(*ReadFunc)(FILE *stream);

/* --- Estrutura Opaca da Árvore B+ --- */
typedef struct BPlusTree BPlusTree;

/**
 * @brief Cria ou abre uma Árvore B+ em disco.
 * * @param filename Nome do arquivo binário.
 * @param order Ordem da árvore B+.
 * @param cmp Função para comparar as chaves.
 * @param key_size Função que retorna o tamanho da chave em bytes.
 * @param val_size Função que retorna o tamanho do registro em bytes.
 * @param write_k Função para serializar a chave no disco.
 * @param read_k Função para desserializar a chave do disco.
 * @param write_v Função para serializar o registro no disco.
 * @param read_v Função para desserializar o registro do disco.
 * @return BPlusTree* Ponteiro para o controlador da árvore.
 */
BPlusTree* bplus_create(const char *filename, int order,
                        CompareFunc cmp, 
                        SizeFunc key_size, SizeFunc val_size,
                        WriteFunc write_k, ReadFunc read_k,
                        WriteFunc write_v, ReadFunc read_v);

/**
 * @brief Insere um par chave-valor na árvore em disco.
 * * @return 1 se sucesso, 0 se duplicado ou erro.
 */
int bplus_insert(BPlusTree *tree, void *key, void *value);

/**
 * @brief Busca um registro pela chave.
 * * @return Ponteiro alocado para o valor (deve ser liberado pelo usuário), ou NULL se não achar.
 */
void* bplus_search(BPlusTree *tree, void *key);

/**
 * @brief Remove um registro a partir da chave.
 * * @return 1 se sucesso, 0 se não encontrado.
 */
int bplus_remove(BPlusTree *tree, void *key);

/**
 * @brief Lista registros contidos em um intervalo aberto (keyA, keyB).
 */
void bplus_range_search(BPlusTree *tree, void *keyA, void *keyB, void (*print_func)(void *val));

/**
 * @brief Imprime a estrutura hierárquica do índice para depuração.
 */
void bplus_print_structure(BPlusTree *tree, void (*print_key)(void *key));

/**
 * @brief Fecha os arquivos e libera o controlador da memória RAM.
 */
void bplus_destroy(BPlusTree *tree);

#endif /* BPLUS_H */