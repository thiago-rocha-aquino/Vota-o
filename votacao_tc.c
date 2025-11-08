// Arquivo: votacao_tc.c
// Sistema de Votação de Trabalhos de Curso

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// constantes
#define MAX_PESSOAS 50
#define MAX_NOME 60
#define MAX_SIGLA 10

// Estruturas 
typedef struct Pessoa Pessoa;
struct Pessoa {
    char nome[MAX_NOME];
    int idade;
};

typedef struct Professor Professor;
struct Professor {
    Pessoa pes;
    int codigo;
    char depto[MAX_SIGLA];
};

typedef struct Aluno Aluno;
struct Aluno {
    Pessoa pes;
    int matricula;
    int ano;
    char depto[MAX_SIGLA];
};

typedef struct TC TC;
struct TC {
    int codigo;
    int autor;        // código do aluno autor
    int orientador;   // código do professor orientador
    char titulo[MAX_NOME];
    int qtdeVotos;
};

typedef struct Eleitor Eleitor;
struct Eleitor {
    char cpf[15];
    bool votou;
    int codigoTC;     // código do TC votado
};

// Vetores globais
Professor docentes[MAX_PESSOAS];
int qtdeDocentes;

Aluno formandos[MAX_PESSOAS];
int qtdeFormandos;

TC listaTCs[MAX_PESSOAS];
int qtdeTCs;

Eleitor comissao[MAX_PESSOAS];
int qtdeEleitores;

// Protótipos das funções
void limparBuffer();
bool verificarArquivo(char *nomeArq);
void lerProfessores();
void lerAlunos();
void lerTCs();
void lerComissao();
bool validarCPF(char *cpf);
void lerVotacaoParcial();
void salvarVotacaoParcial();
void mostrarResultado();
int buscarProfessor(int codigo);
int buscarAluno(int matricula);
int buscarTC(int codigo);
int buscarEleitor(char *cpf);
void menu1();
void menu2();
void entrarComVoto();
void iniciarNovaVotacao();
void continuarVotacao();

// limpar o buffer do teclado (fica mais bonitinho)
void limparBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

//verificar se arquivo existe
bool verificarArquivo(char *nomeArq) {
    FILE *arq = fopen(nomeArq, "r");
    if (arq == NULL) {
        printf("Erro: arquivo %s nao existe!\n", nomeArq);
        return false;
    }
    fclose(arq);
    return true;
}

// ler arquivo 
void lerProfessores() {
    FILE *arq = fopen("professor.txt", "r");
    if (arq == NULL) {
        printf("Erro: arquivo professor.txt nao existe!\n");
        exit(1);
    }
    
    fscanf(arq, "%d", &qtdeDocentes);
    
    for (int i = 0; i < qtdeDocentes; i++) {
        fscanf(arq, "%d %s %d", 
               &docentes[i].codigo, 
               docentes[i].depto, 
               &docentes[i].pes.idade);
        
        // Ler o nome completo (pode conter espaços)
        fgetc(arq); // consome o espaço após a idade
        fgets(docentes[i].pes.nome, MAX_NOME, arq);
        // Remove o \n do final
        docentes[i].pes.nome[strcspn(docentes[i].pes.nome, "\n")] = '\0';
    }
    
    fclose(arq);
}

//  ler arquivo 
void lerAlunos() {
    FILE *arq = fopen("aluno.txt", "r");
    if (arq == NULL) {
        printf("Erro: arquivo aluno.txt nao existe!\n");
        exit(1);
    }
    
    fscanf(arq, "%d", &qtdeFormandos);
    
    for (int i = 0; i < qtdeFormandos; i++) {
        fscanf(arq, "%d %d %s %d", 
               &formandos[i].matricula,
               &formandos[i].ano,
               formandos[i].depto, 
               &formandos[i].pes.idade);
        
        
        fgetc(arq); 
        fgets(formandos[i].pes.nome, MAX_NOME, arq);

        formandos[i].pes.nome[strcspn(formandos[i].pes.nome, "\n")] = '\0';
    }
    
    fclose(arq);
}

