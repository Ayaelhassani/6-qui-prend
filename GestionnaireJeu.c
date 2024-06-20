#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdbool.h>


#define PORT 8080 
#define MAX_JOUEURS 2 // Nombre maximal de clients pouvant se connecter

void error(char *msg) {
    perror(msg);
    exit(1);
}

typedef struct {
    int nombre;
    int teteBoeuf;
} Carte;


void afficherConditionsDuJeu() {
    printf("Bienvenue dans le jeu '6 qui prend'!\n");
    printf("Conditions du jeu:\n");
    printf("1. Chaque joueur commence avec 10 cartes.\n");
    printf("2. À chaque tour, les joueurs choisissent une carte à placer sur la table.\n");
    printf("3. Les cartes que va choisir chaque joueur va etre place dans une rangées.\n");
    printf("4. Lorsqu'une rangée atteint 6 cartes par rangées, le joueur prend cette rangée.\n");
    printf("5. La partie se termine quand un joueur atteint le score de 66 tetes de boeufs et après que les tous les 104 cartes ont été joué.\n");

    printf("\n");
}

// Fonction pour afficher le menu et obtenir le choix de l'utilisateur
bool afficherMenu() {
    char choix[10];

    printf("\nMenu :\n");
    printf("1. Commencer le jeu\n");
    printf("2. Quitter\n");
    printf("Votre choix : ");

    fgets(choix, sizeof(choix), stdin);
    choix[strcspn(choix, "\n")] = 0; // Supprimer le saut de ligne de fgets

    if (strcmp(choix, "1") == 0) {
        return true; // L'utilisateur veut commencer le jeu
    } else if (strcmp(choix, "2") == 0) {
        return false; // L'utilisateur veut quitter
    } else {
        printf("Choix invalide. Veuillez choisir 1 pour commencer ou 2 pour quitter.\n");
        return afficherMenu(); // Demander à nouveau le choix
    }
}

void melangeDesCartes(Carte *tabCart, int numCarte) {
    srand(time(NULL));

    for (int i = numCarte - 1; i > 0; i--) {
        int j = rand() % (i + 1);

        Carte temp = tabCart[i];
        tabCart[i] = tabCart[j];
        tabCart[j] = temp;
    }
}



int cartesPosees = 0;

void poserCartesInitiales(Carte *cartesSurTable, int *sockets, int nb_joueurs) {
    if (cartesPosees == 1) {
        
        return;
    }
    

    // Envoi des mêmes cartes posées sur la table à tous les joueurs
    for (int j = 0; j < nb_joueurs; j++) {
        int n = write(sockets[j], cartesSurTable, sizeof(Carte) * 4);
        if (n < 0) {
            perror("ERROR writing to socket");
        }
    }
    cartesPosees = 1;
}
// Variable globale pour suivre le nombre total de cartes distribuées
int NombreCartesDistribuees = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int partie_commencee = 0;

void *ConnexionJoueur(void *socket_desc) {
    int newsockfd = *(int *)socket_desc;
    char buffer[256];
    char username[50];
    int n;
    int NombreCarteRecu = 0;
    

    n = read(newsockfd, username, sizeof(username));
    if (n < 0)
        error("ERROR reading from socket");

    printf("Nom d'utilisateur : %.*s\n", n, username);
  
  
}
bool cartesSurTablePosees = false;
typedef struct {
    Carte cartes[6]; // Une rangée contient 6 cases pour les cartes
} RangeeCartes;

// Tableau pour stocker les cartes sur la table, avec 4 rangées
RangeeCartes cartesSurTable[4];

void poserCartesSurTable(RangeeCartes *cartesSurTable, int *sockets, int nb_joueurs) {
    if (!cartesSurTablePosees) {
        // Envoi des cartes sur la table à tous les joueurs
        for (int j = 0; j < nb_joueurs; j++) {
            int n = write(sockets[j], cartesSurTable, sizeof(RangeeCartes) * 4);
            if (n < 0) {
                perror("ERROR writing to socket");
            }
        }
        cartesSurTablePosees = true; // Marquer que les cartes ont été posées
    }
}
typedef struct {
    int choixCarte;
    Carte carteChoisie;
} ChoixCarte;

