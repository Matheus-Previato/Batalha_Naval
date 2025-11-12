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


    //criar socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        printf("Erro ao criar o socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    } else {
        printf("Socket criado com sucesso\n");
    }


    //conectar no cliente
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Endereço IP do servidor
    serverAddr.sin_port = htons(51171);  // Porta do servidor

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Erro ao conectar ao servidor: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    } else {
        printf("Conectado ao servidor\n");
    }




    //enviar mensagem
    char texto[100];
    printf("digite oque quer enviar: ");
    fgets(texto, sizeof(texto), stdin);
    //scanf("%s", &texto);
    int bytesSent = send(clientSocket, texto, strlen(texto), 0);
    if (bytesSent == SOCKET_ERROR) {
        printf("Erro ao enviar dados: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    } else {
        printf("Enviado %d bytes para o servidor\n", bytesSent);
    }


    getchar();

    //usar client/server




    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