//buscar professor por código
int buscarProfessor(int codigo) {
    for (int i = 0; i < qtdeDocentes; i++) {
        if (docentes[i].codigo == codigo) {
            return i;
        }
    }
    return -1;
}

//buscar aluno por matrícula
int buscarAluno(int matricula) {
    for (int i = 0; i < qtdeFormandos; i++) {
        if (formandos[i].matricula == matricula) {
            return i;
        }
    }
    return -1;
}

// ler arquivos 
void lerTCs() {
    char *departamentos[] = {"BCC", "BSI", "ADS", "GTI", "BEC"};
    int numDeptos = 5;
    qtdeTCs = 0;
    
    for (int d = 0; d < numDeptos; d++) {
        char nomeArq[20];
        sprintf(nomeArq, "TC_%s.txt", departamentos[d]);
        
        FILE *arq = fopen(nomeArq, "r");
        if (arq == NULL) {
            printf("Erro: arquivo %s nao existe!\n", nomeArq);
            exit(1);
        }
        
        int qtdeTCsDepto;
        fscanf(arq, "%d", &qtdeTCsDepto);
        
        for (int i = 0; i < qtdeTCsDepto; i++) {
            TC tc;
            fscanf(arq, "%d %d %d", 
                   &tc.codigo,
                   &tc.autor,
                   &tc.orientador);
            
            // Ler o título (pode conter espaços)
            fgetc(arq); 
            fgets(tc.titulo, MAX_NOME, arq);
            tc.titulo[strcspn(tc.titulo, "\n")] = '\0';
            
            // Verificar se existe
            if (buscarAluno(tc.autor) == -1) {
                printf("Erro: aluno com codigo %d nao existe!\n", tc.autor);
                exit(1);
            }
            
            // Verificar se existe
            if (buscarProfessor(tc.orientador) == -1) {
                printf("Erro: professor com codigo %d nao existe!\n", tc.orientador);
                exit(1);
            }
            
            tc.qtdeVotos = 0;
            listaTCs[qtdeTCs++] = tc;
        }
        
        fclose(arq);
    }
}


bool validarCPF(char *cpf) {
    // Verificar formato xxx.xxx.xxx-yy
    if (strlen(cpf) != 14) return false;
    if (cpf[3] != '.' || cpf[7] != '.' || cpf[11] != '-') return false;
    
    // Verificar se são dígitos
    for (int i = 0; i < 14; i++) {
        if (i == 3 || i == 7 || i == 11) continue;
        if (!isdigit(cpf[i])) return false;
    }
    
    
    // apenas validar o formato
    return true;
}

// ler arquivo 
void lerComissao() {
    FILE *arq = fopen("comissao.txt", "r");
    if (arq == NULL) {
        printf("Erro: arquivo comissao.txt nao existe!\n");
        exit(1);
    }
    
    fscanf(arq, "%d", &qtdeEleitores);
    
    for (int i = 0; i < qtdeEleitores; i++) {
        fscanf(arq, "%s", comissao[i].cpf);
        
        if (!validarCPF(comissao[i].cpf)) {
            printf("Erro: CPF %s invalido!\n", comissao[i].cpf);
            fclose(arq);
            exit(1);
        }
        
        comissao[i].votou = false;
        comissao[i].codigoTC = 0;
    }
    
    fclose(arq);
}

// buscar TC por código
int buscarTC(int codigo) {
    for (int i = 0; i < qtdeTCs; i++) {
        if (listaTCs[i].codigo == codigo) {
            return i;
        }
    }
    return -1;
}

//buscar eleitor por CPF
int buscarEleitor(char *cpf) {
    for (int i = 0; i < qtdeEleitores; i++) {
        if (strcmp(comissao[i].cpf, cpf) == 0) {
            return i;
        }
    }
    return -1;
}