void comparerCarteEtPlacer(RangeeCartes *cartesSurTable, int choixJoueur, Carte carteJoueur, int *scoresJoueurs) {
    Carte dernieresCartes[4]; // Tableau pour stocker les dernières cartes de chaque rangée

    // Récupérer les dernières cartes de chaque rangée
    for (int i = 0; i < 4; i++) {
        dernieresCartes[i] = cartesSurTable[i].cartes[5]; // La dernière carte de chaque rangée est à l'index 5
    }

    // Trier les dernières cartes par ordre croissant
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3 - i; j++) {
            if (dernieresCartes[j].nombre > dernieresCartes[j + 1].nombre) {
                Carte temp = dernieresCartes[j];
                dernieresCartes[j] = dernieresCartes[j + 1];
                dernieresCartes[j + 1] = temp;
            }
        }
    }

    // Comparer la carte du joueur avec les cartes de la table
    for (int i = 0; i < 4; i++) {
        if (carteJoueur.nombre < dernieresCartes[i].nombre) {
            // Insérer la carte du joueur devant la carte correspondante sur la table
            cartesSurTable[i].cartes[choixJoueur - 1] = carteJoueur;
            break;
        } else if (i == 3) {
            // Si la dernière rangée est complètement remplie, le joueur prend les 6 cartes
            int scoreRangée = 0;
            for (int k = 0; k < 6; ++k) {
                scoreRangée += cartesSurTable[i].cartes[k].teteBoeuf;
            }
            scoresJoueurs[dernierJoueur] += scoreRangée;

            // Repositionner la carte du joueur au début de la rangée
            cartesSurTable[i].cartes[0] = carteJoueur;
        }
    }
}









int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno, player_count = 0;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    pthread_t thread_id;
    int *new_sock;
    int sockets_joueurs[MAX_JOUEURS];
    int nb_joueurs = 0;
    int n;
    int cartesDistribuees = 0;
    Carte cartes[104];
    int joueurActif = 0;
    int finDuJeu = 0;
    int nb_joueurs_demande = 0;
    bool partieCommencee = false; 
    RangeeCartes rangCartesSurTable;
    int gameActive = 1;
    bool rangéesAffichées = false;
    int dernierJoueur = 0;



