#include "lucas.h"

#define process_log(M, ...) custom_log("process", M, ##__VA_ARGS__)

pthread_mutex_t stdio_tx_mutex;


int main(int argc, char *argv[])
{
    if(pthread_mutex_init(&stdio_tx_mutex, NULL) < 0)
        printf(" stdio_tx_mutex init error\r\n");


    fprintf(stderr,"EACCESS: %s\n",strerror(EACCES));
    FILE *fp;

    errno = ENOMEM;
    perror(argv[0]);

    exit(0);
}