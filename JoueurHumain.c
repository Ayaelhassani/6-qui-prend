#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#define PORT 8080 
#define NombreCarteRecu 10
#define MAX_JOUEURS 2 // Nombre maximal de clients pouvant se connecter

void error(char *msg) {
 perror(msg);
 exit(0);
}

typedef struct {
 int nombre;
 int teteBoeuf;
} Carte;

typedef struct {
 Carte cartes[6]; // Une rangée contient 6 cases pour les cartes
} RangeeCartes;

 
 
int main(int argc, char *argv[]) {
 char *hostname = "127.0.0.1";
 int sockfd, portno, n;
 struct sockaddr_in serv_addr;
 struct hostent *server;
 char buffer[256];
 char username[50];
 
 int dernierJoueur = -1;



 if (argc < 3) {
 fprintf(stderr, "Usage: %s hostname port\n", argv[0]);
 exit(0);
 }

 portno = atoi(argv[2]);
 sockfd = socket(AF_INET, SOCK_STREAM, 0);
 if (sockfd < 0)
 error("ERROR opening socket");

 server = gethostbyname(argv[1]);
 if (server == NULL) {
 fprintf(stderr, "ERROR, no such host\n");
 exit(0);
 }

 printf("Veuillez entrer votre nom d'utilisateur : ");
 fgets(username, sizeof(username), stdin);
 username[strcspn(username, "\n")] = 0;

 bzero((char *)&serv_addr, sizeof(serv_addr));
 serv_addr.sin_family = AF_INET;
 bcopy((char *)server->h_addr,
 (char *)&serv_addr.sin_addr.s_addr,
 server->h_length);
 serv_addr.sin_port = htons(portno);

 if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
 error("ERROR connecting");
 else
 printf("Connexion établie avec succès !\n");

 // Envoi du nom d'utilisateur au serveur
 n = write(sockfd, username, strlen(username));
 if (n < 0)
 error("ERROR writing to socket");

 // Réception des 10 cartes envoyées par le serveur après le début du jeu
 // Réception des 10 cartes envoyées par le serveur après le début du jeu
 printf("Réception des cartes :\n");
 Carte cartes[NombreCarteRecu];
 for (int i = 0; i < NombreCarteRecu; ++i) {
 Carte currentCard;
 bzero(&currentCard, sizeof(Carte));
 int n1 = read(sockfd, &currentCard, sizeof(Carte));
 if (n1 <= 0) {
 if (n1 == 0) {
 printf("Le serveur a fermé la connexion.\n");
 } else {
 error("Erreur de lecture du socket");
 }
 // Gérer la fermeture du socket ici (fermeture propre, libération de ressources, etc.)
 close(sockfd);
 exit(1);
 }

 cartes[i] = currentCard;
 printf("Carte %d: Nombre = %d, Têtes de bœufs = %d\n", i + 1, cartes[i].nombre, cartes[i].teteBoeuf);
 }


// Tableau pour stocker les cartes sur table, avec 4 rangées
 // Réception des 4 cartes posées sur la table
printf("Réception des cartes posées sur la table :\n");
Carte cartesSurTable[4];
int gameActive = 1;

 while (gameActive) {
 // Réception des rangées de cartes sur table
 RangeeCartes rangCartesSurTable[4];
 int n1 = read(sockfd, rangCartesSurTable, sizeof(RangeeCartes) * 4);
 if (n1 <= 0) {
 if (n1 == 0) {
 printf("Le serveur a fermé la connexion.\n");
 } else {
 error("Erreur de lecture du socket");
 }
 // Gérer la fermeture du socket ici (fermeture propre, libération de ressources, etc.)
 close(sockfd);
 exit(1);
 }

 // Affichage des rangées de cartes sur table reçues
 for (int i = 0; i < 4; ++i) {
 printf("Rangée %d:\n", i + 1);
 for (int k = 0; k < 6; ++k) {
 // Vérifier si la carte est valide (non vide)
 if (rangCartesSurTable[i].cartes[k].nombre != 0 && rangCartesSurTable[i].cartes[k].teteBoeuf != 0) {
 printf("Carte %d: Numéro = %d, Têtes de bœufs = %d\n", k + 1, rangCartesSurTable[i].cartes[k].nombre, rangCartesSurTable[i].cartes[k].teteBoeuf);
 }
 }
 }


 // Laisser le joueur choisir une carte parmi les 10 qu'il a reçues
 typedef struct {
 int choixCarte;
 Carte carteChoisie;
} ChoixCarte;

// Laisser le joueur choisir une carte parmi les 10 qu'il a reçues
 int choixCarte;
 printf("Veuillez choisir le numéro de la carte que vous souhaitez jouer (de 1 à 10) : ");
 scanf("%d", &choixCarte);

 ChoixCarte choix;
 choix.choixCarte = choixCarte;
 choix.carteChoisie = cartes[choixCarte - 1];

 // Envoyer le choix de carte au serveur
 n = write(sockfd, &choix, sizeof(ChoixCarte));
 if (n < 0) {
 error("Erreur lors de l'envoi du choix de carte au serveur");
 }
 }

 // Fermer la connexion
 close(sockfd);
 return 0;
}