#include <stdio.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>

#define MAX_LINE_LENGTH 128
#define MAX_LINES_TO_GET 32
#define MAX_MSG_LENGTH 256
#define MAX_LINES 10000

struct linia
{
	int nr;
	int id;
	char napis[MAX_LINE_LENGTH];
};

struct client_msg
{
	long typ;
	long klient_id;
	int komenda;
	int linie_id[MAX_LINES_TO_GET];
	char nowa_linia[MAX_LINE_LENGTH];
};

struct server_msg
{
	long typ;
	int error;
	char wiadomosc[MAX_MSG_LENGTH];
};

struct get_msg
{
	long typ;
	int error;
	char wiadomosc[MAX_MSG_LENGTH];
	struct linia linie[MAX_LINES_TO_GET];
};

int msgid;
int shm_lines_id;
int shm_ints_id;
int semid;
struct sembuf sem_buf;

void sem_up(int semnum)
{
    sem_buf.sem_num = semnum;
    sem_buf.sem_op = 1;
    sem_buf.sem_flg = 0;
    if (semop(semid, &sem_buf, 1) == -1) {
		perror("Blad podnoszenia semafora");
		exit(1);
	}
}

void sem_down(int semnum)
{
    sem_buf.sem_num = semnum;
    sem_buf.sem_op = -1;
    sem_buf.sem_flg = 0;
    if (semop(semid, &sem_buf, 1) == -1) {
		perror("Blad opuszczania semafora");
		exit(1);
	}
}

void init_ints()
{
	int *ints = shmat(shm_ints_id, NULL, 0);
    if(ints == (void*) -1)
    {
        perror("Blad przylaczania segmentu pamieci wspoldzielonej ze zmiennymi typu int");
        exit(1);
    }
    ints[0] = 0; //liczba_linii
    ints[1] = 1000; //aktualne_id
    ints[2] = 0; //nreaders
    if(shmdt(ints) == -1)
	{
		perror("Blad odlaczania segmentu pamieci wspoldzielonej ze zmiennymi typu int");
		exit(1);
	}
}

void insert(int pozycja, char *nowa_linia, struct linia *tablica_linii, int *liczba_linii, int *aktualne_id)
{
    int i;
    (*liczba_linii)++;
    for(i = (*liczba_linii) - 1; i >= pozycja; i--)
    {
        tablica_linii[i] = tablica_linii[i-1];
        tablica_linii[i].nr = i+1;
    }
    strcpy((tablica_linii + pozycja - 1) -> napis, nowa_linia);
    (tablica_linii + pozycja - 1) -> nr = pozycja;
    (tablica_linii + pozycja - 1) -> id = *aktualne_id;
    *aktualne_id += 1000;
}

void modify(int pozycja, char *nowa_linia, struct linia *tablica_linii)
{
    strcpy((tablica_linii + pozycja - 1) -> napis, nowa_linia);
    (tablica_linii + pozycja - 1) -> id++;
}

void deletee(int pozycja, struct linia *tablica_linii, int *liczba_linii)
{
    int i;
    for(i = pozycja-1; i < (*liczba_linii)-1; i++)
    {
    	tablica_linii[i] = tablica_linii[i+1];
    	tablica_linii[i].nr = i+1;
    }
    (*liczba_linii)--;
}

int id2nr(int id, struct linia *linie, int *liczba_linii)
{
	int j;
	id = id/1000;
	int id_linii;
	for(j = 0; j < *liczba_linii; j++)
	{
		id_linii = linie[j].id/1000;
		if(id_linii == id)
		{
			return linie[j].nr;
		}
	}
	return -1;
}

int czy_zmodyfikowano(int wyslane_id, struct linia l)
{
	int wyslana_wersja = wyslane_id % 1000;
	int aktualna_wersja = l.id % 1000;
	if(aktualna_wersja > wyslana_wersja)
	{
		return 1;
	}
	return 0;
}

void cleanup()
{
	if(msgctl(msgid, IPC_RMID, NULL) == -1) {
	    perror("Blad usuwania kolejki komunikatow");
	    exit(1);
	}
	if(shmctl(shm_lines_id, IPC_RMID, NULL) == -1) {
	    perror("Blad usuwania segmentu pamieci wspoldzielonej z tablica linii");
	    exit(1);
	}
	if(shmctl(shm_ints_id, IPC_RMID, NULL) == -1) {
	    perror("Blad usuwania segmentu pamieci wspoldzielonej ze zmiennymi typu int");
	    exit(1);
	}
	if(semctl(semid, 0, IPC_RMID) == -1) {
		perror("Blad usuwania tablicy semaforow");
	    exit(1);
	}
	exit(0);
}