// para entrar com voto
void entrarComVoto() {
    char cpf[15];
    int codigoTC;
    
    printf("\nDigite o CPF do eleitor: ");
    scanf("%s", cpf);
    
    if (!validarCPF(cpf)) {
        printf("CPF invalido!\n");
        return;
    }
    
    int indiceEleitor = buscarEleitor(cpf);
    if (indiceEleitor == -1) {
        printf("CPF nao pertence a comissao!\n");
        return;
    }
    
    if (comissao[indiceEleitor].votou) {
        printf("Este eleitor ja votou!\n");
        return;
    }
    
    printf("Digite o codigo do TC: ");
    scanf("%d", &codigoTC);
    
    int indiceTC = buscarTC(codigoTC);
    if (indiceTC == -1) {
        printf("Codigo do TC invalido!\n");
        return;
    }
    
    // Registrar voto
    comissao[indiceEleitor].votou = true;
    comissao[indiceEleitor].codigoTC = codigoTC;
    listaTCs[indiceTC].qtdeVotos++;
    
    printf("Voto registrado com sucesso!\n");
}


void salvarVotacaoParcial() {
    FILE *arq = fopen("parcial.txt", "w");
    if (arq == NULL) {
        printf("Erro ao criar arquivo parcial.txt!\n");
        return;
    }
    
    // Contar quantos já votaram
    int qtdeVotaram = 0;
    for (int i = 0; i < qtdeEleitores; i++) {
        if (comissao[i].votou) qtdeVotaram++;
    }
    
    fprintf(arq, "%d\n", qtdeVotaram);
    
    for (int i = 0; i < qtdeEleitores; i++) {
        if (comissao[i].votou) {
            fprintf(arq, "%s %d\n", comissao[i].cpf, comissao[i].codigoTC);
        }
    }
    
    fclose(arq);
    printf("Votacao suspensa e salva em parcial.txt\n");
}

// ler votação parcial
void lerVotacaoParcial() {
    FILE *arq = fopen("parcial.txt", "r");
    if (arq == NULL) {
        printf("Arquivo parcial.txt nao existe!\n");
        return;
    }
    
    int qtdeVotos;
    fscanf(arq, "%d", &qtdeVotos);
    
    for (int i = 0; i < qtdeVotos; i++) {
        char cpf[15];
        int codigoTC;
        fscanf(arq, "%s %d", cpf, &codigoTC);
        
        int indiceEleitor = buscarEleitor(cpf);
        if (indiceEleitor != -1) {
            comissao[indiceEleitor].votou = true;
            comissao[indiceEleitor].codigoTC = codigoTC;
            
            int indiceTC = buscarTC(codigoTC);
            if (indiceTC != -1) {
                listaTCs[indiceTC].qtdeVotos++;
            }
        }
    }
    
    fclose(arq);
    printf("Votacao parcial carregada com sucesso!\n");
}

