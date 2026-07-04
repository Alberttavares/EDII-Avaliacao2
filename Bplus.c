/* ==============================================
|     Trabalho de Estrutura de Dados II      |
----------------------------------------------
| Arquivo: Bplus.c                           |
----------------------------------------------
|                 Alunos                     |
----------------------------------------------
| Gabriel Vargas de Melo                     |
| Isaac Gabriel Covre Silva                  |
| Albert Rocha Tavares                       |
| Victor Rodrigues Silva                     |
============================================== */

#include "Bplus.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

/**
 * @brief Cabeçalho de controle armazenado no início de cada nó no disco.
 * @details Contém metadados do nó:
 * - eh_folha: true indica nó folha, false indica nó interno.
 * - num_chaves: quantidade de chaves atualmente armazenadas no nó.
 * - offset_pai: offset absoluto em disco do nó pai (-1 se for raiz).
 * - proxima_folha: offset absoluto em disco da próxima folha na lista encadeada (-1 se não houver).
 */
typedef struct {
    bool eh_folha;
    int num_chaves;
    long offset_pai;
    long proxima_folha;
} CabecalhoNo;

/**
 * @brief Estrutura de controle principal da árvore B+.
 * @details Armazena o estado da árvore e as funções callback:
 * - arquivo: ponteiro FILE* para o arquivo binário em disco.
 * - ordem: ordem M da árvore B+ (número máximo de ponteiros por nó).
 * - offset_raiz: offset absoluto do nó raiz no arquivo (-1 se árvore vazia).
 * - comparar: callback para comparação de duas chaves.
 * - tamanho_chave: callback que retorna o tamanho em bytes de uma chave.
 * - tamanho_valor: callback que retorna o tamanho em bytes de um valor.
 * - escrever_chave: callback para serializar uma chave no disco.
 * - ler_chave: callback para desserializar uma chave do disco.
 * - escrever_valor: callback para serializar um valor no disco.
 * - ler_valor: callback para desserializar um valor do disco.
 */
struct BPlusTree {
    FILE *arquivo;
    int ordem;
    long offset_raiz;

    FuncaoComparar comparar;
    FuncaoTamanho tamanho_chave;
    FuncaoTamanho tamanho_valor;
    FuncaoEscrever escrever_chave;
    FuncaoLer ler_chave;
    FuncaoEscrever escrever_valor;
    FuncaoLer ler_valor;
};


/**
 * @brief Calcula o offset absoluto em disco de uma chave dentro de um nó.
 * @param arvore Controlador da árvore.
 * @param deslocamento_no Offset absoluto do nó no arquivo.
 * @param indice Índice da chave (0-based, válido de 0 a ordem-2).
 * @return long Offset absoluto da chave no arquivo.
 */
static long obter_deslocamento_chave(BPlusTree *arvore, long deslocamento_no, int indice) {
    return deslocamento_no + sizeof(CabecalhoNo) + (indice * arvore->tamanho_chave(NULL));
}

/**
 * @brief Calcula o offset absoluto em disco de um valor dentro de um nó.
 * @param arvore Controlador da árvore.
 * @param deslocamento_no Offset absoluto do nó no arquivo.
 * @param indice Índice do valor (0-based, válido de 0 a ordem-2).
 * @return long Offset absoluto do valor no arquivo.
 */
static long obter_deslocamento_valor(BPlusTree *arvore, long deslocamento_no, int indice) {
    long inicio_valores = deslocamento_no + sizeof(CabecalhoNo) + ((arvore->ordem - 1) * arvore->tamanho_chave(NULL));
    return inicio_valores + (indice * arvore->tamanho_valor(NULL));
}

/**
 * @brief Calcula o offset absoluto em disco de um ponteiro filho dentro de um nó.
 * @param arvore Controlador da árvore.
 * @param deslocamento_no Offset absoluto do nó no arquivo.
 * @param indice Índice do ponteiro (0-based, válido de 0 a ordem-1).
 * @return long Offset absoluto do ponteiro no arquivo.
 */
static long obter_deslocamento_ponteiro(BPlusTree *arvore, long deslocamento_no, int indice) {
    long inicio_ponteiros = deslocamento_no + sizeof(CabecalhoNo) + 
                            ((arvore->ordem - 1) * arvore->tamanho_chave(NULL)) + 
                            ((arvore->ordem - 1) * arvore->tamanho_valor(NULL));
    return inicio_ponteiros + (indice * sizeof(long));
}


/**
 * @brief Aloca um novo nó no final do arquivo em disco.
 * @details A estratégia é append-only: posiciona-se no final do arquivo
 * (fseek SEEK_END + ftell), calcula o tamanho total do nó, escreve um
 * bloco de zeros e retorna o offset obtido. Não há reaproveitamento de
 * espaço livre entre inserções.
 * @param arvore Controlador da árvore.
 * @return long Offset absoluto do novo nó no arquivo.
 */
