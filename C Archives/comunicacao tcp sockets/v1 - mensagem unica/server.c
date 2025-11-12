#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>

int main() {
    WSADATA winsocketsDados;
    int temp;
    // Inicializa a biblioteca Winsock
    // MAKEWORD(2, 2) versão 2.2 da winsock
    // winsocketsDados dados sobre a conexão
    temp = WSAStartup(MAKEWORD(2, 2), &winsocketsDados);
    if (temp != 0) {
        printf("WSAStartup falhou: %d\n", temp);
        return 1;
    } else {
        printf("WSAStartup carregado com sucesso\n");
    }





    //Criar um socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //AF_INET indica que é endereço IPv4
    //AF_INET6 indica que é o endereço ipv6
    //SOCK_STREAM indica que é fluxo TCP
    //SOCK_DGRAM indica que o fluxo é UDP
    //IPPROTO_TCP indica que usa o protocolo TCP
    //IPPROTO_UDP indica que usa o protocolo UDP
    if (sock == INVALID_SOCKET) {
        printf("Erro ao criar o socket: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    } else {
        printf("Socket criado com sucesso\n");
    }





    // bind no socket
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;  // Associa a qualquer endereço IP disponível
    server.sin_port = htons(51171); // Porta

    if (bind(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Erro ao associar o socket: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
    return 1;
    } else {
        printf("Bind realizado com sucesso\n");
    }



    // modo escuta
    if (listen(sock, SOMAXCONN) == SOCKET_ERROR) { // SOMAXCONN indica o número maximo de conexoes
        printf("Erro ao colocar o socket em estado de escuta: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    } else {
        printf("Listen realizado com sucesso\n");
    }





    //aceitar conexao
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
    int bytesReceived = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
    if (bytesReceived == SOCKET_ERROR) {
        printf("Erro ao receber dados: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    } else {
        recvBuffer[bytesReceived] = '\0';  // Adiciona um terminador nulo ao final dos dados recebidos
        printf("Recebido %d bytes do servidor: %s\n", bytesReceived, recvBuffer);
    }


    getchar();




    // Fechar o socket criado
    closesocket(sock);

    // Finalizar a biblioteca Winsock
    WSACleanup();
    return 0;
}
