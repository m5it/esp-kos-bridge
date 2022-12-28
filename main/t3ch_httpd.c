#include "esp_wifi.h"

#include "t3ch_httpd.h"
#include "t3ch_nvs.h"
#include "t3ch_time.h"
//#include "t3ch_time.h"
#include "dht.h"
//
//#include "cJSON.h"
//--
static const char *TAG = "T3CH_HTTPD";
static nvs_handle_t nvsh;
static esp_err_t err;
//
static struct tm timeinfo;
//
static struct stats {
    int restart_count;
    char *start_time;
    char *restart_time;
} s;


//-- FUnctioNS
//-------------
// check if any request attribute exists
size_t t3ch_httpd_get_param(httpd_req_t *req, const char *key, char *paramout) {
	size_t buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
		char *buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query(%i) => %s", strlen(buf),buf);
            int x = chrat(buf,'=');
            ESP_LOGI(TAG, "Found separator(%i) at %i", strlen(buf)-x, x);
            if (httpd_query_key_value(buf, key, paramout, strlen(buf)-x) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => %s => %s", key, paramout);
            }
		}
		return buf_len;
	}
	return 0;
}


//-- REQUESTS - RESPONSES
// An HTTP GET handler
static esp_err_t home_get_handler(httpd_req_t *req)
{
	/*char out[256]={0};
	size_t test = t3ch_httpd_get_param(req, "test", &out);
	if( test > 0 ) {
		ESP_LOGI(TAG, "home_get_handler() d1 got param( %d ) => %s", test, out);
	}
	test = t3ch_httpd_get_param(req, "test2", &out);
	if( test > 0 ) {
		ESP_LOGI(TAG, "home_get_handler() d2 got param( %d ) => %s", test, out);
	}*/
	
    /*char*  buf;
    size_t buf_len;

    // Get header value string length and allocate memory for length + 1,
    // extra byte for null termination
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        // Copy null terminated value string into buffer
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Host: %s", buf);
        }
        free(buf);
    }*/

    

    // Set some custom headers
    //httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");
    //httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2");
    //const char* resp_str = (const char*) req->user_ctx;
    //httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);


    httpd_resp_send(req, HTTP_PANEL, strlen(HTTP_PANEL));
    return ESP_OK;
}
//
static const httpd_uri_t home_get = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = home_get_handler,
    //.user_ctx  = "Hello World!"
};

//
static esp_err_t dht_get_handler(httpd_req_t *req)
{
	float temp, humi;
	dht_read_float_data(DHT_TYPE_DHT11,26,&humi,&temp);
	char res[256];
	sprintf(res,"DHT response Humidity: %.1f, Temperature: %.1f<br>\n", humi, temp);
	httpd_resp_send(req, res, strlen(res));
    return ESP_OK;
}
//
static const httpd_uri_t dht_get = {
    .uri       = "/dht",
    .method    = HTTP_GET,
    .handler   = dht_get_handler,
};

//
static esp_err_t reset_get_handler(httpd_req_t *req)
{
	//-- undefined reference to `esp_mesh_lite_erase_rtc_store'
	//esp_mesh_lite_erase_rtc_store();
    nvs_flash_erase();
    esp_restart();
    return ESP_OK;
}
//
static const httpd_uri_t reset_get = {
    .uri       = "/reset",
    .method    = HTTP_GET,
    .handler   = reset_get_handler,
};