static long alocar_novo_no(BPlusTree *arvore) {
    fseek(arvore->arquivo, 0, SEEK_END);
    long deslocamento = ftell(arvore->arquivo);
    
    // Calcula o tamanho máximo exato do nó e preenche com zeros (padding)
    size_t tam_chaves = (arvore->ordem - 1) * arvore->tamanho_chave(NULL);
    size_t tam_valores = (arvore->ordem - 1) * arvore->tamanho_valor(NULL);
    size_t tam_ponteiros = arvore->ordem * sizeof(long);
    size_t tam_total = sizeof(CabecalhoNo) + tam_chaves + tam_valores + tam_ponteiros;
    
    void *zeros = calloc(1, tam_total);
    fwrite(zeros, tam_total, 1, arvore->arquivo);
    free(zeros);
    fflush(arvore->arquivo);
    
    return deslocamento;
}


/**
 * @brief Lê o cabeçalho de um nó do disco.
 * @param arvore Controlador da árvore.
 * @param deslocamento Offset absoluto do nó no arquivo.
 * @return CabecalhoNo Estrutura com os metadados do nó lidos do disco.
 *         Se a leitura falhar, retorna um cabeçalho seguro (folha vazia).
 */
static CabecalhoNo ler_cabecalho(BPlusTree *arvore, long deslocamento) {
    CabecalhoNo cabecalho;
    fseek(arvore->arquivo, deslocamento, SEEK_SET);
    if (fread(&cabecalho, sizeof(CabecalhoNo), 1, arvore->arquivo) != 1) {
        // Trava de segurança para evitar loop infinito se ler lixo
        cabecalho.eh_folha = true; 
        cabecalho.num_chaves = 0;
    }
    return cabecalho;
}

/**
 * @brief Escreve o cabeçalho de um nó no disco.
 * @param arvore Controlador da árvore.
 * @param deslocamento Offset absoluto do nó no arquivo.
 * @param cabecalho Ponteiro para o cabeçalho a ser escrito.
 */
static void escrever_cabecalho(BPlusTree *arvore, long deslocamento, CabecalhoNo *cabecalho) {
    fseek(arvore->arquivo, deslocamento, SEEK_SET);
    fwrite(cabecalho, sizeof(CabecalhoNo), 1, arvore->arquivo);
    fflush(arvore->arquivo);
}

/**
 * @brief Cria um novo nó folha vazio no disco.
 * @param arvore Controlador da árvore.
 * @return long Offset absoluto do novo nó folha no arquivo.
 */
static long criar_no_folha(BPlusTree *arvore) {
    long novo_deslocamento = alocar_novo_no(arvore);
    CabecalhoNo cabecalho = { .eh_folha = true, .num_chaves = 0, .offset_pai = -1, .proxima_folha = -1 };
    escrever_cabecalho(arvore, novo_deslocamento, &cabecalho);
    return novo_deslocamento;
}


/**
 * @brief Cria ou abre uma árvore B+ persistente em disco.
 * @details Se o arquivo já existir, abre em modo r+b e recupera o offset da raiz
 * armazenado nos primeiros 8 bytes. Se não existir, cria o arquivo em w+b,
 * escreve offset_raiz = -1 e inicializa a estrutura.
 * @param nome_arquivo Nome do arquivo binário.
 * @param ordem Ordem da árvore B+.
 * @param comparar Função callback de comparação de chaves.
 * @param tamanho_chave Função callback de tamanho da chave.
 * @param tamanho_valor Função callback de tamanho do valor.
 * @param escrever_chave Função callback de serialização de chave.
 * @param ler_chave Função callback de desserialização de chave.
 * @param escrever_valor Função callback de serialização de valor.
 * @param ler_valor Função callback de desserialização de valor.
 * @return BPlusTree* Ponteiro alocado para o controlador, ou NULL se falhar.
 */
BPlusTree* criar_bmais(const char *nome_arquivo, int ordem, FuncaoComparar comparar,
                              FuncaoTamanho tamanho_chave, FuncaoTamanho tamanho_valor,
                              FuncaoEscrever escrever_chave, FuncaoLer ler_chave,
                              FuncaoEscrever escrever_valor, FuncaoLer ler_valor) {
    
    BPlusTree *arvore = (BPlusTree*) malloc(sizeof(BPlusTree));
    if (!arvore) return NULL;

    arvore->arquivo = fopen(nome_arquivo, "r+b");
    if (!arvore->arquivo) {
        arvore->arquivo = fopen(nome_arquivo, "w+b");
        if (!arvore->arquivo) { free(arvore); return NULL; }
        arvore->offset_raiz = -1;
        fseek(arvore->arquivo, 0, SEEK_SET);
        fwrite(&arvore->offset_raiz, sizeof(long), 1, arvore->arquivo);
        fflush(arvore->arquivo);
    } else {
        fseek(arvore->arquivo, 0, SEEK_SET);
        fread(&arvore->offset_raiz, sizeof(long), 1, arvore->arquivo);
    }

    arvore->ordem = ordem; arvore->comparar = comparar; arvore->tamanho_chave = tamanho_chave;
    arvore->tamanho_valor = tamanho_valor; arvore->escrever_chave = escrever_chave;
    arvore->ler_chave = ler_chave; arvore->escrever_valor = escrever_valor; arvore->ler_valor = ler_valor;
    return arvore;
}

