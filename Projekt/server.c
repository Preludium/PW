#include "init.c"

#define MAX_CLIENTS 16
#define MAX_ROOMS 4
#define MAX_BUFFER 1024
#define SML_BUFFER 16
#define MAX_WORDS 16
#define MAX_NICK 64

int id_global;
int f1, f2;

struct Global {
    int clients, clients_id[MAX_CLIENTS];
    int rooms, rooms_id[MAX_ROOMS][MAX_CLIENTS], rooms_size[MAX_ROOMS];
    char clients_nick[MAX_CLIENTS][MAX_NICK];
};

struct Global* global;

// Zwracanie indeksu w tablicy clients_id w zaleznosci od id_kolejki (-1 jezeli taki nie istnieje)
int retId(int msg_id) {
    for(int i=0; i<global->clients; ++i)
        if(global->clients_id[i] == msg_id) return i;
    return -1;
}

// Zwracanie indeksu w tablicy clients_nick w zaleznosci od podanego nicku
int retNick(char *nick) {
    for (int i = 0; i < global->clients; ++i)
        if (strcmp(global->clients_nick[i], nick) == 0) return i;        
    return -1;
}

// Obsługa kolejki serwera
int handleServerQueue() {
    int status=0;

    int n, id;
    int fifo;
    struct Message msg;

    // Otwieranie kolejki serwera
    if ((fifo = open(server_name, O_RDONLY)) == -1) {
        perror("cannot open server fifo");
        exit(1);
    }
    
    // Odczytaj żądanie
    while ((n = read(fifo, &id, sizeof(id))) > 0) {
        printf("\nClient %d requests addition. Processing treatment...\n", id);

        if (global->clients == MAX_CLIENTS) { // jeśli nie ma wolnych miejsc, to odrzuć
            msg.mtype = ANSWER;
            msg.id_nadawcy = SERWER;
            strcpy(msg.tresc, "DENIAL: Max clients number reached\n");
        }

        else { // w przeciwnym wypadku dopisz go do listy
            
            global->clients_id[global->clients] = id;
            ++global->clients;

            msg.mtype = ANSWER;
            msg.id_nadawcy = SERWER;
            strcpy(msg.tresc, "ACCEPT: Client added successfully. Waiting for nick...\n");
        }

        // Odeślij informację zwrotną
        if (msgsnd(id, &msg, MSG_SIZE, 0) == -1) {
            perror("cannot send answer to client message queue");
            exit(1);
        }

    }

    if (n == -1) {
        perror("cannot read server message queue");
        exit(1);
    }

    close(fifo);
    return status;
 
}

// Obsługa błędów - niezrozumiałe polecenie
void handleError(struct Message msg) {
    int to = msg.id_nadawcy;
    msg.mtype = ANSWER;
    msg.id_nadawcy = SERWER;
    strcpy(msg.tresc, "[SERVER]: Unrecognized command, try 'help'\n");

    if (msgsnd(to, &msg, MSG_SIZE, 0) == -1) {
        perror("cannot send answer to user");
        exit(1);
    }
}

