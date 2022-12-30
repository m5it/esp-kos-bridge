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

#include "t3ch_nvs.h"
#include "esp_log.h"

static const char *TAG = "T3CH_NVS";

esp_err_t t3ch_nvs_get_str(nvs_handle_t NVSH, char *key, char *out) {
	char nvsout[256]={0};
	size_t rs;
	nvs_get_str(NVSH,key,NULL,&rs); // get size
	printf("t3ch_nvs_get_str() key: %s, size: %i\n",key,rs);
	//int rss = (int)rs;
	// int convertdata = static_cast<int>(data);
	esp_err_t err = nvs_get_str(NVSH,key,nvsout,&rs); // get value
	ESP_LOGI(TAG, "t3ch_nvs_get_str() nvsout: %s",nvsout);
	strcpy(out,nvsout);
}