/**
 * @brief Fecha o arquivo em disco e libera o controlador da memória.
 * @param arvore Controlador da árvore a ser destruído.
 */
void destruir_bmais(BPlusTree *arvore) {
    if (arvore) {
        if (arvore->arquivo) fclose(arvore->arquivo);
        free(arvore);
    }
}


/**
 * @brief Busca um registro pela chave (busca por igualdade exata).
 * @details Navega da raiz até a folha comparando chaves nos nós internos.
 * Quando chega na folha, faz uma varredura linear até encontrar a chave
 * ou constatar sua ausência. Cada leitura de chave/ponteiro envolve
 * fseek + fread no disco.
 * @param arvore Controlador da árvore.
 * @param chave Ponteiro para a chave a ser buscada.
 * @return void* Ponteiro alocado (malloc) para o valor encontrado, ou NULL.
 * @note O ponteiro retornado deve ser liberado com free() pelo chamador.
 */
void* buscar_bmais(BPlusTree *arvore, void *chave) {
    if (arvore->offset_raiz == -1) return NULL;
    long deslocamento_atual = arvore->offset_raiz;
    CabecalhoNo cabecalho;

    while (1) {
        cabecalho = ler_cabecalho(arvore, deslocamento_atual);
        int i = 0;
        for (i = 0; i < cabecalho.num_chaves; i++) {
            fseek(arvore->arquivo, obter_deslocamento_chave(arvore, deslocamento_atual, i), SEEK_SET);
            void *ck = arvore->ler_chave(arvore->arquivo);
            int comparacao = arvore->comparar(chave, ck);
            free(ck);
            if (comparacao < 0) break;
        }

        if (cabecalho.eh_folha) {
            for (int j = 0; j < cabecalho.num_chaves; j++) {
                fseek(arvore->arquivo, obter_deslocamento_chave(arvore, deslocamento_atual, j), SEEK_SET);
                void *chave_folha = arvore->ler_chave(arvore->arquivo);
                if (arvore->comparar(chave, chave_folha) == 0) {
                    free(chave_folha);
                    fseek(arvore->arquivo, obter_deslocamento_valor(arvore, deslocamento_atual, j), SEEK_SET);
                    return arvore->ler_valor(arvore->arquivo);
                }
                free(chave_folha);
            }
            return NULL;
        } else {
            fseek(arvore->arquivo, obter_deslocamento_ponteiro(arvore, deslocamento_atual, i), SEEK_SET);
            fread(&deslocamento_atual, sizeof(long), 1, arvore->arquivo);
        }
    }
}


/**
 * @brief Insere uma chave promovida e o ponteiro do novo filho no nó pai.
 * @details É chamada quando um nó filho (folha ou interno) se divide.
 * Três casos:
 *   - Sem pai (filho era raiz): cria nova raiz interna.
 *   - Pai com espaço: desloca chaves/ponteiros e insere.
 *   - Pai cheio: split do pai (lê tudo para arrays, divide, promove
 *     chave mediana recursivamente).
 * @param arvore Controlador da árvore.
 * @param offset_esq Offset do filho esquerdo (nó que já existia).
 * @param chave_promovida Ponteiro para a chave que sobe para o pai.
 * @param offset_dir Offset do filho direito (nó recém-criado).
 */
