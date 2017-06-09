/******************************************************************************
 * Copyright 2013-2014 
 *
 * FileName: UdpServer.c
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
#include "freertos/timers.h"

#include "main.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "esp_types.h"
#include "esp_wifi.h"


#define UDP_DATA_LEN          100

xTaskHandle pvUdpServerThreadHandle;


/**
	* @brief  no.
	* @note   no.
	* @param  no.
	* @retval no.
	*/
void UdpServerTask( void *pvParameters )
{
	int32_t sock_fd , ret;
	struct sockaddr_in ServerAddr;
	struct sockaddr from;
	uint8_t fromlen;
	uint8_t Mac[6];
	
	int32_t nNetTimeout = 60000; 
	char udp_msg[ UDP_DATA_LEN ];

	esp_wifi_get_mac( ESP_IF_WIFI_STA , Mac );
	
	xEventGroupWaitBits( smartconfig_event_group, LINK_OVER_BIT, false, true, portMAX_DELAY );
		
	sock_fd = socket( AF_INET , SOCK_DGRAM , 0 );

	if( sock_fd == -1 )
	{
		printf("Udp server get socket fail!\r\n");

		vTaskDelete(NULL);
		return;
	}

	memset( &ServerAddr , 0 , sizeof( ServerAddr ) );
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_addr.s_addr = INADDR_ANY;
	ServerAddr.sin_port = htons( 6666 );           										// UDP本地服务器端口为6666.
	ServerAddr.sin_len = sizeof( ServerAddr );

	fromlen = sizeof( struct sockaddr_in );

	if( bind( sock_fd , ( struct sockaddr * )&ServerAddr , sizeof( ServerAddr ) ) != 0 )
	{
		printf("Udp server bind port fail!\r\n");
		
		vTaskDelete(NULL);
		return;
	}

	setsockopt( sock_fd ,SOL_SOCKET , SO_RCVTIMEO ,(char*)&nNetTimeout, sizeof(int) );
	
	for( ;; )
	{
		ret = recvfrom( sock_fd , (uint8_t *)udp_msg , UDP_DATA_LEN , 0 , (struct sockaddr *)&from , (socklen_t *)&fromlen );

		if( ret > 0 )
		{
			udp_msg[ret] = '\0';

			printf("Udp server recive msg:%s\r\n" , udp_msg );

			if( strcmp( udp_msg , "find ESP32") == 0)
			{
				sprintf( udp_msg , "I'm a ESP32 device,MAC=%02X%02X%02X%02X%02X%02X\r\n" , Mac[0], Mac[1], Mac[2], Mac[3], Mac[4], Mac[5] );
			}

			sendto( sock_fd , (uint8_t *)udp_msg , strlen( udp_msg ) , 0 , (struct sockaddr *)&from , fromlen );
		}
	}

	vTaskDelete(NULL);
}

/**
	* @brief  no .	  
	* @note   no.
	* @param  no.
	* @retval no.
	*/
void UdpServerInit( void )
{
	pvUdpServerThreadHandle = sys_thread_new( "Udp Server Task" ,  UdpServerTask , NULL, 2048 , 4 );
	if( pvUdpServerThreadHandle != NULL )
	{
		printf("Udp Server thread is Created!\r\n"  );
	}

}

