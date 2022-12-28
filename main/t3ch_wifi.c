#include "nvs.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "t3ch_config.h"

//--
static const char *TAG = "T3CH_WIFI";
// Try retrive AP ssid & pwd from nvs storage
static bool tmpAP_success = false;
static char tmpAP_SSID[128] = {0};
static char tmpAP_PWD[128] = {0};

//
void t3ch_wifi_start_sta(void) {
	printf("t3ch_wifi_start_sta() STARTING\n");
	
	//
	nvs_handle_t nvsh;
	esp_err_t err;
	//
	esp_bridge_create_station_netif(NULL, NULL, false, false);
	//
	StartSTA(STA_SSID, STA_PWD);
}

//
void t3ch_wifi_start_ap(void) {
	//
	nvs_handle_t nvsh;
	esp_err_t err;
	//
	esp_bridge_create_softap_netif(NULL, NULL, true, true);
	//
	err = nvs_open("ap_storage",NVS_READWRITE,&nvsh);
	//
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
	//
    esp_bridge_wifi_set(WIFI_MODE_AP, (tmpAP_success?tmpAP_SSID:AP_SSID), (tmpAP_success?tmpAP_PWD:AP_PWD), NULL);
    //
    //
    nvs_close(nvsh);
}