static void inserir_no_pai(BPlusTree *arvore, long offset_esq, void *chave_promovida, long offset_dir) {
    CabecalhoNo cab_esq = ler_cabecalho(arvore, offset_esq);
    long offset_pai = cab_esq.offset_pai;

    if (offset_pai == -1) {
        long nova_raiz = alocar_novo_no(arvore);
        CabecalhoNo cab_raiz = { .eh_folha = false, .num_chaves = 1, .offset_pai = -1, .proxima_folha = -1 };
        escrever_cabecalho(arvore, nova_raiz, &cab_raiz);

        fseek(arvore->arquivo, obter_deslocamento_chave(arvore, nova_raiz, 0), SEEK_SET);
        arvore->escrever_chave(chave_promovida, arvore->arquivo);
        fseek(arvore->arquivo, obter_deslocamento_ponteiro(arvore, nova_raiz, 0), SEEK_SET);
        fwrite(&offset_esq, sizeof(long), 1, arvore->arquivo);
        fwrite(&offset_dir, sizeof(long), 1, arvore->arquivo);

        cab_esq.offset_pai = nova_raiz;
        escrever_cabecalho(arvore, offset_esq, &cab_esq);

        CabecalhoNo cab_dir = ler_cabecalho(arvore, offset_dir);
        cab_dir.offset_pai = nova_raiz;
        escrever_cabecalho(arvore, offset_dir, &cab_dir);

        arvore->offset_raiz = nova_raiz;
        fseek(arvore->arquivo, 0, SEEK_SET);
        fwrite(&arvore->offset_raiz, sizeof(long), 1, arvore->arquivo);
        fflush(arvore->arquivo);
        return;
    }

    CabecalhoNo cab_pai = ler_cabecalho(arvore, offset_pai);

    if (cab_pai.num_chaves < arvore->ordem - 1) {
        int i = 0;
        for (i = 0; i < cab_pai.num_chaves; i++) {
            fseek(arvore->arquivo, obter_deslocamento_chave(arvore, offset_pai, i), SEEK_SET);
            void *ck = arvore->ler_chave(arvore->arquivo);
            int comparacao = arvore->comparar(chave_promovida, ck);
            free(ck);
            if (comparacao < 0) break;
        }

        for (int j = cab_pai.num_chaves; j > i; j--) {
            fseek(arvore->arquivo, obter_deslocamento_chave(arvore, offset_pai, j - 1), SEEK_SET);
            void *tk = arvore->ler_chave(arvore->arquivo);
            fseek(arvore->arquivo, obter_deslocamento_chave(arvore, offset_pai, j), SEEK_SET);
            arvore->escrever_chave(tk, arvore->arquivo);
            free(tk);

            long temp_ptr;
            fseek(arvore->arquivo, obter_deslocamento_ponteiro(arvore, offset_pai, j), SEEK_SET);
            fread(&temp_ptr, sizeof(long), 1, arvore->arquivo);
            fseek(arvore->arquivo, obter_deslocamento_ponteiro(arvore, offset_pai, j + 1), SEEK_SET);
            fwrite(&temp_ptr, sizeof(long), 1, arvore->arquivo);
        }
        fseek(arvore->arquivo, obter_deslocamento_chave(arvore, offset_pai, i), SEEK_SET);
        arvore->escrever_chave(chave_promovida, arvore->arquivo);

        fseek(arvore->arquivo, obter_deslocamento_ponteiro(arvore, offset_pai, i + 1), SEEK_SET);
        fwrite(&offset_dir, sizeof(long), 1, arvore->arquivo);

        cab_pai.num_chaves++;
        escrever_cabecalho(arvore, offset_pai, &cab_pai);

        CabecalhoNo cab_dir = ler_cabecalho(arvore, offset_dir);
        cab_dir.offset_pai = offset_pai;
        escrever_cabecalho(arvore, offset_dir, &cab_dir);
    } else {
        int total_chaves = cab_pai.num_chaves + 1;
        void **temp_k = malloc(total_chaves * sizeof(void*));
        long *temp_p = malloc((total_chaves + 1) * sizeof(long));

        int i = 0;
        for (i = 0; i < cab_pai.num_chaves; i++) {
            fseek(arvore->arquivo, obter_deslocamento_chave(arvore, offset_pai, i), SEEK_SET);
            void *ck = arvore->ler_chave(arvore->arquivo);
            if (arvore->comparar(chave_promovida, ck) < 0) { free(ck); break; }
            free(ck);
        }

        int idx = 0;
        for (int j = 0; j < cab_pai.num_chaves; j++) {
            if (j == i) { temp_k[idx] = chave_promovida; temp_p[idx + 1] = offset_dir; idx++; }
            fseek(arvore->arquivo, obter_deslocamento_chave(arvore, offset_pai, j), SEEK_SET);
            temp_k[idx] = arvore->ler_chave(arvore->arquivo);
            fseek(arvore->arquivo, obter_deslocamento_ponteiro(arvore, offset_pai, j + 1), SEEK_SET);
            fread(&temp_p[idx + 1], sizeof(long), 1, arvore->arquivo);
            idx++;
        }
        if (i == cab_pai.num_chaves) { temp_k[idx] = chave_promovida; temp_p[idx + 1] = offset_dir; }
        
        fseek(arvore->arquivo, obter_deslocamento_ponteiro(arvore, offset_pai, 0), SEEK_SET);
        fread(&temp_p[0], sizeof(long), 1, arvore->arquivo);

        long offset_novo_pai = alocar_novo_no(arvore);
        CabecalhoNo cab_np = { .eh_folha = false, .num_chaves = 0, .offset_pai = cab_pai.offset_pai, .proxima_folha = -1 };
        escrever_cabecalho(arvore, offset_novo_pai, &cab_np);

        int metade = total_chaves / 2;
        cab_pai.num_chaves = metade;
        for (int j = 0; j < metade; j++) {
            fseek(arvore->arquivo, obter_deslocamento_chave(arvore, offset_pai, j), SEEK_SET);
            arvore->escrever_chave(temp_k[j], arvore->arquivo);
            fseek(arvore->arquivo, obter_deslocamento_ponteiro(arvore, offset_pai, j), SEEK_SET);
            fwrite(&temp_p[j], sizeof(long), 1, arvore->arquivo);
        }
        fseek(arvore->arquivo, obter_deslocamento_ponteiro(arvore, offset_pai, metade), SEEK_SET);
        fwrite(&temp_p[metade], sizeof(long), 1, arvore->arquivo);

        void *nova_chave_promovida = temp_k[metade];

        cab_np.num_chaves = total_chaves - metade - 1;
        for (int j = 0; j < cab_np.num_chaves; j++) {
            fseek(arvore->arquivo, obter_deslocamento_chave(arvore, offset_novo_pai, j), SEEK_SET);
            arvore->escrever_chave(temp_k[metade + 1 + j], arvore->arquivo);
            fseek(arvore->arquivo, obter_deslocamento_ponteiro(arvore, offset_novo_pai, j), SEEK_SET);
            fwrite(&temp_p[metade + 1 + j], sizeof(long), 1, arvore->arquivo);
            
            CabecalhoNo filho = ler_cabecalho(arvore, temp_p[metade + 1 + j]);
            filho.offset_pai = offset_novo_pai;
            escrever_cabecalho(arvore, temp_p[metade + 1 + j], &filho);
        }
        fseek(arvore->arquivo, obter_deslocamento_ponteiro(arvore, offset_novo_pai, cab_np.num_chaves), SEEK_SET);
        fwrite(&temp_p[total_chaves], sizeof(long), 1, arvore->arquivo);
        
        CabecalhoNo ultimo_filho = ler_cabecalho(arvore, temp_p[total_chaves]);
        ultimo_filho.offset_pai = offset_novo_pai;
        escrever_cabecalho(arvore, temp_p[total_chaves], &ultimo_filho);

        escrever_cabecalho(arvore, offset_pai, &cab_pai);
        escrever_cabecalho(arvore, offset_novo_pai, &cab_np);

        inserir_no_pai(arvore, offset_pai, nova_chave_promovida, offset_novo_pai);

        for (int j = 0; j < total_chaves; j++) {
            if (temp_k[j] != chave_promovida && j != metade) free(temp_k[j]);
        }
        free(temp_k); free(temp_p);
    }
}


