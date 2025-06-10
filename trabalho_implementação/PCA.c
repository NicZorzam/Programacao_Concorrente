#include<stdio.h> 
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>
#include<math.h>
#include<time.h>

#define TEXTO //descomente esta linha para imprimir a matriz lida do arquivo

float **mat; //matriz de entrada
float **proj; // matriz de projeção
float cov[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}; //matriz de covariancia
float autovalores[3] = {0.0, 0.0, 0.0}; // lista dos autovalores 
float autovetores [3][3];
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

// funcao que resolve equação cúbica da forma x^3 + a*x^2 + b*x + c = 0
void resolve_cubica(double a, double b, double c) {
    double q = (3*b - a*a)/9;
    double r = (9*a*b - 27*c - 2*a*a*a)/54;
    double delta = q*q*q + r*r;
    double PI = acos(-1);

    if (delta > 0) {
        // Uma raiz real
        double s = cbrt(r + sqrt(delta));
        double t = cbrt(r - sqrt(delta));
        double x1 = -a/3 + (s + t);
        // printf("Autovalor real: %.6f\n", x1);
    } else if (delta == 0) {
        // Raízes reais e pelo menos duas iguais
        double s = cbrt(r);
        double x1 = -a/3 + 2*s;
        double x2 = -a/3 - s;
        // printf("Autovalores reais (dois iguais): %.6f, %.6f\n", x1, x2);
    } else {
        // Três raízes reais
        double theta = acos(r / sqrt(-q*q*q));
        double x1 = 2 * sqrt(-q) * cos(theta/3) - a/3;
        double x2 = 2 * sqrt(-q) * cos((theta + 2*PI)/3) - a/3;
        double x3 = 2 * sqrt(-q) * cos((theta + 4*PI)/3) - a/3;
        // printf("Autovalores reais:\n");
        // printf("λ1 = %.6f\n", x1);
        // printf("λ2 = %.6f\n", x2);
        // printf("λ3 = %.6f\n", x3);
        autovalores[0] = x1;
        autovalores[1] = x2;
        autovalores[2] = x3;
    }
}

void calc_autovalores(){

    // montando coeficientes da equação cubica λ^3 + Aλ^2 + Bλ + C = 0

    // |C - λI| = determinante de:
    // | a-λ  d    e  |
    // | d    b-λ  f  |
    // | e    f    c-λ|

    // formula geral para os coeficientes do polinomio caracteristico:
    double a = cov[0][0];
    double b = cov[1][1];
    double c = cov[2][2];
    double d = cov[0][1];
    double e = cov[0][2];
    double f = cov[1][2];

    double A = -(a + b + c);

    double B = a*b + a*c + b*c - d*d - e*e - f*f;

    double C = a*(b*c - f*f) - d*(d*c - f*e) + e*(d*f - b*e);

    resolve_cubica(A, B, C);
}

void calc_autovetor(long int id) {
  for(long int idV = id; idV < 3; idV += nthreads) {
    double lambda = autovalores[idV];
    double A[3][3];
      // matriz (C - λI)
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        A[i][j] = cov[i][j] - (i == j ? lambda : 0);

    // como o sistema é homogêneo, podemos assumir v[2] = 1 e resolver v[0] v[1]
    double v[3] = {0, 0, 1};

    // resolver o sistema:
    // A[0][0] * v[0] + A[0][1] * v[1] + A[0][2] * 1 = 0
    // A[1][0] * v[0] + A[1][1] * v[1] + A[1][2] * 1 = 0

    double a = A[0][0], b = A[0][1], c = -A[0][2];
    double d = A[1][0], e = A[1][1], f = -A[1][2];

    double det = a*e - b*d;
    if (fabs(det) < 1e-8) {
        // sistema dependente
      v[0] = 1; v[1] = 0;
    } else {
      v[0] = (c*e - b*f)/det;
      v[1] = (a*f - c*d)/det;
    }

    // normalizando vetor
    double norm = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    autovetores[idV][0] = v[0]/norm;
    autovetores[idV][1] = v[1]/norm;
    autovetores[idV][2] = v[2]/norm;
  }
  barreira();
}

void ordena_autovetores() {
  for (int i = 0; i < 2; i++) {
    for (int j = i + 1; j < 3; j++) {
      if (autovalores[i] < autovalores[j]) {
        float tmp = autovalores[i];
        autovalores[i] = autovalores[j];
        autovalores[j] = tmp;
        for (int k = 0; k < 3; k++) {
          float vtmp = autovetores[i][k];
          autovetores[i][k] = autovetores[j][k];
          autovetores[j][k] = vtmp;
        }
      }
    }
  }
}

void projecao(long int id) {
  for (long int i = id; i < numLinhas; i += nthreads) {
    for (int j = 0; j < 3; j++) {
      proj[i][j] = 0.0;
      for (int k = 0; k < 3; k++) {
        proj[i][j] += mat[i][k] * autovetores[j][k];
      }
    }
  }
}

void *tPCA(void *threads) {
  long int id = (long int) threads; //id da thread
  printf("\nOi da thread: %ld\n", id);
  centraliza(id); //centraliza a matriz  
  calcConvar(id); //calcula a matriz de covariancia
  if(id == 0) calc_autovalores(); //calcula os autovalores
  barreira();
  calc_autovetor(id); //calcula os autovetores
  if(id == 0) ordena_autovetores(); //ordena os autovetores
  barreira();
  projecao(id);
  barreira();
  pthread_exit(NULL);
}

int main(int argc, char **argv) {
  clock_t inicio, fim;
  double tempo;
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
  proj = (float**) malloc(numLinhas * sizeof(float*));
  if(proj == NULL) {
    printf("ERRO: Falha ao alocar memoria para a matriz!\n");
    return 3;
  }

  for(int i = 0; i < numLinhas; i++) {
    proj[i] = (float*) malloc(3 * sizeof(float));
    if(proj[i] == NULL) {
      printf("ERRO: Falha ao alocar memoria para a linha %d da matriz!\n", i+1);
      for(int j = 0; j < i; j++) {
        free(proj[j]);
      }
      free(proj);
      return 4;
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
  inicio = clock();
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
  fim = clock();
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
  
  printf("\n");
  for (int h = 0; h < 3; h++){
    printf("autovalor: %f\n", autovalores[h]);
    printf("autovetor: ");
    printf("%f  ", autovetores[h][0]);
    printf("%f  ", autovetores[h][1]);
    printf("%f\n\n", autovetores[h][2]);
  }
  
  for(int i = 0; i < numLinhas; i++) {
    for(int j = 0; j < 3; j++) {
      printf("%.2f ", proj[i][j]);
    }
    printf("\n");
  }
  #endif
  tempo = ((double)(fim - inicio)) / CLOCKS_PER_SEC;
  printf("tempo de execucao: %.9f segundos\n", tempo);  
  sem_destroy(&mutexBar); //destroi o semaforo mutex
  sem_destroy(&cond); //destroi o semaforo cond
  fclose(arq); //fecha o arquivo de entrada
  free(mat);
  free(proj);
  return 0;
}
