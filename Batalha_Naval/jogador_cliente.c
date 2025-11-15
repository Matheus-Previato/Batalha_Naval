/*
 * JOGADOR-CLIENTE 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <pthread.h>
#include <ctype.h>
#include <windows.h>
#include <locale.h>

#pragma comment(lib, "ws2_32.lib")

#define TAMANHO 15
const char AGUA = '~';
const char NAVIO = 'N';
const char TIRO_CERTEIRO = 'X';
const char TIRO_AGUA = 'O';

//Structs/Objetos
typedef struct {
    char tabuleiro_defesa[TAMANHO][TAMANHO];
    char tabuleiro_ataque[TAMANHO][TAMANHO];
} Jogador;

typedef struct {
    char nome[30];
    int tamanho;
    int linhas[5];
    int colunas[5];
    int atingido[5];
} NavioInstancia;

typedef struct {
    char nome[30];
    int tamanho;
} NavioTipo;

NavioTipo frota[] = {
    {"Porta-aviões", 5}, {"Encouracado", 4}, {"Cruzador", 3},
    {"Submarino", 3}, {"Contratorpedeiro", 2}
};
int num_navios = sizeof(frota) / sizeof(frota[0]);

Jogador meu_jogador;
SOCKET servidor_sock = INVALID_SOCKET;
volatile int minha_vez = 1; 
NavioInstancia navios_posicionados[10];
int total_navios = 0;

//Funções Auxiliares
void remover_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') str[len - 1] = '\0';
}

//Funções referentes ao Tabuleiro
void inicializar_tabuleiro(char tabuleiro[TAMANHO][TAMANHO]) {
    for (int i = 0; i < TAMANHO; i++)
        for (int j = 0; j < TAMANHO; j++)
            tabuleiro[i][j] = AGUA;
}

void mostrar_tabuleiro(const char *titulo, char tabuleiro[TAMANHO][TAMANHO]) {
    printf("\n%s\n", titulo);
    printf("    ");
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

int converter_coordenada(const char* coord_str, int* linha, int* coluna) {
    if (!coord_str) return 0;
    int len = (int)strlen(coord_str);
    if (len < 2 || len > 3) return 0;
    char colChar = toupper(coord_str[0]);
    if (colChar < 'A' || colChar > 'A' + TAMANHO - 1) return 0;
    *coluna = colChar - 'A';
    int num = atoi(coord_str + 1);
    if (num < 0 || num >= TAMANHO) return 0;
    *linha = num;
    return 1;
}

int verificar_posicao(char tabuleiro[TAMANHO][TAMANHO], int l, int c, char orientacao, int tamanho) {
    if (orientacao == 'H') {
        if (c + tamanho > TAMANHO) return 0;
        for (int i = 0; i < tamanho; i++) if (tabuleiro[l][c+i] == NAVIO) return 0;
    } else if (orientacao == 'V') {
        if (l + tamanho > TAMANHO) return 0;
        for (int i = 0; i < tamanho; i++) if (tabuleiro[l+i][c] == NAVIO) return 0;
    } else return 0;
    return 1;
}

void realizar_posicionamento(char tabuleiro[TAMANHO][TAMANHO], int l, int c, char orientacao, int tamanho, const char* nome) {
    NavioInstancia n;
    strcpy(n.nome, nome);
    n.tamanho = tamanho;
    for (int i = 0; i < tamanho; i++) {
        if (orientacao == 'H') {
            tabuleiro[l][c+i] = NAVIO;
            n.linhas[i] = l;
            n.colunas[i] = c+i;
        } else {
            tabuleiro[l+i][c] = NAVIO;
            n.linhas[i] = l+i;
            n.colunas[i] = c;
        }
        n.atingido[i] = 0;
    }
    navios_posicionados[total_navios++] = n;
}

//Display dos navios
void posicionar_navios(char tabuleiro[TAMANHO][TAMANHO]) {
    total_navios = 0;
    char input_buffer[256], coord_str[4], orientacao_char;
    int linha, coluna; char orientacao;

    for (int i = 0; i < num_navios; i++) {
        int ok = 0;
        do {
            system("cls");
            mostrar_tabuleiro("=== Tabuleiro de Defesa (Posicionamento) ===", tabuleiro);
            printf("Posicione o seu: %s (Tamanho: %d)\n", frota[i].nome, frota[i].tamanho);
            printf("Digite a coordenada inicial (ex: B10): ");
            fgets(input_buffer, sizeof(input_buffer), stdin);
            sscanf(input_buffer, "%3s", coord_str);
            printf("Digite a orientação (H/V): ");
            fgets(input_buffer, sizeof(input_buffer), stdin);
            sscanf(input_buffer, " %c", &orientacao_char);
            orientacao = toupper(orientacao_char);
            if (!converter_coordenada(coord_str, &linha, &coluna)) {
                printf("Coordenada inválida.\n");
                Sleep(1000);
                continue;
            }
            if (verificar_posicao(tabuleiro, linha, coluna, orientacao, frota[i].tamanho)) {
                realizar_posicionamento(tabuleiro, linha, coluna, orientacao, frota[i].tamanho, frota[i].nome);
                ok = 1;
            } else {
                printf("Posição inválida ou conflito com outro navio.\n");
                Sleep(1000);
            }
        } while (!ok);
    }

    system("cls");
    printf("--- FROTA POSICIONADA! ---\n");
    mostrar_tabuleiro("=== Tabuleiro de Defesa ===", tabuleiro);
    printf("Pressione Enter para continuar...");
    fgets(input_buffer, sizeof(input_buffer), stdin);
    system("cls");
}

//Verificações de derrota, afundamento...
int verificar_derrota(char tabuleiro[TAMANHO][TAMANHO]) {
    for (int i = 0; i < TAMANHO; i++)
        for (int j = 0; j < TAMANHO; j++)
            if (tabuleiro[i][j] == NAVIO) return 0;
    return 1;
}

char* verificar_afundamento(int linha, int coluna) {
    for (int i = 0; i < total_navios; i++) {
        for (int j = 0; j < navios_posicionados[i].tamanho; j++) {
            if (navios_posicionados[i].linhas[j] == linha &&
                navios_posicionados[i].colunas[j] == coluna) {
                navios_posicionados[i].atingido[j] = 1;
                int completo = 1;
                for (int k = 0; k < navios_posicionados[i].tamanho; k++) {
                    if (navios_posicionados[i].atingido[k] == 0) { completo = 0; break; }
                }
                if (completo) return navios_posicionados[i].nome;
            }
        }
    }
    return NULL;
}

//Construtor das mensagens para serem enviadas via sockets
void montar_mensagem(char *buffer, char tipo, int jogador, const char *coord, char acertou, char afundou) {
    sprintf(buffer, "%c/%d/%s/%c/%c", tipo, jogador, coord, acertou, afundou);
}

int interpretar_mensagem(const char *mensagem, char *tipo, int *jogador, char *coord, char *acertou, char *afundou) {
    return sscanf(mensagem, "%c/%d/%[^/]/%c/%c", tipo, jogador, coord, acertou, afundou) == 5;
}


// -------- Thread de recebimento --------
void *receive_thread_cliente(void *arg) {
    char buf[256];
    int n;
    while ((n = recv(servidor_sock, buf, sizeof(buf)-1, 0)) > 0) {
        buf[n] = '\0';
        char tipo, coord[16], acertou, afundou;
        int jogador;
        if (!interpretar_mensagem(buf, &tipo, &jogador, coord, &acertou, &afundou)) {
            printf("Mensagem inválida: %s\n", buf);
            continue;
        }

        if (tipo == 'A' && jogador == 1) {
            // Ataque do servidor (1) ao tabuleiro de defesa do cliente (2)
            int linha, coluna;
            char *navio = NULL;
            if (!converter_coordenada(coord, &linha, &coluna)) continue;

            char r_acerto = 'N', r_afund = '-';

            if (meu_jogador.tabuleiro_defesa[linha][coluna] == NAVIO) {
                meu_jogador.tabuleiro_defesa[linha][coluna] = TIRO_CERTEIRO;
                r_acerto = 'S';
                navio = verificar_afundamento(linha, coluna);
                if (navio) {
                    r_afund = 'A';
                }
            } else if (meu_jogador.tabuleiro_defesa[linha][coluna] == AGUA) {
                meu_jogador.tabuleiro_defesa[linha][coluna] = TIRO_AGUA;
                r_acerto = 'N';
            } else {
                r_acerto = 'N';
            }

            mostrar_tabuleiro("=== Tabuleiro de Defesa (Atualizado) ===", meu_jogador.tabuleiro_defesa);

            if (r_acerto == 'S') {
                printf("\n!! Seu navio foi ATINGIDO em %s!\n", coord);
                if (navio) {
                    printf("!! Seu navio %s foi AFUNDADO!\n", navio);
                }
            } else if (meu_jogador.tabuleiro_defesa[linha][coluna] == TIRO_AGUA) {
                 printf("\n>> O oponente atirou na água em %s.\n", coord);
            } else {
                 printf("\n>> O oponente repetiu jogada em %s.\n", coord);
            }
            
            if (r_acerto == 'S' && verificar_derrota(meu_jogador.tabuleiro_defesa)) {
                printf("Todos os seus navios foram destruídos! Você perdeu.\n");
                char s[64]; montar_mensagem(s, 'S', 2, "-", 'N', 'A');
                send(servidor_sock, s, strlen(s), 0);
                minha_vez = 0; 
                break;
            }

            char resp[256];
            montar_mensagem(resp, 'R', 2, coord, r_acerto, r_afund);
            send(servidor_sock, resp, strlen(resp), 0);

            if (r_acerto == 'S') {
                minha_vez = 1; 
                printf("O oponente acertou. Vez dele de novo.\n");
            } else {
                minha_vez = 2; // Passa a vez para o cliente
            }
        }
        else if (tipo == 'R' && jogador == 1) {
            int linha, coluna;
            if (!converter_coordenada(coord, &linha, &coluna)) continue;

            if (acertou == 'S') {
                meu_jogador.tabuleiro_ataque[linha][coluna] = TIRO_CERTEIRO;
                minha_vez = 2; // Mantém a vez (do cliente)
            } else {
                meu_jogador.tabuleiro_ataque[linha][coluna] = TIRO_AGUA;
                minha_vez = 1; // Passa a vez (para o servidor)
            }

            mostrar_tabuleiro("=== Tabuleiro de Ataque (Atualizado) ===", meu_jogador.tabuleiro_ataque);
            mostrar_tabuleiro("=== Tabuleiro de Defesa ===", meu_jogador.tabuleiro_defesa);

            if (acertou == 'S') {
                printf("\n!! Você ACERTOU em %s!\n", coord);
                if (afundou == 'A') {
                    printf("!! Você AFUNDOU um navio!\n");
                }
                printf("Você acertou! Jogue novamente.\n"); 
            } else {
                printf("\n>> Você ERROU em %s. Água.\n", coord);
                printf("Vez do oponente.\n");
            }
        }
        else if (tipo == 'S') {
            printf("\n*** O oponente foi derrotado! Você VENCEU! ***\n");
            minha_vez = 0; 
            break;
        }
    }

    if (n <= 0) {
        printf("Oponente (Servidor) desconectado.\n");
        minha_vez = 0; 
    }
    return 0;
}

// -------- MAIN --------
int main() {
    setlocale(LC_ALL, "Portuguese");

    WSADATA wsa;
    struct sockaddr_in server_addr;
    pthread_t recv_thread;
    char input_buffer[256];

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Falha ao inicializar Winsock: %d\n", WSAGetLastError());
        return 1;
    }

    servidor_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (servidor_sock == INVALID_SOCKET) {
        printf("Não foi possível criar socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    server_addr.sin_port = htons(8888);

    if (connect(servidor_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Conexão falhou. Verifique se o servidor está rodando.\n");
        closesocket(servidor_sock);
        WSACleanup();
        return 1;
    }

    printf("Conectado ao servidor! O jogo vai começar.\n");

    inicializar_tabuleiro(meu_jogador.tabuleiro_defesa);
    inicializar_tabuleiro(meu_jogador.tabuleiro_ataque);
    posicionar_navios(meu_jogador.tabuleiro_defesa);

    mostrar_tabuleiro("=== Tabuleiro de Ataque ===", meu_jogador.tabuleiro_ataque);
    mostrar_tabuleiro("=== Tabuleiro de Defesa ===", meu_jogador.tabuleiro_defesa);
    

    printf("O servidor começa jogando. Aguarde sua vez.\n");

    if (pthread_create(&recv_thread, NULL, receive_thread_cliente, NULL) != 0) {
        perror("Não foi possível criar a thread de recebimento");
        closesocket(servidor_sock);
        WSACleanup();
        return 1;
    }

    // Loop de envio (main thread)
    while (minha_vez != 0) { 
        if (minha_vez == 2) { // Somente joga se for a vez do Jogador 2 (Cliente)
            printf("\nSua vez de jogar. Digite a jogada (ex: B10): ");
            fflush(stdout);

            if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
                break; 
            }
            remover_newline(input_buffer);

            if (minha_vez == 0) break;
            if (strlen(input_buffer) == 0) continue; 

            int linha, coluna;
            if (!converter_coordenada(input_buffer, &linha, &coluna)) {
                printf("Coordenada inválida! Tente novamente.\n");
                continue;
            }
            
            if (meu_jogador.tabuleiro_ataque[linha][coluna] == TIRO_CERTEIRO ||
                meu_jogador.tabuleiro_ataque[linha][coluna] == TIRO_AGUA) {
                printf("Você já jogou em %s! Escolha outra coordenada.\n", input_buffer);
                continue;
            }

            char msg[256];
            montar_mensagem(msg, 'A', 2, input_buffer, '-', '-');
            if (send(servidor_sock, msg, strlen(msg), 0) < 0) {
                printf("Envio falhou.\n");
                break;
            }
        } else {
            // Se minha_vez == 1 (vez do servidor) ou 0 (fim de jogo),
            // o cliente dorme e espera a vez mudar.
            Sleep(100);
        }
    }
    
    // Encerramento
    printf("Encerrando o jogo...\n");
    pthread_join(recv_thread, NULL); 
    shutdown(servidor_sock, SD_BOTH);
    closesocket(servidor_sock);
    WSACleanup();
    return 0;
}