bool continuerJeu = false;
    do {
        continuerJeu = afficherMenu();

        if (!continuerJeu) {
            printf("Fermeture du serveur.\n");
            close(sockfd); // Fermer le socket
            exit(0);
        }

 afficherConditionsDuJeu();
do {
        printf("Combien de joueurs (entre 2 et 10) pouvez-vous accepter ? : ");
        scanf("%d", &nb_joueurs_demande);
    } while (nb_joueurs_demande < 2 || nb_joueurs_demande > MAX_JOUEURS);
    printf("Attente de connexion de %d joueurs...\n", nb_joueurs_demande);

    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    while (player_count < 2) {
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
        error("ERROR on accept");

    sockets_joueurs[nb_joueurs++] = newsockfd;
    new_sock = malloc(sizeof(int));
    *new_sock = newsockfd;

    if (pthread_create(&thread_id, NULL, ConnexionJoueur, (void *)new_sock) < 0) {
        error("ERROR on thread creation");
    }

    player_count++;
    printf("Joueur %d connecté.\n", player_count);
}



    // Attendre la connexion de en moins deux joueurs
     while (nb_joueurs < MAX_JOUEURS) {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");

        sockets_joueurs[nb_joueurs++] = newsockfd;
        new_sock = malloc(sizeof(int));
        *new_sock = newsockfd;

        if (pthread_create(&thread_id, NULL, ConnexionJoueur, (void *)new_sock) < 0) {
            error("ERROR on thread creation");
        }

        player_count++;
        printf("Joueur %d connecté.\n", player_count);
        sleep(8); 
    }

   

// Distribuer les cartes, démarrer le jeu, etc.
printf("Le jeu peut commencer!\n");




  Carte paquet[104];

  

// Génération des cartes uniques
for (int i = 0; i < 104; i++) {
    paquet[i].nombre = i + 1;
    paquet[i].teteBoeuf = (i % 7) + 1; // Valeurs de tête de bœuf de 1 à 7
}

// Mélange des cartes
melangeDesCartes(paquet, 104);
 
  
    
// Distribuer les cartes à chaque joueur
for (int joueur = 0; joueur < nb_joueurs; joueur++) {
    // Envoyer 10 cartes au joueur actuel
    for (int carte = 0; carte < 10; carte++) {
          // Vérifier si les 104 cartes ont été distribuées
        if (cartesDistribuees >= 104) {
            printf("Fin de la partie. Les 104 cartes ont été distribuées.\n");

            // Fermer la connexion et le serveur
            close(sockfd);
            // Fermer les autres connexions si nécessaires

            exit(0);
        }

       // Envoyer une carte au joueur actuel
           n = write(sockets_joueurs[joueur], &paquet[cartesDistribuees], sizeof(Carte));
            if (n < 0) {
                perror("ERROR writing to socket");
            }
      cartesDistribuees++;   
    }
}

// Initialisation des cartes sur table
   for (int i = 0; i < 4; i++) {
    cartesSurTable[i].cartes[0] = paquet[cartesDistribuees++];
}

// Affichage des cartes sur la table avant le début du jeu
printf("Cartes sur la table avant le début du jeu :\n");
for (int i = 0; i < 4; ++i) {
    printf("Rangée %d:\n", i + 1);
    printf("Carte 1: Numéro = %d, Têtes de bœufs = %d\n", cartesSurTable[i].cartes[0].nombre, cartesSurTable[i].cartes[0].teteBoeuf);
}
// Envoi des cartes sur table aux joueurs
    poserCartesSurTable(cartesSurTable, sockets_joueurs, nb_joueurs);

while (NombreCartesDistribuees < nb_joueurs * 10) {
for (int i = 0; i < 4; i++) {
    cartesSurTable[i].cartes[0] = cartes[i]; // Place une carte dans la première case de chaque rangée
}

}        
    poserCartesSurTable(cartesSurTable, sockets_joueurs, nb_joueurs);

while (gameActive) {
// Réception du choix de carte du joueur
    ChoixCarte choix;
    int n2 = read(newsockfd, &choix, sizeof(ChoixCarte));
    if (n2 <= 0) {
        if (n2 == 0) {
            printf("Le joueur a quitté la partie.\n");
        } else {
            error("Erreur de lecture du socket pour le choix du joueur");
        }
        // Gérer la sortie du joueur ici (retrait de la liste des joueurs, etc.)
        close(newsockfd);
        exit(1);
    }

    // Affichage de la carte choisie par le joueur
    printf("Joueur %d a choisi la carte numéro %d : Nombre = %d, Têtes de bœufs = %d\n",
           dernierJoueur + 1, choix.choixCarte, choix.carteChoisie.nombre, choix.carteChoisie.teteBoeuf);
    
    // Stockage de la carte choisie dans la première rangée
    rangCartesSurTable.cartes[choix.choixCarte - 1] = choix.carteChoisie;

    // Affichage de la carte ajoutée dans la première rangée
    printf("La carte choisie par le joueur a été ajoutée à la première rangée : Carte %d: Nombre = %d, Têtes de bœufs = %d\n",
           choix.choixCarte, rangCartesSurTable.cartes[choix.choixCarte - 1].nombre,
           rangCartesSurTable.cartes[choix.choixCarte - 1].teteBoeuf);


}


// Trier les choix des joueurs en fonction de leur numéro de carte
for (int i = 0; i < nb_joueurs - 1; i++) {
    for (int j = 0; j < nb_joueurs - i - 1; j++) {
        if (choixJoueurs[j].carteChoisie.nombre > choixJoueurs[j + 1].carteChoisie.nombre) {
            // Échanger les choix si nécessaire pour les trier dans l'ordre croissant
            ChoixCarte temp = choixJoueurs[j];
            choixJoueurs[j] = choixJoueurs[j + 1];
            choixJoueurs[j + 1] = temp;
        }
    }


// Afficher les choix des joueurs triés
printf("Choix des joueurs (ordre croissant) :\n");
for (int i = 0; i < nb_joueurs; i++) {
    printf("Joueur %d - Carte choisie : Numéro = %d, Têtes de bœufs = %d\n", i + 1,
           choixJoueurs[i].carteChoisie.nombre, choixJoueurs[i].carteChoisie.teteBoeuf);
}


    



// Fermeture des sockets (éventuellement à la fin du programme)
close(sockfd);
for (int i = 0; i < nb_joueurs; ++i) {
    close(sockets_joueurs[i]);
}
} while (continuerJeu);
    return 0;
}

