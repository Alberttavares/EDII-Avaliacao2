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

/* --- Cabeçalho de Controle de cada Nó no Disco --- */
typedef struct {
    bool eh_folha;
    int num_chaves;
    long offset_pai;
    long proxima_folha; 
} CabecalhoNo;

/* --- Estrutura de Controle Principal --- */
struct BPlusTree {
    FILE *arquivo;
    int ordem;
    long offset_raiz;
    
    CompareFunc comparar;
    SizeFunc tamanho_chave;
    SizeFunc tamanho_valor;
    WriteFunc escrever_chave;
    ReadFunc ler_chave;
    WriteFunc escrever_valor;
    ReadFunc ler_valor;
};

/* --- Funções de Cálculo de Offsets Genéricos --- */

static long obter_offset_chave(BPlusTree *arvore, long offset_no, int indice) {
    return offset_no + sizeof(CabecalhoNo) + (indice * arvore->tamanho_chave(NULL));
}

static long obter_offset_valor(BPlusTree *arvore, long offset_no, int indice) {
    long inicio_valores = offset_no + sizeof(CabecalhoNo) + ((arvore->ordem - 1) * arvore->tamanho_chave(NULL));
    return inicio_valores + (indice * arvore->tamanho_valor(NULL));
}

static long obter_offset_ponteiro(BPlusTree *arvore, long offset_no, int indice) {
    long inicio_ponteiros = offset_no + sizeof(CabecalhoNo) + 
                            ((arvore->ordem - 1) * arvore->tamanho_chave(NULL)) + 
                            ((arvore->ordem - 1) * arvore->tamanho_valor(NULL));
    return inicio_ponteiros + (indice * sizeof(long));
}

/* --- O CORAÇÃO DA CORREÇÃO: Alocação Segura de Bloco --- */
static long alocar_novo_no(BPlusTree *arvore) {
    fseek(arvore->arquivo, 0, SEEK_END);
    long offset = ftell(arvore->arquivo);
    
    // Calcula o tamanho máximo exato do nó e preenche com zeros (padding)
    size_t tam_chaves = (arvore->ordem - 1) * arvore->tamanho_chave(NULL);
    size_t tam_valores = (arvore->ordem - 1) * arvore->tamanho_valor(NULL);
    size_t tam_ponteiros = arvore->ordem * sizeof(long);
    size_t tam_total = sizeof(CabecalhoNo) + tam_chaves + tam_valores + tam_ponteiros;
    
    void *zeros = calloc(1, tam_total);
    fwrite(zeros, tam_total, 1, arvore->arquivo);
    free(zeros);
    fflush(arvore->arquivo);
    
    return offset;
}

/* --- Funções de I/O em Disco --- */

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

static void escrever_cabecalho(BPlusTree *arvore, long deslocamento, CabecalhoNo *cabecalho) {
    fseek(arvore->arquivo, deslocamento, SEEK_SET);
    fwrite(cabecalho, sizeof(CabecalhoNo), 1, arvore->arquivo);
    fflush(arvore->arquivo);
}

static long criar_no_folha(BPlusTree *arvore) {
    long novo_deslocamento = alocar_novo_no(arvore);
    CabecalhoNo cabecalho = { .eh_folha = true, .num_chaves = 0, .offset_pai = -1, .proxima_folha = -1 };
    escrever_cabecalho(arvore, novo_deslocamento, &cabecalho);
    return novo_deslocamento;
}

/* --- Inicialização e Destruição --- */

BPlusTree* bplus_create(const char *filename, int order, CompareFunc cmp, 
                        SizeFunc key_size, SizeFunc val_size,
                        WriteFunc write_k, ReadFunc read_k, WriteFunc write_v, ReadFunc read_v) {
    
    BPlusTree *arvore = (BPlusTree*) malloc(sizeof(BPlusTree));
    if (!arvore) return NULL;

    arvore->arquivo = fopen(filename, "r+b");
    if (!arvore->arquivo) {
        arvore->arquivo = fopen(filename, "w+b");
        if (!arvore->arquivo) { free(arvore); return NULL; }
        arvore->offset_raiz = -1;
        fseek(arvore->arquivo, 0, SEEK_SET);
        fwrite(&arvore->offset_raiz, sizeof(long), 1, arvore->arquivo);
        fflush(arvore->arquivo);
    } else {
        fseek(arvore->arquivo, 0, SEEK_SET);
        fread(&arvore->offset_raiz, sizeof(long), 1, arvore->arquivo);
    }

    arvore->ordem = order; arvore->comparar = cmp; arvore->tamanho_chave = key_size;
    arvore->tamanho_valor = val_size; arvore->escrever_chave = write_k;
    arvore->ler_chave = read_k; arvore->escrever_valor = write_v; arvore->ler_valor = read_v;
    return arvore;
}

