#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>


//VARIÁVEIS GLOBAIS DA CONEXAO
WSADATA winsocketsDados;
SOCKET sock;
struct sockaddr_in server;
SOCKET clientSocket;
struct sockaddr_in clientAddr;

char recvBuffer[512];
int bytesReceived;
char sendBuffer[512];





int iniciarBiblioteca(){
    if (WSAStartup(MAKEWORD(2, 2), &winsocketsDados) != 0) {
        printf("Falha ao inicializar o Winsock\n");
        return 1;
    } else {
        printf("WSAStartup carregado com sucesso\n");
    }
}

int iniciarSocket() {
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        printf("Erro ao criar o socket: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    } else {
        printf("Socket criado com sucesso\n");
    }
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
}

int esperarConexao() {
    if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
        printf("Erro ao colocar o socket em estado de escuta: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    } else {
        printf("Listen realizado com sucesso\n");
    }
    int clientAddrLen = sizeof(clientAddr);

    clientSocket = accept(sock, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket == INVALID_SOCKET) {
        printf("Erro ao aceitar a conexao: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    } else {
        printf("Conexao aceita com sucesso\n");
    }
}

int receberMensagem() {
    bytesReceived = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
    if (bytesReceived == SOCKET_ERROR) {
        printf("Erro ao receber dados: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    } else if (bytesReceived == 0) {
        printf("Conexao fechada pelo cliente\n");
        return -1;
    } else {
        recvBuffer[bytesReceived] = '\0';
        printf("Recebido: %s\n", recvBuffer);
        if (strcmp(recvBuffer, "exit") == 0) {
            printf("Cliente pediu para fechar a conexao\n");
            return -1;
        } else return 1;
    }
}

int enviarMensagem() {
    printf("Digite a mensagem para enviar ao cliente: ");
    fgets(sendBuffer, sizeof(sendBuffer), stdin);
    sendBuffer[strcspn(sendBuffer, "\n")] = 0;

    int bytesSent = send(clientSocket, sendBuffer, strlen(sendBuffer), 0);
    if (bytesSent == SOCKET_ERROR) {
        printf("Erro ao enviar dados: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    }
    if (strcmp(sendBuffer, "exit") == 0) {
        printf("Encerrando a conexao com o cliente\n");
        return -1;
    } else return 1;
}

void finalizaConexao() {
    closesocket(clientSocket);
    closesocket(sock);
    WSACleanup();
    getch();
}


int tratarMsgRecebida() {
    printf("a msg recebida foi %s\n",recvBuffer );
}

int tratarMsgEnviada() {
    printf("a msg enviada foi %s\n",sendBuffer );
}


int main() {
    iniciarBiblioteca();
    iniciarSocket();
    esperarConexao();
    while (1) {
        if (receberMensagem() == -1) {
            break;
        }
        tratarMsgRecebida();

        if (enviarMensagem() == -1) {
            break;
        }
        tratarMsgEnviada();

    }
    finalizaConexao();
    return 0;
}
