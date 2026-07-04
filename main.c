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

void imprimir_funcionario_intervalo(void *val) {
    qtd_resultados_busca++;
    imprimir_funcionario_resumido(val);
}

// Função auxiliar para imprimir ficha completa
void imprimir_ficha_completa(Funcionario *f) {
    int pagamentos_exibidos = 0;

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
            pagamentos_exibidos++;
        }
    }
    if (pagamentos_exibidos == 0) {
        printf(" Nenhum pagamento registrado.\n");
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
void imprimir_chave_bmais(void *chave) {
    ChaveRH *k = (ChaveRH*) chave;
    // Pega só o primeiro nome
    char primeiro_nome[50];
    sscanf(k->nome, "%49s", primeiro_nome);
    printf("(%s, %02d/%02d/%04d)", primeiro_nome, k->data_nascimento.dia, k->data_nascimento.mes, k->data_nascimento.ano);
}

void limpar_entrada() {
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
        limpar_entrada();
    }
}

void ler_campo_obrigatorio(const char *rotulo, char *destino, size_t tamanho) {
    do {
        ler_campo_texto(rotulo, destino, tamanho);
        if (strlen(destino) == 0) {
            printf("Campo obrigatorio. Digite um valor.\n");
        }
    } while (strlen(destino) == 0);
}

int ler_inteiro(const char *rotulo, int minimo, int maximo) {
    int valor;
    int valor_ok = 0;

    do {
        char entrada[100];
        char extra;

        printf("%s", rotulo);
        fgets(entrada, sizeof(entrada), stdin);

        if (sscanf(entrada, " %d %c", &valor, &extra) == 1 && valor >= minimo && valor <= maximo) {
            valor_ok = 1;
        } else {
            printf("Opcao invalida. Digite um numero entre %d e %d.\n", minimo, maximo);
        }
    } while (!valor_ok);

    return valor;
}

