#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#define MAX_TRESC 256
#define MAX_NAPIS 128

const int msgKey = 17;      // klucz
int currentLine = 0;        // wskaznik na obecna linie
int id;                     // id kolejki

struct linia {              // struktura przedstawiajaca linię pliku
    int nr;                 // numer linii
    int id;                 // id linii
    char napis[MAX_NAPIS];  // tresc linii
};

struct client_msg {         // struktura wiadomosci KLIENT -> SERWER
    long typ;               
    long klient_id;         // pid klienta
    int komenda;            // nr polecenia
    int linie_id[32];       // [w zaleznosci od polecenia]
    char nowa_linia[128];   // [w zaleznosci od polecenia]
};

struct get_msg {            // struktura wiadomosci SERWER -> KLIENT (get)
    long typ;
    int error;              // -1 - błąd, 0 - ok
    char wiadomosc[256];    // komunikat o bledzie
    struct linia linie[32]; // żądane linie w odpowiedniej kolejności
};

struct server_msg {         // struktura wiadomosci SERWER -> KLIENT (pozostale)
    long typ;
    int error;              // -1 - błąd, 0 - ok
    char wiadomosc[256];    // komunikat zwrotny
};

// rozmiary do msgsnd i msgrcv
const int SIZE_L = sizeof(struct linia) - sizeof(long),
          SIZE_C = sizeof(struct client_msg) - sizeof(long),
          SIZE_G = sizeof(struct get_msg) - sizeof(long),
          SIZE_S = sizeof(struct server_msg) - sizeof(long);

// funkcja odczytujaca z kolejki to, co odpowie serwer na żądanie: get xxx
void rcvGet() {
    struct get_msg get;
    if (msgrcv(id, &get, SIZE_G, getpid(), 0) == -1) {
        perror("cannot receive get_msg message from server");
        exit(-1);
    }
    printf("%s\n", get.wiadomosc);
    return;
}

// funkcja odczytujaca z kolejki to, co odpowie serwer na żądania pozostałe
void rcvServer() {
    struct server_msg server;
    if (msgrcv(id, &server, SIZE_S, getpid(), 0) == -1) {
        perror("cannot receive server_msg message from server");
        exit(-1);
    }
    printf("%s\n", server.wiadomosc);
    return;
}

// funkcja wysylajaca wiadomosc do serwera
void snd(struct client_msg msg) {
    if (msgsnd(id, &msg, SIZE_C, 0) == -1) {
        perror("cannot send message to server");
        exit(-1);
    }
}

// Dzieli string na wyrazy, zwraca ilosc wyrazow
int split(char str[], char* str2[]) {
    str2[0] = str;
    int i=1;
    while(*str != '\0') {
        if(*str == ' ') {
            *str = '\0';
            str2[i] = ++str;
            ++i;
        }
        else
            ++str;
    }
    //*(--str) = '\0';
    str2[i] = NULL;
    return i;
}

// Dzieli string na n wyrazów, resztę frazy wrzuca do n+1 wyrazu, zwraca ilosc wyrazow
int split2(char str[], char* str2[], int n) {
    str2[0] = str;
    int i=1;
    while(n != 0 && *str != '\0') {
        if(*str == ' ') {
            *str = '\0';
            str2[i] = ++str;
            ++i;  
            n--;
        }
        else
            ++str;
    }
    //*(--str) = '\0';
    str2[i] = NULL;
    return i;
}

// obsługa komendy: get 1 12 14
void get(char buf[MAX_TRESC]) {
    struct client_msg sent;
    sent.klient_id = getpid();
    sent.typ = 1;  
    sent.komenda = 0;
    char* komenda[MAX_TRESC];
    int nWords = split(buf, komenda);

    for(int i = 1; i < nWords; ++i) 
        sent.linie_id[i - 1] = atoi(komenda[i]);
    if (nWords < 32) sent.linie_id[nWords - 1] = -1;
    snd(sent); // wyslij
    rcvGet();  // odbierz
}

