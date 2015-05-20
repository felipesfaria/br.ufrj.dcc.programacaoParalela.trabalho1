/*
    Trabalho de Programação Paralela
    Ciencia da Computação - UFRJ

    Prof: Silvana Roseto

    Alunos: Felipe Faria    109062131
            Felipe Sousa    111306795

    Compile: gcc -g -Wall -fopenmp -o trabalho_prog_par trabalho_prog_par.c
    Run: ./trabalho_prog_par.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include "timer.h"
#define lerEntradas 0
#define PRINTMATRIZ 1
#define PRINTRESULTS 1
#define PRINTCAMINHO 0
#define nThreads 2
#define INF 9999
#define true 1
#define false 0

typedef int bool;

typedef struct Caminho
{
    int *ordem;
	int *visitados;
	int distPer;
	int cidPer;
} Caminho;

int** InitMatriz(int n);
int** CreateMatriz(int n);
void PrintMatriz(int** matriz,int tamanhoMatriz);

Caminho InitCaminho();
Caminho CopiaCaminho(Caminho caminho);
void printCaminho(Caminho caminho);

void visita(Caminho caminho);
void buscaParalela(Caminho caminho);
void visitaParalela(Caminho caminho);

int **custo;
int numCid, raiz, seed=0;
Caminho melhorCaminhoSequencial, melhorCaminhoParalelo;

int main(int argc, char* argv[])
{
    double startTime, endTime, tempoSequencial,tempoParalelo;
	int i, j;
	double eficiencia,speedup;
	Caminho inicial;
    if(argc==1){
        //pega quantidade de cidade e cidade de origem
        scanf("%d", &numCid);
        scanf("%d", &raiz);
	}else{
        numCid = strtol(argv[1], NULL, 10);
        raiz = strtol(argv[2], NULL, 10);
        if(argc>2)
            seed = strtol(argv[3], NULL, 10);
        else
            seed = time(NULL);
	}
    if(lerEntradas){
        //determinando as arestas
        custo = InitMatriz(numCid);
        for(i = 0; i < numCid; i++)
            for(j = 0; j < numCid; j++)
                scanf("%d", &custo[i][j]);
    }else{
        custo = CreateMatriz(numCid);
    }
    PrintMatriz(custo,numCid);

	inicial = InitCaminho();

	melhorCaminhoParalelo= InitCaminho();
    melhorCaminhoParalelo.distPer = INF;

	melhorCaminhoSequencial = InitCaminho();
    melhorCaminhoSequencial.distPer = INF;

    inicial.cidPer = 1;
    inicial.ordem[0] = raiz;
    inicial.visitados[raiz] = 1;

    GET_TIME(startTime);

    # pragma omp parallel num_threads(nThreads)
    visitaParalela(inicial);

    GET_TIME(endTime);
    tempoParalelo = endTime - startTime;

    GET_TIME(startTime);
    visita(inicial);
    GET_TIME(endTime);
    tempoSequencial = endTime - startTime;


    if(PRINTRESULTS){
        printf("##Sequencial\n");
        printf("Caminho de menor custo:");
        for(i = 0; i <= numCid; i++) printf(" %d", melhorCaminhoSequencial.ordem[i]);
        printf("\nMenor custo: %d\n", melhorCaminhoSequencial.distPer);
        printf("Tempo: %.6f\n", tempoSequencial);
        printf("##Paralelo: Threads=%d\n",nThreads);
        printf("Caminho de menor custo:");
        for(i = 0; i <= numCid; i++) printf(" %d", melhorCaminhoParalelo.ordem[i]);
        printf("\nMenor custo: %d\n", melhorCaminhoParalelo.distPer);
        printf("Tempo: %.6f\n", tempoParalelo);
        printf("##Avaliação\n");
        speedup = tempoSequencial/tempoParalelo;
        printf("Speedup = %f\n",speedup);
        eficiencia = speedup/nThreads;
        printf("Eficiencia = %f\n",eficiencia);
    }

	return 0;
}

int** InitMatriz(int n)
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

int** CreateMatriz(int n)
{
    srand(seed);
    int i,j,value;
    int** matriz = InitMatriz(n);
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
    if(!PRINTMATRIZ) return;
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
    int anterior = caminho.ordem[caminho.cidPer-1];
    int i, novaDist;

    printCaminho(caminho);

    if (caminho.cidPer == numCid && custo[anterior][raiz] < INF)
        if(melhorCaminhoSequencial.distPer > caminho.distPer+custo[anterior][raiz])
        {
            melhorCaminhoSequencial = CopiaCaminho(caminho);
            melhorCaminhoSequencial.cidPer++;
            melhorCaminhoSequencial.ordem[melhorCaminhoSequencial.cidPer-1] = raiz;
            melhorCaminhoSequencial.visitados[raiz]=2;
            melhorCaminhoSequencial.distPer+=custo[anterior][raiz];
        }

    for(i = 0; i < numCid; i++)
    {
        if(caminho.visitados[i] > 0) continue;
        if(custo[anterior][i] >= INF) continue;
        novaDist = custo[anterior][i]+caminho.distPer;
        if(novaDist>melhorCaminhoSequencial.distPer) continue;
        Caminho proximo = CopiaCaminho(caminho);
        proximo.cidPer++;
        proximo.ordem[proximo.cidPer-1]=i;
        proximo.visitados[i]=1;
        proximo.distPer=novaDist;
        visita(proximo);
    }
}

void buscaParalela(Caminho caminho)
{
    int anterior = caminho.ordem[caminho.cidPer-1];
    int i, novaDist,myRank, myStart, myFinish;
    bool isWorst;
    myRank = omp_get_thread_num();

    printCaminho(caminho);

    myStart = myRank*(numCid/nThreads);
    myFinish = (myRank+1)*(numCid/nThreads);
    if(myRank==(nThreads-1)) myFinish += numCid%nThreads;

    for(i = myStart; i < myFinish; i++)
    {
        if(caminho.visitados[i] > 0) continue;
        if(custo[anterior][i] >= INF) continue;
        novaDist = custo[anterior][i]+caminho.distPer;

        isWorst=novaDist>melhorCaminhoParalelo.distPer;
        if(isWorst) continue;

        Caminho proximo = CopiaCaminho(caminho);
        proximo.cidPer++;
        proximo.ordem[proximo.cidPer-1]=i;
        proximo.visitados[i]=1;
        proximo.distPer=novaDist;
        visitaParalela(proximo);
    }
}

void visitaParalela(Caminho caminho)
{
    int anterior = caminho.ordem[caminho.cidPer-1];
    int i, novaDist;
    bool isWorst;

    printCaminho(caminho);

    if (caminho.cidPer == numCid && custo[anterior][raiz] < INF){
        #pragma omp critical
        {
            if(melhorCaminhoParalelo.distPer > caminho.distPer+custo[anterior][raiz])
            {
                melhorCaminhoParalelo= CopiaCaminho(caminho);
                melhorCaminhoParalelo.cidPer++;
                melhorCaminhoParalelo.ordem[melhorCaminhoParalelo.cidPer-1] = raiz;
                melhorCaminhoParalelo.visitados[raiz]=2;
                melhorCaminhoParalelo.distPer+=custo[anterior][raiz];

            }
        }
    }

    for(i = 0; i < numCid; i++)
    {
        if(caminho.visitados[i] > 0) continue;
        if(custo[anterior][i] >= INF) continue;
        novaDist = custo[anterior][i]+caminho.distPer;
        #pragma omp critical
        isWorst=novaDist>melhorCaminhoParalelo.distPer;
        if(isWorst) continue;
        Caminho proximo = CopiaCaminho(caminho);
        proximo.cidPer++;
        proximo.ordem[proximo.cidPer-1]=i;
        proximo.visitados[i]=1;
        proximo.distPer=novaDist;
        visitaParalela(proximo);
    }
}

void printCaminho(Caminho caminho)
{
    if(!PRINTCAMINHO) return;
    int i;

    printf("\nCaminho:\nDistancia: %d\nNumero de Cidades: %d\nOrdem:", caminho.distPer, caminho.cidPer);
    for(i = 0; i < caminho.cidPer; i++) printf(" %d", caminho.ordem[i]);
    printf("\nVisitados:");
    for(i = 0; i < numCid; i++) printf(" %d", caminho.visitados[i]);
    printf("\n");
}

Caminho InitCaminho()
{
    int i;

    Caminho caminho;
    caminho.ordem = (int*) malloc((numCid+1) * sizeof(int));
    caminho.visitados = (int*) malloc(numCid * sizeof(int));

    for(i = 0; i < numCid; i++)
    {
        caminho.visitados[i] = 0;
        caminho.ordem[i] = 0;
    }
    caminho.ordem[i+1] = 0;

    caminho.cidPer = 0;
    caminho.distPer = 0;

    return caminho;
}

Caminho CopiaCaminho(Caminho caminho)
{
    int i;
    Caminho caminhoNovo;

    caminhoNovo.ordem = (int*) malloc((numCid+1) * sizeof(int));
    caminhoNovo.visitados = (int*) malloc(numCid * sizeof(int));

    caminhoNovo.cidPer = caminho.cidPer;
    caminhoNovo.distPer = caminho.distPer;

    for(i = 0; i < numCid; i++)
    {
        caminhoNovo.ordem[i] = caminho.ordem[i];
        caminhoNovo.visitados[i] = caminho.visitados[i];
    }

    return caminhoNovo;
}