// Obsługa polecenia: write -u 123 hello user123 <LUB> write -r 2 hello room2
void handleWrite(struct Message msg) {
    char *words[MAX_WORDS];
    split2(msg.tresc, words, 3);

    if (strcmp(words[1], "-u") == 0) { // write user

        struct Message msg2;
        msg2.mtype = ANSWER;
        msg2.id_nadawcy = msg.id_nadawcy;
        sprintf(msg2.tresc, "[%s] writes: %s", global->clients_nick[retId(msg.id_nadawcy)], words[3]);

        int uid = retNick(words[2]);
        if (uid == -1) { // nie ma klienta o podanym id
            uid = msg.id_nadawcy;
            msg2.id_nadawcy = SERWER;
            strcpy(msg2.tresc, "[SERVER]: No user of given nick found\n");
        }

        else {
            uid = global->clients_id[uid];

            if (msgsnd(uid, &msg2, MSG_SIZE, 0) == -1) {
                perror("cannot send message to user");
                exit(1);
            }

            uid = msg.id_nadawcy;
            msg2.mtype = ANSWER;
            msg2.id_nadawcy = SERWER;
            strcpy(msg2.tresc, "[SERVER]: Message sent to user\n");
        }

        if (msgsnd(uid, &msg2, MSG_SIZE, 0) == -1) {
            perror("cannot send answer message to user");
            exit(1);
        }

    }

    else if (strcmp(words[1], "-r") == 0) { // write room
        struct Message msg2;
        msg2.mtype = ANSWER;
        msg2.id_nadawcy = msg.id_nadawcy;
        sprintf(msg2.tresc, "[%s] writes: %s", global->clients_nick[retId(msg.id_nadawcy)], words[3]);
        
        int rid = atoi(words[2])-1, found = 0;

        for(int j = 0; j < global->rooms_size[rid]; j++){
            if(global->rooms_id[rid][j] == msg.id_nadawcy){
                found = 1;
                break;
            }
        }

        if(global->rooms_size[rid] == 0){
            strcpy(msg2.tresc, "[SERVER]: cannot send message to room: room doesn't exist\n");
        }
        else if(found){
            for (int i = 0; i < global->rooms_size[rid]; ++i) {
                if(global->rooms_id[rid][i] == msg.id_nadawcy)      //żeby do siebie nie wysyłał
                    continue;
                if (msgsnd(global->rooms_id[rid][i], &msg2, MSG_SIZE, 0) == -1) {
                    perror("cannot send message to room");
                    exit(1);
                }
            }
            strcpy(msg2.tresc, "[SERVER]: Message sent to room\n");
        }
        else{
            strcpy(msg2.tresc, "[SERVER]: cannot send message to room: you are not a member of this room\n");
        }        
        
        rid = msg.id_nadawcy;
        msg2.mtype = ANSWER;
        msg2.id_nadawcy = SERWER;
        if (msgsnd(rid, &msg2, MSG_SIZE, 0) == -1) {
            perror("cannot send answer message to user");
            exit(1);
        }

    }

    else // error
        handleError(msg);
}

// Obsługa polecenia: finger
void handleFinger(struct Message msg) {
    int to = msg.id_nadawcy;
    char str[MAX_TRESC], tmp[SML_BUFFER];
    char* words[MAX_WORDS];
    split2(msg.tresc, words, 2);

    // jeżeli jest pytanie o dostępne pokoje
    if (strcmp(words[1], "-r\n") == 0) {
        if (global->rooms == 0)
            strcpy(str, "[SERVER]: No rooms available\n");
        
        else {
            strcpy(str, "[SERVER]: Available rooms: ");

            for (int i=0; i<MAX_ROOMS; ++i) {
                if (global->rooms_size[i] != 0) {
                    sprintf(tmp, "%d, ", i+1);
                    strcat(str, tmp);
                }
            }
            strcat(str, "\n");
        }
        
    }

    // jeżeli jest pytanie o użytkowników w danym pokoju
    else if(strcmp(words[1], "-r") == 0){
            int x = atoi(words[2]);
            x--; //normalizacja
            if (global->rooms == 0)
                strcpy(str, "[SERVER]: No rooms available\n");
            else if(x<0 || x>MAX_ROOMS || global->rooms_size[x] == 0)
                strcpy(str, "[SERVER]: Room of a given id is not available\n");
            else{
                strcpy(str, "[SERVER]: Users in room: "); 
                for(int i = 0; i < global->rooms_size[x]; i++){
                    sprintf(tmp, "%s, ", global->clients_nick[ retId(global->rooms_id[x][i]) ]);
                    strcat(str, tmp); 
                }
                strcat(str, "\n");
            }
                
    } 

    // jeżeli sprawdzamy użytkowników (-u lub brak flagi)
    else { 
        strcpy(str, "[SERVER]: Online users: ");

        for (int i=0; i<global->clients; ++i) {
            sprintf(tmp, "%s, ", global->clients_nick[i]);
            strcat(str, tmp);
        }
        strcat(str, "\n");
    }

    msg.mtype = ANSWER;
    msg.id_nadawcy = SERWER;
    strcpy(msg.tresc, str);

    if (msgsnd(to, &msg, MSG_SIZE, 0) == -1) {
        perror("cannot send answer to user");
        exit(1);
    }

}

