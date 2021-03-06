/*
    Trabalho de Programação Paralela
    Ciencia da Computação - UFRJ

    Prof: Silvana Roseto

    Alunos: Felipe Faria    109062131
            Felipe Sousa    111306795

    Compile: gcc -g -Wall -fopenmp -o T1ProgParFelipe T1ProgParFelipe.c
    Run: ./T1ProgParFelipe <OPCOES>
    OPCOES:
    -P	Executar em Paralelo
    -S	Executar em Sequencial
    -R	Escrever Resultados
    -M	Escrever Matriz
    -C	Escrever Caminho Passo a Passo
    -L	Entrada manual/externa: nCidades Raiz Custos...
    -c #	Definir numero de Cidades (default é 4)
    -t #	Definir numero de threads (default é 2)
    -0 #	Definir cidade de origem (default é 0)
    -s #	Definir semente para gerar matriz (default é aleatorio)
    Execucoes:
    ./T1ProgParFelipe -P -S -R -c 14
    ./T1ProgParFelipe -P -S -T -c 14
    ./T1ProgParFelipe -P -S -T -c 14
    ./T1ProgParFelipe -P -S -R -L <cidades_17.in

*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <unistd.h>
#include <ctype.h>
#include "timer.h"
#define INF 9999
#define true 1
#define false 0
//Typedefs
typedef int bool;

typedef struct Caminho
{
    int *ordem;
	int *visitados;
	int distanciaPercorrida;
	int cidPer;
} Caminho;

//Opcoes
int EXECUTAR_PARALELO = 0;
int EXECUTAR_SEQUENCIAL = 0;
int LER_ENTRADAS=0;
int PRINT_MATRIZ=0;
int PRINT_RESULTADOS=0;
int PRINT_CAMINHO=0;
int PRINT_LINHA = 0;
int nThreads=2;
int nCidades = 4;
int origem = 0;
int semente=-1;

//Funcoes
int** NovaMatriz(int n);
int** GerarMatriz(int n);
void PrintMatriz(int** matriz,int tamanhoMatriz);

Caminho NovoCaminho();
Caminho CopiaCaminho(Caminho caminho);
void PrintCaminho(Caminho caminho);

void visita(Caminho caminho);
void buscaParalela(Caminho caminho);
void visitaParalela(Caminho caminho);

void ConfiguraOpcoes(int argc, char* argv[]);
void PrintOpcoes();
void PrintResultados();
void PrintLinha();

//Global
int **custo;
Caminho melhorCaminhoSequencial, melhorCaminhoParalelo;
double  tempoSequencial=0,tempoParalelo=0;
int countVisitasSequenciais,
    countVisitasParalelas;

int main(int argc, char* argv[])
{
    double startTime, endTime;
	int i, j;
	Caminho inicial;

	ConfiguraOpcoes(argc,argv);

    if(LER_ENTRADAS){
        //determinando as arestas
        custo = NovaMatriz(nCidades);
        for(i = 0; i < nCidades; i++)
            for(j = 0; j < nCidades; j++)
                scanf("%d", &custo[i][j]);
    }else{
        custo = GerarMatriz(nCidades);
    }
    PrintMatriz(custo,nCidades);

	inicial = NovoCaminho();

	melhorCaminhoParalelo= NovoCaminho();
    melhorCaminhoParalelo.distanciaPercorrida = INF;

	melhorCaminhoSequencial = NovoCaminho();
    melhorCaminhoSequencial.distanciaPercorrida = INF;

    inicial.cidPer = 1;
    inicial.ordem[0] = origem;
    inicial.visitados[origem] = 1;

    if(EXECUTAR_SEQUENCIAL){
        GET_TIME(startTime);
        visita(inicial);
        GET_TIME(endTime);
        tempoSequencial = endTime - startTime;
    }

    if(EXECUTAR_PARALELO){
        GET_TIME(startTime);

        # pragma omp parallel num_threads(nThreads)
        visitaParalela(inicial);

        GET_TIME(endTime);
        tempoParalelo = endTime - startTime;
    }

    PrintResultados();
    PrintLinha();

	return 0;
}

void PrintResultados(){
    int i;
    double eficiencia,speedup;
    if(!PRINT_RESULTADOS)
        return;
    if(EXECUTAR_SEQUENCIAL){
        printf("##Sequencial\n");
        printf("Caminho de menor custo:");
        for(i = 0; i <= nCidades; i++) printf(" %d", melhorCaminhoSequencial.ordem[i]);
        printf("\nMenor custo: %d\n", melhorCaminhoSequencial.distanciaPercorrida);
        printf("Tempo: %.6f\n", tempoSequencial);
        printf("nVisitas: %%df\n", countVisitasSequenciais);
    }
    if(EXECUTAR_PARALELO){
        printf("##Paralelo: Threads=%d\n",nThreads);
        printf("Caminho de menor custo:");
        for(i = 0; i <= nCidades; i++) printf(" %d", melhorCaminhoParalelo.ordem[i]);
        printf("\nMenor custo: %d\n", melhorCaminhoParalelo.distanciaPercorrida);
        printf("Tempo: %.6f\n", tempoParalelo);
        printf("nVisitas: %%df\n", countVisitasParalelas);
    }
    if(EXECUTAR_PARALELO&&EXECUTAR_SEQUENCIAL){
        printf("##Avaliação\n");
        speedup = tempoSequencial/tempoParalelo;
        printf("Speedup = %f\n",speedup);
        eficiencia = speedup/nThreads;
        printf("Eficiencia = %f\n",eficiencia);
    }
}

void PrintLinha(char* argv[]){
    if(!PRINT_LINHA) return;
    int i;
    if(EXECUTAR_PARALELO)
    {
        printf("Paralelo\t%d\t%d\t%.6f\t%d\t",nCidades,nThreads,tempoParalelo,melhorCaminhoParalelo.distanciaPercorrida);
        for(i = 0; i <= nCidades; i++)
        printf(" %d", melhorCaminhoParalelo.ordem[i]);
        printf("\n");
    }
    if(EXECUTAR_SEQUENCIAL)
    {
        printf("Sequencial\t%d\t%d\t%.6f\t%d\t",nCidades,nThreads,tempoSequencial,melhorCaminhoSequencial.distanciaPercorrida);
        for(i = 0; i <= nCidades; i++)
        printf(" %d", melhorCaminhoSequencial.ordem[i]);
        printf("\n");
    }


}

void ConfiguraOpcoes(int argc, char* argv[]){
    int opt;
    if(argc==1){
        PrintOpcoes();
           exit(EXIT_FAILURE);
    }
    while ((opt = getopt(argc, argv, "PSRMCLTc:t:o:s:")) != -1) {
       switch (opt) {
       case 'P':
            EXECUTAR_PARALELO = 1;
            break;
       case 'S':
            EXECUTAR_SEQUENCIAL = 1;
            break;
       case 'R':
            PRINT_RESULTADOS=1;
           break;
       case 'M':
            PRINT_MATRIZ=1;
           break;
       case 'C':
            PRINT_CAMINHO=1;
           break;
       case 'L':
            LER_ENTRADAS=1;
           break;
       case 'T':
            PRINT_LINHA=1;
           break;
       case 'c':
            nCidades = strtoul(optarg, NULL, 0);
           break;
       case 't':
            nThreads = strtoul(optarg, NULL, 0);
           break;
       case 'o':
            origem = strtoul(optarg, NULL, 0);
           break;
       case 's':
            semente = strtoul(optarg, NULL, 0);
           break;

       default:
           fprintf(stderr, "OPCOES: %s arg...\n",
                   argv[0]);
                   PrintOpcoes();
           exit(EXIT_FAILURE);
       }
    }
    if(semente<0) semente = time(NULL);
}
void PrintOpcoes(){
    printf("OPCOES:\n");
    printf("-P\tExecutar em Paralelo\n");
    printf("-S\tExecutar em Sequencial\n");
    printf("-R\tEscrever Resultados\n");
    printf("-M\tEscrever Matriz\n");
    printf("-C\tEscrever Caminho Passo a Passo\n");
    printf("-L\tEntrada manual/externa: nCidades Raiz Custos...\n");
    printf("-c #\tDefinir numero de Cidades (default é 4)\n");
    printf("-t #\tDefinir numero de threads (default é 2)\n");
    printf("-0 #\tDefinir cidade de origem (default é 0)\n");
    printf("-s #\tDefinir semente para gerar matriz (default é aleatorio)\n");
}

int** NovaMatriz(int n)
{
    int i;
    int** matriz;
    if ((matriz = malloc(n * sizeof( int* ))) == NULL) printf("Nao alocou!\n");
    for (i = 0; i < n; i++ )
    {
    	if (( matriz[i] = malloc( n*sizeof( int* ) )) == NULL )
        {printf("Nao alocou!\n");}
    }
    return matriz;
}

int** GerarMatriz(int n)
{
    srand(semente);
    int i,j,value;
    int** matriz = NovaMatriz(n);
    for(i=0;i<n;i++)
    {
        for(j=0;j<n;j++){
            if(i==j){ matriz[i][j]=INF; continue;}
            value = rand() % 100;
            if(value>90) { matriz[i][j]=INF; continue;}
            matriz[i][j] = value;
        }
    }
    return matriz;
}

void PrintMatriz(int** matriz,int tamanhoMatriz){
    if(!PRINT_MATRIZ) return;
    printf("a\n");
    int i,j;
    for(i=0;i<tamanhoMatriz;i++){
        for(j=0;j<tamanhoMatriz;j++){
            printf("%d ",matriz[i][j]);
        }
        printf("\n");
    }
}

void visita(Caminho caminho)
{
    countVisitasSequenciais++;
    int anterior = caminho.ordem[caminho.cidPer-1];
    int i, novaDist;

    PrintCaminho(caminho);

    if (caminho.cidPer == nCidades && custo[anterior][origem] < INF)
        if(melhorCaminhoSequencial.distanciaPercorrida > caminho.distanciaPercorrida+custo[anterior][origem])
        {
            melhorCaminhoSequencial = CopiaCaminho(caminho);
            melhorCaminhoSequencial.cidPer++;
            melhorCaminhoSequencial.ordem[melhorCaminhoSequencial.cidPer-1] = origem;
            melhorCaminhoSequencial.visitados[origem]=2;
            melhorCaminhoSequencial.distanciaPercorrida+=custo[anterior][origem];
        }

    for(i = 0; i < nCidades; i++)
    {
        if(caminho.visitados[i] > 0) continue;
        if(custo[anterior][i] >= INF) continue;
        novaDist = custo[anterior][i]+caminho.distanciaPercorrida;
        if(novaDist>melhorCaminhoSequencial.distanciaPercorrida) continue;
        Caminho proximo = CopiaCaminho(caminho);
        proximo.cidPer++;
        proximo.ordem[proximo.cidPer-1]=i;
        proximo.visitados[i]=1;
        proximo.distanciaPercorrida=novaDist;
        visita(proximo);
    }
}

void buscaParalela(Caminho caminho)
{
    int anterior = caminho.ordem[caminho.cidPer-1];
    int i, novaDist,myRank, myStart, myFinish;
    bool isWorst;
    myRank = omp_get_thread_num();

    PrintCaminho(caminho);

    myStart = myRank*(nCidades/nThreads);
    myFinish = (myRank+1)*(nCidades/nThreads);
    if(myRank==(nThreads-1)) myFinish += nCidades%nThreads;

    for(i = myStart; i < myFinish; i++)
    {
        if(caminho.visitados[i] > 0) continue;
        if(custo[anterior][i] >= INF) continue;
        novaDist = custo[anterior][i]+caminho.distanciaPercorrida;

        isWorst=novaDist>melhorCaminhoParalelo.distanciaPercorrida;
        if(isWorst) continue;

        Caminho proximo = CopiaCaminho(caminho);
        proximo.cidPer++;
        proximo.ordem[proximo.cidPer-1]=i;
        proximo.visitados[i]=1;
        proximo.distanciaPercorrida=novaDist;
        visitaParalela(proximo);
    }
}

void visitaParalela(Caminho caminho)
{

    #pragma omp critical
    countVisitasParalelas++;

    int anterior = caminho.ordem[caminho.cidPer-1];
    int i, novaDist;
    bool isWorst;

    PrintCaminho(caminho);

    if (caminho.cidPer == nCidades && custo[anterior][origem] < INF){
        #pragma omp critical
        {
            if(melhorCaminhoParalelo.distanciaPercorrida > caminho.distanciaPercorrida+custo[anterior][origem])
            {
                melhorCaminhoParalelo= CopiaCaminho(caminho);
                melhorCaminhoParalelo.cidPer++;
                melhorCaminhoParalelo.ordem[melhorCaminhoParalelo.cidPer-1] = origem;
                melhorCaminhoParalelo.visitados[origem]=2;
                melhorCaminhoParalelo.distanciaPercorrida+=custo[anterior][origem];

            }
        }
    }

    for(i = 0; i < nCidades; i++)
    {
        if(caminho.visitados[i] > 0) continue;
        if(custo[anterior][i] >= INF) continue;
        novaDist = custo[anterior][i]+caminho.distanciaPercorrida;
        #pragma omp critical
        isWorst=novaDist>melhorCaminhoParalelo.distanciaPercorrida;
        if(isWorst) continue;
        Caminho proximo = CopiaCaminho(caminho);
        proximo.cidPer++;
        proximo.ordem[proximo.cidPer-1]=i;
        proximo.visitados[i]=1;
        proximo.distanciaPercorrida=novaDist;
        visitaParalela(proximo);
    }
}

void PrintCaminho(Caminho caminho)
{
    if(!PRINT_CAMINHO) return;
    int i;

    printf("\nCaminho:\nDistancia: %d\nNumero de Cidades: %d\nOrdem:", caminho.distanciaPercorrida, caminho.cidPer);
    for(i = 0; i < caminho.cidPer; i++) printf(" %d", caminho.ordem[i]);
    printf("\nVisitados:");
    for(i = 0; i < nCidades; i++) printf(" %d", caminho.visitados[i]);
    printf("\n");
}

Caminho NovoCaminho()
{
    int i;

    Caminho caminho;
    caminho.ordem = (int*) malloc((nCidades+1) * sizeof(int));
    caminho.visitados = (int*) malloc(nCidades * sizeof(int));

    for(i = 0; i < nCidades; i++)
    {
        caminho.visitados[i] = 0;
        caminho.ordem[i] = 0;
    }
    caminho.ordem[i+1] = 0;

    caminho.cidPer = 0;
    caminho.distanciaPercorrida = 0;

    return caminho;
}

Caminho CopiaCaminho(Caminho caminho)
{
    int i;
    Caminho caminhoNovo;

    caminhoNovo.ordem = (int*) malloc((nCidades+1) * sizeof(int));
    caminhoNovo.visitados = (int*) malloc(nCidades * sizeof(int));

    caminhoNovo.cidPer = caminho.cidPer;
    caminhoNovo.distanciaPercorrida = caminho.distanciaPercorrida;

    for(i = 0; i < nCidades; i++)
    {
        caminhoNovo.ordem[i] = caminho.ordem[i];
        caminhoNovo.visitados[i] = caminho.visitados[i];
    }

    return caminhoNovo;
}
