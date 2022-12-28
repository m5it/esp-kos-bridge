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
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "freertos/event_groups.h"

#include "lwip/opt.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/ip_addr.h"
#include "lwip/lwip_napt.h"
#include "dhcpserver/dhcpserver.h"

#include "esp_bridge_config.h"
#include "esp_bridge_wifi.h"
#include "esp_bridge_internal.h"

#if defined(CONFIG_BRIDGE_EXTERNAL_NETIF_STATION) || defined(CONFIG_BRIDGE_DATA_FORWARDING_NETIF_SOFTAP)

#define BRIDGE_EVENT_STA_CONNECTED  BIT0

static const char *TAG = "bridge_wifi";
static bool esp_bridge_softap_dhcps = false;
static EventGroupHandle_t s_wifi_event_group = NULL;
//<<<<<<< HEAD
//
OVERRIDE_AP_SSID[128]={0};
OVERRIDE_AP_PWD[128]={0};
//
#if CONFIG_MESH_LITE_ENABLE
#include "esp_mesh_lite.h"
#endif

//static esp_err_t esp_bridge_wifi_set(wifi_mode_t mode, const char *ssid, const char *password, const char *bssid)
esp_err_t esp_bridge_wifi_set(wifi_mode_t mode,
                              const char *ssid,
                              const char *password,
                              const char *bssid)
{
	printf("bridge_wifi.c ->  esp_bridge_wifi_set() starting, ssid: %s, pwd: %s\n",ssid, password);
	
    ESP_PARAM_CHECK(ssid);
    ESP_PARAM_CHECK(password);

    wifi_config_t wifi_cfg;
    memset(&wifi_cfg, 0x0, sizeof(wifi_config_t));

    if (mode & WIFI_MODE_STA) {
		printf("bridge_wifi.c -> esp_bridge_wifi_set() MODE_STA!\n");
        memcpy((char *)wifi_cfg.sta.ssid, ssid, sizeof(wifi_cfg.sta.ssid));
        strlcpy((char *)wifi_cfg.sta.password, password, sizeof(wifi_cfg.sta.password));
        if (bssid != NULL) {
            wifi_cfg.sta.bssid_set = 1;
            memcpy((char *)wifi_cfg.sta.bssid, bssid, sizeof(wifi_cfg.sta.bssid));
            ESP_LOGI(TAG, "sta ssid: %s password: %s MAC "MACSTR"", ssid, password, MAC2STR(wifi_cfg.sta.bssid));
        } else {
            ESP_LOGI(TAG, "sta ssid: %s password: %s", ssid, password);
        }

        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_cfg));
    }

    if (mode & WIFI_MODE_AP) {
		printf("bridge_wifi.c ->  esp_bridge_wifi_set() MODE_AP!\n");
		//
        char softap_ssid[ESP_BRIDGE_SSID_MAX_LEN + 1];
        memcpy(softap_ssid, ssid, sizeof(softap_ssid));
#if CONFIG_ESP_BRIDGE_SOFTAP_SSID_END_WITH_THE_MAC
        uint8_t softap_mac[ESP_BRIDGE_MAC_MAX_LEN];
        char suffix[8];
        esp_wifi_get_mac(WIFI_IF_AP, softap_mac);
        snprintf(suffix, sizeof(suffix), "_%02x%02x%02x", softap_mac[3], softap_mac[4], softap_mac[5]);
        strcat(softap_ssid, suffix);
#endif
        memcpy((char *)wifi_cfg.ap.ssid, softap_ssid, sizeof(wifi_cfg.ap.ssid));
        strlcpy((char *)wifi_cfg.ap.password, password, sizeof(wifi_cfg.ap.password));
        wifi_cfg.ap.max_connection = CONFIG_ESP_BRIDGE_SOFTAP_MAX_CONNECT_NUMBER;
        wifi_cfg.ap.authmode = strlen((char *)wifi_cfg.ap.password) < 8 ? WIFI_AUTH_OPEN : WIFI_AUTH_WPA2_PSK;
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_cfg));

        ESP_LOGI(TAG, "softap ssid: %s password: %s", (char *)wifi_cfg.ap.ssid, (char *)wifi_cfg.ap.password);
    }

    return ESP_OK;
}