int main()
{
    int key = 17;
    msgid = msgget(key, IPC_CREAT | 0600);
    shm_lines_id = shmget(key, MAX_LINES*sizeof(struct linia), IPC_CREAT | 0600);
    if(shm_lines_id == -1)
    {
    	perror("Blad tworzenia segmentu pamieci wspoldzielonej na tablice linii");
    	exit(1);
    }
    shm_ints_id = shmget(key+1, 3*sizeof(int), IPC_CREAT | 0600);
    if(shm_ints_id == -1)
    {
    	perror("Blad tworzenia segmentu pamieci wspoldzielonej na zmienne typu int");
    	exit(1);
    }
    semid = semget(key, 2, IPC_CREAT|0600);
    if(semid == -1) {
    	perror("Blad tworzenia tablicy semaforow");
    	exit(1);
    }
    int x;
    for(x = 0; x < 2; x++)
	{
        if(semctl(semid, x, SETVAL, 1) == -1)
		{
        	perror("Blad ustawiania semafora");
        	exit(1);
        }
    }
    if(signal(SIGINT, cleanup) == SIG_ERR)
    {
    	perror("Blad ustawiania funkcji obslugi sygnalu");
    	exit(1);
    }
    init_ints();
    while(1)
    {
    	struct client_msg msg;
        int receive = msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), 1, 0);
        if(receive == -1)
        {
        	perror("Blad odbierania wiadomosci od klienta");
        	exit(1);
        }
        if(fork() == 0)
        {
        	struct linia *tablica_linii = shmat(shm_lines_id, NULL, 0);
        	if(tablica_linii == (void*) -1)
        	{
        		perror("Blad przylaczania segmentu pamieci wspoldzielonej z tablica linii");
        		exit(1);
        	}
        	int *ints = shmat(shm_ints_id, NULL, 0);
        	if(ints == (void*) -1)
        	{
        		perror("Blad przylaczania segmentu pamieci wspoldzielonej ze zmiennymi typu int");
        		exit(1);
        	}
        	int* liczba_linii = &(ints[0]);
			int* aktualne_id = &(ints[1]);
			int* nreaders = &(ints[2]);
        	int szukany_nr, j;
	        if(msg.komenda == 0) //komenda get
	        {
	        	struct get_msg get_response;
	        	get_response.typ = msg.klient_id;
	        	get_response.error = 0;
	        	sem_down(1);
	        	if((*nreaders) == 0)
	        	{
	        		(*nreaders)++;
	        		sem_down(0);
	        	}
	        	else
	        	{
	        		(*nreaders)++;
	        	}
	        	sem_up(1);
	        	for(j = 0; j < MAX_LINES_TO_GET; j++)
	        	{
	        		szukany_nr = msg.linie_id[j];
	        		if(szukany_nr == -1)
	        		{
	        			get_response.linie[j].id = -1;
	        			break;
	        		}
	        		if(szukany_nr > *liczba_linii || szukany_nr < 1)
	        		{
	        			get_response.error = -1;
	        			strcpy(get_response.wiadomosc, "Proba pobrania nieistniejacej linii\n");
	        			break;
	        		}
	        		get_response.linie[j] = tablica_linii[szukany_nr-1];
	        	}
	        	sem_down(1);
	        	(*nreaders)--;
	        	if((*nreaders) == 0)
	        	{
	        		sem_up(0);
	        	}
	        	sem_up(1);
	        	int send = msgsnd(msgid, &get_response, sizeof(get_response)- sizeof(long), 0);
	        	if(send == -1)
		    	{
		    		perror("Blad wysylania wiadomosci do klienta");
		    		exit(1);
		    	}
	        }
	        else
	        {
	        	struct server_msg response;
	        	response.typ = msg.klient_id;
	        	response.error = 0;
	        	sem_down(0);
	        	switch(msg.komenda)
	        	{
	        		case 1: //insert
	        			if(*liczba_linii == MAX_LINES)
	        			{
	        				response.error = -1;
	        				strcpy(response.wiadomosc, "Osiagnieto maksymalna liczbe linii w dokumencie\n");
	        			}
	        			else 
						{
							if(msg.linie_id[0] == 0)
							{
								insert(1, msg.nowa_linia, tablica_linii, liczba_linii, aktualne_id);
								strcpy(response.wiadomosc, "Nowa linia zostala wstawiona\n");
							}
							else
							{
								int nr_poprzedniej = id2nr(msg.linie_id[0], tablica_linii, liczba_linii);
								if(nr_poprzedniej == -1)
								{
									response.error = -1;
	        						strcpy(response.wiadomosc, "Linia po ktorej probujesz wstawic nowa linie nie istnieje lub zostala usunieta\n");
								}
								else
								{
									insert(nr_poprzedniej+1, msg.nowa_linia, tablica_linii, liczba_linii, aktualne_id);
									strcpy(response.wiadomosc, "Nowa linia zostala wstawiona\n");
								}
							}
	        			}
	        			break;
	        		case 2: //delete
	        			for(j = 0; j < MAX_LINES_TO_GET; j++)
	        			{
	        				if(msg.linie_id[j] == -1)
	        				{
	        					break;
	        				}
	        				szukany_nr = id2nr(msg.linie_id[j], tablica_linii, liczba_linii);
			        		if(szukany_nr == -1)
			        		{
			        			response.error = -1;
			        			strcpy(response.wiadomosc, "Podano nieistniejaca lub juz usunieta linie\n");
			        			break;
			        		}
			        		if(czy_zmodyfikowano(msg.linie_id[j], tablica_linii[szukany_nr-1]) == 1)
			        		{
			        			response.error = -1;
			        			strcpy(response.wiadomosc, "Jedna z podanych linii zostala zmodyfikowana od ostatniego pobrania.\n Prosze pobrac ponownie podane linie i ponowic probe usuniecia.\n");
			        			break;
			        		}
	        			}
	        			if(response.error != -1)
	        			{
	        				for(j = 0; j < MAX_LINES_TO_GET; j++)
	        				{
	        					if(msg.linie_id[j] == -1)
		        				{
		        					break;
		        				}
								szukany_nr = id2nr(msg.linie_id[j], tablica_linii, liczba_linii);
		        				deletee(szukany_nr, tablica_linii, liczba_linii);
	        				}
	        				strcpy(response.wiadomosc, "Linie zostaly usuniete\n");
	        			}
	        			break;
	        		case 3: //modify
	        			szukany_nr = id2nr(msg.linie_id[0], tablica_linii, liczba_linii);
	        			if(szukany_nr == -1)
	        			{
	        				response.error = -1;
			        		strcpy(response.wiadomosc, "Podano nieistniejaca lub juz usunieta linie\n");
	        			}
	        			else
	        			{
	        				if(czy_zmodyfikowano(msg.linie_id[0], tablica_linii[szukany_nr-1]) == 1)
			        		{
			        			response.error = -1;
			        			strcpy(response.wiadomosc, "Podana linia zostala zmodyfikowana od ostatniego pobrania.\n Prosze pobrac ja ponownie i ponowic probe modyfikacji\n");
			        		}
			        		else
			        		{
			        			modify(szukany_nr, msg.nowa_linia, tablica_linii);
			        			strcpy(response.wiadomosc, "Linia zostala zmodyfikowana\n");
			        		}
	        			}
	        			break;
	        		default:
	        			response.error = -1;
			        	strcpy(response.wiadomosc, "Bledna komenda\n");
	        	}
	        	sem_up(0);
	        	int send = msgsnd(msgid, &response, sizeof(response)- sizeof(long), 0);
	        	if(send == -1)
		    	{
		    		perror("Blad wysylania wiadomosci do klienta");
		    		exit(1);
		    	}
	        }
	        if(shmdt(ints) == -1)
			{
				perror("Blad odlaczania segmentu pamieci wspoldzielonej ze zmiennymi typu int");
				exit(1);
			}
			if(shmdt(tablica_linii) == -1)
			{
				perror("Blad odlaczania segmentu pamieci wspoldzielonej z tablica linii");
				exit(1);
			}
	        break;
	    }
    }
    return 0;
}
