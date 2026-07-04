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

/**
 * @brief Função de comparação entre duas chaves genéricas.
 * @param chave1 Ponteiro para a primeira chave.
 * @param chave2 Ponteiro para a segunda chave.
 * @return Negativo se chave1 < chave2, zero se iguais, positivo se chave1 > chave2.
 */
typedef int (*FuncaoComparar)(const void *chave1, const void *chave2);

/**
 * @brief Função que retorna o tamanho em bytes de um dado genérico.
 * @param dado Ponteiro para o dado (pode ser NULL para tipos de tamanho fixo).
 * @return Tamanho em bytes do tipo de dado.
 */
typedef size_t (*FuncaoTamanho)(const void *dado);

/**
 * @brief Função para serializar (escrever) um dado genérico em um arquivo.
 * @param dado Ponteiro para o dado a ser escrito.
 * @param fluxo Ponteiro para o arquivo FILE* de saída.
 */
typedef void (*FuncaoEscrever)(const void *dado, FILE *fluxo);

/**
 * @brief Função para desserializar (ler) um dado genérico de um arquivo.
 * @param fluxo Ponteiro para o arquivo FILE* de entrada.
 * @return Ponteiro alocado com malloc contendo o dado lido.
 */
typedef void *(*FuncaoLer)(FILE *fluxo);

/* --- Estrutura Opaca da Árvore B+ --- */
typedef struct BPlusTree BPlusTree;

/**
 * @brief Cria ou abre uma Árvore B+ persistente em disco.
 * @param nome_arquivo Nome do arquivo binário para armazenar a árvore.
 * @param ordem Ordem máxima da árvore B+ (número máximo de ponteiros por nó).
 * @param comparar Função callback para comparação de chaves.
 * @param tamanho_chave Função callback que retorna o tamanho da chave em bytes.
 * @param tamanho_valor Função callback que retorna o tamanho do valor em bytes.
 * @param escrever_chave Função callback para serializar uma chave no disco.
 * @param ler_chave Função callback para desserializar uma chave do disco.
 * @param escrever_valor Função callback para serializar um valor no disco.
 * @param ler_valor Função callback para desserializar um valor do disco.
 * @return BPlusTree* Ponteiro para o controlador alocado, ou NULL em caso de erro.
 * @example
 * BPlusTree *arv = criar_bmais("dados.bin", 5, cmp, tam_c, tam_v, esc_c, ler_c, esc_v, ler_v);
 */
BPlusTree* criar_bmais(const char *nome_arquivo, int ordem,
                              FuncaoComparar comparar,
                              FuncaoTamanho tamanho_chave, FuncaoTamanho tamanho_valor,
                              FuncaoEscrever escrever_chave, FuncaoLer ler_chave,
                              FuncaoEscrever escrever_valor, FuncaoLer ler_valor);

/**
 * @brief Insere um par chave-valor na árvore B+ em disco.
 * @param arvore Controlador da árvore retornado por criar_bmais().
 * @param chave Ponteiro para a chave a ser inserida.
 * @param valor Ponteiro para o valor a ser inserido.
 * @return 1 em caso de sucesso, 0 se a chave já existir (duplicata) ou em caso de erro.
 */
int inserir_bmais(BPlusTree *arvore, void *chave, void *valor);

/**
 * @brief Busca um registro pela chave (busca por igualdade).
 * @param arvore Controlador da árvore.
 * @param chave Ponteiro para a chave a ser buscada.
 * @return void* Ponteiro alocado com malloc para o valor encontrado, ou NULL se não existir.
 * @note O ponteiro retornado deve ser liberado pelo usuário com free().
 */
void* buscar_bmais(BPlusTree *arvore, void *chave);

/**
 * @brief Remove um registro da árvore a partir da chave.
 * @details Estratégia de reconstrução: percorre todas as folhas, coleta os registros
 * exceto o removido, trunca o arquivo e reinsere todos os registros restantes.
 * @param arvore Controlador da árvore.
 * @param chave Ponteiro para a chave a ser removida.
 * @return 1 em caso de sucesso, 0 se a chave não for encontrada.
 */
int remover_bmais(BPlusTree *arvore, void *chave);

/**
 * @brief Lista todos os registros contidos em um intervalo aberto (chave_a, chave_b).
 * @details Percorre a lista encadeada de folhas a partir da primeira folha relevante,
 * invocando imprimir_func para cada valor cuja chave satisfaça chave_a < chave < chave_b.
 * @param arvore Controlador da árvore.
 * @param chave_a Limite inferior do intervalo (exclusivo).
 * @param chave_b Limite superior do intervalo (exclusivo).
 * @param imprimir_func Função callback invocada para cada valor encontrado no intervalo.
 */
void buscar_intervalo_bmais(BPlusTree *arvore, void *chave_a, void *chave_b, void (*imprimir_func)(void *valor));

/**
 * @brief Imprime a estrutura hierárquica da árvore B+ para depuração.
 * @param arvore Controlador da árvore.
 * @param imprimir_chave Função callback para formatar e exibir cada chave.
 */
void imprimir_estrutura_bmais(BPlusTree *arvore, void (*imprimir_chave)(void *chave));

/**
 * @brief Fecha o arquivo em disco e libera toda a memória alocada para o controlador.
 * @param arvore Controlador da árvore a ser destruído.
 */
void destruir_bmais(BPlusTree *arvore);

#endif /* BPLUS_H */