void bplus_destroy(BPlusTree *arvore) {
    if (arvore) {
        if (arvore->arquivo) fclose(arvore->arquivo);
        free(arvore);
    }
}

/* --- Operação de Busca --- */

void* bplus_search(BPlusTree *arvore, void *key) {
    if (arvore->offset_raiz == -1) return NULL;
    long deslocamento_atual = arvore->offset_raiz;
    CabecalhoNo cabecalho;

    while (1) {
        cabecalho = ler_cabecalho(arvore, deslocamento_atual);
        int i = 0;
        for (i = 0; i < cabecalho.num_chaves; i++) {
            fseek(arvore->arquivo, obter_offset_chave(arvore, deslocamento_atual, i), SEEK_SET);
            void *ck = arvore->ler_chave(arvore->arquivo);
            int cmp = arvore->comparar(key, ck);
            free(ck);
            if (cmp < 0) break;
        }

        if (cabecalho.eh_folha) {
            for (int j = 0; j < cabecalho.num_chaves; j++) {
                fseek(arvore->arquivo, obter_offset_chave(arvore, deslocamento_atual, j), SEEK_SET);
                void *chave_folha = arvore->ler_chave(arvore->arquivo);
                if (arvore->comparar(key, chave_folha) == 0) {
                    free(chave_folha);
                    fseek(arvore->arquivo, obter_offset_valor(arvore, deslocamento_atual, j), SEEK_SET);
                    return arvore->ler_valor(arvore->arquivo);
                }
                free(chave_folha);
            }
            return NULL;
        } else {
            fseek(arvore->arquivo, obter_offset_ponteiro(arvore, deslocamento_atual, i), SEEK_SET);
            fread(&deslocamento_atual, sizeof(long), 1, arvore->arquivo);
        }
    }
}

/* --- Lógica de Propagação (Split Pai) --- */

