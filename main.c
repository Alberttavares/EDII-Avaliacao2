/* ==============================================
|     Trabalho de Estrutura de Dados II      |
----------------------------------------------
| Arquivo: main.c                            |
----------------------------------------------
|                 Alunos                     |
----------------------------------------------
| Gabriel Vargas de Melo                     |
| Isaac Gabriel Covre Silva                  |
| Albert Rocha Tavares                       |
| Victor Rodrigues Silva                     |
============================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Bplus.h"
#include "RH.h"

#define MAX_RESULTADOS_BUSCA 100

static Funcionario resultados_busca[MAX_RESULTADOS_BUSCA];
static int qtd_resultados_busca = 0;

// Função auxiliar para imprimir um funcionário resumido (usado na listagem de homônimos)
void imprimir_funcionario_resumido(void *val) {
    Funcionario *f = (Funcionario*) val;
    printf(" -> %s (Nascimento: %02d/%02d/%04d) - Status: %s\n", 
           f->chave.nome, f->chave.data_nascimento.dia, 
           f->chave.data_nascimento.mes, f->chave.data_nascimento.ano, f->status);
}

// Callback auxiliar para armazenar e imprimir resultados de busca por nome
void registrar_funcionario_busca(void *val) {
    Funcionario *f = (Funcionario*) val;

    if (qtd_resultados_busca < MAX_RESULTADOS_BUSCA) {
        resultados_busca[qtd_resultados_busca] = *f;
    }
    qtd_resultados_busca++;

    imprimir_funcionario_resumido(val);
}

// Função auxiliar para imprimir ficha completa
void imprimir_ficha_completa(Funcionario *f) {
    printf("\n=== FICHA DO FUNCIONARIO ===\n");
    printf("Nome: %s\n", f->chave.nome);
    printf("Data Nasc.: %02d/%02d/%04d\n", f->chave.data_nascimento.dia, f->chave.data_nascimento.mes, f->chave.data_nascimento.ano);
    printf("Filiacao: %s (Mae) e %s (Pai)\n", f->nome_mae, f->nome_pai);
    printf("Contato: %s | Tel: %s\n", f->endereco, f->telefone);
    printf("Contrato: Inicio %s | Status: %s | Desligamento: %s\n", f->data_contratacao, f->status, f->data_desligamento);
    printf("--- Historico de Pagamentos (Ultimos 12 meses) ---\n");
    for(int i = 0; i < 12; i++) {
        if(strlen(f->historico_pagamentos[i].mes_ref) > 0) {
            printf(" [%s] R$ %.2f\n", f->historico_pagamentos[i].mes_ref, f->historico_pagamentos[i].valor);
        }
    }
    printf("============================\n");
}

// Função auxiliar para imprimir dados cadastrais sem histórico de pagamentos
void imprimir_ficha_sem_historico(Funcionario *f) {
    printf("\n=== DADOS CADASTRAIS DO FUNCIONARIO ===\n");
    printf("Nome: %s\n", f->chave.nome);
    printf("Data Nasc.: %02d/%02d/%04d\n", f->chave.data_nascimento.dia, f->chave.data_nascimento.mes, f->chave.data_nascimento.ano);
    printf("Mae: %s\n", f->nome_mae);
    printf("Pai: %s\n", f->nome_pai);
    printf("Endereco: %s\n", f->endereco);
    printf("Telefone: %s\n", f->telefone);
    printf("Data de Contratacao: %s\n", f->data_contratacao);
    printf("Status: %s\n", f->status);
    printf("Data de Desligamento: %s\n", f->data_desligamento);
    printf("=======================================\n");
}

// Callback para imprimir chave na visualização estrutural da B+
void imprimir_chave_bplus(void *key) {
    ChaveRH *k = (ChaveRH*) key;
    // Pega só o primeiro nome
    char primeiro_nome[50];
    sscanf(k->nome, "%49s", primeiro_nome);
    printf("(%s, %02d/%02d/%04d)", primeiro_nome, k->data_nascimento.dia, k->data_nascimento.mes, k->data_nascimento.ano);
}

void limpar_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int telefone_valido(const char *telefone) {
    if (strlen(telefone) != 11) {
        return 0;
    }

    for (int i = 0; i < 11; i++) {
        if (!isdigit((unsigned char)telefone[i])) {
            return 0;
        }
    }

    return 1;
}

void formatar_telefone(const char *telefone_digitado, char *telefone_formatado) {
    sprintf(telefone_formatado, "(%c%c)%c%c%c%c%c-%c%c%c%c",
            telefone_digitado[0], telefone_digitado[1],
            telefone_digitado[2], telefone_digitado[3], telefone_digitado[4],
            telefone_digitado[5], telefone_digitado[6],
            telefone_digitado[7], telefone_digitado[8],
            telefone_digitado[9], telefone_digitado[10]);
}

void ler_campo_texto(const char *rotulo, char *destino, size_t tamanho) {
    printf("%s", rotulo);
    fgets(destino, tamanho, stdin);
    size_t pos_quebra = strcspn(destino, "\n");

    if (destino[pos_quebra] == '\n') {
        destino[pos_quebra] = 0;
    } else {
        limpar_buffer();
    }
}

void ler_telefone_formatado(char *destino, size_t tamanho) {
    int telefone_ok = 0;

    do {
        char telefone_digitado[100];
        printf("Telefone (DDD + 9 digitos, somente numeros): ");
        fgets(telefone_digitado, sizeof(telefone_digitado), stdin);
        telefone_digitado[strcspn(telefone_digitado, "\n")] = 0;

        if (telefone_valido(telefone_digitado)) {
            snprintf(destino, tamanho, "(%c%c)%c%c%c%c%c-%c%c%c%c",
                     telefone_digitado[0], telefone_digitado[1],
                     telefone_digitado[2], telefone_digitado[3], telefone_digitado[4],
                     telefone_digitado[5], telefone_digitado[6],
                     telefone_digitado[7], telefone_digitado[8],
                     telefone_digitado[9], telefone_digitado[10]);
            telefone_ok = 1;
        } else {
            printf("Telefone invalido. Digite exatamente 11 numeros, exemplo: 11999999999.\n");
        }
    } while (!telefone_ok);
}

void ler_dados_cadastrais(Funcionario *funcionario) {
    ler_campo_texto("Nome da Mae: ", funcionario->nome_mae, sizeof(funcionario->nome_mae));
    ler_campo_texto("Nome do Pai: ", funcionario->nome_pai, sizeof(funcionario->nome_pai));
    ler_campo_texto("Endereco: ", funcionario->endereco, sizeof(funcionario->endereco));
    ler_telefone_formatado(funcionario->telefone, sizeof(funcionario->telefone));
    ler_campo_texto("Data de Contratacao (DD/MM/AAAA): ", funcionario->data_contratacao, sizeof(funcionario->data_contratacao));
    ler_campo_texto("Status (Ativo/Inativo): ", funcionario->status, sizeof(funcionario->status));

    if (strcmp(funcionario->status, "Inativo") == 0 || strcmp(funcionario->status, "inativo") == 0) {
        ler_campo_texto("Data de Desligamento (DD/MM/AAAA): ", funcionario->data_desligamento, sizeof(funcionario->data_desligamento));
    } else {
        strcpy(funcionario->data_desligamento, "N/A");
    }
}

int main() {
    // Cria ou abre a Árvore B+ em disco (Arquivo "rh_dados.bin", Ordem 5)
    BPlusTree *arvore = bplus_create("rh_dados.bin", 5, 
                                     compara_chaves_rh, 
                                     tamanho_chave_rh, tamanho_valor_rh, 
                                     escreve_chave_rh, le_chave_rh, 
                                     escreve_valor_rh, le_valor_rh);

    if (!arvore) {
        printf("Erro fatal ao inicializar a Arvore B+ em disco.\n");
        return 1;
    }

    int opcao;
    do {
        printf("\n========================================\n");
        printf("|   SISTEMA DE GESTAO DE RH - B+ TREE  |\n");
        printf("========================================\n");
        printf("| 1. Inserir Funcionario               |\n");
        printf("| 2. Buscar Funcionario                |\n");
        printf("| 3. Excluir Funcionario               |\n");
        printf("| 4. Listagem por Intervalo            |\n");
        printf("| 5. Exibir Estrutura do Indice        |\n");
        printf("| 6. Sair                              |\n");
        printf("========================================\n");
        printf("Opcao: ");
        if (scanf("%d", &opcao) != 1) { limpar_buffer(); opcao = 0; }
        limpar_buffer();

        switch (opcao) {
            case 1: { // INSERIR
                Funcionario novo;
                memset(&novo, 0, sizeof(Funcionario));
                
                printf("\n--- Novo Cadastro ---\n");
                printf("Nome completo: ");
                fgets(novo.chave.nome, sizeof(novo.chave.nome), stdin);
                novo.chave.nome[strcspn(novo.chave.nome, "\n")] = 0;
                
                printf("Data de Nascimento (DD MM AAAA): ");
                scanf("%d %d %d", &novo.chave.data_nascimento.dia, &novo.chave.data_nascimento.mes, &novo.chave.data_nascimento.ano);
                limpar_buffer();

                // Verifica se já existe
                Funcionario *existente = (Funcionario*) bplus_search(arvore, &novo.chave);
                if (existente) {
                    printf("\n[AVISO] Funcionario ja cadastrado!\n");
                    imprimir_ficha_completa(existente);
                    printf("\nDeseja realizar atualizacao de dados? (1-Sim / 0-Nao): ");
                    int update;
                    scanf("%d", &update);
                    limpar_buffer();
                    if (update == 1) {
                        novo = *existente;
                        // Na B+, remover e inserir novamente evita sobrescrita parcial de registro.
                        bplus_remove(arvore, &novo.chave);
                        printf("\n--- Atualizacao dos Dados Cadastrais ---\n");
                        ler_dados_cadastrais(&novo);
                        bplus_insert(arvore, &novo.chave, &novo);
                        printf("Atualizacao concluida.\n");
                    }
                    free(existente);
                    break;
                }

                ler_dados_cadastrais(&novo);
                
                // Histórico inicia vazio graças ao memset inicial

                if (bplus_insert(arvore, &novo.chave, &novo)) {
                    printf("\nFuncionario cadastrado com sucesso no disco!\n");
                } else {
                    printf("\nErro ao gravar no disco.\n");
                }
                break;
            }
            case 2: // BUSCAR
            case 3: { // EXCLUIR
                char nome_busca[100];
                printf("\n--- Pesquisa ---\n");
                printf("Nome do funcionario: ");
                fgets(nome_busca, sizeof(nome_busca), stdin);
                nome_busca[strcspn(nome_busca, "\n")] = 0;

                // Cria intervalo falso para achar todos os homônimos
                ChaveRH chave_min = { .data_nascimento = {0, 0, 0} };
                strcpy(chave_min.nome, nome_busca);
                ChaveRH chave_max = { .data_nascimento = {31, 12, 9999} };
                strcpy(chave_max.nome, nome_busca);

                printf("\nProcurando registros...\n");
                qtd_resultados_busca = 0;
                bplus_range_search(arvore, &chave_min, &chave_max, registrar_funcionario_busca);

                if (qtd_resultados_busca == 0) {
                    printf("Nenhum registro correspondente encontrado.\n");
                    break;
                }

                ChaveRH chave_exata;
                if (qtd_resultados_busca == 1) {
                    chave_exata = resultados_busca[0].chave;
                } else {
                    printf("\nForam encontrados homonimos. Digite a data de nascimento exata do funcionario (DD MM AAAA): ");
                    strcpy(chave_exata.nome, nome_busca);
                    scanf("%d %d %d", &chave_exata.data_nascimento.dia, &chave_exata.data_nascimento.mes, &chave_exata.data_nascimento.ano);
                    limpar_buffer();
                }

                Funcionario *encontrado = (Funcionario*) bplus_search(arvore, &chave_exata);
                
                if (!encontrado) {
                    printf("Nenhum registro correspondente encontrado.\n");
                    break;
                }

                if (opcao == 2) {
                    imprimir_ficha_completa(encontrado);
                } else { // opcao == 3 (Excluir)
                    printf("\n--- Confirmacao de Exclusao ---\n");
                    imprimir_ficha_sem_historico(encontrado);
                    
                    printf("\nTem certeza que deseja excluir permanentemente do disco? (1-Sim / 0-Nao): ");
                    int conf;
                    scanf("%d", &conf);
                    limpar_buffer();
                    
                    if (conf == 1) {
                        if(bplus_remove(arvore, &chave_exata)) {
                            printf("Funcionario excluido com sucesso.\n");
                        } else {
                            printf("Erro na exclusao.\n");
                        }
                    } else {
                        printf("Operacao cancelada.\n");
                    }
                }
                free(encontrado);
                break;
            }
            case 4: { // LISTAGEM POR INTERVALO ABERTO
                ChaveRH chaveA = { .data_nascimento = {31, 12, 9999} };
                ChaveRH chaveB = { .data_nascimento = {0, 0, 0} };
                
                printf("\n--- Listagem por Intervalo Alfabético ---\n");
                printf("Nome Inicial (A): ");
                fgets(chaveA.nome, sizeof(chaveA.nome), stdin);
                chaveA.nome[strcspn(chaveA.nome, "\n")] = 0;
                
                printf("Nome Final (B): ");
                fgets(chaveB.nome, sizeof(chaveB.nome), stdin);
                chaveB.nome[strcspn(chaveB.nome, "\n")] = 0;

                printf("\nListando funcionarios no intervalo aberto (%s, %s):\n", chaveA.nome, chaveB.nome);
                bplus_range_search(arvore, &chaveA, &chaveB, imprimir_funcionario_resumido);
                break;
            }
            case 5: { // EXIBIR ESTRUTURA
                bplus_print_structure(arvore, imprimir_chave_bplus);
                break;
            }
            case 6: // SAIR
                printf("\nSincronizando e fechando arquivo em disco...\n");
                bplus_destroy(arvore);
                arvore = NULL;
                printf("Programa encerrado com seguranca.\n");
                break;
            default:
                printf("\nOpcao Invalida.\n");
        }
    } while (opcao != 6);

    return 0;
}
