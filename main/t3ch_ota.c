#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_https_ota.h"
#include "esp_wifi.h"

static const char *TAG = "T3CH_OTA";

bool t3ch_ota_download(char *url) {
	esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");//get_example_netif_from_desc(bind_interface_name);
    if (netif == NULL) {
        ESP_LOGE(TAG, "Can't find netif from interface description");
        //abort();
        return false;
    }
    struct ifreq ifr;
    esp_netif_get_netif_impl_name(netif, ifr.ifr_name);
    ESP_LOGI(TAG, "Bind interface name is %s", ifr.ifr_name);
    
	//
	esp_http_client_config_t config = {
        .url = url,
        //.event_handler = ota_event_handler,
        .keep_alive_enable = true,
        //.cert_pem = FULLCHAIN,
        .if_name  = &ifr,
	};
	config.skip_cert_common_name_check = true;
	//
	esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };
    
    ESP_LOGI(TAG, "Attempting to download update from %s", config.url);
    esp_err_t ret = esp_https_ota(&ota_config);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "OTA Succeed, Rebooting...");
        esp_restart();
        return true;
    } 
    return false;
}
