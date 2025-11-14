/*
 * CLIENTE (client.c)
 * Este é o programa que o jogador vai executar.
 * Ele conecta-se ao servidor e permite enviar e receber jogadas.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h> // Para Sockets
#include <pthread.h>  // Para Threads

// --- Documentação: Constantes do Jogo ---
// (Devem ser idênticas às do servidor)
#define TAMANHO 15
const char AGUA = '~';
const char NAVIO = 'N';
const char TIRO_CERTEIRO = 'X';
const char TIRO_AGUA = 'O';

// --- Documentação: Estrutura do Jogador ---
// O cliente armazena seus próprios tabuleiros
typedef struct {
    char tabuleiro_defesa[TAMANHO][TAMANHO];
    char tabuleiro_ataque[TAMANHO][TAMANHO];
} Jogador;

// --- Documentação: Variáveis Globais ---
Jogador meu_jogador;
SOCKET client_sock; // Socket de conexão

// --- Documentação: Funções do Tabuleiro ---
// (Idênticas às que criámos antes)

void inicializar_tabuleiro(char tabuleiro[TAMANHO][TAMANHO]) {
    for (int i = 0; i < TAMANHO; i++) {
        for (int j = 0; j < TAMANHO; j++) {
            tabuleiro[i][j] = AGUA;
        }
    }
}

void mostrar_tabuleiro(char tabuleiro[TAMANHO][TAMANHO]) {
    // Imprime cabeçalho das COLUNAS (A a O)
    printf("\n   "); 
    for (int j = 0; j < TAMANHO; j++) {
        char letra_coluna = 'A' + j;
        printf("%c  ", letra_coluna);
    }
    printf("\n");

    // Imprime LINHAS (0 a 14) e o conteúdo
    for (int i = 0; i < TAMANHO; i++) {
        if (i < 10) {
            printf("%d  ", i); // Um dígito, dois espaços
        } else {
            printf("%d ", i); // Dois dígitos, um espaço
        }
        for (int j = 0; j < TAMANHO; j++) {
            printf("%c  ", tabuleiro[i][j]); 
        }
        printf("\n"); 
    }
    printf("\n"); 
}

// --- Documentação: Thread de Recebimento ---
// Esta thread fica num loop infinito apenas a "ouvir"
// o que o servidor manda e a imprimir no ecrã.
void *receive_thread(void *arg) {
    char buffer_resposta[256];
    int nBytes;

    while ((nBytes = recv(client_sock, buffer_resposta, sizeof(buffer_resposta), 0)) > 0) {
        buffer_resposta[nBytes] = '\0'; // Adiciona terminador nulo
        
        // Imprime a mensagem do servidor
        printf("\n[Mensagem do Servidor]: %s\n", buffer_resposta);
        
        // TODO: Processar a resposta. 
        // Ex: Se a resposta for "X:B10", devemos atualizar
        // o nosso 'tabuleiro_ataque' na posição B10 com 'X'.
        // Se a resposta for "Sua vez", apenas imprimimos.
        
        printf("Digite sua jogada (ex: B10): "); // Pede nova jogada
        fflush(stdout); // Garante que a mensagem apareça
    }

    if (nBytes <= 0) {
        printf("Servidor desconectado.\n");
        closesocket(client_sock);
    }
    return 0;
}

// --- Documentação: Função Principal (main) ---
// Configura o cliente, conecta, e inicia as threads.
int main() {
    WSADATA wsa;
    struct sockaddr_in server_addr;
    pthread_t recv_thread;
    char buffer_envio[256];

    // 1. Inicializa o Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Falha ao inicializar Winsock: %d\n", WSAGetLastError());
        return 1;
    }

    // 2. Cria o Socket do Cliente
    if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Nao foi possivel criar socket: %d\n", WSAGetLastError());
        return 1;
    }

    // 3. Prepara a estrutura sockaddr_in (Aponta para o Servidor)
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // IP Local (localhost)
    server_addr.sin_port = htons(8888); // Porta (deve ser a mesma do server.c)

    // 4. Conecta ao Servidor
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Conexao falhou: %d\n", WSAGetLastError());
        return 1;
    }

    printf("Conectado ao servidor! Aguardando outro jogador...\n");

    // 5. Inicializa os tabuleiros locais do cliente
    inicializar_tabuleiro(meu_jogador.tabuleiro_defesa);
    inicializar_tabuleiro(meu_jogador.tabuleiro_ataque);

    // TODO: Adicionar lógica de posicionamento de navios.
    // O cliente deve posicionar os navios no 'tabuleiro_defesa'
    // e enviar essas posições para o servidor.
    printf("--- SEU TABULEIRO DE DEFESA (Seus Navios) ---\n");
    mostrar_tabuleiro(meu_jogador.tabuleiro_defesa);
    printf("--- SEU TABULEIRO DE ATAQUE (Tiros no Oponente) ---\n");
    mostrar_tabuleiro(meu_jogador.tabuleiro_ataque);


    // 6. Cria a Thread de Recebimento
    if (pthread_create(&recv_thread, NULL, receive_thread, NULL) < 0) {
        perror("Nao foi possivel criar a thread de recebimento");
        return 1;
    }

    // 7. Loop Principal (Thread de Envio)
    // Lê a jogada do utilizador e envia para o servidor
    printf("Digite sua jogada (ex: B10):\n");
    while (fgets(buffer_envio, sizeof(buffer_envio), stdin) != NULL) {
        // Remove o '\n' (newline) que o fgets captura
        buffer_envio[strcspn(buffer_envio, "\n")] = 0;

        if (strlen(buffer_envio) > 0) {
            // Envia a jogada para o servidor
            if (send(client_sock, buffer_envio, strlen(buffer_envio), 0) < 0) {
                printf("Envio falhou.\n");
                break;
            }
        }
    }

    closesocket(client_sock);
    WSACleanup();
    return 0;
}
