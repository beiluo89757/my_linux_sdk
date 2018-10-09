#include "lucas.h"

#define group_log(M, ...) custom_log("group", M, ##__VA_ARGS__)

pthread_mutex_t stdio_tx_mutex;


int main(int argc, char *argv[])
{
    if(pthread_mutex_init(&stdio_tx_mutex, NULL) < 0)
        printf(" stdio_tx_mutex init error\r\n");


    group_log(" UID is : %d,gID is : %d",getuid(),getgid());
}