# 1 Objetivo do Trabalho

O objetivo deste trabalho prático é consolidar os conceitos de estruturas de dados em me-
mória secundária por meio da implementação de uma Árvore B+ em disco, utilizando a
linguagem C. A estrutura desenvolvida deverá ser estritamente genérica, permitindo o ar-
mazenamento de registros e chaves de busca de quaisquer tipos de dados, sem a necessidade
de modificar o código interno da árvore.

Como prova de conceito e aplicação da estrutura, os alunos deverão desenvolver um
**Sistema de Gestão de Recursos Humanos (RH) e Folha de Pagamento**. Este
sistema utilizará a Árvore B+ como motor de indexação em disco através de uma chave de
busca composta (String e Data).

---

# 2 Árvore B+ para Tipos Genéricos de Dados

A implementação da estrutura da árvore B+ deve ser modular, isolada e contida exclusiva-
mente nos seguintes arquivos:

- **Bplus.h:** Protótipos das funções, definições de estruturas básicas e documentação da API.
- **Bplus.c:** Código-fonte com a lógica de manipulação da Árvore B+ em disco (inserção, busca e remoção).

## 2.1 Requisitos de Armazenamento em Disco

### Persistência

Todos os nós da Árvore B+ (internos e folhas) devem ser lidos e grava-
dos diretamente em um arquivo binário em disco. A memória RAM deve ser utilizada
apenas como área de paginação temporária para o nó que está sendo manipulado no
instante da operação.

### Gerenciamento de Espaço

O arquivo de índice deve prever a reutilização de blocos
livres ou o crescimento linear organizado, garantindo a integridade dos “ponteiros” de
disco após inserções e divisões de nós (cisão).

---

# 3 Descrição do Sistema de Aplicação: Gestão de RH

## 3.1 Contextualização e Escopo

O objetivo deste sistema é automatizar o gerenciamento de funcionários e o histórico de
pagamentos de uma empresa. A aplicação deverá consumir a API genérica da Árvore B+
(desenvolvida em Bplus.h), utilizando uma chave de busca composta formada pelo Nome do
Funcionário (critério principal) e pela data de nascimento (critério de desempate). O sistema
não deve permitir o cadastro de homônimos que possuam a mesma data de nascimento.

## 3.2 Estrutura de Dados do Funcionário

Cada registro de funcionário deve conter, obrigatoriamente, os seguintes campos:

- **Nome:** String / Vetor de caracteres.
- **Data de Nascimento:** Tipo estruturado (Dia/Mês/Ano).
- **Filiação:** Nome da mãe e nome do pai.
- **Dados de Contato:** Endereço residencial e telefone.
- **Dados Contratuais:** Data de contratação, status de atividade (Ativo/Inativo) e data de desligamento (se aplicável).
- **Histórico de Pagamentos:** Vetor estático ou estrutura que armazene os dados de pagamento referentes aos últimos 12 meses trabalhados. Cada funcionário recebe um único pagamento mensal, consolidado no primeiro dia útil de cada mês.

## 3.3 Requisitos Funcionais (Menu de Operações)

O programa deve exibir um menu interativo com as seguintes funcionalidades operadas via terminal:

### 1. Inserir Funcionário

O sistema deve solicitar inicialmente o nome e a data de dasci-
mento.

- **Caso a combinação já exista:** Exibe todos os dados atuais do registro e questiona se o usuário deseja realizar uma atualização (update).
- **Caso não exista:** Solicita os demais dados cadastrais (o histórico de pagamentos deve ser inicializado vazio) e realiza a inserção na Árvore B+.

---

### 2. Buscar Funcionário

O sistema deve solicitar o Nome do funcionário para a pesquisa.

- **Se houver homônimos:** O sistema deve listar na tela o nome e a data de nascimento de todos os registros correspondentes e solicitar que o usuário informe a data de nascimento desejada para o desempate.
- **Resultado:** Exibe na tela a ficha cadastral completa do funcionário selecionado, incluindo o histórico de pagamentos.

---

### 3. Excluir Funcionário

O sistema deve solicitar o Nome do funcionário para a pesquisa.

- **Se houver homônimos:** Aplica o mesmo procedimento de desempate por data de nascimento descrito no item anterior.
- **Confirmação:** O sistema exibe os dados cadastrais (omitindo o histórico de paga-
mentos) e solicita uma confirmação do usuário. Se confirmado, o registro é removido
logicamente/fisicamente da Árvore B+.

---

### 4. Listagem por Intervalo (Busca por Intervalo)

O usuário deve fornecer duas strings delimitadoras (Nome A e Nome B). O sistema deve listar todos os funcionários cujos nomes estejam alfabeticamente contidos no intervalo aberto (Nome A, Nome B).

---

### 5. Exibir Estrutura do Índice

Imprime de forma textual e hierárquica a estrutura atual da Árvore B+ em disco. Cada registro indexado deve ser representado pelo seu primeiro nome e data de nascimento, organizados visualmente de modo que o usuário consiga distinguir claramente as divisões de nós e os níveis da árvore.

---

### 6. Sair

Finaliza a execução do programa de forma segura, garantindo o fechamento e a integridade de todos os arquivos em disco. O programa só deve ser encerrado através desta opção.

---

# 4 Estrutura do Projeto

## 4.1 Organização dos Arquivos

O trabalho a ser entregue deve conter, obrigatoriamente, a seguinte estrutura de arquivos:

- **Bplus.h e Bplus.c:** Implementação estritamente genérica da Árvore B+ em disco.

  Não deve conter nenhuma referência direta aos tipos de dados específicos dos subsis-