static void inserir_no_pai(BPlusTree *arvore, long offset_esq, void *chave_promovida, long offset_dir) {
    CabecalhoNo cab_esq = ler_cabecalho(arvore, offset_esq);
    long offset_pai = cab_esq.offset_pai;

    if (offset_pai == -1) {
        long nova_raiz = alocar_novo_no(arvore);
        CabecalhoNo cab_raiz = { .eh_folha = false, .num_chaves = 1, .offset_pai = -1, .proxima_folha = -1 };
        escrever_cabecalho(arvore, nova_raiz, &cab_raiz);

        fseek(arvore->arquivo, obter_offset_chave(arvore, nova_raiz, 0), SEEK_SET);
        arvore->escrever_chave(chave_promovida, arvore->arquivo);
        fseek(arvore->arquivo, obter_offset_ponteiro(arvore, nova_raiz, 0), SEEK_SET);
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
            fseek(arvore->arquivo, obter_offset_chave(arvore, offset_pai, i), SEEK_SET);
            void *ck = arvore->ler_chave(arvore->arquivo);
            int cmp = arvore->comparar(chave_promovida, ck);
            free(ck);
            if (cmp < 0) break;
        }

        for (int j = cab_pai.num_chaves; j > i; j--) {
            fseek(arvore->arquivo, obter_offset_chave(arvore, offset_pai, j - 1), SEEK_SET);
            void *tk = arvore->ler_chave(arvore->arquivo);
            fseek(arvore->arquivo, obter_offset_chave(arvore, offset_pai, j), SEEK_SET);
            arvore->escrever_chave(tk, arvore->arquivo);
            free(tk);

            long temp_ptr;
            fseek(arvore->arquivo, obter_offset_ponteiro(arvore, offset_pai, j), SEEK_SET);
            fread(&temp_ptr, sizeof(long), 1, arvore->arquivo);
            fseek(arvore->arquivo, obter_offset_ponteiro(arvore, offset_pai, j + 1), SEEK_SET);
            fwrite(&temp_ptr, sizeof(long), 1, arvore->arquivo);
        }
        fseek(arvore->arquivo, obter_offset_chave(arvore, offset_pai, i), SEEK_SET);
        arvore->escrever_chave(chave_promovida, arvore->arquivo);

        fseek(arvore->arquivo, obter_offset_ponteiro(arvore, offset_pai, i + 1), SEEK_SET);
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
            fseek(arvore->arquivo, obter_offset_chave(arvore, offset_pai, i), SEEK_SET);
            void *ck = arvore->ler_chave(arvore->arquivo);
            if (arvore->comparar(chave_promovida, ck) < 0) { free(ck); break; }
            free(ck);
        }

        int idx = 0;
        for (int j = 0; j < cab_pai.num_chaves; j++) {
            if (j == i) { temp_k[idx] = chave_promovida; temp_p[idx + 1] = offset_dir; idx++; }
            fseek(arvore->arquivo, obter_offset_chave(arvore, offset_pai, j), SEEK_SET);
            temp_k[idx] = arvore->ler_chave(arvore->arquivo);
            fseek(arvore->arquivo, obter_offset_ponteiro(arvore, offset_pai, j + 1), SEEK_SET);
            fread(&temp_p[idx + 1], sizeof(long), 1, arvore->arquivo);
            idx++;
        }
        if (i == cab_pai.num_chaves) { temp_k[idx] = chave_promovida; temp_p[idx + 1] = offset_dir; }
        
        fseek(arvore->arquivo, obter_offset_ponteiro(arvore, offset_pai, 0), SEEK_SET);
        fread(&temp_p[0], sizeof(long), 1, arvore->arquivo);

        long offset_novo_pai = alocar_novo_no(arvore);
        CabecalhoNo cab_np = { .eh_folha = false, .num_chaves = 0, .offset_pai = cab_pai.offset_pai, .proxima_folha = -1 };
        escrever_cabecalho(arvore, offset_novo_pai, &cab_np);

        int metade = total_chaves / 2;
        cab_pai.num_chaves = metade;
        for (int j = 0; j < metade; j++) {
            fseek(arvore->arquivo, obter_offset_chave(arvore, offset_pai, j), SEEK_SET);
            arvore->escrever_chave(temp_k[j], arvore->arquivo);
            fseek(arvore->arquivo, obter_offset_ponteiro(arvore, offset_pai, j), SEEK_SET);
            fwrite(&temp_p[j], sizeof(long), 1, arvore->arquivo);
        }
        fseek(arvore->arquivo, obter_offset_ponteiro(arvore, offset_pai, metade), SEEK_SET);
        fwrite(&temp_p[metade], sizeof(long), 1, arvore->arquivo);

        void *nova_chave_promovida = temp_k[metade];

        cab_np.num_chaves = total_chaves - metade - 1;
        for (int j = 0; j < cab_np.num_chaves; j++) {
            fseek(arvore->arquivo, obter_offset_chave(arvore, offset_novo_pai, j), SEEK_SET);
            arvore->escrever_chave(temp_k[metade + 1 + j], arvore->arquivo);
            fseek(arvore->arquivo, obter_offset_ponteiro(arvore, offset_novo_pai, j), SEEK_SET);
            fwrite(&temp_p[metade + 1 + j], sizeof(long), 1, arvore->arquivo);
            
            CabecalhoNo filho = ler_cabecalho(arvore, temp_p[metade + 1 + j]);
            filho.offset_pai = offset_novo_pai;
            escrever_cabecalho(arvore, temp_p[metade + 1 + j], &filho);
        }
        fseek(arvore->arquivo, obter_offset_ponteiro(arvore, offset_novo_pai, cab_np.num_chaves), SEEK_SET);
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

/* --- Inserção e Split de Folha --- */

