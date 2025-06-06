#include<stdio.h> 
#include<stdlib.h>
#include<pthread.h>

#define TEXTO //descomente esta linha para imprimir a matriz lida do arquivo

float **mat; //matriz de entrada

int main(int argc, char **argv) {
  FILE *arq; //arquivo de entrada
  long int nthreads; //número de threads
  int numLinhas; //número de linhas da matriz
  if(argc != 3) {
    printf("ERRO: Entrada invalida! Tente: %s <numero de threads> <nome do arquivo de entrada>\n", argv[0]);
    return 1;
  }
  nthreads = (long int) atoi(argv[1]);
  if(nthreads < 1) {
    printf("ERRO: Numero de threads deve ser maior que 1!\n");
    return 2;
  }
  arq = fopen(argv[2], "rb");
  fread(&numLinhas, sizeof(int), 1, arq); //le o numero de linhas do arquivo de entrada 
  mat = (float**) malloc(numLinhas*sizeof(float*)); 
  if(mat == NULL) {
    printf("ERRO: Falha ao alocar memoria para a matriz!\n");
    fclose(arq);
    return 3;
  }
  for(int i = 0; i < numLinhas; i++) {
    mat[i] = (float*) malloc(3*sizeof(float));
    if(mat[i] == NULL) {
      printf("ERRO: Falha ao alocar memoria para a linha %d da matriz!\n", i+1);
      fclose(arq);
      for(int j = 0; j < i; j++) {
        free(mat[j]);
      }
      free(mat);
      return 4;
    }
    if(fread(mat[i], sizeof(float), 3, arq) != 3) { //leitura de 3 floats por linha
      printf("ERRO: Falha ao ler a linha %d do arquivo!\n", i+1);
      fclose(arq);
      for(int j = 0; j <= i; j++) {
        free(mat[j]);
      }
      free(mat);
      return 5;
    }
  }
  #ifdef TEXTO
  printf("Matriz lida do arquivo:\n");
  for(int i = 0; i < numLinhas; i++) {
    for(int j = 0; j < 3; j++) {
      printf("%.2f ", mat[i][j]);
    }
    printf("\n");
  }
  #endif
  fclose(arq);
  free(mat);
  return 0;
}
