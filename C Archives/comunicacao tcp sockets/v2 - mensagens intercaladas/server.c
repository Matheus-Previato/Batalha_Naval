#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>

int main() {
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

    while (1) {
        bytesReceived = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
        if (bytesReceived == SOCKET_ERROR) {
            printf("Erro ao receber dados: %d\n", WSAGetLastError());
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        } else if (bytesReceived == 0) {
            printf("Conexão fechada pelo cliente\n");
            break;
        } else {
            recvBuffer[bytesReceived] = '\0';
            printf("Recebido: %s\n", recvBuffer);
            if (strcmp(recvBuffer, "exit") == 0) {
                printf("Cliente pediu para fechar a conexão\n");
                break;
            }
        }

        printf("Digite a mensagem para enviar ao cliente: ");
        fgets(sendBuffer, sizeof(sendBuffer), stdin);
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
    }

    closesocket(clientSocket);
    closesocket(sock);
    WSACleanup();
    return 0;
}
