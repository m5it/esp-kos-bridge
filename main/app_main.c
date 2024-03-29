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

/**    Started by Espressif Systems - modified by by t3ch aka B.K.
 * -------------------------------------------------------------------
 *               ESP-KOS-BRIDGE => WiFi Extender / Timer
 *               https://github.com/m5it/esp-kos-bridge
 * -------------------------------------------------------------------
 *            If you like project consider donating. 
 *                   Donate - Welcome - Thanks!
 *    https://www.paypal.com/donate/?hosted_button_id=QGRYL4SL5N4FE
 * Donate - Donar - Spenden - Daruj - пожертвовать - दान करना - 捐 - 寄付
 */
 
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
#include "t3ch_config.h"
//-- 
// Includes for console & ping
#include "esp_mac.h"              // contain MAC2STR()
#include "esp_console.h"
#include "t3ch_console.h"
#include "t3ch_log.h"
//--
// Includes for DHT sensor
#include "dht.h"
#include <esp_http_server.h>
//
static esp_console_repl_t *s_repl = NULL;
//
#define BUTTON_NUM            1
#define BUTTON_SW1            CONFIG_GPIO_BUTTON_SW1
#define BUTTON_PRESS_TIME     5000000
#define BUTTON_REPEAT_TIME    5
//
static const char *TAG = "MAIN";
static button_handle_t g_btns[BUTTON_NUM] = {0};
static bool button_long_press = false;
static esp_timer_handle_t restart_timer;

//--
//
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
void start_after_wifi() {
	ESP_LOGI(TAG,"start_after_wifi() STARTING!");
	
	t3ch_time_sntp_init();
    if( t3ch_time_sntp_updated()==false ) {
		ESP_LOGI(TAG, "Time is not set yet. Updating trough NTP.");
		t3ch_time_sntp_update();
	}
}
//
void start_after_gotip() {
    //
	#ifdef ENABLE_HTTPS
	    StartHTTPS();
	#else
		StartHTTP();
	#endif
}
//--
//
void app_main(void)
{
	//
    esp_log_level_set("*", ESP_LOG_INFO);
    //
#ifdef ENABLE_LOG
	t3ch_log_init();
    esp_log_set_vprintf( &t3ch_log ); // set our function for logging from t3ch_log.c
    ESP_LOGI(TAG,"Enabling t3ch_log()...!");
#endif
	//
	t3ch_wifi_init(AP_SSID, AP_PWD, VERSION, VERSION_STRING);
    //
	ESP_LOGI(TAG,"#--\n");
	ESP_LOGI(TAG,"# ESP-KOS-BRIDGE Starting v%s - %s\n",t3ch_version(), t3ch_version_string());
	ESP_LOGI(TAG,"#--\n");
	
	//
	ESP_LOGI(TAG,"Initializing storage.");
    esp_err_t err = esp_storage_init();
    ESP_ERROR_CHECK( err );
    
    //-- Use nvs_flash_erase() here just to fix super problems.
    //nvs_flash_erase();
    //--
    
    //
	esp_bridge_wifi_set_events_ap_start( start_after_wifi );
	esp_bridge_wifi_set_events_sta_gotip( start_after_gotip );
	
    //
    ESP_LOGI(TAG,"Initializing network interfaces.");
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

	//
	ESP_LOGI(TAG,"Preparing console.");
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    //
    repl_config.prompt = "kos>";
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
    register_sta();
    register_test();
    register_dht();
    register_ap();
    // Start console REPL
    ESP_ERROR_CHECK(esp_console_start_repl(s_repl));
    //--
    t3ch_events_init();
    time_timer_init();
    
    //--
    // configure sta & ap from t3ch_config or console or web
    t3ch_wifi_start_ap();
    t3ch_wifi_start_sta();
    
    ///-- This things below should start after station is connected to X ap and AP is created or similar!
    ///------------------------------------------------––------------------------------------------------
    //--
    /*t3ch_time_sntp_init();
    if( t3ch_time_sntp_updated()==false ) {
		ESP_LOGI(TAG, "Time is not set yet. Updating trough NTP.");
		t3ch_time_sntp_update();
	}
	
    //--
    //
#ifdef ENABLE_HTTPS
    StartHTTPS();
#else
	StartHTTP();
#endif
    */
}
