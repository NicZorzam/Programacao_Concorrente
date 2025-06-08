#include<stdio.h> 
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>

#define TEXTO //descomente esta linha para imprimir a matriz lida do arquivo

float **mat; //matriz de entrada
float cov[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}; //matriz de covariancia 
float media[3] = {0, 0, 0}; //média de cada coluna da matriz
int nthreads; //número de threads
int numLinhas; //número de linhas da matriz
int bloqueadas = 0; //variavel para controlar o número de threads bloqueadas
sem_t mutexBar, cond;
pthread_mutex_t mutex; //mutex para evitar corrida de dados

void printMatriz() {
  for(int i = 0; i < numLinhas; i++) {
    for(int j = 0; j < 3; j++) {
      printf("%.2f ", mat[i][j]);
    }
    printf("\n");
  }
}

void barreira() { //funcao de barreira para sincronizar as threads
  if(nthreads != 1) { //se houver apenas uma thread, não precisa de barreira
    sem_wait(&mutexBar);
    bloqueadas++;
    if(bloqueadas < nthreads) {
      sem_post(&mutexBar); //libera o semáforo se não for a última thread
      sem_wait(&cond); //bloqueia a thread até que todas as outras cheguem
      bloqueadas--;
      if(bloqueadas == 0) sem_post(&mutexBar); //libera o semáforo se for a última thread
      else sem_post(&cond); //libera o semáforo para a próxima thread
    } 
    else {
      bloqueadas--;
      sem_post(&cond); //libera o semáforo para a próxima thread
    }
  }
}

void centraliza(long int id){ //funcao que centraliza a matriz
  float valor;
  for(int i = 0; i < 3; i++) {
    for(int j = id; j < numLinhas; j += nthreads) {
      valor = mat[j][i]/numLinhas; //valor que sera somado a media 
      pthread_mutex_lock(&mutex); //trava o mutex para evitar corrida de dados
      media[i] += valor; //soma os valores da media de cada coluna
      pthread_mutex_unlock(&mutex); //destrava o mutex
    }
  }
  barreira();
  for(int i = 0; i < 3; i++) {
    for(int j = id; j < numLinhas; j += nthreads) {
      mat[j][i] -= media[i]; //subtrai a média de cada coluna
    }
  }
  barreira();
}

void calcConvar(long int id) {
  float valor; 
  for(int i = 0; i < 3; i++) {
    for(int j = 0; j < 3; j++) {
      for(int k = id; k < numLinhas; k += nthreads) {
        valor = (mat[k][i]*mat[k][j])/(numLinhas - 1); 
        pthread_mutex_lock(&mutex); //trava o mutex para evitar corrida de dados
        cov[i][j] += valor; //calcula a matriz de covariancia
        pthread_mutex_unlock(&mutex); //destrava o mutex
      }
    }
  }
  barreira();
}

void *tPCA(void *threads) {
  long int id = (long int) threads; //id da thread
  printf("\nOi da thread: %ld\n", id);
  centraliza(id); //centraliza a matriz  
  calcConvar(id); //calcula a matriz de covariancia
  pthread_exit(NULL);
}

int main(int argc, char **argv) {
  FILE *arq; //arquivo de entrada
  pthread_t *threads; //vetor de threads
  if(argc != 3) {
    printf("ERRO: Entrada invalida! Tente: %s <numero de threads> <nome do arquivo de entrada>\n", argv[0]);
    return 1;
  }
  nthreads = atoi(argv[1]);
  if(nthreads < 1) {
    printf("ERRO: Numero de threads deve ser maior que 1!\n");
    return 2;
  }  
  arq = fopen(argv[2], "rb");
  fread(&numLinhas, sizeof(int), 1, arq); //le o numero de linhas do arquivo de entrada 
  if(nthreads > numLinhas) nthreads = numLinhas;
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
  printMatriz(); //imprime a matriz lida do arquivo
  #endif
  threads = (pthread_t*) malloc(nthreads*sizeof(pthread_t));
  if(threads == NULL) {
    printf("ERRO: Falha ao alocar memoria para as threads!\n");
    fclose(arq);
    for(int i = 0; i < numLinhas; i++) {
      free(mat[i]);
    }
    free(mat);
    return 6;
  }
  sem_init(&mutexBar, 0, 1); //inicializa o semaforo mutex
  sem_init(&cond, 0, 0); //inicializa o semaforo cond
  for(long int i = 0; i < nthreads; i++) {
    if(pthread_create(&threads[i], NULL, tPCA, (void *) i)) { //cria as threads
      printf("ERRO: Falha ao criar a thread %ld!\n", i);
      return 7;
    }
  }
  for(int i = 0; i < nthreads; i++) {
    if(pthread_join(threads[i], NULL)) { //espera as threads terminarem
      printf("ERRO: Falha ao encerrar a thread %d!\n", i);
      return 8;
    }
  }
  #ifdef TEXTO
  printf("\nMedia coluna 1: %f\n", media[0]);
  printf("Media coluna 2: %f\n", media[1]);
  printf("Media coluna 3: %f\n\n", media[2]);
  printf("Matriz centralizada:\n");
  printMatriz();
  printf("\nMatriz de covariancia:\n");
  for(int i = 0; i < 3; i++) {
    for(int j = 0; j < 3; j++) {
      printf("%.2f ", cov[i][j]);
    }
    printf("\n");
  }
  #endif 
  sem_destroy(&mutexBar); //destroi o semaforo mutex
  sem_destroy(&cond); //destroi o semaforo cond
  fclose(arq); //fecha o arquivo de entrada
  free(mat);
  return 0;
}
