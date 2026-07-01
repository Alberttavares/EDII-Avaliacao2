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

/* --- Tipos e Estruturas de Dados --- */
typedef struct {
    int dia, mes, ano;
} Data;

typedef struct {
    char nome[100];
    Data data_nascimento;
} ChaveRH;

typedef struct {
    char mes_ref[10]; // Ex: "05/2026"
    float valor;
} Pagamento;

typedef struct {
    ChaveRH chave; 
    char nome_mae[100];
    char nome_pai[100];
    char endereco[200];
    char telefone[20];
    char data_contratacao[11];
    char status[10];           // "Ativo" ou "Inativo"
    char data_desligamento[11];
    Pagamento historico_pagamentos[12];
} Funcionario;

/* --- Declaração das Callbacks para a Árvore B+ --- */
int compara_chaves_rh(const void *k1, const void *k2);
size_t tamanho_chave_rh(const void *k);
size_t tamanho_valor_rh(const void *v);
void escreve_chave_rh(const void *k, FILE *f);
void* le_chave_rh(FILE *f);
void escreve_valor_rh(const void *v, FILE *f);
void* le_valor_rh(FILE *f);

#endif /* RH_H */