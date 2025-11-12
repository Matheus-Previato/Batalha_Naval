#include <windows.h>
#include <stdio.h>
#include <string.h>

// Função que será executada por cada thread
DWORD WINAPI f1(void* data) {
    int threadNum = *(int*)data;
    printf("Thread %d comecou execucao\n", threadNum);
    printf("Hello world from Thread %d\n", threadNum);
    printf("Mais um Hello world from Thread %d\n", threadNum);
    printf("Thread %d finalizou execucao\n", threadNum);
    /*
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
    return 0;
}

int main() {
    int n;
    printf("Digite um valor inteiro para n: ");
    scanf("%d", &n);

    // Array para armazenar identificadores das threads
    HANDLE* threads = (HANDLE*)malloc(n * sizeof(HANDLE));
    int* threadArgs = (int*)malloc(n * sizeof(int));

    // Cria n threads
    for (int i = 0; i < n; i++) {
        threadArgs[i] = i + 1; // Argumento para a thread
        threads[i] = CreateThread(
            NULL,                 // Atributos de segurança
            0,                    // Tamanho da pilha (0 usa o padrão)
            f1,                   // Função a ser executada
            &threadArgs[i],       // Parâmetro para a função
            0,                    // Bandeiras de criação
            NULL);                // Identificador da thread (não usado)

        if (threads[i] == NULL) {
            printf("Erro ao criar a thread %d\n", i + 1);
            return 1;
        }
    }

    // Espera todas as threads terminarem
    WaitForMultipleObjects(n, threads, TRUE, INFINITE);

    // Fecha os identificadores das threads
    for (int i = 0; i < n; i++) {
        CloseHandle(threads[i]);
    }

    // Libera memória alocada
    free(threads);
    free(threadArgs);
    getch();
    return 0;
}
