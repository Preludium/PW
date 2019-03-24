#include "init.c"
#define MAX_WORDS 10

int id;

// Zamykanie programu
void handleClose() {
    if (msgctl(id, IPC_RMID, 0) == -1) {
        perror("cannot unlink message queue");
        exit(1);
    }
    exit(0);
}

// Potrzebne do wypisywania znaku zachęty
void handle() { }

// MAIN
int main(int argc, char const *argv[])
{
    int fifo;
    char buf[MAX_TRESC];
    char nick[128];
    char *msg_nick[MAX_WORDS];

    // Tworzenie kolejki komunikatów dla klienta
    if ((id = msgget(IPC_PRIVATE, IPC_CREAT | 0666)) == -1) {
        perror("cannot create message queue for client");
        exit(1);
    }

    // Jeśli id == 0, to poszukaj nowego id
    while (id == 0) {
        if (msgctl(id, IPC_RMID, 0) == -1) {
            perror("cannot unlink message queue");
            exit(1);
        }

        if ((id = msgget(IPC_PRIVATE, IPC_CREAT | 0666)) == -1) {
            perror("cannot create message queue for client");
            exit(1);
        }
    }
    
    // Otwieranie kolejki serwera
    if ((fifo = open(server_name, O_WRONLY)) == -1) {
        perror("cannot open server fifo");
        exit(1);
    }

    write(fifo, &id, sizeof(int));
    close(fifo);

    struct Message sent, received;
    sent.mtype = REQUEST;
    sent.id_nadawcy = id;

    // Czekanie na odpowiedź serwera
    if (msgrcv(id, &received, MSG_SIZE, ANSWER, 0) == -1) {
        perror("cannot receive answer message");
        exit(1);
    }

    // Wyświetlanie odpowiedzi i sprawdzanie, czy klient został dodany
    printf("%s\n", received.tresc);
    if (strncmp(received.tresc, "DENIAL", 6) == 0)
        return 0;
    
    signal(SIGUSR1, handle); // Do wypisywania znaku zachęty
    signal(SIGINT, handleClose);

    // Podawanie nicku i oczekiwanie na przyjęcie
    while(1)
    {
        printf("\e[1m\e[95m> \e[0m");
        fflush(stdin);  // Czyszczenie bufora
        fgets(buf, MAX_TRESC, stdin);
        strcpy(sent.tresc, buf);
        
        if (msgsnd(id, &sent, MSG_SIZE, 0) == -1) {
            perror("cannot send answer to client message queue");
            exit(1);
        }

        if (msgrcv(id, &received, MSG_SIZE, ANSWER, 0) == -1) {
            perror("cannot receive answer message");
            exit(1);
        }

        printf("%s", received.tresc);

        split(received.tresc, msg_nick);

        if(strcmp(msg_nick[0], "ACCEPT:") == 0){
            strcpy(nick, msg_nick[4]); 
            nick[strlen(nick) - 2] = '\0';
            break;
        }
    }

    // Ciągłe oczekiwanie na odpowiedź/wiadomość
    if (fork() == 0){
        while(1){
            // Wypisywanie komunikatu z kolejki
            if (msgrcv(id, &received, MSG_SIZE, ANSWER, 0) == -1) {
                perror("cannot receive answer message");
                exit(1);
            }

            if (strcmp(received.tresc, "EXIT") == 0)
            {
                printf("Zapraszamy ponownie\n");
                msgctl(id, IPC_RMID, 0);
                kill(getppid(), SIGKILL);
                exit(1);
            }
            else
            {
                // Wypisz
                printf("%s", received.tresc);
                kill(getppid(), SIGUSR1); //Do wypisywania znaku zachęty
            }
        }
    } 


    // Konsola klienta
    while(1){
        printf("\e[1m\e[95m%s@%s\e[0m:~\e[1m\e[34mcommandline\e[0m$ ", nick, server_name);

        fflush(stdin);  // Czyszczenie bufora
        fgets(buf, MAX_TRESC, stdin);
        strcpy(sent.tresc, buf);
        
        if (msgsnd(id, &sent, MSG_SIZE, 0) == -1) {
            perror("cannot send answer to client message queue");
            exit(1);
        }
        pause();    // Do wypisywania znaku zachęty
    }
    return 0;
}