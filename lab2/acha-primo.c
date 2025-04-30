/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Nicolly Zorzam Moura - DRE: 121037550*/
/* Codigo: Dada uma sequencia consecutiva de numeros naturais (inteiros positivos) de 1 a N (N muito grande),
identificar todos os numeros primos dessa sequencia e retornar a quantidade total de numeros primos encontrados */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

long long int N;
long long int prox_num = 1;
long long int total_primos = 0;

pthread_mutex_t mutex_num;
pthread_mutex_t mutex_total;

int ehPrimo(long long int n) {
    int i;
        if (n<=1) return 0;
        if (n==2) return 1;
        if (n%2==0) return 0;
        for (i=3; i<sqrt(n)+1; i+=2)
            if(n%i==0) return 0;
        return 1;
}

//funcao executada pelas threads
void *ExecutaTarefa(void *arg) {
    long int id = (long int) arg;
    long long int num;

    printf("Thread : %ld esta executando...\n", id);

    while (1) {
        //--entrada na SC
        pthread_mutex_lock(&mutex_num);
        if (prox_num > N) {
            pthread_mutex_unlock(&mutex_num);
            break;
        }
        num = prox_num;
        prox_num++; //incrementa a variavel compartilhada 
        pthread_mutex_unlock(&mutex_num);
        //--saida da SC

        //-- verifica se o numero e primo
        if (ehPrimo(num)) {
            //-- entrada na SC para atualizar total_primos
            pthread_mutex_lock(&mutex_total);
            total_primos++; //incrementa a variavel compartilhada 
            pthread_mutex_unlock(&mutex_total);
            //--saida da SC
        }
    }

    printf("Thread %ld: terminou!\n", id);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    pthread_t *tid; //identificadores das threads no sistema
    int nthreads; //qtde de threads (passada linha de comando)

    if (argc < 3) {
        printf("Digite: %s <numero de elementos> <numero de threads>\n", argv[0]);
        return 1;
    }

    N = atoll(argv[1]);
    nthreads = atoi(argv[2]);

    //--aloca as estruturas
    tid = (pthread_t*) malloc(sizeof(pthread_t)*(nthreads+1));
    if(tid==NULL) {puts("ERRO--malloc"); return 2;}

    //--inicilaiza o mutex (lock de exclusao mutua)
    pthread_mutex_init(&mutex_num, NULL);
    pthread_mutex_init(&mutex_total, NULL);

    // marcar o tempo
    struct timespec inicio, fim;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    //--cria as threads
   for(long int t=0; t<nthreads; t++) {
    if (pthread_create(&tid[t], NULL, ExecutaTarefa, (void *)t)) {
      printf("--ERRO: pthread_create()\n"); exit(-1);
    }
   }

    //--espera todas as threads terminarem
   for (int t=0; t<nthreads; t++) {
    if (pthread_join(tid[t], NULL)) {
        printf("--ERRO: pthread_join() \n"); exit(-1); 
    } 
   } 

   clock_gettime(CLOCK_MONOTONIC, &fim);

    pthread_mutex_destroy(&mutex_num);
    pthread_mutex_destroy(&mutex_total);

    double tempo = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;

    printf("Total de primos entre 1 e %lld: %lld\n", N, total_primos);
    printf("Tempo com %d threads: %.6f segundos\n", nthreads, tempo);
    
    return 0;
}