/******************************************************************************
 * Copyright 2013-2014 
 *
 * FileName:TcpServer.c
 *
 * Description: 
 *
 * Modification history:
 *     2014/12/1, v1.0 create this file.
*******************************************************************************/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "main.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"

xTaskHandle pvTcpServerThreadHandle;

int StartUp( uint16_t port )
{
	int sock_fd = 0;
	struct sockaddr_in name;

	sock_fd = socket( PF_INET, SOCK_STREAM, 0 );
	if (sock_fd == -1)
	{
		return -1;
	}
	
	memset( &name, 0, sizeof( name ) );
	
	name.sin_family = AF_INET;
	name.sin_port = htons(port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);
	if ( bind( sock_fd, (struct sockaddr *)&name, sizeof(name)) < 0 )
	{
		return -1;
	}

	if ( port == 0)  /* if dynamically allocating a port */
	{
		return -1;
	}
	if ( listen(sock_fd, 5) < 0 )
	{
		return -1;
	}
	
	return(sock_fd);	
}

/**
	* @brief  no .	  
	* @note   no.
	* @param  no.
	* @retval no.
	*/

void tcp_server_thread( void *pvParameters )
{
	char *ReadBuf = NULL;
	int ReadLen;
	int server_sock = -1, client_sock = -1 ;
	
	struct sockaddr_in client_name;
	socklen_t client_name_len = sizeof( client_name );

	xEventGroupWaitBits( smartconfig_event_group, LINK_OVER_BIT, false, true, portMAX_DELAY );

	ReadBuf = malloc( 1461 );
		
	if( ReadBuf == NULL )
	{
		vTaskDelete(NULL);
		return;
	}
	
	server_sock = StartUp( 88 );

	if( server_sock == -1 )
	{
		printf("Tcp server startup fail!\r\n");

		vTaskDelete(NULL);
		return;
	}

	for( ;; )
	{
		client_sock = accept( server_sock , (struct sockaddr *)&client_name , &client_name_len );

		if( client_sock != -1)
		{
			printf("accept socket!\r\n");
			for( ;; )
			{
				ReadLen = recv( client_sock , ReadBuf , 1460 , 0 );
				if( ReadLen > 0 )
				{
					ReadBuf[ReadLen] = '\0';
					printf("Tcp server recive msg:%s!\r\n" , ReadBuf );
					send( client_sock , ReadBuf , ReadLen , 0 );
				}
				else
				{
					if( ( ReadLen < 0 )&& ( errno != EAGAIN ) )
					{
						break;
					}
					else if( ReadLen == 0 )
					{
						break;
					}
				}
			}
		}

		close( client_sock );
		printf("close socket!\r\n");
	}
}

/**
	* @brief  no .	  
	* @note   no.
	* @param  no.
	* @retval no.
	*/
void TcpServerInit( void )
{
	pvTcpServerThreadHandle = sys_thread_new("tcp server thread" ,  tcp_server_thread , NULL, 2048 , 5 );
	if( pvTcpServerThreadHandle != NULL )
	{
		printf("Tcp Server thread is Created!\r\n"  );
	}
}

