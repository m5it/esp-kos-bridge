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

#include "esp_wifi.h"
#include "esp_ota_ops.h"
#include "esp_https_ota.h"
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
/*
[0;32mI (207308) T3CH_HTTPD: Found URL query(209) => upload=%EF%BF%BD%06%02%20%EF%BF%BD%14%08%40%EF%BF%BD%00%00%00%00%00%00%00%00%EF%BF%BD%01%00%00%00%00%01%20%00%40%3F%EF%BF%BD5%03%002T%CD%AB%00%00%00%00%00%00%00%00%00%00%00%00v1.0.0-217-g9d64b6&POS=0&ALL=16872[0m
[0;32mI (207332) T3CH_HTTPD: Found separator(203) at 6[0m
[0;32mI (207343) T3CH_HTTPD: Found URL query parameter => POS => 0[0m
[0;32mI (207343) T3CH_HTTPD: Found URL query(209) => upload=%EF%BF%BD%06%02%20%EF%BF%BD%14%08%40%EF%BF%BD%00%00%00%00%00%00%00%00%EF%BF%BD%01%00%00%00%00%01%20%00%40%3F%EF%BF%BD5%03%002T%CD%AB%00%00%00%00%00%00%00%00%00%00%00%00v1.0.0-217-g9d64b6&POS=0&ALL=16872[0m
[0;32mI (207367) T3CH_HTTPD: Found separator(203) at 6[0m
[0;32mI (207378) T3CH_HTTPD: Found URL query parameter => ALL => 16872[0m
ota_update_post_handler() upload start, data( 2 / 16872 )
*/
size_t t3ch_httpd_get_param(httpd_req_t *req, const char *key, char *paramout) {
	size_t buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
		char *buf = malloc(buf_len);
		memset(buf,'\0',buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query(%i) => %s", strlen(buf),buf);
            int x = chrat(buf,'=');
            ESP_LOGI(TAG, "Found separator(%i) at %i", strlen(buf)-x, x);
            //
            //char tmpbuf[strlen(buf)-x];
            //memset(tmpbuf,'\0',strlen(buf)-x);
            //
            if (httpd_query_key_value(buf, key, paramout, strlen(buf)-x) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => %s => %s", key, paramout);
            //    strcpy(paramout,tmpbuf);
            }
		}
		free(buf);
		return buf_len;
	}
	return 0;
}


//-- REQUESTS - RESPONSES
//
//-- HOME
//
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
    //nvs_flash_erase();
    esp_restart();
    return ESP_OK;
}
//
static const httpd_uri_t reset_get = {
    .uri       = "/reset",
    .method    = HTTP_GET,
    .handler   = reset_get_handler,
};