// obsługa komendy: insert 123 TRESC LUB insert TRESC
void insert(char buf[MAX_TRESC]) {
    struct client_msg sent;
    sent.klient_id = getpid();
    sent.typ = 1;  
    char* komenda[MAX_TRESC];
    char out[MAX_TRESC];
    int nWords = split2(buf, komenda, 2);                   
    struct client_msg help;                                
    help.typ = 1;                                           
    help.klient_id = getpid();                             
    help.komenda = 0;
    struct get_msg get;
    
    if (atoi(komenda[1]) == 0) {
        sent.linie_id[0] =  0;                                     
        strcpy(out, komenda[1]);
        if (nWords > 2) strcat(out, komenda[2]);

    }  else {
        help.linie_id[0] = atoi(komenda[1]) - 1;
        help.linie_id[1] = -1;
        snd(help);                                
        if (msgrcv(id, &get, SIZE_G, getpid(), 0) == -1) {
            perror("cannot receive get_msg message from server");
            exit(-1);
        }
        sent.linie_id[0] =  get.linie[0].id; 
        strcpy(out, komenda[2]);
    }

    sent.linie_id[1] =  -1;
    strcpy(sent.nowa_linia, out);
    sent.komenda = 1; 
    snd(sent);
    rcvServer();
}

// obsługa komendy: delete 1 11 23
void delete(char buf[MAX_TRESC]) {
    struct client_msg sent;
    sent.klient_id = getpid();
    sent.typ = 1;  
    char* komenda[MAX_TRESC];
    int nWords = split(buf, komenda);
    struct client_msg help;                                 
    help.typ = 1;                                          
    help.klient_id = getpid();                              
    help.komenda = 0;
    struct get_msg get;

    for (int i = 1; i < nWords; ++i) help.linie_id[i - 1] = atoi(komenda[i]);
    if (nWords < 32) help.linie_id[nWords - 1] = -1;

    snd(help);        
    if (msgrcv(id, &get, SIZE_G, getpid(), 0) == -1) {
        perror("cannot receive get_msg message from server");
        exit(-1);
    }

    for (int i = 0; i < nWords; ++i) sent.linie_id[i] = get.linie[i].id;
    if (nWords < 32) sent.linie_id[nWords - 1] = -1;


    sent.komenda = 2;         
    snd(sent);
    rcvServer();            
}

// obsługa komendy: modify 13 TRESC
void modify(char buf[MAX_TRESC]) {
    struct client_msg sent;
    sent.klient_id = getpid();
    sent.typ = 1;  
    char* komenda[MAX_TRESC];
    char out[MAX_TRESC];
    split2(buf, komenda, 2);
    struct client_msg help;                                 
    help.typ = 1;                                           
    help.klient_id = getpid();                              
    help.komenda = 0;
    struct get_msg get;

    help.linie_id[0] = atoi(komenda[1]);
    help.linie_id[1] = -1;

    snd(help);        
    if (msgrcv(id, &get, SIZE_G, getpid(), 0) == -1) {
        perror("cannot receive get_msg message from server");
        exit(-1);
    }
    sent.linie_id[0] =  get.linie[0].id; 
    sent.linie_id[1] = -1;
    strcpy(out, komenda[2]);
    strcpy(sent.nowa_linia, out);
    sent.komenda = 3;
    snd(sent);
    rcvServer();    
    return;
}

// MAIN
int main(int argc, char* argv[]) {
    const int RECEIVE = getpid(), SEND = 1;
    char buf[MAX_TRESC];
    char *komenda[MAX_TRESC]; 
    if ((id = msgget(msgKey, IPC_CREAT | 0666)) == -1) {    // tworzymy kolejke komunikatów ID=17
        perror("cannot create messege queue for a client"); // - typ = 1 to wiadomosc KLIENT -> SERWER
        exit(-1);                                           // - typ = PID klienta to wiadomosc SERWER -> KLIENT
    }
        while (1) {
        printf("Wprowadz komende: \n");
        fflush(stdin);
        fgets(buf, MAX_TRESC, stdin);
        if (strncmp(buf, "get ", 4) == 0) { 
            get(buf);
        } else if (strncmp(buf, "insert ", 7) == 0) {
            insert(buf);
        } else if (strncmp(buf, "delete ", 7) == 0) {
            delete(buf);
        } else if (strncmp(buf, "modify ", 7) == 0) {
            modify(buf);
        } else {
            printf("Podano nieprawidłową komendę!\n");
            continue;                                                                        
        }
    }        
    return 0;
}