/* Event handler for catching system events */
static void wifi_event_sta_disconnected_handler(void *arg, esp_event_base_t event_base,
                                                int32_t event_id, void *event_data)
{
	printf("DEBUG wifi_event_sta_disconnected_handler() STARTING.\n");
#if !CONFIG_BRIDGE_STATION_CANCEL_AUTO_CONNECT_WHEN_DISCONNECTED
    ESP_LOGE(TAG, "Disconnected. Connecting to the AP again...");
    esp_wifi_connect();
#endif
    xEventGroupClearBits(s_wifi_event_group, BRIDGE_EVENT_STA_CONNECTED);
}

static void ip_event_sta_got_ip_handler(void *arg, esp_event_base_t event_base,
                                        int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    ESP_LOGI(TAG, "Connected with IP Address:" IPSTR, IP2STR(&event->ip_info.ip));

    esp_bridge_netif_network_segment_conflict_update(event->esp_netif);
    xEventGroupSetBits(s_wifi_event_group, BRIDGE_EVENT_STA_CONNECTED);
}

static void wifi_event_ap_start_handler(void *arg, esp_event_base_t event_base,
                                        int32_t event_id, void *event_data)
{
	printf("DEBUG wifi_event_ap_start_handler() STARTING.\n");
	//
    esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");

    if (netif) {
		printf("DEBUG wifi_event_ap_start_handler() netif Looks OK!\n");
        if (esp_bridge_softap_dhcps) {
			printf("DEBUG wifi_event_ap_start_handler() STARTING dhcps!\n");
            esp_netif_dhcps_stop(netif);
            esp_netif_dns_info_t dns;
            dns.ip.u_addr.ip4.addr = ESP_IP4TOADDR(114, 114, 114, 114);
            dns.ip.type = IPADDR_TYPE_V4;
            dhcps_offer_t dhcps_dns_value = OFFER_DNS;
            ESP_ERROR_CHECK(esp_netif_dhcps_option(netif, ESP_NETIF_OP_SET, ESP_NETIF_DOMAIN_NAME_SERVER, &dhcps_dns_value, sizeof(dhcps_dns_value)));
            ESP_ERROR_CHECK(esp_netif_set_dns_info(netif, ESP_NETIF_DNS_MAIN, &dns));
            ESP_ERROR_CHECK(esp_netif_dhcps_start(netif));
        }

        esp_netif_ip_info_t netif_ip;
        esp_netif_get_ip_info(netif, &netif_ip);
        ip_napt_enable(netif_ip.ip.addr, 1);
    }
}

/*static void wifi_event_ap_staconnected_handler(void *arg, esp_event_base_t event_base,
                                               int32_t event_id, void *event_data)
{
	wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
    ESP_LOGI(TAG, "STA Connecting to the AP "MACSTR" join, AID=%d",MAC2STR(event->mac), event->aid);
}*/

/*static void wifi_event_ap_stadisconnected_handler(void *arg, esp_event_base_t event_base,
                                                  int32_t event_id, void *event_data)
{
    ESP_LOGE(TAG, "STA Disconnect to the AP");
}*/

static esp_err_t esp_bridge_wifi_init(void)
{
	printf("bridge_wifi.c -> esp_bridge_wifi_init() STARTING.\n");
    if (s_wifi_event_group) {
		printf("bridge_wifi.c -> esp_bridge_wifi_init() LooKS OK.\n");
        return ESP_OK;
    }

    s_wifi_event_group = xEventGroupCreate();

	printf("bridge_wifi.c -> esp_bridge_wifi_init() STARTING WIFI_INIT_CONFIG_DEFAULT().\n");
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t err = esp_wifi_init(&cfg);
    if( err!=ESP_OK ) {
		printf("bridge_wifi.c -> esp_bridge_wifi_init() FAILED!\n");
	}
    ESP_ERROR_CHECK(err);

	printf("bridge_wifi.c -> esp_bridge_wifi_init() STARTING esp_wifi_set_storage...\n");
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    printf("bridge_wifi.c -> esp_bridge_wifi_init() STARTING esp_wifi_set_mode(WIFI_MODE_NULL)!\n");
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));

	printf("bridge_wifi.c -> esp_bridge_wifi_init() STARTING esp_wifi_start()!\n");
    esp_wifi_start();

    return ESP_OK;
}
#endif /* CONFIG_BRIDGE_EXTERNAL_NETIF_STATION || CONFIG_BRIDGE_DATA_FORWARDING_NETIF_SOFTAP */

