/* Disciplina: Programação Concorrente */
/* Profa.: Silvana Rossetto */
/* Exercicio 1 - Nicolly Zorzam Moura  */
/* DRE: 121037550 */
/* Codigo: Produto interno de vetores de floats */
/* Baseado no código soma_vetor_conc.c da atividade 2 */

#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>
#include <math.h> 

//variaveis globais
//tamanho do vetor
long int n;
//vetores de elementos
float *vet1;
float *vet2;
//numero de threads
int nthreads;

//funcao executada pelas threads
//estrategia de divisao de tarefas: blocos de n/nthreads elementos
void *ProdutoVetor (void *tid) {
  long int id = (long int) tid; //identificador da thread
  int ini, fim, bloco; //auxiliares para divisao do vetor em blocos
  float produto_local=0, *ret; //somatorio local
  
  bloco = n/nthreads; //tamanho do bloco de dados de cada thread
  ini = id*bloco; //posicao inicial do vetor
  fim = ini + bloco; //posicao final do vetor
  if (id==(nthreads-1)) fim = n; //a ultima thread trata os elementos restantes no caso de divisao nao exata

  //calcula o produto local dos vetores 
  for(int i=ini; i<fim; i++) {
     produto_local += vet1[i]*vet2[i];
  }

  //retorna o resultado do produto
  ret = (float*) malloc(sizeof(float));
  if (ret!=NULL) *ret = produto_local;
  else printf("--ERRO: malloc() thread\n");
  pthread_exit((void*) ret);
}

//funcao principal do programa
int main(int argc, char *argv[]) {
  FILE *arq; //arquivo de entrada
  size_t ret; //retorno da funcao de leitura no arquivo de entrada
  double prod_int_ori; // produto interno registrado no arquivo

  float prod_par_global; //resultado do produto interno concorrente
  float *prod_retorno_threads; //auxiliar para retorno das threads

  pthread_t *tid_sistema; //vetor de identificadores das threads no sistema

  //valida e recebe os valores de entrada
  if(argc < 3) { printf("Use: %s <arquivo de entrada> <numero de threads> \n", argv[0]); exit(-1); }

  //abre o arquivo de entrada com os valores para serem somados
  arq = fopen(argv[1], "rb");
  if(arq==NULL) { printf("--ERRO: fopen()\n"); exit(-1); }

  //le o tamanho do vetor (primeira linha do arquivo)
  ret = fread(&n, sizeof(long int), 1, arq);
  if(!ret) {
     fprintf(stderr, "Erro de leitura das dimensoes da matriz arquivo \n");
     return 3;
  }

  //aloca espaco de memoria e carrega os vetores de entrada
  vet1 = (float*) malloc (sizeof(float) * n);
  if(vet1==NULL) { printf("--ERRO: malloc()\n"); exit(-1); }
  ret = fread(vet1, sizeof(float), n, arq);
  if(ret < n) {
     fprintf(stderr, "Erro de leitura dos elementos do vetor 1\n");
     return 4;
  }

  vet2 = (float*) malloc (sizeof(float) * n);
  if(vet2==NULL) { printf("--ERRO: malloc()\n"); exit(-1); }
  ret = fread(vet2, sizeof(float), n, arq);
  if(ret < n) {
     fprintf(stderr, "Erro de leitura dos elementos do vetor 2\n");
     return 4;
  }

  //le o numero de threads da entrada do usuario 
  nthreads = atoi(argv[2]);
  //limita o numero de threads ao tamanho do vetor
  if(nthreads>n) nthreads = n;

  //aloca espaco para o vetor de identificadores das threads no sistema
  tid_sistema = (pthread_t *) malloc(sizeof(pthread_t) * nthreads);
  if(tid_sistema==NULL) { printf("--ERRO: malloc()\n"); exit(-1); }

  //cria as threads
  for(long int i=0; i<nthreads; i++) {
    if (pthread_create(&tid_sistema[i], NULL, ProdutoVetor, (void*) i)) {
       printf("--ERRO: pthread_create()\n"); exit(-1);
    }
  }

  //espera todas as threads terminarem e calcula o produto interno total das threads
  //retorno = (float*) malloc(sizeof(float));
  prod_par_global=0;
  for(int i=0; i<nthreads; i++) {
     if (pthread_join(tid_sistema[i], (void *) &prod_retorno_threads)) {
        printf("--ERRO: pthread_join()\n"); exit(-1);
     }
     prod_par_global += *prod_retorno_threads;
     free(prod_retorno_threads);
  }

  //imprime os resultados
  printf("\n");
  printf("Produto interno concorrente = %.26f\n", prod_par_global);
  //le o produto interno registrado no arquivo
  ret = fread(&prod_int_ori, sizeof(double), 1, arq); 
  printf("Produto interno original    = %.26lf\n", prod_int_ori);
  double e = fabs((prod_int_ori - prod_par_global) / prod_int_ori); // calculo da variacao relativa
  printf("Variacao relativa           = %.26lf\n", e);

  //desaloca os espacos de memoria
  free(vet1);
  free(vet2);
  free(tid_sistema);
  //fecha o arquivo
  fclose(arq);
  return 0;
}

/*
De maneira geral, apesar dos possíveis problemas com float e das diferenças na ordem de avaliação das somas (que
pode causar mudanças no resultado final), a variação relativa mostrou que o cálculo com paralelismo do produto
interno ainda é bem viável e útil em grandes escalas, pois as discrepâncias observadas foram extremamente pequenas.
*/