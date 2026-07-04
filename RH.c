/* ==============================================
|     Trabalho de Estrutura de Dados II      |
----------------------------------------------
| Arquivo: RH.c                              |
----------------------------------------------
|                 Alunos                     |
----------------------------------------------
| Gabriel Vargas de Melo                     |
| Isaac Gabriel Covre Silva                  |
| Albert Rocha Tavares                       |
| Victor Rodrigues Silva                     |
============================================== */

#include "RH.h"
#include <string.h>
#include <stdlib.h>

/**
 * @brief Implementação da callback de comparação para ChaveRH.
 * @details Compara primeiro pelo nome (ordem alfabética com strcmp).
 * Em caso de empate, desempata por ano, depois mês, depois dia.
 * @param k1 Ponteiro para a primeira ChaveRH.
 * @param k2 Ponteiro para a segunda ChaveRH.
 * @return int Negativo se k1 < k2, zero se iguais, positivo se k1 > k2.
 */
int compara_chaves_rh(const void *k1, const void *k2) {
    ChaveRH *chave1 = (ChaveRH*) k1;
    ChaveRH *chave2 = (ChaveRH*) k2;
    
    // 1. Critério Principal: Nome
    int comparacao_nome = strcmp(chave1->nome, chave2->nome);
    if (comparacao_nome != 0) {
        return comparacao_nome;
    }
    
    // 2. Critério de Desempate: Data de Nascimento (Ano, Mês, Dia)
    if (chave1->data_nascimento.ano != chave2->data_nascimento.ano)
        return chave1->data_nascimento.ano - chave2->data_nascimento.ano;
        
    if (chave1->data_nascimento.mes != chave2->data_nascimento.mes)
        return chave1->data_nascimento.mes - chave2->data_nascimento.mes;
        
    return chave1->data_nascimento.dia - chave2->data_nascimento.dia;
}

/**
 * @brief Retorna o tamanho fixo de ChaveRH em bytes.
 * @param k Ponteiro para a chave (não utilizado — tamanho fixo).
 * @return size_t sizeof(ChaveRH).
 */
size_t tamanho_chave_rh(const void *k) {
    (void)k;
    return sizeof(ChaveRH);
}

/**
 * @brief Retorna o tamanho fixo de Funcionario em bytes.
 * @param v Ponteiro para o valor (não utilizado — tamanho fixo).
 * @return size_t sizeof(Funcionario).
 */
size_t tamanho_valor_rh(const void *v) {
    (void)v;
    return sizeof(Funcionario);
}

/**
 * @brief Serializa uma ChaveRH em arquivo binário via fwrite.
 * @param k Ponteiro para a ChaveRH a ser escrita.
 * @param f Ponteiro para o arquivo FILE* de saída.
 */
void escreve_chave_rh(const void *k, FILE *f) {
    fwrite(k, sizeof(ChaveRH), 1, f);
}

/**
 * @brief Desserializa uma ChaveRH de arquivo binário via fread.
 * @param f Ponteiro para o arquivo FILE* de entrada.
 * @return void* Ponteiro alocado (malloc) para a ChaveRH lida.
 */
void* le_chave_rh(FILE *f) {
    ChaveRH *chave = malloc(sizeof(ChaveRH));
    fread(chave, sizeof(ChaveRH), 1, f);
    return chave;
}

/**
 * @brief Serializa um Funcionario em arquivo binário via fwrite.
 * @param v Ponteiro para o Funcionario a ser escrito.
 * @param f Ponteiro para o arquivo FILE* de saída.
 */
void escreve_valor_rh(const void *v, FILE *f) {
    fwrite(v, sizeof(Funcionario), 1, f);
}

/**
 * @brief Desserializa um Funcionario de arquivo binário via fread.
 * @param f Ponteiro para o arquivo FILE* de entrada.
 * @return void* Ponteiro alocado (malloc) para o Funcionario lido.
 */
void* le_valor_rh(FILE *f) {
    Funcionario *func = malloc(sizeof(Funcionario));
    fread(func, sizeof(Funcionario), 1, f);
    return func;
}