//-- TIMER
//
static esp_err_t timer_get_handler(httpd_req_t *req)
{
	printf("timer_get_handler() STARTING.\n");
	//char res[512]={0};
	char opt_stat[4]={0};
	char opt_add[4]={0};
	char opt_del[4]={0};
	char opt_start[4]={0};
	char opt_stop[4]={0};
	
	t3ch_httpd_get_param(req, "stat", &opt_stat);
	t3ch_httpd_get_param(req, "add", &opt_add);
	t3ch_httpd_get_param(req, "del", &opt_del);
	t3ch_httpd_get_param(req, "start", &opt_start);
	t3ch_httpd_get_param(req, "stop", &opt_stop);
	
	if( strlen(opt_stat)>0 ) {
		printf("timer_get_handler() retriving stat.\n");
		char res[64];
		memset(res,'\0',64);
		sprintf(res,"{\"success\":true,\"size\":\"%i\", \"pos\":\"%i\",\"running\":%s}",time_timer_size(),time_timer_pos(),(time_timer_running()?"true":"false"));
		httpd_resp_send(req, res, strlen(res));
	}
	else if( strlen(opt_add)>0 ) {
		printf("timer_get_handler() starting ADD.\n");
		char startHour[4]={0};
		char startMin[4]={0};
		char endHour[4]={0};
		char endMin[4]={0};
		char gpio[4]={0};
		t3ch_httpd_get_param(req, "startHour", &startHour);
		t3ch_httpd_get_param(req, "startMin", &startMin);
		t3ch_httpd_get_param(req, "endHour", &endHour);
		t3ch_httpd_get_param(req, "endMin", &endMin);
		t3ch_httpd_get_param(req, "gpio", &gpio);
		
		printf("timer_get_handler() (D1) sHour: %i, sMin: %i, eHour: %i, eMin: %i, gpio: %i\n",
		    startHour, startMin, endHour, endMin, gpio);
		
		int sHour = strtol(startHour, (char**)NULL, 10);
		int sMin  = strtol(startMin, (char**)NULL, 10);
		int eHour = strtol(endHour, (char**)NULL, 10);
		int eMin  = strtol(endMin, (char**)NULL, 10);
		int igpio = strtol(gpio, (char**)NULL, 10);
		
		printf("timer_get_handler() (D2) sHour: %i, sMin: %i, eHour: %i, eMin: %i, gpio: %i\n",
		    sHour, sMin, eHour, eMin, igpio);
		
		bool suc = time_timer_add(sHour, sMin, eHour, eMin, igpio);
		if( suc ) time_timer_save();
		char res[64];
		memset(res,'\0',64);
		sprintf(res,"{\"success\":%s,\"size\":\"%i\", \"pos\":\"%i\",\"running\":%s}",(suc?"true":"false"), time_timer_size(),time_timer_pos(),(time_timer_running()?"true":"false"));
		httpd_resp_send(req, res, strlen(res));
	}
	else if( strlen(opt_del)>0 ) {
		printf("timer_get_handler() starting DEL\n");
		int pos = strtol(opt_del, (char**)NULL, 10);
		time_timer_del( pos );
		char res[64];
		memset(res,'\0',64);
		sprintf(res,"{\"success\":true,\"size\":\"%i\", \"pos\":\"%i\",\"running\":%s}",time_timer_size(),time_timer_pos(),(time_timer_running()?"true":"false"));
		httpd_resp_send(req, res, strlen(res));
	}
	else if( strlen(opt_start)>0 ) {
		printf("timer_get_handler() starting START\n");
		time_timer_start();
		char res[64];
		memset(res,'\0',64);
		sprintf(res,"{\"success\":true}");
		httpd_resp_send(req, res, strlen(res));
	}
	else if( strlen(opt_stop)>0 ) {
		printf("timer_get_handler() starting STOP\n");
		time_timer_stop();
		char res[32];
		memset(res,'\0',32);
		sprintf(res,"{\"success\":true}");
		httpd_resp_send(req, res, strlen(res));
	}
	else {
		printf("timer_get_handler() retriving json of timers.\n");
		int len = time_timer_gen();
		char tmp[len+1];
		memset(tmp,'\0',len+1);
		time_timer_get(tmp);
		//char *tmp;
		//int len = time_timer_get(tmp);
		//char *tmp = time_timer_get();
		printf("timer_get_handler tmp(%i/%i): %s\n",len,strlen(tmp),tmp);
		//printf("timer_get_handler tmp(%i): %s\n",strlen(tmp),tmp);
		//strcpy(res,tmp);
		httpd_resp_send(req, tmp, strlen(tmp));
	}
	//httpd_resp_send(req, res, strlen(res));
    return ESP_OK;
}
//
static const httpd_uri_t timer_get = {
    .uri       = "/timer/",
    .method    = HTTP_GET,
    .handler   = timer_get_handler,
};

