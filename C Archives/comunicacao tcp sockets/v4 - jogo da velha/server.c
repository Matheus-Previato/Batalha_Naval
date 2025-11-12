#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>



char tab[3][3];
int vetor[10];
char ganhador = ' ';
int jogadas = 0;
int reiniciar = 0;

void iniciar(){
    int i,j;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            tab[i][j] = ' ';
        }
    }
    for (i = 0; i < 10; i++) {
        vetor[i] = 0;
    }
}

verificaGanhador() {

    // Verifica as linhas
    for (int i = 0; i < 3; i++) {
        if (tab[i][0] != ' ' && tab[i][0] == tab[i][1] && tab[i][1] == tab[i][2]) {
            ganhador = tab[i][0];
        }
    }

    // Verifica as colunas
    for (int i = 0; i < 3; i++) {
        if (tab[0][i] != ' ' && tab[0][i] == tab[1][i] && tab[1][i] == tab[2][i]) {
            ganhador = tab[0][i];
        }
    }

    // Verifica a diagonal principal
    if (tab[0][0] != ' ' && tab[0][0] == tab[1][1] && tab[1][1] == tab[2][2]) {
        ganhador = tab[0][0];
    }

    // Verifica a diagonal secundária
    if (tab[0][2] != ' ' && tab[0][2] == tab[1][1] && tab[1][1] == tab[2][0]) {
        ganhador = tab[0][2];
    }
}

int main() {
    iniciar();
    WSADATA winsocketsDados;
    int temp;

    temp = WSAStartup(MAKEWORD(2, 2), &winsocketsDados);
    if (temp != 0) {
        printf("WSAStartup falhou: %d\n", temp);
        return 1;
    } else {
        printf("WSAStartup carregado com sucesso\n");
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        printf("Erro ao criar o socket: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    } else {
        printf("Socket criado com sucesso\n");
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(51171);

    if (bind(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Erro ao associar o socket: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    } else {
        printf("Bind realizado com sucesso\n");
    }

    if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
        printf("Erro ao colocar o socket em estado de escuta: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    } else {
        printf("Listen realizado com sucesso\n");
    }

    SOCKET clientSocket;
    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    clientSocket = accept(sock, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket == INVALID_SOCKET) {
        printf("Erro ao aceitar a conexão: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    } else {
        printf("Conexão aceita com sucesso\n");
    }

    char recvBuffer[512];
    int bytesReceived;
    char sendBuffer[512];
    system("cls");
    while (1) {
        reiniciar = 0;
        printf("Aguardando jogada do adversario...\n");
        bytesReceived = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
        if (bytesReceived == SOCKET_ERROR) {
            printf("Erro ao receber dados: %d\n", WSAGetLastError());
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        } else if (bytesReceived == 0) {
            printf("Tabuleiro \n");
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++){
                    if (j == 2) {
                        printf("%c ",tab[i][j]);
                    } else {
                        printf("%c | ",tab[i][j]);
                    }

                }
                printf("\n");
            }
            printf("Voce perdeu\n");
            break;
        } else {
            int posRcv = atoi(recvBuffer);
            recvBuffer[bytesReceived] = '\0';
            //printf("Recebido: %s\n", recvBuffer);
            if (strcmp(recvBuffer, "exit") == 0) {
                printf("Servidor pediu para fechar a conexão\n");
                break;
            }
            vetor[posRcv] = 1;
            switch (posRcv) {
            case 1:
                tab[0][0] = 'X';
                break;
            case 2:
                tab[0][1] = 'X';
                break;
            case 3:
                tab[0][2] = 'X';
                break;
            case 4:
                tab[1][0] = 'X';
                break;
            case 5:
                tab[1][1] = 'X';
                break;
            case 6:
                tab[1][2] = 'X';
                break;
            case 7:
                tab[2][0] = 'X';
                break;
            case 8:
                tab[2][1] = 'X';
                break;
            case 9:
                tab[2][2] = 'X';
                break;
            }
        jogadas++;
        verificaGanhador();
        if ((jogadas == 9) && (ganhador ==' ')) {
            printf("Empate...reiniciando jogo...\n");
            reiniciar = 1;
            ganhador = ' ';
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    tab[i][j] = ' ';
                }
            }
            for (int i = 0; i < 10; i++) {
                vetor[i] = 0;
            }
        }
        if (ganhador != ' ') {
            printf("Tabuleiro \n");
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++){
                    if (j == 2) {
                        printf("%c ",tab[i][j]);
                    } else {
                        printf("%c | ",tab[i][j]);
                    }
                }
                printf("\n");
            }
            if (ganhador == 'X') {
                printf("Voce perdeu...\n");
                break;
            } else {
                printf("Voce ganhou...\n");
                break;
            }
        }

            //printf("Recebido: %s\n", recvBuffer);
            if (strcmp(recvBuffer, "exit") == 0) {
                printf("Cliente pediu para fechar a conexão\n");
                break;
            }
        }
        if (reiniciar != 1) {
        printf("Tabuleiro \n");
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++){
                if (j == 2) {
                    printf("%c ",tab[i][j]);
                } else {
                    printf("%c | ",tab[i][j]);
                }

            }
            printf("\n");
        }
        int pos;
        int ok = 0;
        while (ok != 1) {
        printf("Sua vez de jogar...\n");
        printf("Escolha uma posicao para jogar: 1 - 9: ");
        scanf("%d",&pos);
        if (vetor[pos] == 0) {
            vetor[pos] = 1;
            ok = 1;
            switch (pos) {
            case 1:
                tab[0][0] = 'O';
                break;
            case 2:
                tab[0][1] = 'O';
                break;
            case 3:
                tab[0][2] = 'O';
                break;
            case 4:
                tab[1][0] = 'O';
                break;
            case 5:
                tab[1][1] = 'O';
                break;
            case 6:
                tab[1][2] = 'O';
                break;
            case 7:
                tab[2][0] = 'O';
                break;
            case 8:
                tab[2][1] = 'O';
                break;
            case 9:
                tab[2][2] = 'O';
                break;
            }
        } else {
            printf("Jogada nao permitida, escolha novamente...\n");
        }
        }
        jogadas++;
        system("cls");
        fflush(stdin);
        sprintf(sendBuffer, "%d", pos);

        //printf("Digite a mensagem para enviar ao cliente: ");
        //fgets(sendBuffer, sizeof(sendBuffer), stdin);
        sendBuffer[strcspn(sendBuffer, "\n")] = 0;

        int bytesSent = send(clientSocket, sendBuffer, strlen(sendBuffer), 0);
        if (bytesSent == SOCKET_ERROR) {
            printf("Erro ao enviar dados: %d\n", WSAGetLastError());
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }
        if (strcmp(sendBuffer, "exit") == 0) {
            printf("Encerrando a conexão com o cliente\n");
            break;
        }
        verificaGanhador();
        if (ganhador != ' ') {
            printf("Tabuleiro \n");
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++){
                    if (j == 2) {
                        printf("%c ",tab[i][j]);
                    } else {
                        printf("%c | ",tab[i][j]);
                    }
                }
                printf("\n");
            }
            if (ganhador == 'X') {
                printf("Voce perdeu...\n");
                break;
            } else {
                printf("Voce ganhou...\n");
                break;
            }
        }
        }
    }

    closesocket(clientSocket);
    closesocket(sock);
    WSACleanup();
    getch();
    return 0;
}
