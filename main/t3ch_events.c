#include "t3ch_events.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "esp_log.h"
//--
static const char *TAG = "T3CH_EVENTS";

//
static void t3ch_event_ap_stach(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
	wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
    ESP_LOGI(TAG, "STA Connecting to the AP "MACSTR" join, AID=%d",MAC2STR(event->mac), event->aid);
    stats.count_sta_connect++;
}
//
static void t3ch_event_ap_stadh(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
	wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
    ESP_LOGI(TAG, "STA Disconnecting from the AP "MACSTR"",MAC2STR(event->mac));
    stats.count_sta_disconnect++;
}
//
void t3ch_events_get_count_sta_connect(void) {
	printf("t3ch_events_count_sta_connect: %i\n",stats.count_sta_connect);
}

//
void t3ch_events_init(void) {
	//
	stats.count_sta_connect = 0;
	stats.count_sta_disconnect = 0;
	//
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED, &t3ch_event_ap_stach, NULL, NULL));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, &t3ch_event_ap_stadh, NULL, NULL));

}