int bplus_insert(BPlusTree *arvore, void *key, void *value) {
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
            fseek(arvore->arquivo, obter_offset_chave(arvore, offset_atual, i), SEEK_SET);
            void *ck = arvore->ler_chave(arvore->arquivo);
            int cmp = arvore->comparar(key, ck);
            free(ck);
            if (cmp < 0) break;
        }
        fseek(arvore->arquivo, obter_offset_ponteiro(arvore, offset_atual, i), SEEK_SET);
        fread(&offset_atual, sizeof(long), 1, arvore->arquivo);
        cabecalho = ler_cabecalho(arvore, offset_atual);
    }

    for (int i = 0; i < cabecalho.num_chaves; i++) {
        fseek(arvore->arquivo, obter_offset_chave(arvore, offset_atual, i), SEEK_SET);
        void *ck = arvore->ler_chave(arvore->arquivo);
        if (arvore->comparar(key, ck) == 0) { free(ck); return 0; }
        free(ck);
    }

    if (cabecalho.num_chaves < arvore->ordem - 1) {
        int i = 0;
        for (i = 0; i < cabecalho.num_chaves; i++) {
            fseek(arvore->arquivo, obter_offset_chave(arvore, offset_atual, i), SEEK_SET);
            void *ck = arvore->ler_chave(arvore->arquivo);
            if (arvore->comparar(key, ck) < 0) { free(ck); break; }
            free(ck);
        }

        for (int j = cabecalho.num_chaves; j > i; j--) {
            fseek(arvore->arquivo, obter_offset_chave(arvore, offset_atual, j - 1), SEEK_SET);
            void *tk = arvore->ler_chave(arvore->arquivo);
            fseek(arvore->arquivo, obter_offset_chave(arvore, offset_atual, j), SEEK_SET);
            arvore->escrever_chave(tk, arvore->arquivo);
            free(tk);

            fseek(arvore->arquivo, obter_offset_valor(arvore, offset_atual, j - 1), SEEK_SET);
            void *tv = arvore->ler_valor(arvore->arquivo);
            fseek(arvore->arquivo, obter_offset_valor(arvore, offset_atual, j), SEEK_SET);
            arvore->escrever_valor(tv, arvore->arquivo);
            free(tv);
        }

        fseek(arvore->arquivo, obter_offset_chave(arvore, offset_atual, i), SEEK_SET);
        arvore->escrever_chave(key, arvore->arquivo);
        fseek(arvore->arquivo, obter_offset_valor(arvore, offset_atual, i), SEEK_SET);
        arvore->escrever_valor(value, arvore->arquivo);

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
            fseek(arvore->arquivo, obter_offset_chave(arvore, offset_atual, i), SEEK_SET);
            void *ck = arvore->ler_chave(arvore->arquivo);
            fseek(arvore->arquivo, obter_offset_valor(arvore, offset_atual, i), SEEK_SET);
            void *cv = arvore->ler_valor(arvore->arquivo);

            if (!inserido && arvore->comparar(key, ck) < 0) {
                temp_k[idx_temp] = key; temp_v[idx_temp] = value;
                idx_temp++; inserido = 1;
            }
            temp_k[idx_temp] = ck; temp_v[idx_temp] = cv;
            idx_temp++;
        }
        if (!inserido) { temp_k[idx_temp] = key; temp_v[idx_temp] = value; }

        int metade = total / 2;
        cabecalho.num_chaves = 0;
        for (int i = 0; i < metade; i++) {
            fseek(arvore->arquivo, obter_offset_chave(arvore, offset_atual, i), SEEK_SET);
            arvore->escrever_chave(temp_k[i], arvore->arquivo);
            fseek(arvore->arquivo, obter_offset_valor(arvore, offset_atual, i), SEEK_SET);
            arvore->escrever_valor(temp_v[i], arvore->arquivo);
            cabecalho.num_chaves++;
        }

        cabecalho_nova.num_chaves = 0;
        for (int i = metade; i < total; i++) {
            fseek(arvore->arquivo, obter_offset_chave(arvore, offset_nova, cabecalho_nova.num_chaves), SEEK_SET);
            arvore->escrever_chave(temp_k[i], arvore->arquivo);
            fseek(arvore->arquivo, obter_offset_valor(arvore, offset_nova, cabecalho_nova.num_chaves), SEEK_SET);
            arvore->escrever_valor(temp_v[i], arvore->arquivo);
            cabecalho_nova.num_chaves++;
        }

        escrever_cabecalho(arvore, offset_atual, &cabecalho);
        escrever_cabecalho(arvore, offset_nova, &cabecalho_nova);

        void *chave_promovida = temp_k[metade];
        inserir_no_pai(arvore, offset_atual, chave_promovida, offset_nova);

        for (int i = 0; i < total; i++) {
            if (temp_k[i] != key) free(temp_k[i]);
            if (temp_v[i] != value) free(temp_v[i]);
        }
        free(temp_k); free(temp_v);
        return 1;
    }
}

/* --- Remoção --- */

typedef struct {
    void *chave;
    void *valor;
} RegistroTemporario;

