#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 7777

int main() {
    // Créer la socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        printf("Erreur lors de la création de la socket\n");
        return EXIT_FAILURE;
    }

    // Préparer l'adresse du serveur
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr) <= 0) {
        printf("Adresse du serveur invalide\n");
        return EXIT_FAILURE;
    }

    // Se connecter au serveur
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Erreur lors de la connexion au serveur\n");
        return EXIT_FAILURE;
    }

    // Envoyer et recevoir des données...
    
    // Fermer la socket
    close(clientSocket);

    return EXIT_SUCCESS;
}