/**
 * @brief Insere um par chave-valor na árvore B+.
 * @details Navega até a folha apropriada, verifica duplicatas e:
 *   - Se a folha tem espaço: desloca chaves/valores para a direita
 *     e insere na posição correta (mantendo ordem).
 *   - Se a folha está cheia: faz o split (lê M+1 pares para arrays
 *     temporários, divide ao meio, escreve metade em cada nó, promove
 *     a chave do meio para o pai via inserir_no_pai()).
 * @param arvore Controlador da árvore.
 * @param chave Ponteiro para a chave a ser inserida.
 * @param valor Ponteiro para o valor a ser inserido.
 * @return int 1 se sucesso, 0 se chave duplicada.
 * @example
 * ChaveRH k = { .nome = "Joao", .data_nascimento = {15, 5, 1990} };
 * Funcionario f;
 * f.chave = k;
 * inserir_bmais(arv, &k, &f);
 */
int inserir_bmais(BPlusTree *arvore, void *chave, void *valor) {
    if (arvore->offset_raiz == -1) {
        arvore->offset_raiz = criar_no_folha(arvore);
        fseek(arvore->arquivo, 0, SEEK_SET);
        fwrite(&arvore->offset_raiz, sizeof(long), 1, arvore->arquivo);
        fflush(arvore->arquivo);
    }

    long offset_atual = arvore->offset_raiz;
    CabecalhoNo cabecalho = ler_cabecalho(arvore, offset_atual);

    while (!cabecalho.eh_folha) {
        int i = 0;
        for (i = 0; i < cabecalho.num_chaves; i++) {
            fseek(arvore->arquivo, obter_deslocamento_chave(arvore, offset_atual, i), SEEK_SET);
            void *ck = arvore->ler_chave(arvore->arquivo);
            int comparacao = arvore->comparar(chave, ck);
            free(ck);
            if (comparacao < 0) break;
        }
        fseek(arvore->arquivo, obter_deslocamento_ponteiro(arvore, offset_atual, i), SEEK_SET);
        fread(&offset_atual, sizeof(long), 1, arvore->arquivo);
        cabecalho = ler_cabecalho(arvore, offset_atual);
    }

    for (int i = 0; i < cabecalho.num_chaves; i++) {
        fseek(arvore->arquivo, obter_deslocamento_chave(arvore, offset_atual, i), SEEK_SET);
        void *ck = arvore->ler_chave(arvore->arquivo);
        if (arvore->comparar(chave, ck) == 0) { free(ck); return 0; }
        free(ck);
    }

    if (cabecalho.num_chaves < arvore->ordem - 1) {
        int i = 0;
        for (i = 0; i < cabecalho.num_chaves; i++) {
            fseek(arvore->arquivo, obter_deslocamento_chave(arvore, offset_atual, i), SEEK_SET);
            void *ck = arvore->ler_chave(arvore->arquivo);
            if (arvore->comparar(chave, ck) < 0) { free(ck); break; }
            free(ck);
        }

        for (int j = cabecalho.num_chaves; j > i; j--) {
            fseek(arvore->arquivo, obter_deslocamento_chave(arvore, offset_atual, j - 1), SEEK_SET);
            void *tk = arvore->ler_chave(arvore->arquivo);
            fseek(arvore->arquivo, obter_deslocamento_chave(arvore, offset_atual, j), SEEK_SET);
            arvore->escrever_chave(tk, arvore->arquivo);
            free(tk);

            fseek(arvore->arquivo, obter_deslocamento_valor(arvore, offset_atual, j - 1), SEEK_SET);
            void *tv = arvore->ler_valor(arvore->arquivo);
            fseek(arvore->arquivo, obter_deslocamento_valor(arvore, offset_atual, j), SEEK_SET);
            arvore->escrever_valor(tv, arvore->arquivo);
            free(tv);
        }

        fseek(arvore->arquivo, obter_deslocamento_chave(arvore, offset_atual, i), SEEK_SET);
        arvore->escrever_chave(chave, arvore->arquivo);
        fseek(arvore->arquivo, obter_deslocamento_valor(arvore, offset_atual, i), SEEK_SET);
        arvore->escrever_valor(valor, arvore->arquivo);

        cabecalho.num_chaves++;
        escrever_cabecalho(arvore, offset_atual, &cabecalho);
        return 1;
    } else {
        long offset_nova = alocar_novo_no(arvore);
        CabecalhoNo cabecalho_nova = { .eh_folha = true, .num_chaves = 0, .offset_pai = cabecalho.offset_pai, .proxima_folha = cabecalho.proxima_folha };
        escrever_cabecalho(arvore, offset_nova, &cabecalho_nova);
        cabecalho.proxima_folha = offset_nova;

        int total = cabecalho.num_chaves + 1;
        void **temp_k = malloc(total * sizeof(void*));
        void **temp_v = malloc(total * sizeof(void*));

        int inserido = 0, idx_temp = 0;
        for (int i = 0; i < cabecalho.num_chaves; i++) {
            fseek(arvore->arquivo, obter_deslocamento_chave(arvore, offset_atual, i), SEEK_SET);
            void *ck = arvore->ler_chave(arvore->arquivo);
            fseek(arvore->arquivo, obter_deslocamento_valor(arvore, offset_atual, i), SEEK_SET);
            void *cv = arvore->ler_valor(arvore->arquivo);

            if (!inserido && arvore->comparar(chave, ck) < 0) {
                temp_k[idx_temp] = chave; temp_v[idx_temp] = valor;
                idx_temp++; inserido = 1;
            }
            temp_k[idx_temp] = ck; temp_v[idx_temp] = cv;
            idx_temp++;
        }
        if (!inserido) { temp_k[idx_temp] = chave; temp_v[idx_temp] = valor; }

        int metade = total / 2;
        cabecalho.num_chaves = 0;
        for (int i = 0; i < metade; i++) {
            fseek(arvore->arquivo, obter_deslocamento_chave(arvore, offset_atual, i), SEEK_SET);
            arvore->escrever_chave(temp_k[i], arvore->arquivo);
            fseek(arvore->arquivo, obter_deslocamento_valor(arvore, offset_atual, i), SEEK_SET);
            arvore->escrever_valor(temp_v[i], arvore->arquivo);
            cabecalho.num_chaves++;
        }

        cabecalho_nova.num_chaves = 0;
        for (int i = metade; i < total; i++) {
            fseek(arvore->arquivo, obter_deslocamento_chave(arvore, offset_nova, cabecalho_nova.num_chaves), SEEK_SET);
            arvore->escrever_chave(temp_k[i], arvore->arquivo);
            fseek(arvore->arquivo, obter_deslocamento_valor(arvore, offset_nova, cabecalho_nova.num_chaves), SEEK_SET);
            arvore->escrever_valor(temp_v[i], arvore->arquivo);
            cabecalho_nova.num_chaves++;
        }

        escrever_cabecalho(arvore, offset_atual, &cabecalho);
        escrever_cabecalho(arvore, offset_nova, &cabecalho_nova);

        void *chave_promovida = temp_k[metade];
        inserir_no_pai(arvore, offset_atual, chave_promovida, offset_nova);

        for (int i = 0; i < total; i++) {
            if (temp_k[i] != chave) free(temp_k[i]);
            if (temp_v[i] != valor) free(temp_v[i]);
        }
        free(temp_k); free(temp_v);
        return 1;
    }
}


