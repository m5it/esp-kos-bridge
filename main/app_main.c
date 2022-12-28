// Copyright 2021-2022 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// AND BY KOS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include "nvs.h"
//#include "nvs_flash.h"
#include "t3ch_nvs.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_event.h"

#include "esp_bridge.h"
#include "web_server.h"
#include "iot_button.h"
#if defined(CONFIG_BRIDGE_USE_WIFI_PROVISIONING_OVER_BLE)
#include "wifi_prov_mgr.h"
#endif
//--
//
//#include "t3ch_config.h"
//-- 
// Includes for console & ping
#include "esp_mac.h"              // contain MAC2STR()
#include "esp_console.h"
#include "t3ch_console.h"
//--
// Includes for DHT sensor
#include "dht.h"
#include <esp_http_server.h>
//
//#include "cJSON.h"
//--
//
static esp_console_repl_t *s_repl = NULL;

#define BUTTON_NUM            1
#define BUTTON_SW1            CONFIG_GPIO_BUTTON_SW1
#define BUTTON_PRESS_TIME     5000000
#define BUTTON_REPEAT_TIME    5

static const char *TAG = "MAIN";
static button_handle_t g_btns[BUTTON_NUM] = {0};
static bool button_long_press = false;
static esp_timer_handle_t restart_timer;

static esp_err_t esp_storage_init(void)
{
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    return ret;
}

/*static void button_press_up_cb(void *hardware_data, void *usr_data)
{
    ESP_LOGI(TAG, "BTN: BUTTON_PRESS_UP");
    if (button_long_press) {
        ESP_ERROR_CHECK(esp_timer_stop(restart_timer));
        button_long_press = false;
    }
}

static void button_press_repeat_cb(void *hardware_data, void *usr_data)
{
    uint8_t press_repeat = iot_button_get_repeat((button_handle_t)hardware_data);
    ESP_LOGI(TAG, "BTN: BUTTON_PRESS_REPEAT[%d]", press_repeat);
}

static void button_long_press_start_cb(void *hardware_data, void *usr_data)
{
    ESP_LOGI(TAG, "BTN: BUTTON_LONG_PRESS_START");
    button_long_press = true;
    ESP_ERROR_CHECK(esp_timer_start_once(restart_timer, BUTTON_PRESS_TIME));
}

static void restart_timer_callback(void* arg)
{
    ESP_LOGI(TAG, "Restore factory settings");
    nvs_flash_erase();
    esp_restart();
}

static void esp_bridge_create_button(void)
{
    const esp_timer_create_args_t restart_timer_args = {
            .callback = &restart_timer_callback,
            .name = "restart"
    };
    ESP_ERROR_CHECK(esp_timer_create(&restart_timer_args, &restart_timer));

    button_config_t cfg = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {
            .gpio_num = BUTTON_SW1,
            .active_level = 0,
        },
    };
    g_btns[0] = iot_button_create(&cfg);
    iot_button_register_cb(g_btns[0], BUTTON_PRESS_UP, button_press_up_cb, 0);
    iot_button_register_cb(g_btns[0], BUTTON_PRESS_REPEAT, button_press_repeat_cb, 0);
    iot_button_register_cb(g_btns[0], BUTTON_LONG_PRESS_START, button_long_press_start_cb, 0);
}
*/

//
static struct mytimer {
    //
    struct tm start_time;
    struct tm end_time;
    //
    bool running;
};
//
struct mytimer myt[2];
TaskHandle_t h_myt;
static uint8_t ucptp_myt;

