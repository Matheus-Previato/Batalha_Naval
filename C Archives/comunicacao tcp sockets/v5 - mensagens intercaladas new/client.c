#include <winsock2.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//VARIÁVEIS GLOBAIS DA CONEXAO
WSADATA winsocketsDados;
SOCKET clientSocket;
struct sockaddr_in serverAddr;

char sendBuffer[512];
char recvBuffer[512];
int bytesReceived;


int iniciarBiblioteca(){
    if (WSAStartup(MAKEWORD(2, 2), &winsocketsDados) != 0) {
        printf("Falha ao inicializar o Winsock\n");
        return 1;
    } else {
        printf("WSAStartup carregado com sucesso\n");
    }
}

int iniciarClientSocket(){
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        printf("Erro ao criar o socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    } else {
        printf("Socket criado com sucesso\n");
    }
}

void iniciarServerSocket() {
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(51171);
}

int iniciarConexao() {
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Erro ao conectar ao servidor: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    } else {
        printf("Conectado ao servidor\n");
        return 1;
    }
}

int enviarMensagem(){
    printf("Digite a mensagem para enviar ao servidor: ");
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
        printf("Encerrando a conexao com o servidor\n");
        return -1;
    } else return 0;
}

int receberMensagem() {
    bytesReceived = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
    if (bytesReceived == SOCKET_ERROR) {
        printf("Erro ao receber dados: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    } else if (bytesReceived == 0) {
        printf("Conexao fechada pelo servidor\n");
        return -1;
    } else {
        recvBuffer[bytesReceived] = '\0';
        printf("Recebido: %s\n", recvBuffer);
        if (strcmp(recvBuffer, "exit") == 0) {
            printf("Servidor pediu para fechar a conexao\n");
            return -1;
        } else return 1;
    }
}

void finalizaConexao(){
    closesocket(clientSocket);
    WSACleanup();
    getch();
}


int tratarMsgRecebida() {
    printf("a msg recebida foi %s \n",recvBuffer );
}

int tratarMsgEnviada() {
    printf("a msg enviada foi %s \n",sendBuffer );
}


int main() {
    iniciarBiblioteca();
    iniciarClientSocket();
    iniciarServerSocket();
    if (iniciarConexao() == -1) {
        return -1;
    }

    while (1) {
        if (enviarMensagem() == -1) {
            break;
        }
        tratarMsgEnviada();

        if (receberMensagem() == -1) {
            break;
        }
        tratarMsgRecebida();

    }

    finalizaConexao();
    return 0;
}
