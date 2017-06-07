#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "driver/gpio.h"
#include "esp_smartconfig.h"
#include "esp_types.h"
//#include "UdpServer.h"

/* FreeRTOS Task Handle -------------------------------------------------------*/
TaskHandle_t user_key_task_handle = NULL;

/* FreeRTOS Semaphore Handle --------------------------------------------------*/
SemaphoreHandle_t xUserKeySemaphore = NULL;


/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t smartconfig_event_group = NULL;

static const char *TAG = "example";

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
#define     LINKED_BIT      (BIT0)
#define     CONNECTED_BIT   (BIT1)

#define 	LED_GPIO_NUM	GPIO_NUM_25

#define 	USER_KEY_GPIO_SEL	GPIO_SEL_0
#define 	USER_KEY_GPIO_NUM	GPIO_NUM_0


esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
		xEventGroupWaitBits(smartconfig_event_group, LINKED_BIT, false, true, portMAX_DELAY);
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. */
        esp_wifi_connect();
		xEventGroupClearBits(smartconfig_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;

}

static void smartconfig_done(smartconfig_status_t status, void *pdata)
{
    switch(status) {
        case SC_STATUS_WAIT:
            ESP_LOGI(TAG,"SC_STATUS_WAIT\n");
            break;
        case SC_STATUS_FIND_CHANNEL:
            ESP_LOGI(TAG,"SC_STATUS_FIND_CHANNEL\n");
            break;
        case SC_STATUS_GETTING_SSID_PSWD:
            ESP_LOGI(TAG,"SC_STATUS_GETTING_SSID_PSWD\n");
            smartconfig_type_t *type = pdata;
            if (*type == SC_TYPE_ESPTOUCH) {
                ESP_LOGI(TAG,"SC_TYPE:SC_TYPE_ESPTOUCH\n");
            } else {
                ESP_LOGI(TAG,"SC_TYPE:SC_TYPE_AIRKISS\n");
            }
            break;
        case SC_STATUS_LINK:
            ESP_LOGI(TAG,"SC_STATUS_LINK\n");
			
			wifi_config_t wifi_config;
			wifi_config.sta = *((wifi_sta_config_t *)pdata);
	        ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA , &wifi_config) );
			
	        ESP_ERROR_CHECK( esp_wifi_disconnect() );
			xEventGroupSetBits(smartconfig_event_group, LINKED_BIT);
            break;
        case SC_STATUS_LINK_OVER:
           ESP_LOGI(TAG,"SC_STATUS_LINK_OVER\n");
            if (pdata != NULL) {
				//SC_TYPE_ESPTOUCH
                uint8_t phone_ip[4] = {0};

                memcpy(phone_ip, (uint8_t*)pdata, 4);
                ESP_LOGI(TAG,"Phone ip: %d.%d.%d.%d\n",phone_ip[0],phone_ip[1],phone_ip[2],phone_ip[3]);
            } else {
            	//SC_TYPE_AIRKISS - support airkiss v2.0
//				airkiss_start_discover();
			}
            esp_smartconfig_stop();
			xEventGroupSetBits(smartconfig_event_group, CONNECTED_BIT);
            break;
    }
	
}

static void initialise_wifi(void)
{
    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_init(wifi_event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
/*	
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "TP-LINK_9C84",
            .password = "tp_luyou",
        },
    };
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
*/	
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}
/**
	* @brief  no .    
	* @note   no.
	* @param  no.
	* @retval no.
	*/
void user_key_task( void *pvParameters )
{
	vSemaphoreCreateBinary( xUserKeySemaphore );
	xSemaphoreTake( xUserKeySemaphore , 20 / portTICK_PERIOD_MS );
	
	for( ;; )
	{
		if( xSemaphoreTake( xUserKeySemaphore, portMAX_DELAY) == pdTRUE )
		{
			printf( "user key gpio intr!\n" );

			uint8_t i;

			for( i = 0; i < 10 ; i++ )
			{
				vTaskDelay(500 / portTICK_PERIOD_MS);

				if( gpio_get_level( USER_KEY_GPIO_NUM ) == 1 )
				{
					break;
				}
			}

			if( i == 10 )										// °´¼ü°´ÏÂ5s.
			{
				esp_smartconfig_stop();
				ESP_ERROR_CHECK( esp_smartconfig_set_type( SC_TYPE_ESPTOUCH ) );
    			ESP_ERROR_CHECK( esp_smartconfig_start( smartconfig_done ) );
			}
		}
		
	}
}
/**
    * @brief  no .    
    * @note   no.
    * @param  no.
    * @retval no.
    */
void led_init( void )
{
	gpio_set_direction( LED_GPIO_NUM , GPIO_MODE_OUTPUT );
}

/**
    * @brief  no .    
    * @note   no.
    * @param  no.
    * @retval no.
    */
void led_task( void *pvParameters )
{
	int level = 0;
	
	for( ;; )
	{
        gpio_set_level( LED_GPIO_NUM , level);
        level = !level;
        vTaskDelay(300 / portTICK_PERIOD_MS);
	}
}

/**
    * @brief  no .    
    * @note   no.
    * @param  no.
    * @retval no.
    */
void user_key_gpio_isr_handler(void* arg)
{
	static BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;

	if( xUserKeySemaphore != NULL )
	{
		xSemaphoreGiveFromISR( xUserKeySemaphore, &xHigherPriorityTaskWoken );
	}
	
}

/**
    * @brief  no .    
    * @note   no.
    * @param  no.
    * @retval no.
    */
void UserKeyInit( )
{
	gpio_config_t pGPIOConfig;

	pGPIOConfig.pin_bit_mask = USER_KEY_GPIO_SEL;
	pGPIOConfig.mode = GPIO_MODE_INPUT;
	pGPIOConfig.pull_up_en = GPIO_PULLUP_DISABLE;
	pGPIOConfig.intr_type = GPIO_INTR_NEGEDGE;

	gpio_config( (gpio_config_t *)&pGPIOConfig );

    //install gpio isr service
    gpio_install_isr_service( ESP_INTR_FLAG_LEVEL1 );
	//hook isr handler for specific gpio pin
	gpio_isr_handler_add( USER_KEY_GPIO_NUM , user_key_gpio_isr_handler , (void *)USER_KEY_GPIO_NUM );
}

void app_main(void)
{
	smartconfig_event_group = xEventGroupCreate();

    nvs_flash_init();
	initialise_wifi();
	UserKeyInit();
	led_init();
	
	xTaskCreate( &user_key_task, "user key task", 1024, NULL, 6, &user_key_task_handle );
	configASSERT( user_key_task_handle );
	
	xTaskCreate( &led_task, "led task", 512, NULL, 5, NULL );
//	UdpServerInit( );
}