#if defined(CONFIG_BRIDGE_EXTERNAL_NETIF_STATION)

esp_netif_t* esp_bridge_create_station_netif(esp_netif_ip_info_t* ip_info, uint8_t mac[6], bool data_forwarding, bool enable_dhcps)
{
	printf("bridge_wifi.c -> esp_bridge_create_station_netif() STARTING\n");
    esp_netif_t *wifi_netif = NULL;
    wifi_mode_t mode = WIFI_MODE_NULL;

    if (data_forwarding || enable_dhcps) {
		printf("DEBUG esp_bridge_create_station_netif() returning NULL!\n");
        return wifi_netif;
    }

    esp_bridge_wifi_init();
    wifi_netif = esp_netif_create_default_wifi_sta();
    esp_bridge_netif_list_add(wifi_netif, NULL);

    esp_wifi_get_mode(&mode);
    mode |= WIFI_MODE_STA;
    ESP_ERROR_CHECK(esp_wifi_set_mode(mode));

    wifi_sta_config_t router_config;
    /* Get WiFi Station configuration */
    esp_wifi_get_config(WIFI_IF_STA, (wifi_config_t*)&router_config);

    /* Get Wi-Fi Station ssid success */
    if (strlen((const char*)router_config.ssid)) {
        ESP_LOGI(TAG, "Found ssid %s",     (const char*) router_config.ssid);
        ESP_LOGI(TAG, "Found password %s", (const char*) router_config.password);
    }

    /* Register our event handler for Wi-Fi, IP and Provisioning related events */
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &wifi_event_sta_disconnected_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_sta_got_ip_handler, NULL, NULL));

    return wifi_netif;
}
#endif /* CONFIG_BRIDGE_EXTERNAL_NETIF_STATION */

#if defined(CONFIG_BRIDGE_DATA_FORWARDING_NETIF_SOFTAP)
static esp_err_t softap_netif_dhcp_status_change_cb(esp_ip_addr_t *ip_info)
{
    esp_err_t ret = ESP_FAIL;
    ESP_LOGW(TAG, "SoftAP IP network segment has changed, deauth all station");
    if (esp_wifi_deauth_sta(0) == ESP_OK) {
        ret = ESP_OK;
    }

    return ret;
}

//

