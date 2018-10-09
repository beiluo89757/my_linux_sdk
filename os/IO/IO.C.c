#include "lucas.h"

#define IO_log(M, ...) custom_log("IO", M, ##__VA_ARGS__)

pthread_mutex_t stdio_tx_mutex;




// off_t	 lseek(int, off_t, int whence);

// • 若 w h e n c e是 S E E K _ S E T ，则将该文件的位移量设置为距文件开始处 offset 个字节。
// • 若w h e n c e 是 S E E K _ C U R， 则 将 该 文 件 的 位 移 量 设 置 为 其 当 前 值 加 o f f s e t , o f f s e t 可为正或负。
// • 若w h e n c e是 S E E K _ E N D，则将该文件的位移量设置为文件长度加 o f f s e t , o f f s e t 可为正或负。

int main(int argc, char *argv[])
{
    if(pthread_mutex_init(&stdio_tx_mutex, NULL) < 0)
        printf(" stdio_tx_mutex init error\r\n");

    int         fd;
    ssize_t     len;
    char        buf[1024];

    off_t       seek_len;

    fd = open("hellworld.txt",O_RDWR | O_CREAT,0777);
    require_action(fd>=0,exit,IO_log("open failed"));

    seek_len = lseek(fd, -1, SEEK_END);
    require_action(seek_len!=-1,exit,IO_log("seek failed"));

    len = read(fd,buf,sizeof(buf));

    IO_log("read lenth is :%d",len);

    if(len>0)
        IO_log("%s",buf);


    close(fd);

exit:
    exit(0);   
}