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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "esp_sntp.h"
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
#include "t3ch_config.h"
//-- 
// Includes for console & ping
#include "esp_mac.h"              // contain MAC2STR()
#include "esp_console.h"
#include "t3ch_console.h"
//--
// HTTPD
//#include "t3ch_httpd.h"
//--
// Includes for DHT sensor
#include "dht.h"
#include <esp_http_server.h>
//--
//
time_t now;
struct tm timeinfo;
char strftime_buf[64];
//
static esp_console_repl_t *s_repl = NULL;

#define BUTTON_NUM            1
#define BUTTON_SW1            CONFIG_GPIO_BUTTON_SW1
#define BUTTON_PRESS_TIME     5000000
#define BUTTON_REPEAT_TIME    5

static const char *TAG = "main";
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
void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

void app_sntp_init(void) {
	sntp_servermode_dhcp(1);      // accept NTP offers from DHCP server, if any
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(1, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
	sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
	sntp_init();
}

void app_sntp_update(void) {
	int retry = 0;
    const int retry_count = 15;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    time(&now);
    localtime_r(&now, &timeinfo);
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
    ESP_ERROR_CHECK(esp_console_start_repl(s_repl));
    //--
    t3ch_events_init();
    //
    esp_bridge_create_all_netif();
    
    //--
    //
    //PrepareAP(AP_SSID, AP_PWD);
    //StartScan();
    //
    StartWeb();
    
    //--
    // Try retrive AP ssid & pwd from nvs storage
    bool tmpAP_success = false;
	nvs_handle_t nvsh;
	char tmpAP_SSID[128] = {0};
	char tmpAP_PWD[128] = {0};
	err = nvs_open("ap_storage",NVS_READWRITE,&nvsh);
	if( err==ESP_OK ) {
		char nvsout[256]={0};
		size_t rs;
		nvs_get_str(nvsh,"ssid",NULL,&rs);
		err = nvs_get_str(nvsh,"ssid",nvsout,&rs);
		if( err==ESP_OK ) {
			printf("esp_bridge_create_softap_netif() configuring ssid: %s from nvs!\n",nvsout);
			strncpy(tmpAP_SSID, (uint8_t*)nvsout, strlen(nvsout)+1);
			tmpAP_success = true;
		}
		else printf("esp_bridge_create_softap_netif() get ap_ssid from nvs failed.\n");
		
		nvsout[256];
		nvs_get_str(nvsh,"pwd",NULL,&rs);
		err = nvs_get_str(nvsh,"pwd",nvsout,&rs);
		if( err==ESP_OK ) {
			printf("esp_bridge_create_softap_netif() configuring pwd: %s from nvs!\n",nvsout);
			strncpy(tmpAP_PWD, (uint8_t*)nvsout, strlen(nvsout)+1);
		}
		else {
			printf("esp_bridge_create_softap_netif() get ap_ssid from nvs failed.\n");
			tmpAP_success = false;
		}
	}
	else printf("esp_bridge_create_softap_netif() open of storage nvs failed.\n");
    esp_bridge_wifi_set(WIFI_MODE_AP, (tmpAP_success?tmpAP_SSID:AP_SSID), (tmpAP_success?tmpAP_PWD:AP_PWD), NULL);
    // configure sta from t3ch_config or console or web
    StartSTA(STA_SSID, STA_PWD);
    
    //--
    // prepare time
    setenv("TZ", "CST-0", 1);
    tzset();
    time(&now);
    localtime_r(&now, &timeinfo);
    //
    app_sntp_init();
    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2016 - 1900)) {
        ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
	    app_sntp_update();
    }
    //
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time is: %s", strftime_buf);
}
