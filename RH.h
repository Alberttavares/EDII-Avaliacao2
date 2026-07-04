/* ==============================================
|     Trabalho de Estrutura de Dados II      |
----------------------------------------------
| Arquivo: RH.h                              |
----------------------------------------------
|                 Alunos                     |
----------------------------------------------
| Gabriel Vargas de Melo                     |
| Isaac Gabriel Covre Silva                  |
| Albert Rocha Tavares                       |
| Victor Rodrigues Silva                     |
============================================== 
*/

#ifndef RH_H
#define RH_H

#include <stdio.h>


/**
 * @brief Representa uma data (dia, mês, ano).
 */
typedef struct {
    int dia, mes, ano;
} Data;

/**
 * @brief Chave composta usada na árvore B+: nome + data de nascimento.
 * @details A ordem de ordenação é: nome (ordem alfabética) e, em caso
 * de homônimos, data de nascimento (ano > mês > dia).
 */
typedef struct {
    char nome[100];
    Data data_nascimento;
} ChaveRH;

/**
 * @brief Representa um pagamento mensal de um funcionário.
 */
typedef struct {
    char mes_ref[10]; /**< Mês de referência no formato "MM/AAAA" (ex: "05/2026"). */
    float valor;
} Pagamento;

/**
 * @brief Estrutura completa de um funcionário (registro armazenado como valor na árvore B+).
 * @details O campo chave é uma ChaveRH que também é armazenada separadamente
 * como chave da árvore para permitir navegação pelos índices.
 */
typedef struct {
    ChaveRH chave;
    char nome_mae[100];
    char nome_pai[100];
    char endereco[200];
    char telefone[20];
    char data_contratacao[11];
    char status[10];           /**< "Ativo" ou "Inativo". */
    char data_desligamento[11];
    Pagamento historico_pagamentos[12]; /**< Últimos 12 meses de pagamento. */
} Funcionario;


/**
 * @brief Compara duas chaves ChaveRH.
 * @details Critério principal: nome (strcmp). Critério de desempate:
 * ano, depois mês, depois dia da data de nascimento.
 * @param k1 Ponteiro para a primeira ChaveRH.
 * @param k2 Ponteiro para a segunda ChaveRH.
 * @return int Negativo se k1 < k2, zero se iguais, positivo se k1 > k2.
 */
int compara_chaves_rh(const void *k1, const void *k2);

/**
 * @brief Retorna o tamanho fixo da estrutura ChaveRH em bytes.
 * @param k Ponteiro para a chave (ignorado, tamanho fixo).
 * @return size_t sizeof(ChaveRH).
 */
size_t tamanho_chave_rh(const void *k);

/**
 * @brief Retorna o tamanho fixo da estrutura Funcionario em bytes.
 * @param v Ponteiro para o valor (ignorado, tamanho fixo).
 * @return size_t sizeof(Funcionario).
 */
size_t tamanho_valor_rh(const void *v);

/**
 * @brief Serializa uma ChaveRH escrevendo-a em um arquivo binário.
 * @param k Ponteiro para a ChaveRH a ser escrita.
 * @param f Ponteiro para o arquivo FILE* de saída.
 */
void escreve_chave_rh(const void *k, FILE *f);

/**
 * @brief Desserializa uma ChaveRH lendo-a de um arquivo binário.
 * @param f Ponteiro para o arquivo FILE* de entrada.
 * @return void* Ponteiro alocado (malloc) para a ChaveRH lida.
 */
void* le_chave_rh(FILE *f);

/**
 * @brief Serializa um Funcionario escrevendo-o em um arquivo binário.
 * @param v Ponteiro para o Funcionario a ser escrito.
 * @param f Ponteiro para o arquivo FILE* de saída.
 */
void escreve_valor_rh(const void *v, FILE *f);

/**
 * @brief Desserializa um Funcionario lendo-o de um arquivo binário.
 * @param f Ponteiro para o arquivo FILE* de entrada.
 * @return void* Ponteiro alocado (malloc) para o Funcionario lido.
 */
void* le_valor_rh(FILE *f);

#endif /* RH_H */