int bplus_remove(BPlusTree *arvore, void *key) {
    if (arvore->offset_raiz == -1) return 0;

    long offset_atual = arvore->offset_raiz;
    CabecalhoNo cabecalho = ler_cabecalho(arvore, offset_atual);
    while (!cabecalho.eh_folha) {
        fseek(arvore->arquivo, obter_offset_ponteiro(arvore, offset_atual, 0), SEEK_SET);
        fread(&offset_atual, sizeof(long), 1, arvore->arquivo);
        cabecalho = ler_cabecalho(arvore, offset_atual);
    }

    RegistroTemporario *registros = NULL;
    int quantidade = 0;
    int encontrado = 0;

    while (offset_atual != -1) {
        cabecalho = ler_cabecalho(arvore, offset_atual);

        for (int i = 0; i < cabecalho.num_chaves; i++) {
            fseek(arvore->arquivo, obter_offset_chave(arvore, offset_atual, i), SEEK_SET);
            void *chave_atual = arvore->ler_chave(arvore->arquivo);
            fseek(arvore->arquivo, obter_offset_valor(arvore, offset_atual, i), SEEK_SET);
            void *valor_atual = arvore->ler_valor(arvore->arquivo);

            if (arvore->comparar(key, chave_atual) == 0) {
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
        bplus_insert(arvore, registros[i].chave, registros[i].valor);
        free(registros[i].chave);
        free(registros[i].valor);
    }

    free(registros);
    return 1;
}

/* --- Buscas Extras --- */

void bplus_range_search(BPlusTree *arvore, void *keyA, void *keyB, void (*print_func)(void *val)) {
    if (arvore->offset_raiz == -1) return;
    long offset_atual = arvore->offset_raiz;
    CabecalhoNo cabecalho = ler_cabecalho(arvore, offset_atual);

    while (!cabecalho.eh_folha) {
        int i = 0;
        for (i = 0; i < cabecalho.num_chaves; i++) {
            fseek(arvore->arquivo, obter_offset_chave(arvore, offset_atual, i), SEEK_SET);
            void *ck = arvore->ler_chave(arvore->arquivo);
            if (arvore->comparar(keyA, ck) < 0) { free(ck); break; }
            free(ck);
        }
        fseek(arvore->arquivo, obter_offset_ponteiro(arvore, offset_atual, i), SEEK_SET);
        fread(&offset_atual, sizeof(long), 1, arvore->arquivo);
        cabecalho = ler_cabecalho(arvore, offset_atual);
    }

    bool continuar = true;
    while (offset_atual != -1 && continuar) {
        cabecalho = ler_cabecalho(arvore, offset_atual);
        for (int i = 0; i < cabecalho.num_chaves; i++) {
            fseek(arvore->arquivo, obter_offset_chave(arvore, offset_atual, i), SEEK_SET);
            void *ck = arvore->ler_chave(arvore->arquivo);

            if (arvore->comparar(ck, keyA) > 0 && arvore->comparar(ck, keyB) < 0) {
                fseek(arvore->arquivo, obter_offset_valor(arvore, offset_atual, i), SEEK_SET);
                void *valor_lido = arvore->ler_valor(arvore->arquivo);
                print_func(valor_lido);
                free(valor_lido);
            }
            if (arvore->comparar(ck, keyB) >= 0) { continuar = false; free(ck); break; }
            free(ck);
        }
        offset_atual = cabecalho.proxima_folha;
    }
}

static void imprimir_no_recursivo(BPlusTree *arvore, long offset, int nivel, void (*print_key)(void *key)) {
    if (offset == -1) return;
    CabecalhoNo cabecalho = ler_cabecalho(arvore, offset);
    
    for (int i = 0; i < nivel; i++) printf("    ");
    printf("[%s] Nivel %d (Offset: %ld): ", cabecalho.eh_folha ? "FOLHA" : "INTERNO", nivel, offset);
    
    for (int i = 0; i < cabecalho.num_chaves; i++) {
        fseek(arvore->arquivo, obter_offset_chave(arvore, offset, i), SEEK_SET);
        void *ck = arvore->ler_chave(arvore->arquivo);
        print_key(ck);
        printf(" | ");
        free(ck);
    }
    printf("\n");
    
    if (!cabecalho.eh_folha) {
        for (int i = 0; i <= cabecalho.num_chaves; i++) {
            long offset_filho;
            fseek(arvore->arquivo, obter_offset_ponteiro(arvore, offset, i), SEEK_SET);
            fread(&offset_filho, sizeof(long), 1, arvore->arquivo);
            imprimir_no_recursivo(arvore, offset_filho, nivel + 1, print_key);
        }
    }
}

void bplus_print_structure(BPlusTree *arvore, void (*print_key)(void *key)) {
    if (arvore->offset_raiz == -1) { printf("[ESTRUTURA] Arvore Vazia.\n"); return; }
    imprimir_no_recursivo(arvore, arvore->offset_raiz, 0, print_key);
}
