#include <stdio.h>
#include <stdlib.h>
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
void PrintMatriz(int** matriz,int tamanhoMatriz);
Caminho InitCaminho();
Caminho CopiaCaminho(Caminho caminho);
void visita(Caminho caminho);
void printCaminho(Caminho caminho);

int **custo;
int numCid, raiz;
Caminho melhorCaminho;

int main()
{
	int i, j;
	Caminho inicial;

	//pega quantidade de cidade e cidade de origem
	scanf("%d", &numCid);

	//determinando as arestas
	custo = InitMatriz(numCid);
	for(i = 0; i < numCid; i++)
		for(j = 0; j < numCid; j++)
			scanf("%d", &custo[i][j]);

	scanf("%d", &raiz);

	inicial = InitCaminho();
	melhorCaminho = InitCaminho();
    melhorCaminho.distPer = INF;

    inicial.cidPer = 1;
    inicial.ordem[0] = raiz;
    inicial.visitados[raiz] = 1;
    visita(inicial);

    printf("Caminho de menor custo:");
    for(i = 0; i <= numCid; i++) printf(" %d", melhorCaminho.ordem[i]);
    printf("\nMenor custo: %d\n", melhorCaminho.distPer);

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

void PrintMatriz(int** matriz,int tamanhoMatriz){
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
    int i;

    printCaminho(caminho);

    if (caminho.cidPer == numCid && custo[anterior][raiz] < INF)
        if(melhorCaminho.distPer > caminho.distPer+custo[anterior][raiz])
        {
            melhorCaminho = CopiaCaminho(caminho);
            melhorCaminho.cidPer++;
            melhorCaminho.ordem[melhorCaminho.cidPer-1] = raiz;
            melhorCaminho.visitados[raiz]=2;
            melhorCaminho.distPer+=custo[anterior][raiz];
        }

    for(i = 0; i < numCid; i++)
    {
        if(caminho.visitados[i] > 0) continue;
        if(custo[anterior][i] >= INF) continue;
        Caminho proximo = CopiaCaminho(caminho);
        proximo.cidPer++;
        proximo.ordem[proximo.cidPer-1]=i;
        proximo.visitados[i]=1;
        proximo.distPer+=custo[anterior][i];
        visita(proximo);
    }
}

void printCaminho(Caminho caminho)
{
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
