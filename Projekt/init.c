#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <string.h>

#define MAX_TRESC 1024

#define SERWER 0   // ID nadawcy

#define REQUEST 1  // żądanie do serwera
#define ANSWER 2   // odpowiedź serwera

char server_name[] = "server_1234";

struct Message {
    long mtype;             // flaga wiadomości
    int id_nadawcy;         // ID jednoznacznie identyfikujące nadawcę
    char tresc[MAX_TRESC];  // wiadomość lub komenda dla serwera
} toSize;

const int MSG_SIZE = sizeof(toSize) - sizeof(long);

// Dzieli string na wyrazy
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

// Dzieli string na n wyrazów, resztę frazy wrzuca do n+1 wyrazu
void split2(char str[], char* str2[], int n) {
    str2[0] = str;
    int i=1;
    while(n != 0) {
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
}