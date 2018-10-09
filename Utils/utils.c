#include "debug.h"

void printRawData (char type, char *p, int len)
{
    extern pthread_mutex_t stdio_tx_mutex;
    
	pthread_mutex_lock( &stdio_tx_mutex );

	if (type==U_RECV)
		printf ("UART <--:   ");
	else if (type==U_SEND)
		printf ("UART -->:");
	else if (type==CUR_STATUS)
		printf ("CUR_STATUS -->:\r\n");
	else if (type==lucas_status)
		printf ("lucas_status -->:   ");

	for (int i =0; i<len; i++) 
	{
		if(p[i]==0x0d)
		{
			printf("CR");
		}
		else
		 printf ("%c", p[i]);
		//printf ("%02x", p[i]);
		printf (" ");
	}
	printf ("\r\n");

	pthread_mutex_unlock( &stdio_tx_mutex );
}
void msleep(uint32_t timeout)
{
    struct timeval Timeout;

	Timeout.tv_sec = 0;
	Timeout.tv_usec = 1000*timeout;
    select(0, NULL, NULL, NULL, &Timeout);
}


uint32_t get_os_time(void)
{
    return __TIME__;
}

void lower_to_upper(char *buf, uint32_t len)
{
    for(int i=0; i<len; i++)
    {
        buf[i] = toupper(buf[i]);
    }
}


void setStrTime(char * strTime)
{
    time_t timep;
    time (&timep);
    struct tm *p;
    p=localtime(&timep);
    sprintf(strTime,"%d/%02d/%02d %02d:%02d:%02d",(1900+p->tm_year),(1+p->tm_mon),p->tm_mday,p->tm_hour,p->tm_min, p->tm_sec);
}
