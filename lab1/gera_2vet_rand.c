/* Disciplina: Programação Concorrente */
/* Profa.: Silvana Rossetto */
/* Exercicio 1 - Nicolly Zorzam Moura  */
/* DRE: 121037550 */
/*
Programa auxiliar para gerar dois vetores de floats randômicos e calcular o produto interno entre eles.
Baseado no programa gera_vet_rand.c da atividade 2
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX 1000 //valor maximo de um elemento do vetor
//descomentar o define abaixo caso deseje imprimir uma versao do vetor gerado no formato texto
#define TEXTO

int main(int argc, char*argv[]) {
    float *vetor1; //vetor 1 que será gerado
    float *vetor2; //vetor 2 que será gerado
    long int n; //qtde de elementos do vetor
    float elem1; //valor gerado para incluir no vetor 1
    float elem2; //valor gerado para incluir no vetor 2
    double prod_int=0; //produto interno dos vetores
    float sinal1, sinal2;
    FILE * descritorArquivo; //descritor do arquivo de saida
    size_t ret; //retorno da funcao de escrita no arquivo de saida
 
    //recebe os argumentos de entrada
    if(argc < 3) {
       fprintf(stderr, "Digite: %s <dimensao> <nome arquivo saida>\n", argv[0]);
       return 1;
    }
    n = atoi(argv[1]);
 
    //aloca memoria para o vetor 1
    vetor1 = (float*) malloc(sizeof(float) * n);
    if(!vetor1) {
       fprintf(stderr, "Erro de alocao da memoria do vetor\n");
       return 2;
    }

    //aloca memoria para o vetor 1
    vetor2 = (float*) malloc(sizeof(float) * n);
    if(!vetor2) {
       fprintf(stderr, "Erro de alocao da memoria do vetor\n");
       return 2;
    }
 
    //preenche os vetores com valores float aleatorios
    srand(time(NULL));
    
    for(long int i=0; i<n; i++) {
        sinal1 = (rand() % 2 == 0) ? 1.0 : -1.0;
        sinal2 = (rand() % 2 == 0) ? 1.0 : -1.0;

         elem1 = (rand() % MAX)/3.0 * sinal1;
         vetor1[i] = elem1;

         elem2 = (rand() % MAX)/3.0 * sinal2;
         vetor2[i] = elem2;

         prod_int += elem1*elem2; //acumula a multiplicacao no produto interno
    }
 
    //imprimir na saida padrao os vetores gerados
    #ifdef TEXTO
    fprintf(stdout, "%ld\n", n);
    for(long int i=0; i<n; i++) {
       fprintf(stdout, "%f ",vetor1[i]);
    }
    fprintf(stdout, "\n");
    for(long int i=0; i<n; i++) {
        fprintf(stdout, "%f ",vetor2[i]);
     }
    fprintf(stdout, "\n");
    fprintf(stdout, "%lf\n", prod_int);
    #endif
 
    //escreve os vetores no arquivo
    //abre o arquivo para escrita binaria
    descritorArquivo = fopen(argv[2], "wb");
    if(!descritorArquivo) {
       fprintf(stderr, "Erro de abertura do arquivo\n");
       return 3;
    }
    //escreve a dimensao
    ret = fwrite(&n, sizeof(long int), 1, descritorArquivo);
    //escreve os elementos do vetor 1
    ret = fwrite(vetor1, sizeof(float), n, descritorArquivo);
    if(ret < n) {
       fprintf(stderr, "Erro de escrita no  arquivo\n");
       return 4;
    }
    //escreve os elementos do vetor 2
    ret = fwrite(vetor2, sizeof(float), n, descritorArquivo);
    if(ret < n) {
       fprintf(stderr, "Erro de escrita no  arquivo\n");
       return 4;
    }
    //escreve o produto interno
    ret = fwrite(&prod_int, sizeof(double), 1, descritorArquivo);
 
    //finaliza o uso das variaveis
    fclose(descritorArquivo);
    free(vetor1);
    free(vetor2);
    return 0;
 } 
 