temas (como structs de funcionários ou produtos).

- **main.c (e arquivos .c/.h auxiliares do sistema):** Implementação do menu do sistema de RH, das estruturas do funcionário e das funções de callback passadas para a árvore.

- **Makefile:** Arquivo responsável por automatizar os comandos de compilação e execu-
ção.

## 4.2 Compilação e Execução

O arquivo Makefile deve ser projetado para ambiente Linux e precisa disponibilizar, no mínimo, os seguintes comandos:

- `make`: Compila o projeto e gera o executável final.
- `make run`: Compila e executa o sistema de RH.
- `make clean`: Remove os arquivos objetos (.o), o arquivo executável e os arquivos de dados binários gerados em disco.

## 4.3 Requisitos Mínimos do Relatório

O relatório deve ser entregue em formato PDF, em estilo acadêmico (normas ABNT), ali-
nhamento do texto justificado e conter as seguintes seções:

1. **Identificação:** Grupo, nome completo, matrícula e turma (de cada membro do grupo).

2. **Introdução:** Breve descrição do trabalho realizado (1 a 2 parágrafos).

3. **Árvore B+ Genérica em Disco**

   - **Arquitetura do Nó:** Apresente a(s) struct(s) utilizada(s) para representar os nós internos e folhas e como elas são armazenadas em disco.
   - **Mecanismo para tratar tipo genérico de dado:** Explique detalhadamente como a árvore manipula dados genéricos. Descreva a assinatura das funções de callback utilizadas para delegação de responsabilidades (comparação, cálculo de tamanho em bytes e escrita/leitura em disco).
   - **Operações de Disco:** Enumere e explique as funções de inserção, busca por igualdade, busca por intervalo e a lógica de cisalhamento. Para cada função, detalhe como é feito o controle de leitura e escrita física (uso de fseek, fread, fwrite), o tratamento dos “ponteiros” de disco e o reaproveitamento de espaços livres (se houver).

4. **Implementação do Sistema de RH**

   - Apresente as structs de dados criadas para o funcionário e para a chave composta.
   - Explique detalhadamente a lógica implementada na função de comparação para re-
alizar o desempate por data de nascimento em caso de homônimos.
   - Explique como o fluxo de busca interage com o usuário quando múltiplos registros correspondentes são encontrados.

5. **Resultados e Demonstração de Persistência**

Para comprovar que a árvore está operando em disco e mantendo a integridade dos dados, apresente os seguintes cenários de teste para ambos os sistemas:

### Cenário de Carga e Persistência

- Demonstre a inserção de uma massa de dados (mínimo de 50 registros) que force a ocorrência de pelo menos 3 níveis na árvore (evidenciado por capturas de tela do comando de impressão da estrutura).
- Encerre o programa através da opção “Sair”, reinicie-o e realize uma busca por um funcionário cadastrado na etapa anterior, provando que a árvore foi corretamente carregada a partir do arquivo em disco.

### Cenário de Exceção no Cadastro (Homônimos)

- Demonstre a tentativa de cadastrar dois funcionários com o mesmo nome e a mesma data de nascimento, comprovando que o sistema rejeita a duplicidade ou abre o fluxo de atualização.
- Demonstre o cadastro de dois funcionários com o mesmo nome, mas com datas de nascimento diferentes, comprovando que a chave composta aceita e ordena corretamente os homônimos.

### Cenários de Exclusão e Integridade do Índice

- **Exclusão Simples com Confirmação:** Demonstre a busca e a exclusão de um funcionário que possui nome único no sistema. Apresente a tela de confirmação antes da remoção e o estado do índice após a operação.
- **Exclusão de Homônimo:** Demonstre o fluxo de exclusão quando o nome digitado possui homônimos no sistema. Evidencie o momento em que o sistema lista as opções, solicita a data de nascimento para desempate e remove estritamente o registro selecionado, mantendo o outro homônimo intacto.
- **Validação Pós-Exclusão:** Após realizar as exclusões, utilize a opção de imprimir a estrutura da árvore para demonstrar visualmente que os nós foram reorganizados corretamente em disco (mantendo as propriedades de preenchimento mínimo da Árvore B+) e realize uma busca pelos registros excluídos para comprovar que eles não são mais localizados.

---

# 5 Grupos

O trabalho deverá ser realizado em grupos de até quatro alunos. Os nomes de todos os participantes de cada grupo devem constar no início de cada arquivo entregue (menos no makefile).

---

# 6 Entrega do Trabalho

Todos os arquivos devem ser compactados em um arquivo .zip. O nome do arquivo deve seguir o seguinte formato:

> GrupoX[nome-do-aluno-1][nome-do-aluno-2][nome-do-aluno-3][nome-do-aluno-4].zip.

Onde X é o número do grupo. Os demais campos no nome do arquivo são auto-explicativos.

A entrega deverá ser feita pelo classroom, até dia **03/07/2026, 18:00 h.**

---

# 7 Avaliação

A nota da avaliação (NA1) levará em conta a implementação (IMP), o relatório (REL) e a média das notas das provas sobre o trabalho (P R). O cálculo é dado por:

```text
NA1 = EIMP · EREL ·

      (IMP · 20) + (REL · 30) + (P R · 50)
      ------------------------------------
                  100
```

onde:

- EIMP = 1 se a implementação foi entregue E compilou e EIMP = 0, caso contrário.
- EREL = 1 se o relatório foi entregue atendendo os requisitos mínimos e EREL = 0, caso contrário.

---

# 8 Plágio

> Em caso de identificação de plágio, todos os trabalhos envolvidos receberão a nota ZERO.
