#include <winsock2.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

void error_handling(char *message) {
    fprintf(stderr, "%s: %d\n", message, WSAGetLastError());
    WSACleanup();
    exit(1);
}

int main() {
    WSADATA winsocketsDados;
    if (WSAStartup(MAKEWORD(2, 2), &winsocketsDados) != 0) {
        error_handling("Falha ao inicializar o Winsock");
    } else {
        printf("WSAStartup carregado com sucesso\n");
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        error_handling("Erro ao criar o socket");
    } else {
        printf("Socket criado com sucesso\n");
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(51171);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        error_handling("Erro ao conectar ao servidor");
    } else {
        printf("Conectado ao servidor\n");
    }

    fd_set readfds, writefds;
    char recvBuffer[512];
    char sendBuffer[512];
    int bytesReceived, bytesSent;

    while (1) {
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);

        FD_SET(clientSocket, &readfds);
        FD_SET(clientSocket, &writefds);

        int activity = select(0, &readfds, &writefds, NULL, NULL);
        if (activity == SOCKET_ERROR) {
            error_handling("Erro no select");
        }

        if (FD_ISSET(clientSocket, &readfds)) {
            bytesReceived = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
            if (bytesReceived == SOCKET_ERROR) {
                error_handling("Erro ao receber dados");
            } else if (bytesReceived == 0) {
                printf("Conexão fechada pelo servidor\n");
                break;
            } else {
                recvBuffer[bytesReceived] = '\0';
                printf("Servidor: %s\n", recvBuffer);
                if (strcmp(recvBuffer, "exit") == 0) {
                    printf("Servidor pediu para fechar a conexão\n");
                    break;
                }
            }
        }

        // Leitura da entrada padrão
        if (kbhit()) {
            if (fgets(sendBuffer, sizeof(sendBuffer), stdin) != NULL) {
                sendBuffer[strcspn(sendBuffer, "\n")] = 0;
                if (FD_ISSET(clientSocket, &writefds)) {
                    bytesSent = send(clientSocket, sendBuffer, strlen(sendBuffer), 0);
                    if (bytesSent == SOCKET_ERROR) {
                        error_handling("Erro ao enviar dados");
                    }
                    if (strcmp(sendBuffer, "exit") == 0) {
                        printf("Encerrando a conexão com o servidor\n");
                        break;
                    }
                }
            }
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