/**
 * @brief Estrutura auxiliar para armazenar pares chave-valor em memória
 * durante a operação de remoção.
 */
typedef struct {
    void *chave;
    void *valor;
} RegistroTemporario;

/**
 * @brief Remove um registro da árvore B+ pela chave.
 * @details Estratégia simplificada (não implementa merge/rebalance padrão):
 *   1. Percorre todas as folhas via encadeamento proxima_folha.
 *   2. Coleta em arrays temporários todos os pares exceto o que deve ser removido.
 *   3. Trunca o arquivo inteiro com ftruncate().
 *   4. Reinsere todos os pares restantes um a um com inserir_bmais().
 * @param arvore Controlador da árvore.
 * @param chave Ponteiro para a chave a ser removida.
 * @return int 1 se sucesso, 0 se a chave não for encontrada.
 */
int remover_bmais(BPlusTree *arvore, void *chave) {
    if (arvore->offset_raiz == -1) return 0;

    long offset_atual = arvore->offset_raiz;
    CabecalhoNo cabecalho = ler_cabecalho(arvore, offset_atual);
    while (!cabecalho.eh_folha) {
        fseek(arvore->arquivo, obter_deslocamento_ponteiro(arvore, offset_atual, 0), SEEK_SET);
        fread(&offset_atual, sizeof(long), 1, arvore->arquivo);
        cabecalho = ler_cabecalho(arvore, offset_atual);
    }

    RegistroTemporario *registros = NULL;
    int quantidade = 0;
    int encontrado = 0;

    while (offset_atual != -1) {
        cabecalho = ler_cabecalho(arvore, offset_atual);

        for (int i = 0; i < cabecalho.num_chaves; i++) {
            fseek(arvore->arquivo, obter_deslocamento_chave(arvore, offset_atual, i), SEEK_SET);
            void *chave_atual = arvore->ler_chave(arvore->arquivo);
            fseek(arvore->arquivo, obter_deslocamento_valor(arvore, offset_atual, i), SEEK_SET);
            void *valor_atual = arvore->ler_valor(arvore->arquivo);

            if (arvore->comparar(chave, chave_atual) == 0) {
                encontrado = 1;
                free(chave_atual);
                free(valor_atual);
            } else {
                RegistroTemporario *novo_vetor = realloc(registros, (quantidade + 1) * sizeof(RegistroTemporario));
                if (!novo_vetor) {
                    free(chave_atual);
                    free(valor_atual);
                    for (int j = 0; j < quantidade; j++) {
                        free(registros[j].chave);
                        free(registros[j].valor);
                    }
                    free(registros);
                    return 0;
                }

                registros = novo_vetor;
                registros[quantidade].chave = chave_atual;
                registros[quantidade].valor = valor_atual;
                quantidade++;
            }
        }

        offset_atual = cabecalho.proxima_folha;
    }

    if (!encontrado) {
        for (int i = 0; i < quantidade; i++) {
            free(registros[i].chave);
            free(registros[i].valor);
        }
        free(registros);
        return 0;
    }

    fflush(arvore->arquivo);
    if (ftruncate(fileno(arvore->arquivo), 0) != 0) {
        for (int i = 0; i < quantidade; i++) {
            free(registros[i].chave);
            free(registros[i].valor);
        }
        free(registros);
        return 0;
    }

    rewind(arvore->arquivo);
    arvore->offset_raiz = -1;
    fwrite(&arvore->offset_raiz, sizeof(long), 1, arvore->arquivo);
    fflush(arvore->arquivo);

    for (int i = 0; i < quantidade; i++) {
        inserir_bmais(arvore, registros[i].chave, registros[i].valor);
        free(registros[i].chave);
        free(registros[i].valor);
    }

    free(registros);
    return 1;
}