//
esp_netif_t* esp_bridge_create_softap_netif(esp_netif_ip_info_t* ip_info, uint8_t mac[6], bool data_forwarding, bool enable_dhcps)
{
	printf("bridge_wifi.c -> esp_bridge_create_softap_netif() STARTING\n");
    esp_netif_ip_info_t netif_ip;
    esp_netif_ip_info_t allocate_ip_info;
    esp_netif_t *wifi_netif = NULL;
    wifi_mode_t mode = WIFI_MODE_NULL;

    if (!data_forwarding) {
        return wifi_netif;
    }

	printf("bridge_wifi.c -> esp_bridge_create_softap_netif() STARTING esp_bridge_wifi_init()\n");
    esp_bridge_wifi_init();

     wifi_ap_config_t ap_config;
    /* Get WiFi ap configuration */
    esp_wifi_get_config(WIFI_IF_AP, (wifi_config_t*)&ap_config);
    printf("bridge_wifi.c -> esp_bridge_create_softap_netif() AP ssid %s\n",     (const char*)ap_config.ssid);
    printf("bridge_wifi.c -> esp_bridge_create_softap_netif() AP password %s\n", (const char*)ap_config.password);
	
	//
	if( strlen(OVERRIDE_AP_SSID)>0 ) {
		printf("bridge_wifi.c -> esp_bridge_create_softap_netif() overriding SSID: %s & PWD: %s\n", OVERRIDE_AP_SSID, OVERRIDE_AP_PWD);
		strncpy(ap_config.ssid, (uint8_t*)OVERRIDE_AP_SSID, strlen(OVERRIDE_AP_SSID)+1);
		strncpy(ap_config.ssid, (uint8_t*)OVERRIDE_AP_PWD, strlen(OVERRIDE_AP_PWD)+1);
	}
	
	//
    wifi_config_t wifi_config_ap = {
        .ap = {
            .ssid = (const char*)ap_config.ssid,
            .ssid_len = strlen( (const char*)ap_config.ssid ),
            .channel = 6,
            .password = (const char*)ap_config.password,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .pmf_cfg = {
                    .required = false,
            },
        },
    };
    
    
    //esp_wifi_stop();
    //vTaskDelay(3000/portTICK_PERIOD_MS);
    //esp_wifi_start();
	//ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config_ap));
	
	printf("bridge_wifi.c -> esp_bridge_create_softap_netif() STARTING esp_netif_create_default_wifi_ap()\n");
    wifi_netif = esp_netif_create_default_wifi_ap();
	vTaskDelay(3000/portTICK_PERIOD_MS);
	esp_err_t err = esp_netif_dhcps_stop(wifi_netif);
	if( err!=ESP_OK ) {
		printf("bridge_wifi.c -> esp_bridge_create_softap_netif() FAILED esp_netif_create_default_wifi_ap()\n");
	}
    ESP_ERROR_CHECK(err);

    if (ip_info) {
        ESP_ERROR_CHECK(esp_netif_set_ip_info(wifi_netif, ip_info));
    } else {
        if (enable_dhcps) {
			printf("bridge_wifi.c -> esp_bridge_create_softap_netif() STARTING esp_bridge_netif_request_ip()\n");
            esp_bridge_netif_request_ip(&allocate_ip_info);
            ESP_ERROR_CHECK(esp_netif_set_ip_info(wifi_netif, &allocate_ip_info));
        }
    }
    ESP_ERROR_CHECK(esp_netif_get_ip_info(wifi_netif, &netif_ip));
    printf("bridge_wifi.c -> esp_bridge_create_softap_netif() got IP Address:" IPSTR"\n", IP2STR(&netif_ip.ip));
    esp_bridge_netif_list_add(wifi_netif, softap_netif_dhcp_status_change_cb);

    esp_bridge_softap_dhcps = enable_dhcps;

    /* Register our event handler for Wi-Fi, IP and Provisioning related events */
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_AP_START, &wifi_event_ap_start_handler, NULL, NULL));
//<<<<<<< HEAD
    //ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED, &wifi_event_ap_staconnected_handler, NULL, NULL));
    //ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, &wifi_event_ap_stadisconnected_handler, NULL, NULL));
#ifdef CONFIG_MESH_LITE_ENABLE
    printf("bridge_wifi.c -> esp_bridge_create_softap_netif() NEW OPTION LITE_ENABLE STARTING!!!!\n");
    ESP_ERROR_CHECK(esp_event_handler_instance_register(ESP_MESH_LITE_EVENT, ESP_EVENT_ANY_ID, &esp_mesh_lite_event_ip_changed_handler, NULL, NULL));
#endif
//=======
//    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED, &wifi_event_ap_staconnected_handler, NULL, NULL));
//    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, &wifi_event_ap_stadisconnected_handler, NULL, NULL));
//>>>>>>> master
    esp_wifi_get_mode(&mode);
    mode |= WIFI_MODE_AP;
    ESP_ERROR_CHECK(esp_wifi_set_mode(mode));

	printf("bridge_wifi.c -> esp_bridge_create_softap_netif() DONE!\n");
	if( wifi_netif==NULL ) {
		printf("bridge_wifi.c -> esp_bridge_create_softap_netif() wifi_netif is NULL!\n");
	}
    //ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config_ap));

    return wifi_netif;
}
#endif // CONFIG_BRIDGE_DATA_FORWARDING_NETIF_SOFTAP
