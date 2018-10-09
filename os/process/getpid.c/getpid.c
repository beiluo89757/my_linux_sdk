#include "lucas.h"

#define process_log(M, ...) custom_log("process", M, ##__VA_ARGS__)

pthread_mutex_t stdio_tx_mutex;


int main(int argc, char *argv[])
{
    if(pthread_mutex_init(&stdio_tx_mutex, NULL) < 0)
        printf(" stdio_tx_mutex init error\r\n");


    process_log("current process ID is : %d",getpid());
}