// Obsługa polecenia: createRoom user1 user2
void handleCreateRoom(struct Message msg) {
    int to = msg.id_nadawcy, nr_room = 0, found = 0, j;
    char str[MAX_TRESC], room_msg[3];
    char* words[MAX_WORDS];
    int x = split(msg.tresc, words);
    for(nr_room; nr_room < MAX_ROOMS; ++nr_room)
    {
        if(global->rooms_size[nr_room] == 0)
        {
            found = 1;
            break;
        }
    }
    
    if(found)
    {
        int n;
        global->rooms++;
        global->rooms_size[nr_room] = 1;
        global->rooms_id[nr_room][0] = to;

        for(int i = 1; i < x && i < MAX_CLIENTS; i++){
            words[i][strlen(words[i])-1] = '\0';
            if ((n = retNick(words[i])) == -1) continue;

            global->rooms_size[nr_room] += 1;
            global->rooms_id[nr_room][i] = global->clients_id[n];
        }

        for (int i=x; i<MAX_CLIENTS; ++i)
            global->rooms_id[nr_room][i] = -1;

        sprintf(str, "[SERVER]: Room number %d created\n", nr_room+1);
    }
    else
        strcpy(str, "[SERVER]: Max rooms number reached\n");

    msg.id_nadawcy = SERWER;
    msg.mtype = ANSWER;
    strcpy(msg.tresc, str);    

    if (msgsnd(to, &msg, MSG_SIZE, 0) == -1) {
        perror("cannot send answer to user");
        exit(1);
    }
}

// Obsuga polecenia: joinRoom 2
void handleJoinRoom(struct Message msg){
    int to = msg.id_nadawcy, to_join, joined = 0;
    char *words[MAX_WORDS], str[MAX_TRESC];
    split(msg.tresc, words);

    to_join = atoi(words[1]);
    to_join--;
    if(global->rooms_size[to_join] > 0)
    {
        for(int i = 1; i < MAX_CLIENTS; ++i)
        {
            if(global->rooms_id[to_join][i] == -1)
            {
                global->rooms_id[to_join][i] = to;
                global->rooms_size[to_join]++;
                joined = 1;
                strcpy(str, "[SERVER]: You have joined to the room number ");
                strcat(str, words[1]);
                break;
            }
        }

        if(joined == 0)
            strcpy(str, "[SERVER]: Room is full\n");
    }
    else
    {
        strcpy(str, "[SERVER]: Room does not exist\n");
    }
    
    msg.id_nadawcy = SERWER;
    msg.mtype = ANSWER;
    strcpy(msg.tresc, str);    

    if (msgsnd(to, &msg, MSG_SIZE, 0) == -1) {
        perror("cannot send answer to user");
        exit(1);
    }
}

// Obsługa polecenia: deleteRoom 2
void handleDeleteRoom(struct Message msg) {
    char *words[2], str[MAX_TRESC];
    int to = msg.id_nadawcy, to_del;
    int n = split(msg.tresc, words);

    if (n == 1) handleError(msg);

    to_del = atoi(words[1]);
    to_del--;
    if(global->rooms_id[to_del][0] == to)
    {
        for(int i = 0; i < MAX_CLIENTS; i++)
            global->rooms_id[to_del][i] = -1;

        global->rooms--;
        global->rooms_size[to_del] = 0;

        sprintf(str, "[SERVER]: Room number %d deleted\n", atoi(words[1]));
    }
    else
    {
        strcpy(str, "[SERVER]: Only room creator can delete room\n");
    }


    msg.id_nadawcy = SERWER;
    msg.mtype = ANSWER;
    strcpy(msg.tresc, str);    

    if (msgsnd(to, &msg, MSG_SIZE, 0) == -1) {
        perror("cannot send answer to user");
        exit(1);
    }
}

// Obsługa polecenia: help
void handleHelp(struct Message msg) {
    struct Message msg2;
    msg2.mtype = ANSWER;
    msg2.id_nadawcy = SERWER;

    char str[MAX_TRESC];
    strcpy(str, "[SERVER]: Commands list:\n");
    strcat(str, "write [OPTION] [ID] [TEXT] - sends message given as [TEXT] to user [-u] or room [-r] of a given ID\n");
    strcat(str, "finger [OPTION] [ID]- shows signed users [-u], created rooms [-r] or users that joined the room [-r] of a given ID\n");
    strcat(str, "createRoom [IDs...] - creates room that involves sender and optionally users of a given IDs\n");
    strcat(str, "joinRoom [ROOM_ID] - joins sender to the room of a given ID\n");
    strcat(str, "deleteRoom [ROOM_ID] - deletes room of a given ID\n");
    strcat(str, "quit - removes client from server\n");

    strcpy(msg2.tresc, str);

    if (msgsnd(msg.id_nadawcy, &msg2, MSG_SIZE, 0) == -1) {
        perror("cannot send answer to user");
        exit(1);
    }

}

