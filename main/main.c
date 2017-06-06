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

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t smartconfig_event_group = NULL;

static const char *TAG = "example";

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
#define     LINKED_BIT      (BIT0)
#define     CONNECTED_BIT   (BIT1)



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
                printf("Phone ip: %d.%d.%d.%d\n",phone_ip[0],phone_ip[1],phone_ip[2],phone_ip[3]);
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



void app_main(void)
{
	smartconfig_event_group = xEventGroupCreate();

    nvs_flash_init();
	initialise_wifi();
	
	ESP_ERROR_CHECK( esp_smartconfig_set_type( SC_TYPE_ESPTOUCH ) );
    ESP_ERROR_CHECK( esp_smartconfig_start(smartconfig_done ) );
	
    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
    int level = 0;
    while (true) {
        gpio_set_level(GPIO_NUM_4, level);
        level = !level;
        vTaskDelay(300 / portTICK_PERIOD_MS);
    }
}

