#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Função que será executada por cada thread
void* threadFunc(void* arg) {
    int threadNum = *(int*)arg;
    printf("Thread %d executando\n", threadNum);
    printf("Hello world from Thread %d\n", threadNum); /*
    if (threadNum == 1) {
        printf("Hello world from Thread %d\n", threadNum);
    }
    if (threadNum == 2) {
        printf("Hello world from Thread %d\n", threadNum);
    }
    if (threadNum == 3) {
        printf("Hello world from Thread %d\n", threadNum);
    }
    if (threadNum == 4) {
        printf("Hello world from Thread %d\n", threadNum);
    }*/
    return NULL;
}

int main() {
    int n;
    printf("Digite um valor inteiro: ");
    scanf("%d", &n);

    if (n <= 0) {
        fprintf(stderr, "O valor deve ser um inteiro positivo\n");
        return 1;
    }

    pthread_t* threads = (pthread_t*)malloc(n * sizeof(pthread_t));
    int* threadNums = (int*)malloc(n * sizeof(int));
    int resultado;

    for (int i = 0; i < n; ++i) {
        threadNums[i] = i + 1; // Numerar as threads a partir de 1
        resultado = pthread_create(&threads[i], NULL, threadFunc, &threadNums[i]);
        if (resultado) {
            fprintf(stderr, "Erro ao criar a thread %d: %d\n", i + 1, resultado);
            return 1;
        }
    }

    for (int i = 0; i < n; ++i) {
        resultado = pthread_join(threads[i], NULL);
        if (resultado) {
            fprintf(stderr, "Erro ao esperar a thread %d: %d\n", i + 1, resultado);
            return 1;
        }
    }

    free(threads);
    free(threadNums);

    return 0;
}