// Obsługa nadawania nicku
void handleNick(struct Message msg) {
    char *words[MAX_WORDS];
    struct Message msg2;
    int n = split(msg.tresc, words);
    msg2.id_nadawcy = SERWER;
    msg2.mtype = ANSWER;

    if (strcmp(words[0], "nick") == 0) {
        words[1][strlen(words[1]) - 1] = '\0';
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (strcmp(words[1], global->clients_nick[i]) == 0) {
                strcpy(msg2.tresc, "DENIAL: Nickname already exists. Pick another one\n");
                if (msgsnd(msg.id_nadawcy, &msg2, MSG_SIZE, 0) == -1) {
                    perror("cannot send answer to user");
                    exit(1);
                }
                return;
            }
        }

        
        strcpy(global->clients_nick[retId(msg.id_nadawcy)], words[1]);
        sprintf(msg2.tresc, "ACCEPT: Welcome to chat %s!\n", words[1]);
        if (msgsnd(msg.id_nadawcy, &msg2, MSG_SIZE, 0) == -1) {
            perror("cannot send answer to user");
            exit(1);
        }
    } else {
        strcpy(msg2.tresc, "DENIAL: Choose your nickname first! (write: nick yourNick)\n");
        if (msgsnd(msg.id_nadawcy, &msg2, MSG_SIZE, 0) == -1) {
            perror("cannot send answer to user");
            exit(1);
        }
    }
}

//opuszczenie chat-u przez klienta
void handleQuit(struct Message msg) {
    int to = msg.id_nadawcy, id;
    id = retId(to);
    
    struct Message msg2;

    for(int i = 0; i < MAX_ROOMS; i++){
        if(global->rooms_size[i] == 0)
            continue;
        else
        {
            for(int j = 0; j < global->rooms_size[i]; j++){
                if(global->rooms_id[i][0] == to){       //jeżeli jest twórcą
                    msg2.id_nadawcy = msg.id_nadawcy;
                    msg2.mtype = REQUEST;
                    sprintf(msg2.tresc, "deleteRoom %d", i+1);
                    handleDeleteRoom(msg2);
                    break;
                }
                else if(global->rooms_id[i][j] == to){
                    global->rooms_id[i][j] = global->rooms_id[i][global->rooms_size[i] - 1];
                    global->rooms_id[i][global->rooms_size[i] - 1] = -1;
                    --global->rooms_size[i];
                    break;                    
                }
            }       
        }     
    }
    //zezwalamy klientowi, żeby się zakończył
    msg2.id_nadawcy = SERWER;
    msg2.mtype = ANSWER;
    strcpy(msg2.tresc, "EXIT");
    if (msgsnd(msg.id_nadawcy, &msg2, MSG_SIZE, 0) == -1) {
        perror("cannot send answer to user");
        exit(1);
    }

     //na miejsce wychodzącego wstawiamy ostatniego na liście - id i nick
    global->clients_id[id] = global->clients_id[global->clients - 1];
    strcpy(global->clients_nick[id], global->clients_nick[global->clients - 1]);
    //tam gdzie był ten ostatni czyścimy
    global->clients_id[global->clients - 1] = -1;
    strcpy(global->clients_nick[global->clients - 1], ""); 
    //zmniejszamy liczbę klientów
    --global->clients;
}

// Obsługa kolejek klientów
int handleRequests() {
    int status=0;

    int n;
    struct Message msg;

    for (int i=0; i<global->clients; ++i) {

        // Odczytaj żądanie i je obsłuż
        while ((n = msgrcv(global->clients_id[i], &msg, MSG_SIZE, REQUEST, IPC_NOWAIT)) > 0) {
            printf("\nRequest received from client %d. Executing...\n", msg.id_nadawcy);
            
            if (global->clients_nick[retId(msg.id_nadawcy)][0] == '\0') handleNick(msg);
            else if (strncmp(msg.tresc, "write", 5) == 0) handleWrite(msg);
            else if (strncmp(msg.tresc, "finger", 6) == 0) handleFinger(msg);
            else if (strncmp(msg.tresc, "createRoom", 10) == 0) handleCreateRoom(msg);
            else if (strncmp(msg.tresc, "joinRoom", 8) == 0) handleJoinRoom(msg);
            else if (strncmp(msg.tresc, "deleteRoom", 10) == 0) handleDeleteRoom(msg);
            else if (strncmp(msg.tresc, "help", 4) == 0) handleHelp(msg);
            else if (strncmp(msg.tresc, "quit", 4) == 0) {
                handleQuit(msg);
                break; // bo warunek while już nie może być sprawdzany
            }
            else handleError(msg);

        }

        // Jeśli wystąpił błąd, ale nie jest to błąd pustej kolejki komunikatów
        if (n == -1 && errno != ENOMSG) {
            perror("cannot read client message queue");
            exit(1);
        }

    }

    return status;
}

