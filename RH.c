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

/* --- Callback: Comparação com Desempate --- */
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

/* --- Callbacks: Tamanho em Bytes --- */
size_t tamanho_chave_rh(const void *k) {
    (void)k; // Evita warning de unused parameter
    return sizeof(ChaveRH);
}

size_t tamanho_valor_rh(const void *v) {
    (void)v;
    return sizeof(Funcionario);
}

/* --- Callbacks: Escrita e Leitura de Chave em Disco --- */
void escreve_chave_rh(const void *k, FILE *f) {
    fwrite(k, sizeof(ChaveRH), 1, f);
}

void* le_chave_rh(FILE *f) {
    ChaveRH *chave = malloc(sizeof(ChaveRH));
    fread(chave, sizeof(ChaveRH), 1, f);
    return chave;
}

/* --- Callbacks: Escrita e Leitura de Valor em Disco --- */
void escreve_valor_rh(const void *v, FILE *f) {
    fwrite(v, sizeof(Funcionario), 1, f);
}

void* le_valor_rh(FILE *f) {
    Funcionario *func = malloc(sizeof(Funcionario));
    fread(func, sizeof(Funcionario), 1, f);
    return func;
}