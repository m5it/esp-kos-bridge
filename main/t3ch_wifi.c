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

#include "nvs.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "t3ch_config.h"

//--
static const char *TAG = "T3CH_WIFI";
char APSSID[32]={0};
char APPWD[64]={0};
char STASSID[32]={0};
char STAPWD[64]={0};

char *t3ch_version_string(void) {
	return VERSION_STRING;
}

char *t3ch_version(void) {
	return VERSION;
}
//
void t3ch_wifi_start_sta(void) {
	printf("t3ch_wifi_start_sta() STARTING\n");
	
	//
	nvs_handle_t nvsh2;
	esp_err_t err;
	//
	bool success = false;
	char SSID[32] = {0};
	char PWD[64] = {0};
	//
	err = nvs_open("sta_storage",NVS_READWRITE,&nvsh2);
	//
	if( err==ESP_OK ) {
		char nvsout[64]={0};
		//
		err = t3ch_nvs_get_str(nvsh2, "ssid",&SSID);
		printf("t3ch_wifi_start_sta() ssid: %s\n",SSID);
		//if( err==ESP_OK ) {
		if(err!=ESP_OK) {
			printf("t3ch_wifi_start_sta() something wrong with ssid?: %s\n",SSID);
		}
		if(strlen(SSID)>0) {
			//strncpy(SSID, (uint8_t*)nvsout, strlen(nvsout)+1);
			printf("t3ch_wifi_start_sta() starting STA anyway... for me looks good!\n");
			success = true;
		}
		//
		err = t3ch_nvs_get_str(nvsh2, "pwd",&PWD);
		printf("t3ch_wifi_start_sta() pwd: %s\n",PWD);
		if( err==ESP_OK ) {
			//strncpy(PWD, (uint8_t*)nvsout, strlen(nvsout)+1);
		}
		
		if( success ) {
			printf("t3ch_wifi_start_sta() STARTING esp_bridge_create_station_netif()...\n");
			//
			esp_bridge_create_station_netif(NULL, NULL, false, false);
			//
			StartSTA(SSID, PWD);
		}
		else {
			printf("t3ch_wifi_start_sta() not success. not starting...\n");
		}
	}
	else printf("t3ch_wifi_start_sta() open of storage nvs failed.\n");
	nvs_close(nvsh2);
}

//
bool t3ch_wifi_update_sta(char *ssid, char *pwd) {
	printf("t3ch_wifi_update_sta() STARTING. ssid:%s, pwd: %s\n", ssid, pwd);
	
	//
	nvs_handle_t nvsh;
    esp_err_t err = nvs_open("sta_storage",NVS_READWRITE,&nvsh);
    if( err!=ESP_OK ) {
		printf("t3ch_wifi_update_ap() Failed (d1).\n");
		return false;
	}
	//
	err = nvs_set_str(nvsh,"ssid",ssid);
	if(err!=ESP_OK) {
		printf("t3ch_wifi_update_ap() Failed (d2).\n");
		return false;
	}
	//
	err = nvs_set_str(nvsh,"pwd",pwd);
	if(err!=ESP_OK) {
		printf("t3ch_wifi_update_ap() Failed (d3).\n");
		return false;
	}
	//
	err = nvs_commit( nvsh );
	if(err!=ESP_OK) {
		printf("t3ch_wifi_update_ap() Failed (d4).\n");
		return false;
	}
	nvs_close(nvsh);
	return true;
}

//
void t3ch_wifi_start_ap(void) {
	//
	nvs_handle_t nvsh1;
	esp_err_t err;
	//
	bool success = false;
	char SSID[32] = {0};
	char PWD[64] = {0};
	//
	esp_bridge_create_softap_netif(NULL, NULL, true, true);
	//
	err = nvs_open("ap_storage",NVS_READWRITE,&nvsh1);
	//
	if( err==ESP_OK ) {
		char nvsout[64]={0};
		//
		err = t3ch_nvs_get_str(nvsh1, "ssid",&SSID);
		printf("t3ch_wifi_start_ap() ssid: %s\n",SSID);
		if( err==ESP_OK ) {
			//strncpy(SSID, (uint8_t*)nvsout, strlen(nvsout)+1);
			printf("t3ch_wifi_start_ap() ssid looks ok?!\n");
			success = true;
		}
		//
		err = t3ch_nvs_get_str(nvsh1, "pwd",&PWD);
		printf("t3ch_wifi_start_ap() pwd: %s\n",PWD);
		if( err==ESP_OK ) {
			//strncpy(PWD, (uint8_t*)nvsout, strlen(nvsout)+1);
		}
	}
	else printf("t3ch_wifi_start_ap() open of storage nvs failed.\n");
	//
    esp_bridge_wifi_set(WIFI_MODE_AP, (success?SSID:AP_SSID), (success?(strlen(PWD)>0?PWD:""):AP_PWD), NULL);
    //
    //
    nvs_close(nvsh1);
}

//
bool t3ch_wifi_update_ap(char *ssid, char *pwd) {
	//
	nvs_handle_t nvsh;
    esp_err_t err = nvs_open("ap_storage",NVS_READWRITE,&nvsh);
    if( err!=ESP_OK ) {
		printf("t3ch_wifi_update_ap() Failed (d1).\n");
		return false;
	}
	//
	err = nvs_set_str(nvsh,"ssid",ssid);
	if(err!=ESP_OK) {
		printf("t3ch_wifi_update_ap() Failed (d2).\n");
		return false;
	}
	//
	err = nvs_set_str(nvsh,"pwd",pwd);
	if(err!=ESP_OK) {
		printf("t3ch_wifi_update_ap() Failed (d3).\n");
		return false;
	}
	//
	err = nvs_commit( nvsh );
	if(err!=ESP_OK) {
		printf("t3ch_wifi_update_ap() Failed (d4).\n");
		return false;
	}
	nvs_close(nvsh);
	//
	esp_bridge_wifi_set(WIFI_MODE_AP, ssid, pwd, NULL);
	    
	return true;
}