// Handler zamykający program
void handleClose() {
    struct Message msg2;
    msg2.id_nadawcy = SERWER;
    msg2.mtype = ANSWER;
    strcpy(msg2.tresc, "EXIT");
    for(int i = 0; i < global->clients; i++){
        if (msgsnd(global->clients_id[i], &msg2, MSG_SIZE, 0) == -1) {
            perror("cannot send answer to user");
            exit(1);
        }   
    }  

    if (unlink(server_name) == -1) {
        perror("cannot unlink server fifo");
        exit(1);
    }
    if (shmdt((const void*)global) == -1) {
        perror("cannot detach shm");
        exit(1);
    }
    if (shmctl(id_global, IPC_RMID, NULL) == -1) {
        perror("cannot remove shm");
        exit(1);
    }
    kill(f1, SIGKILL); // zabicie handleServerQueue
    kill(f2, SIGKILL); // zabicie handleRequests
    exit(0);           // wyjście z programu
}

// Polecenie finger dla serwera
void finger() {
    if (global->clients == 0) {
        printf("No users online.\n");
        return;
    }

    char str[MAX_BUFFER], tmp[SML_BUFFER];

    strcpy(str, "Online users' IDs: ");
    for (int i=0; i<global->clients; ++i) {
        sprintf(tmp, "%d, ", global->clients_id[i]);
        strcat(str, tmp);
    }
    strcat(str, "\n");

    printf("%s", str);
}

// Wypisywanie bledu polecenia dla serwera
void error() {
    printf("Unrecognized command.\n");
}

// Obsługa linii poleceń
void handleCommandline() {
    int fifo, n;
    char buf[MAX_BUFFER];

    printf("\e[1m\e[33m[SERVER]@%s\e[0m:~\e[1m\e[34mcommandline\e[0m$ ", server_name);
    fgets(buf, MAX_BUFFER, stdin);

    if (strncmp(buf, "finger", 6) == 0) finger();
    else if (strncmp(buf, "exit", 4) == 0) handleClose();
    else error();

}

// MAIN
int main(int argc, char const *argv[])
{
    // Definicje początkowe

    global = (struct Global*)malloc(sizeof(struct Global));
    global->clients = 0;
    global->rooms = 0;

    for (int i=0; i<MAX_CLIENTS; ++i) {
        global->clients_id[i] = -1;
    }

    for (int i=0; i<MAX_ROOMS; ++i) {
        global->rooms_size[i] = 0;
        for(int j = 0; j < MAX_CLIENTS; j++)
            global->rooms_id[i][j] = -1;
    }

    // Tworzenie kolejki serwera
    
    if (mkfifo(server_name, S_IRUSR | S_IWUSR) == -1) {
        perror("cannot create server fifo");
        exit(1);
    }

    key_t key;
    if ((key = ftok(argv[0], 'T')) == -1) {
        perror("cannot get key for shm");
        exit(1);
    }

    if ((id_global = shmget(key, sizeof(global), IPC_CREAT | 0666)) == -1) {
        perror("cannot get shm");
        exit(1);
    }

    if ((global = shmat(id_global, NULL, 0)) == (void*)-1) {
        perror("cannot attach shm");
        exit(1);
    }

    // Pętla fifo serwera
    f1 = fork();
    if (f1 == 0) {
        while(1) {
            handleServerQueue(); //clients, clients_id[]
        }
    }

    // Pętla obsługi żądań klientów
    f2 = fork();
    if (f2 == 0) {
        while(1) {
            handleRequests(); //rooms, rooms_id[][], rooms_size[]
        }
    }

    // Pętla serwera
    signal(SIGINT, handleClose);

    while(1) {
        handleCommandline();
    }

    return 0;
}