//-- WIFI INFO
//
static esp_err_t wifi_get_handler(httpd_req_t *req)
{
	char res[128]={0};
	char ap_ssid[32]={0};
	char ap_pwd[64]={0};
	char sta_ssid[32]={0};
	char sta_pwd[64]={0};
	//
	wifi_ap_config_t cap;
	wifi_sta_config_t csta;
	//
	esp_wifi_get_config(WIFI_IF_AP, (wifi_config_t*)&cap);
	esp_wifi_get_config(WIFI_IF_STA, (wifi_config_t*)&csta);

    //
    if (strlen((const char*)cap.ssid)) {
        strcpy(ap_ssid,(const char*) cap.ssid);
        strcpy(ap_pwd,(const char*) cap.password);
    }
    else {
		printf("wifi_get_handler() cap.ssid not set!?");
	}
	
	//
    if (strlen((const char*)csta.ssid)) {
        strcpy(sta_ssid,(const char*) csta.ssid);
        strcpy(sta_pwd,(const char*) csta.password);
    }
    else {
		printf("wifi_get_handler() csta.ssid not set!");
		strcpy(sta_ssid,"Not set");
		strcpy(sta_pwd,"Not set");
	}
	
	//
	sprintf(res,"{\"success\":true,\"ap_ssid\":\"%s\",\"ap_pwd\":\"%s\",\"sta_ssid\":\"%s\",\"sta_pwd\":\"%s\"}",
	    ap_ssid, ap_pwd, sta_ssid, sta_pwd);
	//
	httpd_resp_send(req, res, strlen(res));
    return ESP_OK;
}
//
static const httpd_uri_t wifi_get = {
    .uri       = "/wifi",
    .method    = HTTP_GET,
    .handler   = wifi_get_handler,
};

//-- TIME
//
static esp_err_t time_get_handler(httpd_req_t *req) {
	//
	char strftime_buf[64];
	char res[128]={0};
	char out[128]={0};
	// settm should be JSON generated object
	size_t chk = t3ch_httpd_get_param(req, "settm", &out);
	// set new settings
	if( chk > 0 ) {
		printf("timetm_get_handler() settm(%ld) => %s\n", chk, out);
		
		char tm_hour[4]={0};
		char tm_min[4]={0};
		char tm_sec[4]={0};
		char tm_year[4]={0};
		char tm_mon[4]={0};
		char tm_mday[4]={0};
		
		//
		chk = t3ch_httpd_get_param(req, "tm_hour", &tm_hour);
		if(chk<=0) strcpy(tm_hour,"23");//tm_hour = "23";
		chk = t3ch_httpd_get_param(req, "tm_min", &tm_min);
		if(chk<=0) strcpy(tm_min,"0");//tm_min = "0";
		chk = t3ch_httpd_get_param(req, "tm_sec", &tm_sec);
		if(chk<=0) strcpy(tm_sec,"0");//tm_sec = "0";
		//
		chk = t3ch_httpd_get_param(req, "tm_year", &tm_year);
		printf("tm_year chk: %i\n",chk);
		if(chk<=0) strcpy(tm_year,"2022");//tm_year = "2022";
		
		chk = t3ch_httpd_get_param(req, "tm_mon", &tm_mon);
		printf("tm_mon chk: %i\n",chk);
		if(chk<=0) strcpy(tm_mon,"12");//tm_mon = "12";
		
		chk = t3ch_httpd_get_param(req, "tm_mday", &tm_mday);
		printf("tm_mday chk: %i\n",chk);
		if(chk<=0) strcpy(tm_mday,"1");//tm_mday = "1";
		
		printf("timetm_get_handler() tm_hour(%ld) => %s:%s:%s\n", chk, tm_hour, tm_min, tm_sec);
		//
		int tmhour = strtol(tm_hour, (char**)NULL, 10);
		int tmmin  = strtol(tm_min, (char**)NULL, 10);
		int tmsec  = strtol(tm_sec, (char**)NULL, 10);
		//
		int tmyear = strtol(tm_year, (char**)NULL, 10);
		int tmmon  = strtol(tm_mon, (char**)NULL, 10);
		int tmmday = strtol(tm_mday, (char**)NULL, 10);
		printf("timetm_get_handler() tmhour:tmmin:tmsec => %i:%i:%i\n", tmhour, tmmin, tmsec);
		printf("timetm_get_handler() tmyear:tmmon:tmmday => %i:%i:%i\n", tmyear, tmmon, tmmday);
		//
		struct tm tm;
	    tm.tm_year = tmyear - 1900;
	    tm.tm_mon  = tmmon;
	    tm.tm_mday = tmmday;
	    tm.tm_hour = tmhour;
	    tm.tm_min  = tmmin;
	    tm.tm_sec  = tmsec;
		
		printf("timetm_get_handler() setting time...\n");
		t3ch_time_set_tm( tm );
	}
	
	//
	t3ch_time_get(&strftime_buf);
	sprintf(res,"{\"success\":true,\"data\":\"%s\"}",strftime_buf);
	//
	httpd_resp_send(req, res, strlen(res));
    return ESP_OK;
}
//
static const httpd_uri_t time_get = {
    .uri       = "/time/",
    .method    = HTTP_GET,
    .handler   = time_get_handler,
};


