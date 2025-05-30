/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Nicolly Zorzam Moura - DRE: 121037550 */
/* Codigo: Produtoras e consumidoras verificando primalidade */


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>

// variaveis globais
long long int *buffer;
int in = 0, out = 0;
int *primos_por_thread;
int total_primos = 0;
long int N;
int M, C;
long int contador_consumidos = 0;

sem_t slotCheio, slotVazio;
sem_t mutexBuffer, mutexContador;

// funcao de checagem de primo
int ehPrimo(long long int n) {
    int i;
    if (n <= 1) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;
    for (i = 3; i < sqrt(n) + 1; i += 2)
        if (n % i == 0) return 0;
    return 1;
}

// funcao produtora
void *produtora(void *arg) {
    for (long int i = 1; i <= N; i++) { // Loop N vezes para N itens
        sem_wait(&slotVazio);
        sem_wait(&mutexBuffer);
        buffer[in] = i;
        in = (in + 1) % M;
        sem_post(&mutexBuffer);
        sem_post(&slotCheio);
    }

    // sinaliza fim da producao, para que os consumidores possam sair do sem_wait e verificar a condição de término
    for (int i = 0; i < C; i++) {
        sem_post(&slotCheio);
    }
    pthread_exit(NULL);
}

// funcao consumidoras
void *consumidora(void *arg) {
    int id = *(int *)arg;
    free(arg);
    int local_primos = 0;

    while (1) {
        sem_wait(&slotCheio);
        sem_wait(&mutexBuffer);

        if (contador_consumidos < N) {
            long long int item = buffer[out];
            out = (out + 1) % M;
            contador_consumidos++;

            sem_post(&mutexBuffer);
            sem_post(&slotVazio);

            if (ehPrimo(item)) {
                local_primos++;
                sem_wait(&mutexContador);
                total_primos++;
                sem_post(&mutexContador);
            }
        } else {
            // todos os N elementos foram consumidos ou estão sendo
            sem_post(&mutexBuffer);
            break;
        }
    }

    primos_por_thread[id] = local_primos;
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if(argc<4) {
        printf("Digite: %s <qtde de numeros (N)> <tamanho do buffer (M)> <qtde de threads consumidoras (C)>\n", argv[0]);
        return 1;
    }
    N = atoi(argv[1]);
    M = atoi(argv[2]);
    C = atoi(argv[3]);

    buffer = malloc(M * sizeof(long long int));
    if (buffer == NULL) {
        printf("erro ao alocar buffer");
        return 1;
    }
    primos_por_thread = calloc(C, sizeof(int));
    if (primos_por_thread == NULL) {
        printf("erro ao alocar primos_por_thread");
        return 1;
    }

    sem_init(&slotCheio, 0, 0);
    sem_init(&slotVazio, 0, M);
    sem_init(&mutexBuffer, 0, 1);
    sem_init(&mutexContador, 0, 1);

    pthread_t prod_thread;
    pthread_t cons_threads[C];

    if(pthread_create(&prod_thread, NULL, produtora, NULL)){
        printf("Erro na criacao do thread produtor\n");
        exit(1);
    }

    for (int i = 0; i < C; i++) {
        id = (int *) malloc(sizeof(int));
        *id = i;
        if (pthread_create(&cons_threads[i], NULL, consumidora, (void *) (id))) {
        printf("Erro na criacao do thread consumudor\n");
        exit(1);
        }
    }

    pthread_join(prod_thread, NULL);
    for (int i = 0; i < C; i++) {
        if (pthread_join(cons_threads[i], NULL)) {
        printf("--ERRO: pthread_join() \n"); exit(-1); 
        } 
    }

    // Resultado
    printf("Total de primos: %d\n", total_primos);

    
    int vencedora = 0;
    for (int i = 1; i < C; i++) {
        if (primos_por_thread[i] > primos_por_thread[vencedora]) {
            vencedora = i;
        }
    }
    printf("Vencedora: thread %d com %d primos\n", vencedora, primos_por_thread[vencedora]);
    
    free(buffer);
    free(primos_por_thread);
    sem_destroy(&slotCheio);
    sem_destroy(&slotVazio);
    sem_destroy(&mutexBuffer);
    sem_destroy(&mutexContador);

    return 0;
}