#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;

int main(int argc, char **argv) {
    int file;

    uint16_t hostEndianessPort = 13;

    if((file = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    {
        if (errno == ENOENT)
            perror("brak dostepu do pliku");
        else
            perror("przy otwieranie pliku");
        return 0;
    }
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(hostEndianessPort);
    
    connect(file, (sockaddr*) &addr, sizeof(addr));

    char buff[64];
    while(int n = read(file, buff, sizeof(buff)))
    {
        write(1, buff, n);
    }

    shutdown(file, SHUT_RDWR);

    close(file);

    return 0;
}