//-- FREE
//
static esp_err_t free_get_handler(httpd_req_t *req)
{
	char res[64];
	/*char tmp_start_time[8];
	char tmp_restart_time[8];
	char tmp_restart_count[8];
	// append additional statistics: start_time, restart_time, restart_count
	nvs_open("httpd_storage",NVS_READWRITE,&nvsh);
	t3ch_nvs_get_str(nvsh,"start_time",tmp_start_time);
	t3ch_nvs_get_str(nvsh,"restart_time",tmp_restart_time);
	t3ch_nvs_get_str(nvsh,"restart_count",tmp_restart_count);*/
	//
	//sprintf(res,"{\"success\":true,\"data\":\"%d\",\"start_time\":\"%s\",\"restart_time\":\"%s\",\"restart_count\":\"%s\"}", heap_caps_get_free_size(MALLOC_CAP_8BIT), tmp_start_time, tmp_restart_time, tmp_restart_count);
	sprintf(res,"{\"success\":true,\"data\":\"%d\"}", heap_caps_get_free_size(MALLOC_CAP_8BIT));
	//
	//nvs_close( nvsh );
	//
	httpd_resp_send(req, res, strlen(res));
    return ESP_OK;
}
//
static const httpd_uri_t free_get = {
    .uri       = "/free",
    .method    = HTTP_GET,
    .handler   = free_get_handler,
};

//
esp_err_t httpd_error(httpd_req_t *req, httpd_err_code_t err)
{
    /*if (strcmp("/hello", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/hello URI is not available");
        return ESP_OK;
    } else if (strcmp("/echo", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/echo URI is not available");
        return ESP_FAIL;
    }*/
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Not found.");
    return ESP_FAIL;
}

bool StartWeb(void) {
	httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    //config.lru_purge_enable = true;

	//--
    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &home_get);
        httpd_register_uri_handler(server, &dht_get);
        httpd_register_uri_handler(server, &reset_get);
        httpd_register_uri_handler(server, &free_get);
        httpd_register_uri_handler(server, &time_get);
        httpd_register_uri_handler(server, &wifi_get);
        httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, httpd_error);
    }
    
    //-- Save some data into nvs for statistics
    //
    err = nvs_open("httpd_storage",NVS_READWRITE,&nvsh);
    if( err!=ESP_OK ) return false;
    //
    char nvsout[128];
    char timeout[64];
    t3ch_time_get(timeout);
    
    // START TIME
    err = t3ch_nvs_get_str(nvsh,"start_time",nvsout);
    if( err!=ESP_OK ) { // start_time dont exists. lets set it.
		err = nvs_set_str(nvsh,"start_time",timeout);
		if( err==ESP_OK ) {
			ESP_LOGI(TAG, "start_time is set: %s",timeout);
		}
	}
	
	// RESTART TIME
	err = t3ch_nvs_get_str(nvsh,"restart_time",nvsout);
    if( err!=ESP_OK ) { //
		ESP_LOGI(TAG, "restart_time not set, setting to: %s",timeout);
	}
	else {
		ESP_LOGI(TAG, "restart_time was set: %s",nvsout);
	}
	err = nvs_set_str(nvsh,"restart_time",timeout);
	if( err!=ESP_OK ) {
		ESP_LOGI(TAG, "Failed setting restart_time");
	}
	
	// RESTART COUNT
	err = t3ch_nvs_get_str(nvsh,"restart_count",nvsout);
    if( err!=ESP_OK ) { //
		ESP_LOGI(TAG, "restart_count not set, setting to 1");
		nvs_set_str(nvsh,"restart_count","1");
	}
	else {
		int restart_count = strtol( nvsout, (char**)NULL, 10);
		restart_count += 1;
		ESP_LOGI(TAG, "restart_count increasing: %i",restart_count);
		char tmp[3];
		sprintf(tmp,"%i",restart_count);
		nvs_set_str(nvsh,"restart_count",tmp);
	}
	nvs_commit( nvsh );
	nvs_close( nvsh );
}
