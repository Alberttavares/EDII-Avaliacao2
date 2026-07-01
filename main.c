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
#include "Bplus.h"
#include "RH.h"

// Função auxiliar para imprimir um funcionário resumido (usado na listagem de homônimos)
void imprimir_funcionario_resumido(void *val) {
    Funcionario *f = (Funcionario*) val;
    printf(" -> %s (Nascimento: %02d/%02d/%04d) - Status: %s\n", 
           f->chave.nome, f->chave.data_nascimento.dia, 
           f->chave.data_nascimento.mes, f->chave.data_nascimento.ano, f->status);
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

// Função para gerar 50 funcionários e forçar os 3 níveis da árvore
void inserir_carga_massa(BPlusTree *arvore) {
    printf("\nIniciando insercao massiva de 50 funcionarios...\n");
    
    for (int i = 1; i <= 50; i++) {
        Funcionario novo;
        memset(&novo, 0, sizeof(Funcionario));
        
        // Gera nomes padronizados, ex: "Funcionario 01", "Funcionario 02"
        // Para a árvore B+ espalhar melhor as chaves, vamos misturar um pouco os nomes
        sprintf(novo.chave.nome, "Teste Funcionario %02d", i);
        
        // Datas padronizadas
        novo.chave.data_nascimento.dia = (i % 28) + 1; // Dias de 1 a 28
        novo.chave.data_nascimento.mes = (i % 12) + 1; // Meses de 1 a 12
        novo.chave.data_nascimento.ano = 1970 + i;
        
        // Dados genéricos
        strcpy(novo.nome_mae, "Mae Generica");
        strcpy(novo.nome_pai, "Pai Generico");
        strcpy(novo.endereco, "Rua do Teste, 123");
        strcpy(novo.telefone, "99999-0000");
        strcpy(novo.data_contratacao, "01/01/2026");
        strcpy(novo.status, "Ativo");
        strcpy(novo.data_desligamento, "N/A");
        
        // Insere na árvore
        if (bplus_insert(arvore, &novo.chave, &novo)) {
            printf(" Inserido: %s\n", novo.chave.nome);
        } else {
            printf(" Falha ao inserir: %s (Duplicado ou erro)\n", novo.chave.nome);
        }
    }
    printf("\nCarga massiva finalizada! Use a opcao 5 para ver a estrutura da arvore.\n");
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
        printf("| 7. [TESTE] Carga de 50 Registros     |\n");
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
                        // Na B+, remover e inserir novamente é a forma mais segura de atualizar sem corromper tamanho genérico
                        bplus_remove(arvore, &existente->chave);
                        // Copia dados base para preencher o resto
                        novo = *existente; 
                        printf("Novo Status (Ativo/Inativo): ");
                        fgets(novo.status, sizeof(novo.status), stdin);
                        novo.status[strcspn(novo.status, "\n")] = 0;
                        bplus_insert(arvore, &novo.chave, &novo);
                        printf("Atualizacao concluida.\n");
                    }
                    free(existente);
                    break;
                }

                printf("Nome da Mae: ");
                fgets(novo.nome_mae, sizeof(novo.nome_mae), stdin);
                novo.nome_mae[strcspn(novo.nome_mae, "\n")] = 0;
                
                printf("Nome do Pai: ");
                fgets(novo.nome_pai, sizeof(novo.nome_pai), stdin);
                novo.nome_pai[strcspn(novo.nome_pai, "\n")] = 0;
                
                printf("Endereco: ");
                fgets(novo.endereco, sizeof(novo.endereco), stdin);
                novo.endereco[strcspn(novo.endereco, "\n")] = 0;
                
                printf("Telefone: ");
                fgets(novo.telefone, sizeof(novo.telefone), stdin);
                novo.telefone[strcspn(novo.telefone, "\n")] = 0;
                
                printf("Data de Contratacao (DD/MM/AAAA): ");
                fgets(novo.data_contratacao, sizeof(novo.data_contratacao), stdin);
                novo.data_contratacao[strcspn(novo.data_contratacao, "\n")] = 0;
                
                strcpy(novo.status, "Ativo");
                strcpy(novo.data_desligamento, "N/A");
                
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
                bplus_range_search(arvore, &chave_min, &chave_max, imprimir_funcionario_resumido);

                printf("\nPara confirmar, digite a data de nascimento exata do funcionario (DD MM AAAA): ");
                ChaveRH chave_exata;
                strcpy(chave_exata.nome, nome_busca);
                scanf("%d %d %d", &chave_exata.data_nascimento.dia, &chave_exata.data_nascimento.mes, &chave_exata.data_nascimento.ano);
                limpar_buffer();

                Funcionario *encontrado = (Funcionario*) bplus_search(arvore, &chave_exata);
                
                if (!encontrado) {
                    printf("Nenhum registro correspondente encontrado.\n");
                    break;
                }

                if (opcao == 2) {
                    imprimir_ficha_completa(encontrado);
                } else { // opcao == 3 (Excluir)
                    printf("\n--- Confirmacao de Exclusao ---\n");
                    printf("Nome: %s\nData Nasc: %02d/%02d/%04d\nStatus: %s\n", 
                           encontrado->chave.nome, encontrado->chave.data_nascimento.dia, 
                           encontrado->chave.data_nascimento.mes, encontrado->chave.data_nascimento.ano, encontrado->status);
                    
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
                printf("Programa encerrado com seguranca.\n");
                break;
            case 7: { // CARGA MASSIVA
                inserir_carga_massa(arvore);
                break;
            }
                
            default:
                printf("\nOpcao Invalida.\n");
        }
    } while (opcao != 7);

    return 0;
}