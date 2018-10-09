#include "lucas.h"

#define process_log(M, ...) custom_log("process", M, ##__VA_ARGS__)

pthread_mutex_t stdio_tx_mutex;
#define MAXLINE 65536

int main(int argc, char *argv[])
{
    if(pthread_mutex_init(&stdio_tx_mutex, NULL) < 0)
        printf(" stdio_tx_mutex init error\r\n");

    char    buf[MAXLINE];
    pid_t   pid;
    int     status;

    printf("%% ");
    while(fgets(buf, MAXLINE, stdin) != NULL)
    {
        process_log("fgets buf len is ------------------:1");
        buf[strlen(buf) -1] = 0;/* replace newline with null */
        pid = fork();
        process_log("fork pid is: %d",pid);

        require_action(pid>=0,exit,process_log("fork error !"));
        if(pid == 0)/*child*/
        {
            process_log("I am the child    PID:  %d",pid);            
            execlp(buf,buf,(char *)0);
            process_log("couldn't execute: %s",buf);
        }
        else
        {
            process_log("I am the father   PID:  %d",pid); 
            pid = waitpid(pid,&status,0);
            require_action(pid>=0,exit,process_log("waitpid error !"));
        }
        printf("%% ");
    }
    
exit:
    exit(0);
}