/**
 * @brief Lista todos os registros cuja chave pertence ao intervalo aberto (chave_a, chave_b).
 * @details Navega até a primeira folha relevante (onde chaves > chave_a),
 * depois percorre linearmente a lista encadeada de folhas via campo
 * proxima_folha. Para cada chave no intervalo, invoca imprimir_func
 * com o valor correspondente. A busca para quando chave >= chave_b.
 * @param arvore Controlador da árvore.
 * @param chave_a Limite inferior do intervalo (exclusivo).
 * @param chave_b Limite superior do intervalo (exclusivo).
 * @param imprimir_func Função callback para processar/exibir cada valor encontrado.
 * @example
 * ChaveRH a = { .nome = "Ana", .data_nascimento = {0,0,0} };
 * ChaveRH b = { .nome = "Carlos", .data_nascimento = {31,12,9999} };
 * buscar_intervalo_bmais(arv, &a, &b, imprimir_funcionario_intervalo);
 */
void buscar_intervalo_bmais(BPlusTree *arvore, void *chave_a, void *chave_b, void (*imprimir_func)(void *valor)) {
    if (arvore->offset_raiz == -1) return;
    long offset_atual = arvore->offset_raiz;
    CabecalhoNo cabecalho = ler_cabecalho(arvore, offset_atual);

    while (!cabecalho.eh_folha) {
        int i = 0;
        for (i = 0; i < cabecalho.num_chaves; i++) {
            fseek(arvore->arquivo, obter_deslocamento_chave(arvore, offset_atual, i), SEEK_SET);
            void *ck = arvore->ler_chave(arvore->arquivo);
            if (arvore->comparar(chave_a, ck) < 0) { free(ck); break; }
            free(ck);
        }
        fseek(arvore->arquivo, obter_deslocamento_ponteiro(arvore, offset_atual, i), SEEK_SET);
        fread(&offset_atual, sizeof(long), 1, arvore->arquivo);
        cabecalho = ler_cabecalho(arvore, offset_atual);
    }

    bool continuar = true;
    while (offset_atual != -1 && continuar) {
        cabecalho = ler_cabecalho(arvore, offset_atual);
        for (int i = 0; i < cabecalho.num_chaves; i++) {
            fseek(arvore->arquivo, obter_deslocamento_chave(arvore, offset_atual, i), SEEK_SET);
            void *ck = arvore->ler_chave(arvore->arquivo);

            if (arvore->comparar(ck, chave_a) > 0 && arvore->comparar(ck, chave_b) < 0) {
                fseek(arvore->arquivo, obter_deslocamento_valor(arvore, offset_atual, i), SEEK_SET);
                void *valor_lido = arvore->ler_valor(arvore->arquivo);
                imprimir_func(valor_lido);
                free(valor_lido);
            }
            if (arvore->comparar(ck, chave_b) >= 0) { continuar = false; free(ck); break; }
            free(ck);
        }
        offset_atual = cabecalho.proxima_folha;
    }
}

