# Relatorio de Testes

Data: 2026-07-03

## Objetivo

Validar a padronizacao dos nomes das funcoes para portugues e garantir que o sistema de RH com Arvore B+ continua compilando e executando os principais fluxos funcionais.

## Ambiente

- Sistema: Linux
- Compilador: `gcc`
- Comando de compilacao: `make -B`
- Diretorios temporarios usados para testes funcionais: `/tmp/edii-avaliacao2-*`

## Testes executados

| Teste | Comando/Fluxo | Resultado |
| --- | --- | --- |
| Compilacao completa | `make -B` | Aprovado. O projeto compilou com `-Wall -Wextra -g` sem erros. |
| Busca por nomes antigos da API | `rg` para `bplus_create`, `bplus_insert`, `bplus_search`, `bplus_remove`, `bplus_range_search`, `bplus_print_structure`, `bplus_destroy`, `CompareFunc`, `SizeFunc`, `WriteFunc`, `ReadFunc` e `limpar_buffer` | Aprovado. Nenhuma ocorrencia encontrada em arquivos `.c` e `.h`. |
| Fluxo integrado | Inserir 3 funcionarios, incluindo homonimos; buscar homonimo por data; listar intervalo alfabetico; excluir funcionario; buscar funcionario excluido; imprimir estrutura | Aprovado. Cadastros gravados, busca retornou os homonimos corretos, intervalo listou os registros esperados, exclusao removeu o registro e a busca posterior nao encontrou o funcionario excluido. |
| Validacao de telefone | Cadastro com telefone invalido `123`, seguido de telefone valido `11999999999` | Aprovado. O sistema rejeitou o telefone invalido e aceitou o telefone correto antes de gravar o cadastro. |
| Atualizacao de cadastro duplicado | Inserir funcionario, tentar inserir a mesma chave novamente, aceitar atualizacao e buscar o registro | Aprovado. O sistema identificou duplicidade, atualizou os dados cadastrais e a busca mostrou os dados novos. |
| Split da Arvore B+ | Inserir 6 funcionarios em arvore de ordem 5 e imprimir estrutura | Aprovado. A estrutura passou a ter raiz interna e duas folhas, confirmando divisao de no. |

## Resultado geral

Todos os testes executados foram aprovados. A padronizacao dos nomes das funcoes nao quebrou a compilacao nem os fluxos principais do programa.
