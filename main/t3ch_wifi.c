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
#include "esp_log.h"
//
static const char *TAG = "T3CH_WIFI";
//
char STASSID[32]={0};
char STAPWD[64]={0};
//
char VERSION[12]         = {0};
char VERSION_STRING[128] = {0};
char AP_SSID[32]         = {0};
char AP_PWD[64]          = {0};
//
bool t3ch_wifi_scan_running = false;
#define T3CH_WIFI_MAX_SCAN_SIZE 10
uint16_t t3ch_wifi_scan_number = T3CH_WIFI_MAX_SCAN_SIZE;
wifi_ap_record_t t3ch_wifi_scan_ap_info[T3CH_WIFI_MAX_SCAN_SIZE];
uint16_t t3ch_wifi_scan_ap_count = 0;
char t3ch_wifi_scan_json[256]={0};

//--
// Check if STA interface is up
bool t3ch_wifi_sta_isup(void) {
	esp_netif_t *sta_if = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
	if( sta_if==NULL ) {
		return false;
	}
	return true;
}

// Configure STA interface UP
bool t3ch_wifi_sta_up(void) {
	esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t err = esp_wifi_init(&cfg);
    if( err!=ESP_OK ) {
	    ESP_ERROR_CHECK(err);
	    return false;
	}
	//
	err = esp_wifi_set_mode(WIFI_MODE_APSTA);
	if( err!=ESP_OK ) {
	    ESP_ERROR_CHECK(err);
		return false;
	}
	return true;
}
//
bool t3ch_wifi_scan_start(void) {
	//
	if( t3ch_wifi_scan_running ) {
		printf("t3ch_wifi_scan_start() Failed, already running!");
		return false;
	}
	// Check if STA interface is up. If not set it up.
	if( !t3ch_wifi_sta_isup() ) {
		printf("t3ch_wifi_scan_start() configuring STA interface.\n");
		if( !t3ch_wifi_sta_up() ) {
			printf("t3ch_wifi_scan_start() Failed configuring STA interface!\n");
			return false;
		}
	}
	//
	t3ch_wifi_scan_ap_count = 0;
	//t3ch_wifi_scan_ap_info[T3CH_WIFI_MAX_SCAN_SIZE];
	memset(t3ch_wifi_scan_ap_info, 0, sizeof(t3ch_wifi_scan_ap_info));
	//
	t3ch_wifi_scan_running = true;
	// Start scan
	esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&t3ch_wifi_scan_number, t3ch_wifi_scan_ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&t3ch_wifi_scan_ap_count));
    ESP_LOGI(TAG, "Total APs scanned = %u", t3ch_wifi_scan_ap_count);
    for (int i = 0; (i < T3CH_WIFI_MAX_SCAN_SIZE) && (i < t3ch_wifi_scan_ap_count); i++) {
        ESP_LOGI(TAG, "SSID \t\t%s", t3ch_wifi_scan_ap_info[i].ssid);
        ESP_LOGI(TAG, "RSSI \t\t%d", t3ch_wifi_scan_ap_info[i].rssi);
        ESP_LOGI(TAG, "MODE \t\t%d", t3ch_wifi_scan_ap_info[i].authmode);
        ESP_LOGI(TAG, "Channel \t\t%d\n", t3ch_wifi_scan_ap_info[i].primary);
    }
    //
    t3ch_wifi_scan_running = false;
    return true;
}
//
int t3ch_wifi_scan_gen(void) {
	int size=0,len=256;
	t3ch_wifi_scan_json[len];
	memset(t3ch_wifi_scan_json,'\0',len);
	//
	if( t3ch_wifi_scan_ap_count<=0 ) return 0;
	//
	size += sprintf(t3ch_wifi_scan_json+size,"[");
	for (int i = 0; (i < T3CH_WIFI_MAX_SCAN_SIZE) && (i < t3ch_wifi_scan_ap_count); i++) {
        //ESP_LOGI(TAG, "SSID \t\t%s", t3ch_wifi_scan_ap_info[i].ssid);
        //ESP_LOGI(TAG, "RSSI \t\t%d", t3ch_wifi_scan_ap_info[i].rssi);
        //ESP_LOGI(TAG, "MODE \t\t%d", t3ch_wifi_scan_ap_info[i].authmode);
        //ESP_LOGI(TAG, "Channel \t\t%d\n", t3ch_wifi_scan_ap_info[i].primary);
        size += sprintf(t3ch_wifi_scan_json+size,
            "{\"ssid\":\"%s\",\"rssi\":\"%d\",\"mode\":\"%d\",\"chan\":\"%d\"}%s",
            t3ch_wifi_scan_ap_info[i].ssid,
            t3ch_wifi_scan_ap_info[i].rssi,
            t3ch_wifi_scan_ap_info[i].authmode,
            t3ch_wifi_scan_ap_info[i].primary,
            (i>=(t3ch_wifi_scan_ap_count-1)?"":","));
        printf("t3ch_wifi_scan_gen() DEBUG size: %d\n",size);
        // increase json if too small
        if( size>=(len-128) ) {
			len += 128;
			printf("t3ch_wifi_scan_gen() increasing json len: %d\n",len);
			char tmp[len];//={0}; # variable sized object can not be initialized! :)
			strcpy(tmp,t3ch_wifi_scan_json);
			t3ch_wifi_scan_json[len];
			memset(t3ch_wifi_scan_json,'\0',len);
			strcpy(t3ch_wifi_scan_json,tmp);
		}
    }
    size += sprintf(t3ch_wifi_scan_json+size,"]");
    printf("t3ch_wifi_scan_gen() DONE size: %i\n",size);
    return size;
}
//
void t3ch_wifi_scan_get(char *out) {
	strcpy(out,t3ch_wifi_scan_json);
}
//--
//
char *t3ch_version_string(void) {
	return VERSION_STRING;
}
//
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

//
void t3ch_wifi_init(char ap_ssid[], char ap_pwd[], char version[], char version_string[]) {
	ESP_LOGI(TAG,"t3ch_wifi_init() STARTING. AP_SSID: %s, AP_PWD: %s, VERSION: %s, VS: %s",
	    ap_ssid, ap_pwd, version, version_string);
	strcpy(AP_SSID,ap_ssid);
	strcpy(AP_PWD,ap_pwd);
	strcpy(VERSION,version);
	strcpy(VERSION_STRING,version_string);
}