// mostrar resultado final
void mostrarResultado() {
    FILE *arq = fopen("resultado.txt", "w");
    if (arq == NULL) {
        printf("Erro ao criar arquivo resultado.txt!\n");
        return;
    }
    
    // Encontrar o(s) vencedor(es)
    int maxVotos = 0;
    for (int i = 0; i < qtdeTCs; i++) {
        if (listaTCs[i].qtdeVotos > maxVotos) {
            maxVotos = listaTCs[i].qtdeVotos;
        }
    }
    
    // Mostrar vencedor(es)
    fprintf(arq, "TC vencedor\n");
    for (int i = 0; i < qtdeTCs; i++) {
        if (listaTCs[i].qtdeVotos == maxVotos) {
            int indiceAluno = buscarAluno(listaTCs[i].autor);
            int indiceProf = buscarProfessor(listaTCs[i].orientador);
            
            fprintf(arq, "Codigo: %d\n", listaTCs[i].codigo);
            fprintf(arq, "Titulo: %s\n", listaTCs[i].titulo);
            fprintf(arq, "Aluno: %s\n", formandos[indiceAluno].pes.nome);
            fprintf(arq, "Depto aluno: %s\n", formandos[indiceAluno].depto);
            fprintf(arq, "Orientador: %s\n", docentes[indiceProf].pes.nome);
            fprintf(arq, "Depto orientador: %s\n\n", docentes[indiceProf].depto);
        }
    }
    
    // Eleitores que votaram
    fprintf(arq, "Eleitores que votaram\n");
    for (int i = 0; i < qtdeEleitores; i++) {
        if (comissao[i].votou) {
            fprintf(arq, "%s %d\n", comissao[i].cpf, comissao[i].codigoTC);
        }
    }
    
    // Eleitores que não votaram
    fprintf(arq, "\nEleitores que nao votaram\n");
    for (int i = 0; i < qtdeEleitores; i++) {
        if (!comissao[i].votou) {
            fprintf(arq, "%s\n", comissao[i].cpf);
        }
    }
    
    fclose(arq);
    printf("Resultado salvo em resultado.txt\n");
}


void menu2() {
    char opcao;
    
    while (1) {
        printf("\nMENU2:\n");
        printf("a) Entrar com voto\n");
        printf("b) Suspender votacao\n");
        printf("c) Concluir votacao\n");
        printf("Opcao: ");
        scanf(" %c", &opcao);
        
        if (opcao == 'a' || opcao == 'A') {
            entrarComVoto();
        }
        else if (opcao == 'b' || opcao == 'B') {
            salvarVotacaoParcial();
            exit(0);
        }
        else if (opcao == 'c' || opcao == 'C') {
            mostrarResultado();
            exit(0);
        }
        else {
            printf("Opcao invalida! Digite 'a', 'b' ou 'c'\n");
        }
    }
}

// Iniciar nova votação
void iniciarNovaVotacao() {
    // Zerar todos os votos
    for (int i = 0; i < qtdeEleitores; i++) {
        comissao[i].votou = false;
        comissao[i].codigoTC = 0;
    }
    for (int i = 0; i < qtdeTCs; i++) {
        listaTCs[i].qtdeVotos = 0;
    }
    
    menu2();
}

// Continuar votação
void continuarVotacao() {
    FILE *arq = fopen("parcial.txt", "r");
    if (arq == NULL) {
        printf("Arquivo parcial.txt nao existe!\n");
        return;
    }
    fclose(arq);
    
    lerVotacaoParcial();
    menu2();
}


void menu1() {
    char opcao;
    
    while (1) {
        printf("\nMENU1:\n");
        printf("a) Iniciar nova votacao\n");
        printf("b) Continuar votacao gravada\n");
        printf("Opcao: ");
        scanf(" %c", &opcao);
        
        if (opcao == 'a' || opcao == 'A') {
            iniciarNovaVotacao();
            break;
        }
        else if (opcao == 'b' || opcao == 'B') {
            continuarVotacao();
            break;
        }
        else {
            printf("Opcao invalida! Digite 'a' ou 'b'\n");
        }
    }
}


int main() {
    printf("Sistema de Votacao de Trabalhos de Curso\n");
    printf("=========================================\n\n");
    
    printf("Carregando dados dos arquivos...\n");
    
   
    lerProfessores();
    printf("Professores carregados: %d\n", qtdeDocentes);
    
    lerAlunos();
    printf("Alunos carregados: %d\n", qtdeFormandos);
    
    lerTCs();
    printf("TCs carregados: %d\n", qtdeTCs);
    
    lerComissao();
    printf("Eleitores carregados: %d\n", qtdeEleitores);
    
    printf("\nDados carregados com sucesso!\n");
    
    
    menu1();
    
    return 0;
}