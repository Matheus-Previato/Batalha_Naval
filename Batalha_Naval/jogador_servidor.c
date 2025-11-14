/*
 * JOGADOR-SERVIDOR (jogador_servidor.c)
 * Este é o Anfitrião (Host) do jogo.
 *
 * VERSÃO: Correção do bug de "estado" (reset de variáveis).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <pthread.h>
#include <ctype.h>

// --- FUNÇÃO DE AJUDA PARA LIMPAR BUFFER ---
// Remove o newline (\n) do final de uma string lida por fgets
void remover_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

// --- Constantes do Jogo ---
#define TAMANHO 15
const char AGUA = '~';
const char NAVIO = 'N';
const char TIRO_CERTEIRO = 'X';
const char TIRO_AGUA = 'O';

// --- Estrutura do Jogador ---
typedef struct {
    char tabuleiro_defesa[TAMANHO][TAMANHO];
    char tabuleiro_ataque[TAMANHO][TAMANHO];
} Jogador;

// --- Variáveis Globais (Estado do Jogo) ---
Jogador meu_jogador;
SOCKET oponente_sock; 
int minha_vez = 1;

// --- Funções do Tabuleiro (Inicializar e Mostrar) ---
void inicializar_tabuleiro(char tabuleiro[TAMANHO][TAMANHO]) {
    for (int i = 0; i < TAMANHO; i++)
        for (int j = 0; j < TAMANHO; j++)
            tabuleiro[i][j] = AGUA;
}

void mostrar_tabuleiro(char tabuleiro[TAMANHO][TAMANHO]) {
    printf("\n   "); 
    for (int j = 0; j < TAMANHO; j++) printf("%c  ", 'A' + j);
    printf("\n");
    for (int i = 0; i < TAMANHO; i++) {
        if (i < 10) printf("%d  ", i);
        else printf("%d ", i);
        for (int j = 0; j < TAMANHO; j++) printf("%c  ", tabuleiro[i][j]);
        printf("\n"); 
    }
    printf("\n"); 
}

// --- Funções de Posicionamento de Navios ---

typedef struct {
    char nome[30];
    int tamanho;
} NavioTipo;

NavioTipo frota[] = {
    {"Porta-avioes", 5}, {"Encouracado", 4}, {"Cruzador", 3},
    {"Submarino", 3}, {"Contratorpedeiro", 2}
};
int num_navios = sizeof(frota) / sizeof(frota[0]);

int converter_coordenada(char* coord_str, int* linha, int* coluna) {
    if (coord_str == NULL || (strlen(coord_str) != 2 && strlen(coord_str) != 3)) return 0;

    *coluna = toupper(coord_str[0]) - 'A';
    *linha = atoi(&coord_str[1]);

    if (*coluna < 0 || *coluna >= TAMANHO || *linha < 0 || *linha >= TAMANHO) {
        return 0;
    }
    return 1;
}

int verificar_posicao(char tabuleiro[TAMANHO][TAMANHO], int l, int c, char orientacao, int tamanho) {
    if (orientacao == 'H') {
        if (c + tamanho > TAMANHO) {
            printf("Erro: O navio sai do tabuleiro!\n");
            return 0;
        }
        for (int i = 0; i < tamanho; i++) {
            if (tabuleiro[l][c + i] == NAVIO) {
                printf("Erro: O navio sobrepoe outro!\n");
                return 0;
            }
        }
    } else if (orientacao == 'V') {
        if (l + tamanho > TAMANHO) {
            printf("Erro: O navio sai do tabuleiro!\n");
            return 0;
        }
        for (int i = 0; i < tamanho; i++) {
            if (tabuleiro[l + i][c] == NAVIO) {
                printf("Erro: O navio sobrepoe outro!\n");
                return 0;
            }
        }
    } else {
        printf("Erro: Orientacao invalida (Use 'H' ou 'V').\n");
        return 0; 
    }
    return 1;
}

void realizar_posicionamento(char tabuleiro[TAMANHO][TAMANHO], int l, int c, char orientacao, int tamanho) {
    for (int i = 0; i < tamanho; i++) {
        if (orientacao == 'H') {
            tabuleiro[l][c + i] = NAVIO;
        } else {
            tabuleiro[l + i][c] = NAVIO;
        }
    }
}

// --- Função de Posicionamento (VERSÃO FINAL CORRIGIDA) ---
void posicionar_navios(char tabuleiro[TAMANHO][TAMANHO]) {
    char input_buffer[256]; // Buffer de leitura único
    char coord_str[4];      // "O14" + \0
    char orientacao_char;
    int linha, coluna;
    char orientacao;
    int posicionado_corretamente;
    int coord_valida, pos_valida;

    for (int i = 0; i < num_navios; i++) {
        posicionado_corretamente = 0;
        
        do {
            // --- A CORREÇÃO ESTÁ AQUI ---
            // Inicializa as variáveis em CADA tentativa
            // para apagar dados "lixo" ou antigos.
            strcpy(coord_str, ""); // Esvazia a string de coordenada
            orientacao_char = ' '; // Reseta a orientação
            linha = -1;            // Reseta linha/coluna
            coluna = -1;
            orientacao = ' ';
            // --- FIM DA CORREÇÃO ---
            
            coord_valida = 0;
            pos_valida = 0;
            
            system("cls"); 
            printf("--- SEU TABULEIRO DE DEFESA ---\n");
            mostrar_tabuleiro(tabuleiro);
            
            printf("Posicione o seu: %s (Tamanho: %d)\n", frota[i].nome, frota[i].tamanho);
            
            // 1. Pede a coordenada
            printf("Digite a coordenada inicial (ex: B10): ");
            if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) continue;
            sscanf(input_buffer, "%3s", coord_str); // Extrai os 3 primeiros chars

            // 2. Pede a orientação
            printf("Digite a orientacao (H para Horizontal, V para Vertical): ");
            if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) continue;
            sscanf(input_buffer, " %c", &orientacao_char); // Extrai o char
            orientacao = toupper(orientacao_char);

            // 3. Valida a coordenada
            coord_valida = converter_coordenada(coord_str, &linha, &coluna);
            if (!coord_valida) {
                printf("Coordenada invalida! Pressione Enter para tentar novamente...");
                fgets(input_buffer, sizeof(input_buffer), stdin); // Pausa
                continue; 
            }

            // 4. Valida a posição
            pos_valida = verificar_posicao(tabuleiro, linha, coluna, orientacao, frota[i].tamanho);
            if (pos_valida) {
                // 5. Posiciona
                realizar_posicionamento(tabuleiro, linha, coluna, orientacao, frota[i].tamanho);
                posicionado_corretamente = 1; 
            } else {
                printf("Posicao invalida! Pressione Enter para tentar novamente...");
                fgets(input_buffer, sizeof(input_buffer), stdin); // Pausa
            }
            
        } while (posicionado_corretamente == 0);
    }
    
    system("cls");
    printf("--- FROTA POSICIONADA! ---\n");
    mostrar_tabuleiro(tabuleiro);
    printf("Pronto para o combate!\n");
    printf("Pressione Enter para continuar...");
    fgets(input_buffer, sizeof(input_buffer), stdin); // Pausa
    system("cls");
}

// --- Thread de Recebimento ---
void *receive_thread_servidor(void *arg) {
    char buffer_resposta[256];
    int nBytes;

    while ((nBytes = recv(oponente_sock, buffer_resposta, sizeof(buffer_resposta), 0)) > 0) {
        buffer_resposta[nBytes] = '\0';
        
        printf("\n[Oponente Jogou]: %s\n", buffer_resposta);
        
        // TODO: Processar a jogada do oponente
        
        minha_vez = 1; 
        printf("E a sua vez. Digite a jogada (ex: B10): ");
        fflush(stdout);
    }

    if (nBytes <= 0) {
        printf("Oponente desconectado.\n");
        closesocket(oponente_sock);
    }
    return 0;
}

// --- Função Principal (main) ---
int main() {
    WSADATA wsa;
    SOCKET server_sock;
    struct sockaddr_in server_addr, client_addr;
    int c = sizeof(struct sockaddr_in);
    pthread_t recv_thread;
    char input_buffer[256]; // Buffer de envio

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Falha ao inicializar Winsock: %d\n", WSAGetLastError());
        return 1;
    }

    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Nao foi possivel criar socket: %d\n", WSAGetLastError());
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8888);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind falhou: %d\n", WSAGetLastError());
        return 1;
    }
    listen(server_sock, 1);

    printf("Inicializando tabuleiros...\n");
    inicializar_tabuleiro(meu_jogador.tabuleiro_defesa);
    inicializar_tabuleiro(meu_jogador.tabuleiro_ataque);
    
    posicionar_navios(meu_jogador.tabuleiro_defesa);
    
    printf("Aguardando oponente se conectar (Jogador-Cliente)...\n");

    oponente_sock = accept(server_sock, (struct sockaddr *)&client_addr, &c);
    if (oponente_sock == INVALID_SOCKET) {
        printf("Accept falhou: %d\n", WSAGetLastError());
        return 1;
    }
    
    closesocket(server_sock);
    printf("Oponente conectado! O jogo vai comecar.\n");
    
    if (pthread_create(&recv_thread, NULL, receive_thread_servidor, NULL) < 0) {
        perror("Nao foi possivel criar a thread de recebimento");
        return 1;
    }

    printf("E a sua vez (Jogador-Servidor). Digite a jogada (ex: B10):\n");
    while (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
        remover_newline(input_buffer); // Remove o \n

        if (minha_vez == 1) {
            if (strlen(input_buffer) > 0) {
                if (send(oponente_sock, input_buffer, strlen(input_buffer), 0) < 0) {
                    printf("Envio falhou.\n");
                    break;
                }
                minha_vez = 2; 
                printf("Jogada enviada. Aguardando oponente...\n");
            }
        } else {
            printf("Nao e a sua vez! Aguarde o oponente.\n");
        }
    }

    closesocket(oponente_sock);
    WSACleanup();
    return 0;
}
