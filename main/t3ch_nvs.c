#include "t3ch_nvs.h"
#include "esp_log.h"

static const char *TAG = "T3CH_NVS";

esp_err_t t3ch_nvs_get_str(nvs_handle_t NVSH, char *key, char *out) {
	char nvsout[256]={0};
	size_t rs;
	nvs_get_str(NVSH,key,NULL,&rs);
	err = nvs_get_str(NVSH,key,nvsout,&rs);
	ESP_LOGI(TAG, "t3ch_nvs_get_str() nvsout: %s",nvsout);
	strcpy(out,nvsout);
}