/**
 * @brief Função recursiva auxiliar para imprimir um nó e seus descendentes.
 * @param arvore Controlador da árvore.
 * @param deslocamento Offset do nó a ser impresso.
 * @param nivel Nível atual na árvore (0 para raiz).
 * @param imprimir_chave Função callback para formatar cada chave.
 */
static void imprimir_no_recursivo(BPlusTree *arvore, long deslocamento, int nivel, void (*imprimir_chave)(void *chave)) {
    if (deslocamento == -1) return;
    CabecalhoNo cabecalho = ler_cabecalho(arvore, deslocamento);
    
    for (int i = 0; i < nivel; i++) printf("    ");
    printf("[%s] Nivel %d (Deslocamento: %ld): ", cabecalho.eh_folha ? "FOLHA" : "INTERNO", nivel, deslocamento);
    
    for (int i = 0; i < cabecalho.num_chaves; i++) {
        fseek(arvore->arquivo, obter_deslocamento_chave(arvore, deslocamento, i), SEEK_SET);
        void *ck = arvore->ler_chave(arvore->arquivo);
        if (i > 0) printf(" | ");
        imprimir_chave(ck);
        free(ck);
    }
    printf("\n");
    
    if (!cabecalho.eh_folha) {
        for (int i = 0; i <= cabecalho.num_chaves; i++) {
            long offset_filho;
            fseek(arvore->arquivo, obter_deslocamento_ponteiro(arvore, deslocamento, i), SEEK_SET);
            fread(&offset_filho, sizeof(long), 1, arvore->arquivo);
            imprimir_no_recursivo(arvore, offset_filho, nivel + 1, imprimir_chave);
        }
    }
}

/**
 * @brief Imprime a estrutura hierárquica completa da árvore B+.
 * @details Exibe cada nó com seu tipo (FOLHA/INTERNO), nível, offset em disco
 * e as chaves armazenadas. A recursão percorre todos os filhos.
 * @param arvore Controlador da árvore.
 * @param imprimir_chave Função callback para formatar e exibir cada chave.
 */
void imprimir_estrutura_bmais(BPlusTree *arvore, void (*imprimir_chave)(void *chave)) {
    if (arvore->offset_raiz == -1) { printf("[ESTRUTURA] Arvore Vazia.\n"); return; }
    imprimir_no_recursivo(arvore, arvore->offset_raiz, 0, imprimir_chave);
}