void TEMPOLARY_TIMER(void * pvp) {
	while(true) {
		for(int i=0; i<2; i++) {
			//
			if( !myt[i].running && t3ch_time_chk( myt[i].start_time, myt[i].end_time ) ) {
				printf("Temporary timer at %i not running, Starting.\n",i);
				
				bool chk = t3ch_time_chk( myt[i].start_time, myt[i].end_time );
				printf("Temporary timer is true: %s\n", (chk?"YES":"NO") );
				myt[i].running = true;
				gpio_set_level(GPIO_NUM_26,1);
			}
			else if( myt[i].running && !t3ch_time_chk( myt[i].start_time, myt[i].end_time ) ) {
				printf("Temporary timer at %i running, Shutting down.\n",i);
				myt[i].running = false;
				gpio_set_level(GPIO_NUM_26,0);
			}
			else if( myt[i].running ) {
				printf("Temporary timer at %i running...\n",i);
			}
			else {
				printf("Temporary timer at %i not running...\n",i);
			}
		}
		
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
}

void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_INFO);
	
	
	//
	printf("Initializing storage.");
    esp_err_t err = esp_storage_init();
	ESP_ERROR_CHECK( err );
    
    //
    printf("Initializing network interfaces.");
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

	//
	printf("Preparing console.");
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    
    // install console REPL environment
#if CONFIG_ESP_CONSOLE_UART
	ESP_LOGI(TAG,"DEBUG console d1");
    esp_console_dev_uart_config_t uart_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&uart_config, &repl_config, &s_repl));
#elif CONFIG_ESP_CONSOLE_USB_CDC
	ESP_LOGI(TAG,"DEBUG console d2");
    esp_console_dev_usb_cdc_config_t cdc_config = ESP_CONSOLE_DEV_CDC_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_usb_cdc(&cdc_config, &repl_config, &s_repl));
#elif CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG
	ESP_LOGI(TAG,"DEBUG console d3");
    esp_console_dev_usb_serial_jtag_config_t usbjtag_config = ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_usb_serial_jtag(&usbjtag_config, &repl_config, &repl));
#endif

    //-- Register Additional console commands
    //
    register_free();
    register_reset();
    register_list();
    register_test();
    register_dht();
    register_ap();
    // Start console REPL
    repl_config.prompt = "kos>";
    ESP_ERROR_CHECK(esp_console_start_repl(s_repl));
    //--
    t3ch_events_init();
    //
    //esp_bridge_create_all_netif();
    
    //
    StartWeb();
    
    //--
    // configure sta & ap from t3ch_config or console or web
    t3ch_wifi_start_ap();
    t3ch_wifi_start_sta();
    
    //--
    //StartScan();
    
    //--
    /*t3ch_time_sntp_init();
    if( t3ch_time_sntp_updated()==false ) {
		ESP_LOGI(TAG, "Time is not set yet. Updating trough NTP.");
		t3ch_time_sntp_update();
	}*/
	
	//--
	//
	/*char *json_string = "[{\"id\":\"25139\",\"date\":\"2016-10-27\",\"name\":\"Komfy Switch With Camera DKZ-201S\\/W Password Disclosure\"},{\"id\":\"25117\",\"date\":\"2016-10-24\",\"name\":\"NETDOIT weak password Vulnerability\"}]";
	cJSON *root = cJSON_Parse(json_string);
	int n = cJSON_GetArraySize(root);
	printf("DEBUG JSON...: %i\n",n);
	*/
	//--
	//
	//t3ch_time_tm(&myt[0].start_time);
	//t3ch_time_tm(&myt[0].end_time);
	//time_t tmptime;
    //time( &tmptime );
	//localtime_r(&tmptime,&myt[0].start_time);
	//localtime_r(&tmptime,&myt[0].end_time);
	myt[0].start_time.tm_hour = 17;
	myt[0].start_time.tm_min  = 30;
	//
	myt[0].end_time.tm_hour   = 23;
	myt[0].end_time.tm_min    = 0;
	//
	myt[0].running            = false;
	//--
	//
	//t3ch_time_tm(&myt[1].start_time);
	//t3ch_time_tm(&myt[1].end_time);
	myt[1].start_time.tm_hour = 4;
	myt[1].start_time.tm_min  = 0;
	//
	myt[1].end_time.tm_hour   = 8;
	myt[1].end_time.tm_min    = 0;
	//
	myt[1].running            = false;
	
	// START TIMER
	/*xTaskCreate(
		TEMPOLARY_TIMER,
		"TEMPTIMER",
		3000,
		&ucptp_myt,
		tskIDLE_PRIORITY, 
		&h_myt
	);*/
}
