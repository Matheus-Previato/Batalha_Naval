#include <winsock2.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main() {
    WSADATA winsocketsDados;
    if (WSAStartup(MAKEWORD(2, 2), &winsocketsDados) != 0) {
        printf("Falha ao inicializar o Winsock\n");
        return 1;
    } else {
        printf("WSAStartup carregado com sucesso\n");
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        printf("Erro ao criar o socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    } else {
        printf("Socket criado com sucesso\n");
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(51171);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Erro ao conectar ao servidor: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    } else {
        printf("Conectado ao servidor\n");
    }

    char sendBuffer[512];
    char recvBuffer[512];
    int bytesReceived;

    while (1) {
        printf("Digite a mensagem para enviar ao servidor: ");
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
            printf("Encerrando a conexão com o servidor\n");
            break;
        }

        bytesReceived = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
        if (bytesReceived == SOCKET_ERROR) {
            printf("Erro ao receber dados: %d\n", WSAGetLastError());
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        } else if (bytesReceived == 0) {
            printf("Conexão fechada pelo servidor\n");
            break;
        } else {
            recvBuffer[bytesReceived] = '\0';
            printf("Recebido: %s\n", recvBuffer);
            if (strcmp(recvBuffer, "exit") == 0) {
                printf("Servidor pediu para fechar a conexão\n");
                break;
            }
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