//-- WIFI INFO
//
static esp_err_t wifi_get_handler(httpd_req_t *req)
{
	char out_sta[128]={0}, out_ap[128]={0}, 
		out_scan_start[4]={0}, out_scan_get[4]={0};
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
	size_t chk_setap, chk_setsta;
	chk_setap  = t3ch_httpd_get_param(req, "setap", &out_ap);
	chk_setsta = t3ch_httpd_get_param(req, "setsta", &out_sta);
	t3ch_httpd_get_param(req, "scan_start", &out_scan_start);
	t3ch_httpd_get_param(req, "scan_get", &out_scan_get);
	//
	printf("DEBUG wifi_get_handler() out_ap: %s, chk_setap: %d\n", out_ap, chk_setap);
	printf("DEBUG wifi_get_handler() out_sta: %s, chk_setsta: %d\n", out_sta, chk_setsta);
	printf("DEBUG wifi_get_handler() out_scan_start: %s\n", out_scan_start);
	printf("DEBUG wifi_get_handler() out_scan_get: %s\n", out_scan_start);
	
	// set new settings
	if     ( strlen(out_ap)>0 ) {
		//
		chk_setap = t3ch_httpd_get_param(req, "ssid", &ap_ssid);
		printf("wifi_get_handler() setap... chk: %d, new ssid: %s", chk_setap, ap_ssid);
		//
		chk_setap = t3ch_httpd_get_param(req, "pwd", &ap_pwd);
		printf("wifi_get_handler() setap... chk: %d, new pwd: %s", chk_setap, ap_pwd);
		
		//
	    bool suc = t3ch_wifi_update_ap( ap_ssid, ap_pwd );
	    
		//
		sprintf(res,"{\"success\":%s,\"ap_ssid\":\"%s\",\"ap_pwd\":\"%s\"}",
		    (suc?"true":"false"), ap_ssid, ap_pwd);
	}
	else if( strlen(out_sta)>0 ) {
		//
		chk_setsta = t3ch_httpd_get_param(req, "ssid", &sta_ssid);
		printf("wifi_get_handler() setsta... chk: %d, new ssid: %s", chk_setsta, sta_ssid);
		//
		chk_setsta = t3ch_httpd_get_param(req, "pwd", &sta_pwd);
		printf("wifi_get_handler() setsta... chk: %d, new pwd: %s", chk_setsta, sta_pwd);
		
		//
	    bool suc = t3ch_wifi_update_sta( sta_ssid, sta_pwd );
	    
	    //
	    if( suc ) {
			printf("wifi_get_handler() setsta restarting esp 5s...\n");
			vTaskDelay(5000 / portTICK_PERIOD_MS);
			esp_restart();
		}
	    
		//
		sprintf(res,"{\"success\":%s,\"sta_ssid\":\"%s\",\"sta_pwd\":\"%s\"}",
		    (suc?"true":"false"), sta_ssid, sta_pwd);
	}
	//
	else if( strlen(out_scan_start)>0 ) {
		bool suc = t3ch_wifi_scan_start();
		//
		sprintf(res,"{\"success\":%s}",(suc?"true":"false"));
	}
	//
	else if( strlen(out_scan_get)>0 ) {
		int size = t3ch_wifi_scan_gen();
		if( size<=0 ) {
			//
			sprintf(res,"{\"success\":false}");
		}
		else {
			char data[size+1];
			memset(data,'\0',size+1);
			t3ch_wifi_scan_get( data );
			// fix response variable size
			res[size+129];
			memset(res,'\0',size+129);
			//
			sprintf(res,"{\"success\":true,\"data\":%s}",data);
			printf("DEBUG scanGet response(%i, %i): %s\n",strlen(res), strlen(data), res);
		}
	}
	//
	else {
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
		/*char tmpversion[4] = {0};
		sprintf(tmpversion,"%d",t3ch_version());
		char encversion[16] = {0};
		esp_web_url_encode(tmpversion,tmpversion,encversion,strlen(tmpversion));
		printf("wifi_get_handler() DEBUG tmpversion: %s, encversion: %s\n",tmpversion, encversion);
		*/
		sprintf(res,"{\"success\":true,\"ap_ssid\":\"%s\",\"ap_pwd\":\"%s\",\"sta_ssid\":\"%s\",\"sta_pwd\":\"%s\",\"version\":\"%s\",\"version_string\":\"%s\"}",
		    ap_ssid, ap_pwd, sta_ssid, sta_pwd, t3ch_version(), t3ch_version_string());
	}
	//
	httpd_resp_send(req, res, strlen(res));
    return ESP_OK;
}
//
static const httpd_uri_t wifi_get = {
    .uri       = "/wifi/",
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

//_-
/*static esp_err_t ota_data_post_handler(httpd_req_t *req)
{
    char *buf = ((web_server_context_t*) (req->user_ctx))->scratch;
    int total_len = req->content_len;
    int remaining_len = req->content_len;
    int received_len = 0;
    esp_err_t err = ESP_FAIL;
    esp_ota_handle_t update_handle = 0;
    const esp_partition_t *update_partition = esp_web_get_ota_update_partition();
    // check post data size
    if (update_partition->size < total_len) {
        ESP_LOGE(TAG, "ota data too long, partition size is %u, bin size is %d", update_partition->size, total_len);
        goto err_handler;
    }
    ESP_LOGI(TAG, "bin size is %d", total_len);
    memset(buf, 0x0, ESP_BRIDGE_WEB_SCRATCH_BUFSIZE * sizeof(char));
    // Send a message to MCU.
    // esp_at_port_write_data((uint8_t*)s_ota_start_response, strlen(s_ota_start_response));
    printf("%s\r\n", s_ota_start_response);
    // start ota
    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ota begin failed (%s)", esp_err_to_name(err));
        goto err_handler;
    }
    // receive ota data
    while (remaining_len > 0) {
        received_len = httpd_req_recv(req, buf, MIN(remaining_len, ESP_BRIDGE_WEB_SCRATCH_BUFSIZE)); // Receive the file part by part into a buffer
        if (received_len <= 0) { // received error
            if (received_len == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }
            ESP_LOGE(TAG, "Failed to receive post ota data, err = %d", received_len);
            esp_ota_end(update_handle);
            goto err_handler;
        }else { // received successfully
            err = esp_ota_write(update_handle, buf, received_len);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "ota write failed (%s)", esp_err_to_name(err));
                esp_ota_end(update_handle);
                goto err_handler;
            }
            remaining_len -= received_len;
        }
    }
    err = esp_web_ota_end(update_handle, update_partition);
    if (err != ESP_OK) {
        goto err_handler;
    }
    esp_web_response_ok(req);
    // esp_at_port_write_data((uint8_t*)s_ota_receive_success_response, strlen(s_ota_receive_success_response));
    printf("%s\r\n", s_ota_receive_success_response);
    ESP_LOGI(TAG, "ota end successfully, please restart");
    return ESP_OK;

err_handler:
    esp_web_response_error(req, HTTPD_500);
    // esp_at_port_write_data((uint8_t*)s_ota_receive_fail_response, strlen(s_ota_receive_fail_response));
    printf("%s\r\n", s_ota_receive_fail_response);
    return ESP_FAIL;
}*/
//static int update_upload_size;
static esp_ota_handle_t update_handle = 0;
static const esp_partition_t *update_partition;
static int dl0=0, dl1=0, dl2=0; 
//
static esp_err_t ota_update_post_handler(httpd_req_t *req)
{
	printf("ota_update_post_handler() starting..., total_len: %d\n",req->content_len);
	char res[64];
	int total_len = req->content_len;
    int remaining_len = req->content_len;
    int received_len = 0;
    int len=0;
	char out_download[512]={0}, out_upload[512]={0};
	size_t chk_download, chk_upload;
	//
	esp_err_t err;
	//esp_ota_handle_t update_handle = 0;
    //const esp_partition_t *update_partition;// = esp_web_get_ota_update_partition();
	//
	chk_download = t3ch_httpd_get_param(req, "download", &out_download);
	chk_upload   = t3ch_httpd_get_param(req, "upload", &out_upload);
	
	printf("ota_update_post_handler() total_len: %d\n",total_len);
	printf("ota_update_post_handler() download debug chk_download: %d, out_download: %s\n",chk_download, out_download);
	printf("ota_update_post_handler() upload debug chk_upload: %d, out_upload: %s\n",chk_upload, out_upload);
	
	//-- 1.) Option
	// Download image from http url
	if( strlen(out_download)>0 ) {
		printf("ota_update_post_handler() download start, url: %s\n",out_download);
		//
		char *newdec = (char*)malloc(strlen(out_download));
        memset(newdec,'\0', strlen(out_download));
        int tmpl = t3ch_urldecode(out_download,newdec,strlen(out_download));
        
        printf("ota_update_post_handler() updating from %s\n",newdec);
        //
		if( t3ch_ota_download(newdec) ) {
			free(newdec);
			esp_restart();
		}
		//
		sprintf(res,"{\"success\":true}");
		httpd_resp_send(req, res, strlen(res));
		printf("ota_update_post_handler() download done\n");
	    return ESP_OK;
	}
	//-- 2.) Option
	// Upload image as urlencoded chunks of data
	if( strlen(out_upload)>0 ) {
		char at[12]={0};
		char of[12]={0};
		char test[12]={0};
		t3ch_httpd_get_param(req, "at", &at);
		t3ch_httpd_get_param(req, "of", &of);
		t3ch_httpd_get_param(req, "test", &test);
		printf("ota_update_post_handler() upload start, data( %s / %s )\n",at,of);
		// receive ota data
		//char totalbuf[total_len+1];
		char *totalbuf = (char*)malloc(total_len);
		memset(totalbuf,'\0', total_len);
		
		int totallen = 0;
		int bs = 320;
		char buf[bs];
		memset(buf,'\0',bs);
		//
	    while (remaining_len > 0) {
	        received_len = httpd_req_recv(req, buf, bs); // Receive the file part by part into a buffer
	        //received_len = esp_http_client_read(req,buf,bs);
	        printf("ota_update_post_handler() received_len: %d, remaining: %d\n",received_len, remaining_len);
	        if( received_len<=0 ) {
				printf("ota_update_post_handler() received_len 0... breaking... remaining_len: %d\n",remaining_len);
				
				esp_ota_end(update_handle);
	            goto err_handler;
	            
				break;
			}
	        
	        //
	        printf("ota_update_post_handler() appending to totalbuf, len: %d / %d\n",received_len, totallen);
	        memcpy(totalbuf+totallen, buf, received_len);
	        totallen += received_len;
	        //char dec[received_len];
	        //memset(dec,'\0',received_len);
	        /*char *dec = b64decode( buf );
	        printf("ota_update_post_handler() Got base64decoded dec: %s\n",dec);
	        
	        char newdec[received_len];
	        memset(newdec,'\0', received_len);
	        int tmpl = t3ch_urldecode(dec,newdec,received_len);
	        printf("ota_update_post_handler() Got urldecoded newdec(%d): %s\n",tmpl,newdec);
	        //free(dec);
	        //
	        if( len==0 ) {
				printf("ota_update_post_handler() first chunk..\n");
				//hexDump("debug1: ",&buf, received_len, 20);
				hexDump("debug1: ",&newdec, sizeof(newdec)/sizeof(newdec[0]), 20);
				//char *pch = strstr(buf,"\r\n\r\n");
				//if( pch!=NULL ) {
				//	printf("ota_update_post_handler() fixing header data pch: \n%s\n",(pch+4));
				//	hexDump("debug2: ",(pch+4), sizeof(pch)/sizeof((pch-4)[0]), 20);
				//}
			}
	        else {
		        //buf[strcspn(buf, "\r\n")] = 0;
		        printf("ota_update_post_handler() no fixing... received %d\n",sizeof(newdec)/sizeof(newdec[0]));
		        //
				//err = esp_ota_write(update_handle, buf, received_len);
	            //if (err != ESP_OK) {
	            //    printf("ota_update_post_handler() ota write failed (%s)\n", esp_err_to_name(err));
	            //    esp_ota_end(update_handle);
	            //    goto err_handler;
	            //}
			}*/
	        remaining_len -= received_len;
	        //len += received_len;
		}
		printf("ota_update_post_handler() leaving while loop..., totallen: %d\n",totallen);
		
		
		char *dec = b64decode( totalbuf );
        printf("ota_update_post_handler() Got base64decoded dec: %s\n",dec);
        
        char *newdec = (char*)malloc(strlen(dec));
        memset(newdec,'\0', strlen(dec));
        int tmpl = t3ch_urldecode(dec,newdec,strlen(dec));
        printf("ota_update_post_handler() Got urldecoded newdec size: %d, getSize: %d, strlen: %d\n",tmpl,getSize(newdec),strlen(newdec));
		
		if( strlen(test)<=0 ) {
			// First
			if( getInt(at)==0 ) {
				printf("ota_update_post_handler() upload, first chunk!\n");
				//
				update_partition = esp_web_get_ota_update_partition();
				// check post data size
			    //if (update_partition->size < total_len) {
			    //    printf("ota_update_post_handler() ota data too long, partition size is %u, bin size is %d", update_partition->size, total_len);
			    //    goto err_handler;
			    //}
			    // start ota
			    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
			    if (err != ESP_OK) {
			        printf("ota_update_post_handler() ota begin failed (%s)", esp_err_to_name(err));
			        goto err_handler;
			    }
				//
				err = esp_ota_write(update_handle, newdec, tmpl);
	            if (err != ESP_OK) {
	                printf("ota_update_post_handler() ota write failed (%s)", esp_err_to_name(err));
	                esp_ota_end(update_handle);
	                goto err_handler;
	            }
	            
	            dl0=0;
	            dl1=0;
	            dl2=0;
	            dl0 += tmpl;
	            dl1 += getSize(newdec);
	            dl2 += strlen(newdec);
			}
			// Last
			else if( getInt(at)==getInt(of) ) {
				printf("ota_update_post_handler() upload, last chunk!\n");
				//
				err = esp_ota_write(update_handle, newdec, tmpl);
	            if (err != ESP_OK) {
	                printf("ota_update_post_handler() ota write failed (%s)", esp_err_to_name(err));
	                esp_ota_end(update_handle);
	                goto err_handler;
	            }
	            printf("ota_update_post_handler() entering esp_ota_end()!!! luck!\n");
	            //
	            err = esp_ota_end(update_handle);
			    if (err != ESP_OK) {
			        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
			            printf("ota_update_post_handler() Image validation failed, image is corrupted\n");
			        }
			        printf("ota_update_post_handler() esp_ota_end failed (%s)!\n", esp_err_to_name(err));
			        goto err_handler;
			    }
			    printf("ota_update_post_handler() entering next..\n");
			    err = esp_ota_set_boot_partition(update_partition);
			    if (err != ESP_OK) {
			        printf("ota_update_post_handler() esp_ota_set_boot_partition failed (%s)!\n", esp_err_to_name(err));
			        goto err_handler;
			    }
			    
			    dl0 += tmpl;
	            dl1 += getSize(newdec);
	            dl2 += strlen(newdec);
	            
				printf("ota_update_post_handler() upload, last chunk! Update success!!! :) dl0: %i, dl1: %i, dl2: %i\n",dl0,dl1,dl2);
			}
			// Other
			else {
				printf("ota_update_post_handler() upload, other chunk!\n");
				//
				err = esp_ota_write(update_handle, newdec, tmpl);
	            if (err != ESP_OK) {
	                printf("ota_update_post_handler() ota write failed (%s)", esp_err_to_name(err));
	                esp_ota_end(update_handle);
	                goto err_handler;
	            }
	            
	            dl0 += tmpl;
	            dl1 += getSize(newdec);
	            dl2 += strlen(newdec);
			}
			
			//
			//printf("ota_update_post_handler() DEBUG update_handle...\n");
			//printf("ota_update_post_handler() update_handle size: %d\n",esp_get_ota_handle_size(update_handle));
		}
		else {
			printf("ota_update_post_handler() Got urldecoded newdec: %s\n", newdec);
		}
		
		//
		free(dec);
		free(totalbuf);
		free(newdec);
		
		//
		sprintf(res,"{\"success\":true,\"at\":%s,\"of\":%s}",at,of);
		httpd_resp_send(req, res, strlen(res));
		printf("ota_update_post_handler() upload done\n");
	    return ESP_OK;
	
	}
	return ESP_FAIL; // to debug only
//--
	//
    err_handler:
	    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed.");
	    printf("ota_update_post_handler() Failed!\n");
	    return ESP_FAIL;
	
	/*
	//-- 3.) Option
	// POST download image as multipart/form-data
	//int total_len = req->content_len;
    int remaining_len = req->content_len;
    int received_len = 0;
    int bs = 320, cc = 0, abs=0;
    char buf[bs+1];
    memset(buf,'\0',bs+1);
    //char ret[640]={0};
    esp_err_t err;
    char lastbuf[1025];
    int lastbufsize=0;
    memset(lastbuf,'\0',1025);
    
    esp_ota_handle_t update_handle = 0;
    const esp_partition_t *update_partition = esp_web_get_ota_update_partition();
    // check post data size
    if (update_partition->size < total_len) {
        printf("ota_update_post_handler() ota data too long, partition size is %u, bin size is %d", update_partition->size, total_len);
        goto err_handler;
    }
    
    // start ota
    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
    if (err != ESP_OK) {
        printf("ota_update_post_handler() ota begin failed (%s)", esp_err_to_name(err));
        goto err_handler;
    }
    
    printf("ota_update_post_handler() total_len: %d\n",total_len);
	// receive ota data
    while (remaining_len > 0) {
        received_len = httpd_req_recv(req, buf, bs); // Receive the file part by part into a buffer
        //received_len = esp_http_client_read(req,buf,bs);
        printf("ota_update_post_handler() received_len: %d, remaining: %d\n",received_len, remaining_len);
        if( received_len<=0 ) {
			printf("ota_update_post_handler() received_len 0... breaking... remaining_len: %d\n",remaining_len);
			
			esp_ota_end(update_handle);
            goto err_handler;
            
			break;
		}
        remaining_len -= received_len;
        
        //-- First Chunk looks like this:
        //
        // 
	    //    ------WebKitFormBoundaryyWDvv9OrSE4pzWnm
		//	Content-Disposition: form-data; name="file"; filename="esp-kos-bridge.bin"
		//	Content-Type: application/macbinary
		//
		//-- Last chunk contain this as last line:
		//
		//------WebKitFormBoundaryyWDvv9OrSE4pzWnm--
		//
        
        printf("ota_update_post_handler() buf(%d / %d): %s\n",sizeof(buf)/sizeof(buf[0]), strlen(buf), buf);
        abs += strlen(buf);
        //--
        //
		if( cc==0 ) {
			printf("ota_update_post_handler() first chunk!\n");
			// fix data from HTTP header and multipart/form-data strings
			char *pch = strstr(buf,"\r\n\r\n");
			if( pch!=NULL ) {
				printf("ota_update_post_handler() fixing header data pch: \n%s\n",(pch+4));
				char newbuf[bs+1];
				memset(newbuf,'\0',bs+1);
				int newbs = sizeof(pch)/sizeof( (pch+4)[0] );
				printf("ota_update_post_handler() fixing header newbs: %d / %d\n",newbs,sizeof(pch));
				//newbuf = (pch+4);
				memcpy(newbuf,(pch+4), strlen(pch+4)+1);
				printf("ota_update_post_handler() fixing header newbuf: \n%s\n",newbuf);
				//
				//size += sprintf(ret+size,"%s",(pch+4));
				err = esp_ota_write(update_handle, newbuf, strlen(newbuf));
	            if (err != ESP_OK) {
	                printf("ota_update_post_handler() ota write failed (%s)", esp_err_to_name(err));
	                esp_ota_end(update_handle);
	                goto err_handler;
	            }
			}
			else {
				printf("ota_update_post_handler() first chunk! no match for .r.n!\n");
				err = esp_ota_write(update_handle, buf, received_len);
	            if (err != ESP_OK) {
	                ESP_LOGE(TAG, "ota write failed (%s)", esp_err_to_name(err));
	                esp_ota_end(update_handle);
	                goto err_handler;
	            }
			}
		}
		else if( remaining_len<=0 ) {
			printf("ota_update_post_handler() last chunk! lastbuf( %d ): %s\n",strlen(lastbuf), lastbuf);
			//
			//size += sprintf(ret+size,"%s",buf);
			char newbuf[bs+1];
			int size=0;
			memset(newbuf,'\0',bs+1);
			char *pch = strtok(buf,"\r\n");
			if( pch!=NULL ) {
				while(pch!=NULL) {
					printf("ota_update_post_handler() last chunk debug: %s\n",pch);
					// Remove last line ex.: ------WebKitFormBoundaryyWDvv9OrSE4pzWnm--
					if( match("^\-.*WebKitFormBoundary.*\-\-",pch)==0 ) {
						int pchsize = strlen(pch);//sizeof(pch)/sizeof(pch[0]);
						printf("ota_update_post_handler() creating newbuf size: %d / %d\n", size, pchsize);
						memcpy(newbuf+size, pch, pchsize);
						size += pchsize;//sizeof(pch)-1;
						//memcpy(newbuf+size, "\n", 1);
						//size += 1;
					}
					pch = strtok(NULL,"\r\n");
				}
				printf("ota_update_post_handler() last chunk, newbuf: \n%s\n",newbuf);
				err = esp_ota_write(update_handle, newbuf, strlen(newbuf));
	            if (err != ESP_OK) {
	                printf("ota_update_post_handler() ota write failed (%s)", esp_err_to_name(err));
	                esp_ota_end(update_handle);
	                goto err_handler;
	            }
			}
			else {
				printf("ota_update_post_handler() last chunk no match .r.n!\n");
				err = esp_ota_write(update_handle, buf, received_len);
	            if (err != ESP_OK) {
	                printf("ota_update_post_handler() ota write failed (%s)", esp_err_to_name(err));
	                esp_ota_end(update_handle);
	                goto err_handler;
	            }
			}
			break;
		}
		else if( remaining_len<=1024 ) {
			printf("ota_update_post_handler() creating lastbuf\n");
			memcpy(lastbuf+lastbufsize,buf,received_len);
			lastbufsize+=received_len;
		}
		else {
			printf("ota_update_post_handler() mid chunk no match .r.n! len: %d\n",received_len);
			err = esp_ota_write(update_handle, buf, received_len);
            if (err != ESP_OK) {
                printf("ota_update_post_handler() ota write failed (%s)", esp_err_to_name(err));
                esp_ota_end(update_handle);
                goto err_handler;
            }
		}
        cc++;
	}
	
	err = esp_ota_end(update_handle);
    if (err != ESP_OK) {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
            printf("ota_update_post_handler() Image validation failed, image is corrupted\n");
        }
        printf("ota_update_post_handler() esp_ota_end failed (%s)!\n", esp_err_to_name(err));
        goto err_handler;
    }
    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        printf("ota_update_post_handler() esp_ota_set_boot_partition failed (%s)!\n", esp_err_to_name(err));
        goto err_handler;
    }
	sprintf(res,"{\"success\":true}");
	httpd_resp_send(req, res, strlen(res));
	printf("ota_update_post_handler() done\n");
	//httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
//--
//
err_handler:
    //esp_web_response_error(req, HTTPD_500);
    //httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Not found.");
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed.");
    printf("ota_update_post_handler() Failed!\n");
    return ESP_FAIL;*/
}
//
static const httpd_uri_t ota_update_post = {
    .uri       = "/update/*",
    .method    = HTTP_POST,
    .handler   = ota_update_post_handler,
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
    /* For any other URI send 404 and close socket 
     * More on error codes, here:
     * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/protocols/esp_http_server.html*/
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Not found.");
    return ESP_FAIL;
}

bool StartWeb(void) {
	httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    //config.lru_purge_enable = true;
    config.uri_match_fn = httpd_uri_match_wildcard;
	//--
    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &home_get);
        httpd_register_uri_handler(server, &dht_get);
        httpd_register_uri_handler(server, &reset_get);
        httpd_register_uri_handler(server, &free_get);
        httpd_register_uri_handler(server, &time_get);
        httpd_register_uri_handler(server, &timer_get);
        httpd_register_uri_handler(server, &wifi_get);
        httpd_register_uri_handler(server, &ota_update_post);
        httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, httpd_error);
    }
    
    //-- Save some data into nvs for statistics
    //
    esp_err_t err = nvs_open("httpd_storage",NVS_READWRITE,&nvsh);
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