int data_valida(Data data) {
    int dias_mes[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (data.ano < 1 || data.mes < 1 || data.mes > 12 || data.dia < 1) {
        return 0;
    }

    if ((data.ano % 400 == 0) || (data.ano % 4 == 0 && data.ano % 100 != 0)) {
        dias_mes[2] = 29;
    }

    return data.dia <= dias_mes[data.mes];
}

int texto_para_data(const char *entrada, Data *destino) {
    Data data_lida;
    char extra;

    if ((sscanf(entrada, " %d/%d/%d %c", &data_lida.dia, &data_lida.mes, &data_lida.ano, &extra) == 3 ||
         sscanf(entrada, " %d %d %d %c", &data_lida.dia, &data_lida.mes, &data_lida.ano, &extra) == 3) &&
        data_valida(data_lida)) {
        *destino = data_lida;
        return 1;
    }

    return 0;
}

void ler_data(const char *rotulo, Data *destino) {
    int data_ok = 0;

    do {
        char entrada[100];
        Data data_lida;

        printf("%s", rotulo);
        fgets(entrada, sizeof(entrada), stdin);
        entrada[strcspn(entrada, "\n")] = 0;

        if (texto_para_data(entrada, &data_lida)) {
            *destino = data_lida;
            data_ok = 1;
        } else {
            printf("Data invalida. Use DD/MM/AAAA ou DD MM AAAA.\n");
        }
    } while (!data_ok);
}

void ler_data_texto(const char *rotulo, char *destino, size_t tamanho) {
    Data data;
    ler_data(rotulo, &data);
    snprintf(destino, tamanho, "%02d/%02d/%04d", data.dia, data.mes, data.ano);
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

int textos_iguais_sem_maiusculas(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }

    return *a == '\0' && *b == '\0';
}

void ler_status(char *destino, size_t tamanho) {
    int status_ok = 0;

    do {
        char entrada[100];

        ler_campo_texto("Status (Ativo/Inativo): ", entrada, sizeof(entrada));
        if (textos_iguais_sem_maiusculas(entrada, "Ativo")) {
            snprintf(destino, tamanho, "Ativo");
            status_ok = 1;
        } else if (textos_iguais_sem_maiusculas(entrada, "Inativo")) {
            snprintf(destino, tamanho, "Inativo");
            status_ok = 1;
        } else {
            printf("Status invalido. Digite Ativo ou Inativo.\n");
        }
    } while (!status_ok);
}

void ler_dados_cadastrais(Funcionario *funcionario) {
    ler_campo_obrigatorio("Nome da Mae: ", funcionario->nome_mae, sizeof(funcionario->nome_mae));
    ler_campo_obrigatorio("Nome do Pai: ", funcionario->nome_pai, sizeof(funcionario->nome_pai));
    ler_campo_obrigatorio("Endereco: ", funcionario->endereco, sizeof(funcionario->endereco));
    ler_telefone_formatado(funcionario->telefone, sizeof(funcionario->telefone));
    ler_data_texto("Data de Contratacao (DD/MM/AAAA): ", funcionario->data_contratacao, sizeof(funcionario->data_contratacao));
    ler_status(funcionario->status, sizeof(funcionario->status));

    if (strcmp(funcionario->status, "Inativo") == 0) {
        ler_data_texto("Data de Desligamento (DD/MM/AAAA): ", funcionario->data_desligamento, sizeof(funcionario->data_desligamento));
    } else {
        strcpy(funcionario->data_desligamento, "N/A");
    }
}

int main() {
    // Cria ou abre a Árvore B+ em disco (Arquivo "rh_dados.bin", Ordem 5)
    BPlusTree *arvore = criar_bmais("rh_dados.bin", 5, 
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
        opcao = ler_inteiro("Opcao: ", 1, 6);

        switch (opcao) {
            case 1: { // INSERIR
                Funcionario novo;
                memset(&novo, 0, sizeof(Funcionario));
                
                printf("\n--- Novo Cadastro ---\n");
                ler_campo_obrigatorio("Nome completo: ", novo.chave.nome, sizeof(novo.chave.nome));
                
                ler_data("Data de Nascimento (DD/MM/AAAA): ", &novo.chave.data_nascimento);

                // Verifica se já existe
                Funcionario *existente = (Funcionario*) buscar_bmais(arvore, &novo.chave);
                if (existente) {
                    printf("\n[AVISO] Funcionario ja cadastrado!\n");
                    imprimir_ficha_completa(existente);
                    int update = ler_inteiro("\nDeseja realizar atualizacao de dados? (1-Sim / 0-Nao): ", 0, 1);
                    if (update == 1) {
                        novo = *existente;
                        // Na B+, remover e inserir novamente evita sobrescrita parcial de registro.
                        remover_bmais(arvore, &novo.chave);
                        printf("\n--- Atualizacao dos Dados Cadastrais ---\n");
                        ler_dados_cadastrais(&novo);
                        inserir_bmais(arvore, &novo.chave, &novo);
                        printf("Atualizacao concluida.\n");
                    }
                    free(existente);
                    break;
                }

                ler_dados_cadastrais(&novo);
                
                // Histórico inicia vazio graças ao memset inicial

                if (inserir_bmais(arvore, &novo.chave, &novo)) {
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
                ler_campo_obrigatorio("Nome do funcionario: ", nome_busca, sizeof(nome_busca));

                // Cria intervalo falso para achar todos os homônimos
                ChaveRH chave_min = { .data_nascimento = {0, 0, 0} };
                strcpy(chave_min.nome, nome_busca);
                ChaveRH chave_max = { .data_nascimento = {31, 12, 9999} };
                strcpy(chave_max.nome, nome_busca);

                printf("\nProcurando registros...\n");
                qtd_resultados_busca = 0;
                buscar_intervalo_bmais(arvore, &chave_min, &chave_max, registrar_funcionario_busca);

                if (qtd_resultados_busca == 0) {
                    printf("Nenhum registro correspondente encontrado.\n");
                    break;
                }

                ChaveRH chave_exata;
                if (qtd_resultados_busca == 1) {
                    chave_exata = resultados_busca[0].chave;
                } else {
                    strcpy(chave_exata.nome, nome_busca);
                    printf("\nForam encontrados homonimos. ");
                    ler_data("Digite a data de nascimento exata do funcionario (DD/MM/AAAA): ", &chave_exata.data_nascimento);
                }

                Funcionario *encontrado = (Funcionario*) buscar_bmais(arvore, &chave_exata);
                
                if (!encontrado) {
                    printf("Nenhum registro correspondente encontrado.\n");
                    break;
                }

                if (opcao == 2) {
                    imprimir_ficha_completa(encontrado);
                } else { // opcao == 3 (Excluir)
                    printf("\n--- Confirmacao de Exclusao ---\n");
                    imprimir_ficha_sem_historico(encontrado);
                    
                    int conf = ler_inteiro("\nTem certeza que deseja excluir permanentemente do disco? (1-Sim / 0-Nao): ", 0, 1);
                    
                    if (conf == 1) {
                        if(remover_bmais(arvore, &chave_exata)) {
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
                do {
                    ler_campo_obrigatorio("Nome Inicial (A): ", chaveA.nome, sizeof(chaveA.nome));
                    ler_campo_obrigatorio("Nome Final (B): ", chaveB.nome, sizeof(chaveB.nome));

                    if (strcmp(chaveA.nome, chaveB.nome) >= 0) {
                        printf("Intervalo invalido. O nome inicial deve vir antes do nome final.\n");
                    }
                } while (strcmp(chaveA.nome, chaveB.nome) >= 0);

                printf("\nListando funcionarios no intervalo aberto (%s, %s):\n", chaveA.nome, chaveB.nome);
                qtd_resultados_busca = 0;
                buscar_intervalo_bmais(arvore, &chaveA, &chaveB, imprimir_funcionario_intervalo);
                if (qtd_resultados_busca == 0) {
                    printf("Nenhum funcionario encontrado no intervalo informado.\n");
                }
                break;
            }
            case 5: { // EXIBIR ESTRUTURA
                printf("\n--- Estrutura do Indice ---\n");
                imprimir_estrutura_bmais(arvore, imprimir_chave_bmais);
                break;
            }
            case 6: // SAIR
                printf("\nSincronizando e fechando arquivo em disco...\n");
                destruir_bmais(arvore);
                arvore = NULL;
                printf("Programa encerrado com seguranca.\n");
                break;
            default:
                printf("\nOpcao Invalida.\n");
        }
    } while (opcao != 6);

    return 0;
}
