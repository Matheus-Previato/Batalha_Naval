#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>

void error_handling(char *message) {
    fprintf(stderr, "%s: %d\n", message, WSAGetLastError());
    WSACleanup();
    exit(1);
}

int main() {
    WSADATA winsocketsDados;
    int temp;

    temp = WSAStartup(MAKEWORD(2, 2), &winsocketsDados);
    if (temp != 0) {
        error_handling("WSAStartup falhou");
    } else {
        printf("WSAStartup carregado com sucesso\n");
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        error_handling("Erro ao criar o socket");
    } else {
        printf("Socket criado com sucesso\n");
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(51171);

    if (bind(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        error_handling("Erro ao associar o socket");
    } else {
        printf("Bind realizado com sucesso\n");
    }

    if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
        error_handling("Erro ao colocar o socket em estado de escuta");
    } else {
        printf("Listen realizado com sucesso\n");
    }

    SOCKET clientSocket;
    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    clientSocket = accept(sock, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket == INVALID_SOCKET) {
        error_handling("Erro ao aceitar a conexão");
    } else {
        printf("Conexão aceita com sucesso\n");
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
                printf("Conexão fechada pelo cliente\n");
                break;
            } else {
                recvBuffer[bytesReceived] = '\0';
                printf("Cliente: %s\n", recvBuffer);
                if (strcmp(recvBuffer, "exit") == 0) {
                    printf("Cliente pediu para fechar a conexão\n");
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
                        printf("Encerrando a conexão com o cliente\n");
                        break;
                    }
                }
            }
        }
    }

    closesocket(clientSocket);
    closesocket(sock);
    WSACleanup();
    return 0;
}
