/* * SERVIDOR (server.c)
 * Este código é a base para o servidor do jogo Batalha Naval.
 * Ele aceita dois jogadores e gerencia o estado do jogo.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h> // Para Sockets
#include <pthread.h>  // Para Threads e Mutex

// --- Documentação: Constantes do Jogo ---
#define TAMANHO 15
const char AGUA = '~';
const char NAVIO = 'N';
const char TIRO_CERTEIRO = 'X';
const char TIRO_AGUA = 'O';

// --- Documentação: Estrutura do Jogador ---
// (Esta estrutura agora existe no servidor)
typedef struct {
    char tabuleiro_defesa[TAMANHO][TAMANHO];
    char tabuleiro_ataque[TAMANHO][TAMANHO];
} Jogador;

// --- Documentação: Memória Compartilhada (Estado do Jogo) ---
// Estas são as variáveis globais que AMBAS as threads (jogadores)
// vão aceder. Por isso, precisamos protegê-las com um mutex.
Jogador jogador1;
Jogador jogador2;
int vez_do_jogador = 1; // Controla o turno (1 ou 2)
int jogadores_conectados = 0;

// Mutex para exclusão mútua (requisito do trabalho)
pthread_mutex_t mutex_estado_jogo;

// --- Documentação: Funções do Tabuleiro ---
// (Estas funções são as mesmas que criámos antes)
void inicializar_tabuleiro(char tabuleiro[TAMANHO][TAMANHO]) {
    for (int i = 0; i < TAMANHO; i++) {
        for (int j = 0; j < TAMANHO; j++) {
            tabuleiro[i][j] = AGUA;
        }
    }
}

// --- Documentação: Função da Thread (handle_client) ---
// Esta função é o coração do servidor. Cada jogador terá
// uma destas a correr, tratando da sua comunicação.
void *handle_client(void *socket_desc) {
    SOCKET client_sock = *(SOCKET*)socket_desc;
    char buffer_cliente[256];
    int nBytes;
    
    // Identifica qual jogador esta thread representa
    int id_jogador;
    pthread_mutex_lock(&mutex_estado_jogo);
    jogadores_conectados++;
    id_jogador = jogadores_conectados; // O primeiro a ligar é 1, o segundo é 2
    printf("Jogador %d conectou.\n", id_jogador);
    pthread_mutex_unlock(&mutex_estado_jogo);

    // TODO: Adicionar lógica de posicionamento de navios aqui
    // (O servidor deve esperar que o cliente envie as posições)
    
    // Loop principal do jogo (simplificado)
    while ((nBytes = recv(client_sock, buffer_cliente, sizeof(buffer_cliente), 0)) > 0) {
        buffer_cliente[nBytes] = '\0'; // Adiciona terminador nulo

        // --- Início da Seção Crítica (Exclusão Mútua) ---
        pthread_mutex_lock(&mutex_estado_jogo);

        // Verifica se é a vez deste jogador
        if (id_jogador == vez_do_jogador) {
            printf("Jogador %d jogou: %s\n", id_jogador, buffer_cliente);
            
            // TODO: Processar a jogada (buffer_cliente contém "B10", por ex.)
            // 1. Converter "B10" para matriz[10][1]
            // 2. Verificar no tabuleiro_defesa do OUTRO jogador
            // 3. Atualizar o tabuleiro_ataque do jogador ATUAL
            // 4. Enviar a resposta (X, O) de volta para AMBOS os clientes
            
            // Passa a vez
            vez_do_jogador = (id_jogador == 1) ? 2 : 1; 
            
            // Envia resposta (exemplo simples)
            char* resposta = "Jogada processada.";
            send(client_sock, resposta, strlen(resposta), 0);

        } else {
            // Não é a vez do jogador
            char* resposta = "Nao e a sua vez.";
            send(client_sock, resposta, strlen(resposta), 0);
        }
        
        pthread_mutex_unlock(&mutex_estado_jogo);
        // --- Fim da Seção Crítica ---
    }

    printf("Jogador %d desconectou.\n", id_jogador);
    closesocket(client_sock);
    return 0;
}


// --- Documentação: Função Principal (main) ---
// Configura o servidor, aceita conexões e inicia as threads.
int main() {
    WSADATA wsa;
    SOCKET server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    int c = sizeof(struct sockaddr_in);
    
    // 1. Inicializa o Mutex
    pthread_mutex_init(&mutex_estado_jogo, NULL);

    // 2. Inicializa os Tabuleiros (Memória Compartilhada)
    printf("Inicializando tabuleiros do servidor...\n");
    inicializar_tabuleiro(jogador1.tabuleiro_defesa);
    inicializar_tabuleiro(jogador1.tabuleiro_ataque);
    inicializar_tabuleiro(jogador2.tabuleiro_defesa);
    inicializar_tabuleiro(jogador2.tabuleiro_ataque);

    // 3. Inicializa o Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Falha ao inicializar Winsock: %d\n", WSAGetLastError());
        return 1;
    }

    // 4. Cria o Socket do Servidor
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Nao foi possivel criar socket: %d\n", WSAGetLastError());
        return 1;
    }

    // 5. Prepara a estrutura sockaddr_in
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8888); // Porta do jogo

    // 6. Bind (Liga) o socket à porta
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind falhou: %d\n", WSAGetLastError());
        return 1;
    }

    // 7. Listen (Aguarda conexões)
    listen(server_sock, 2); // Ouve até 2 jogadores

    // 8. Aceita conexões e cria threads
    printf("Aguardando 2 jogadores se conectarem...\n");
    while (jogadores_conectados < 2) {
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &c);
        if (client_sock == INVALID_SOCKET) {
            printf("Accept falhou: %d\n", WSAGetLastError());
            continue;
        }
        
        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_client, (void*)&client_sock) < 0) {
            perror("Nao foi possivel criar a thread");
            return 1;
        }
    }

    printf("Jogo iniciado! Ambos os jogadores estao conectados.\n");
    
    // O servidor agora fica a correr (as threads tratam do jogo)
    // Para um servidor real, a thread principal esperaria as threads do jogo terminarem
    while(1) {
        // Mantém o servidor vivo
    }
    
    pthread_mutex_destroy(&mutex_estado_jogo);
    closesocket(server_sock);
    WSACleanup();
    return 0;
}
