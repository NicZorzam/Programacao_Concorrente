#include<stdio.h>
#include<stdlib.h>
#include<time.h>

#define TEXTO

float geraNumAleatorio() { //funcao que gera floats aleatorios 
  float min = -100.0, max = 100.0; //range dos valores (-100 a 100)
  float val; //variavel que recebe o numero aleatorio
  val = min + (max - min)*((float)rand()/RAND_MAX);
  return val;
}

int main(int argc, char **argv) {
  float **mat; //matriz onde serao guardado os valores 
  int numLinhas; //quantidade de linhas da matriz 
  FILE *descritorArquivo; //descritor do arquivo de saida
  if(argc != 3) { 
    printf("ERRO: Entrada invalida! Tente './GeraMatriz <numero de linhas> <nome do arquivo de saida>'\n");
    return 1;
  }  
  numLinhas = atoi(argv[1]);
  if(numLinhas < 1) {
    printf("ERRO: O numero de linhas precisa ser positivo!\n");
    return 2;
  } 
  mat = (float**) malloc(numLinhas*sizeof(float*));
  if(mat == NULL) {
    printf("ERRO: A alocacao nao foi realizada com sucesso!\n");
    return 3;
  }
  srand(time(NULL)); 
  for(int i = 0; i < numLinhas; i++) {
    mat[i] = (float*) malloc(3*sizeof(float));
    if(mat[i] == NULL) { //verificar se a memoria foi alocada com sucesso
      printf("ERRO: A alocacao nao foi realizada com sucesso!\n");
      for(int j = 0; j < i; j++) { //liberar a memoria previamente alocada
        free(mat[j]);
      }
      free(mat);
      return 4;
    }
    for(int j = 0; j < 3; j++) { //atualiza os valores de cada linha
      mat[i][j] = geraNumAleatorio();
    }
  }
  #ifdef TEXTO //mostra na saida padrao a matriz 
  for(int i = 0; i < numLinhas; i++) {
    for(int j = 0; j < 3; j++) {
      printf("%.2f ", mat[i][j]);
    }
    printf("\n");
  }
  #endif 
  descritorArquivo = fopen(argv[2], "wb");
  if(descritorArquivo == NULL) {
    printf("ERRO: Falha na abertura do arquivo!\n");
    return 5;
  }
  fwrite(&numLinhas, sizeof(int), 1, descritorArquivo); //insere o numero de linhas no arquivo
  for(int i = 0; i < numLinhas; i++) {
    if(fwrite(mat[i], sizeof(float), 3, descritorArquivo) != 3) { //insere os elementos de cada linha no arquivo
      printf("ERRO: Escrita nao finalizada com sucesso!\n");
      return 6;
    } 
  }
  fclose(descritorArquivo);
  free(mat);
  return 0;
}
