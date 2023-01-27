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
#include "cJSON.h"

//--
static const char *TAG = "T3CH_HTTPD";
static nvs_handle_t nvsh;
httpd_handle_t server = NULL;
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
		memset(buf,'\0',buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            //ESP_LOGI(TAG, "Found URL query(%i) => %s", strlen(buf),buf);
            int x = chrat(buf,'=');
            //ESP_LOGI(TAG, "Found separator(%i) at %i", strlen(buf)-x, x);
            //
            //char tmpbuf[strlen(buf)-x];
            //memset(tmpbuf,'\0',strlen(buf)-x);
            //
            if (httpd_query_key_value(buf, key, paramout, strlen(buf)-x) == ESP_OK) {
            //    ESP_LOGI(TAG, "Found URL query parameter => %s => %s", key, paramout);
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
//static int user_ctx = 0;
/*//
void free_user_ctx(void *ctx) {
	printf("free_user_ctx() STARTING!!!\n");
	free(ctx);
}*/
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
	
	//
	/*if(!req->sess_ctx) {
		printf("Missing session!\n");
		//req->sess_ctx = (time_t*)malloc(sizeof(time_t));
		//req->sess_ctx = t3ch_time_ts();
		req->sess_ctx   = malloc(sizeof(int));
		req->free_ctx = free_user_ctx;
	}
	//
	else {
		printf("Got session: %d\n",req->sess_ctx);
	}
	
	if(!req->user_ctx) {
		printf("User ctx missing..\n");
	}
	else {
		printf("User ctx: %d\n",*(int*)req->user_ctx);
		user_ctx = *(int*)req->user_ctx;
		if( user_ctx==0 ) {
			printf("Setting user_ctx...\n");
			user_ctx = t3ch_time_ts();
			req->user_ctx = user_ctx;
		}
		else {
			printf("Exists user_ctx: %d\n",user_ctx);
		}
		//
		char sessionId[8]={0};
		sprintf(sessionId,"%d",user_ctx);
		httpd_resp_set_hdr(req, "SessionID", sessionId);
	}*/
	
	httpd_resp_set_hdr(req, "Server", "KOS");
	//
    httpd_resp_send(req, HTTP_PANEL, strlen(HTTP_PANEL));
    return ESP_OK;
}
//
static const httpd_uri_t home_get = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = home_get_handler,
    //.user_ctx  = "Hello World!"
    //.user_ctx  = &user_ctx,
};

#if !defined(ENABLE_WSS)
//
static esp_err_t dht_get_handler(httpd_req_t *req)
{
	float temp, humi;
	dht_read_float_data(DHT_TYPE_DHT11,4,&humi,&temp);
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
    //.user_ctx  = &user_ctx,
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
    //.user_ctx  = &user_ctx,
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
    //.user_ctx  = &user_ctx,
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
    //.user_ctx  = &user_ctx,
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
    //.user_ctx  = &user_ctx,
};

//-- FREE
//
static esp_err_t free_get_handler(httpd_req_t *req)
{
	char res[64];
	//
	sprintf(res,"{\"success\":true,\"data\":\"%d\"}", heap_caps_get_free_size(MALLOC_CAP_8BIT));
	//
	httpd_resp_send(req, res, strlen(res));
    return ESP_OK;
}
//
static const httpd_uri_t free_get = {
    .uri       = "/free",
    .method    = HTTP_GET,
    .handler   = free_get_handler,
    //.user_ctx  = &user_ctx,
};

//-- LOG
//
static esp_err_t log_get_handler(httpd_req_t *req)
{
	printf("log_get_handler() STARTING!\n");
	int fromPos=0;
	char arg_fromPos[8]={0};
	char arg_update[8]={0};
	t3ch_httpd_get_param(req, "fromPos", &arg_fromPos);
	t3ch_httpd_get_param(req, "update", &arg_update);
	
	printf("log_get_handler() DEBUG arg_fromPos(%i): %s\n",strlen(arg_fromPos),arg_fromPos);
	printf("log_get_handler() DEBUG arg_update(%i): %s\n",strlen(arg_update),arg_update);
	//
	if( strlen(arg_update)>0 ) {
		//
		int size = t3ch_log_gen_new(getInt(arg_update));
		if( size>0 ) {
			char res[size];
			memset(res,'\0',size);
			t3ch_log_get(res);
			printf("log_get_handler() test get log d1 UPDATE res: %s\n",res);
			httpd_resp_send(req, res, strlen(res));
			return ESP_OK;
		}
		else {
			char res[64];
			sprintf(res,"{\"success\":false}");
			printf("log_get_handler() test get log d2 UPDATE res: %s\n",res);
			httpd_resp_send(req, res, strlen(res));
			return ESP_OK;
		}
	}
	else if( strlen(arg_fromPos)>0 ) {
		fromPos = getInt( arg_fromPos );
		printf("log_get_handler() changing fromPos: %i\n",fromPos);
	}
	
	int size = t3ch_log_gen_old(fromPos);
	
	// No more results or just there is no results.
	if( size<=0 ) {
		char res[64];
		sprintf(res,"{\"success\":false}");
		printf("log_get_handler() test get log size 0, res: %s\n",res);
		httpd_resp_send(req, res, strlen(res));
		return ESP_OK;
	}
	
	// Looks we got results
	printf("log_get_handler() test get log size: %i\n",size);
	char res[size];
	memset(res,'\0',size);
	t3ch_log_get(res);
	printf("log_get_handler() test get log: %s\n",res);
	httpd_resp_send(req, res, strlen(res));
	
    return ESP_OK;
}
//
static const httpd_uri_t log_get = {
    .uri       = "/log/*",
    .method    = HTTP_GET,
    .handler   = log_get_handler,
    //.user_ctx  = &user_ctx,
};
#endif // #if !defined(ENABLE_WSS)

//-- OTA UPDATES
//
static esp_ota_handle_t update_handle = 0;
static const esp_partition_t *update_partition;
static time_t update_ts;
//
char task_url_download[256]={0};
//
void task_ota_download(void *pv) {
	printf("task_ota_download STARTED!, url: %s\n",task_url_download);
	t3ch_ota_download(task_url_download);
}
//
void task_ota_upload(void *pv) {
	printf("task_ota_upload STARTED!\n");
	//
	esp_err_t err = esp_ota_end(update_handle);
    if (err != ESP_OK) {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
            printf("ota_update_post_handler() Image validation failed, image is corrupted\n");
        }
        printf("ota_update_post_handler() esp_ota_end failed (%s)!\n", esp_err_to_name(err));
        //goto err_handler;
    }
    else {
	    printf("ota_update_post_handler() entering next..\n");
	    err = esp_ota_set_boot_partition(update_partition);
	    if (err != ESP_OK) {
	        printf("ota_update_post_handler() esp_ota_set_boot_partition failed (%s)!\n", esp_err_to_name(err));
	        //goto err_handler;
	    }
	    else {
			printf("ready to reset? successs? :)");
			esp_restart();
		}
	}
}

//
static esp_err_t ota_update_post_handler(httpd_req_t *req)
{
	printf("ota_update_post_handler() STARTING..., total_len: %d\n",req->content_len);
	char res[64];
	int total_len = req->content_len;
    int remaining_len = req->content_len;
    int received_len = 0;
    int len=0;
	char out_download[512]={0}, out_upload[512]={0};
	size_t chk_download, chk_upload;
	update_ts = t3ch_time_ts();
	esp_err_t err;
	
	//
	chk_download = t3ch_httpd_get_param(req, "download", &out_download);
	chk_upload   = t3ch_httpd_get_param(req, "upload", &out_upload);
	
	ESP_LOGI(TAG,"ota_update_post_handler() DEBUG out_download: %s, out_upload: %s",out_download,out_upload);
	
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
		strcpy(task_url_download,newdec);
		printf("before xTaskCreate, downloadUrl: %s\n", task_url_download);
		xTaskCreate(
		    task_ota_download,
		    "task_ota_download",
		    15000,
		    NULL,
		    1,
		    NULL
		);
		
		//
		sprintf(res,"{\"success\":true}");
		httpd_resp_send(req, res, strlen(res));
		printf("ota_update_post_handler() download done\n");
	    return ESP_OK;
	}
	//-- 2.) Option (NOT WORK YET)
	// Upload image as urlencoded chunks of data
	if( strlen(out_upload)>0 ) {
		char at[12]={0};
		char of[12]={0};
		//char test[12]={0};
		t3ch_httpd_get_param(req, "at", &at);
		t3ch_httpd_get_param(req, "of", &of);
		//t3ch_httpd_get_param(req, "test", &test);
		//printf("ota_update_post_handler() upload start, data( %s / %s )\n",at,of);
		
		//
		int bs = 320;
		char buf[bs];
		memset(buf,'\0',bs);
		//
	    while (remaining_len > 0) {
	        received_len = httpd_req_recv(req, buf, bs); // Receive the file part by part into a buffer
	        //printf("ota_update_post_handler() received_len: %d, remaining: %d, buf: %s\n",received_len, remaining_len, buf);
	        
			//
	        if( received_len<=0 ) {
				printf("ota_update_post_handler() received_len 0... breaking... remaining_len: %d\n",remaining_len);
				
				esp_ota_end(update_handle);
	            goto err_handler;
	            
				break;
			}
	        
	        //hexDump("HexDump buf: ",buf, received_len, 20);
	        // First chunk
	        if( getInt(at)==0 && remaining_len==total_len ) {
				//
				update_partition = esp_web_get_ota_update_partition();
				// check post data size
			    if (update_partition->size < total_len) {
			        printf("ota_update_post_handler() ota data too long, partition size is %u, bin size is %d\n", update_partition->size, total_len);
			        goto err_handler;
			    }
			    // start ota
			    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
			    if (err != ESP_OK) {
			        printf("ota_update_post_handler() ota begin failed (%s)\n", esp_err_to_name(err));
			        goto err_handler;
			    }
				//
				err = esp_ota_write(update_handle, buf, received_len);
	            if (err != ESP_OK) {
	                printf("ota_update_post_handler() ota write failed (%s)\n", esp_err_to_name(err));
	                esp_ota_end(update_handle);
	                goto err_handler;
	            }
			}
			// Last chunk
			else if( getInt(at)==getInt(of) && (remaining_len-received_len)==0 ) {
				//
				err = esp_ota_write(update_handle, buf, received_len);
	            if (err != ESP_OK) {
	                printf("ota_update_post_handler() ota write failed (%s)", esp_err_to_name(err));
	                esp_ota_end(update_handle);
	                goto err_handler;
	            }
	            // End ota and set boot partition with xTask..*
			    xTaskCreate(
				    task_ota_upload,
				    "task_ota_upload",
				    15000,
				    NULL,
				    1,
				    NULL
				);
			}
			// Middle chunks
	        else {
				//
				err = esp_ota_write(update_handle, buf, received_len);
	            if (err != ESP_OK) {
	                printf("ota_update_post_handler() ota write failed (%s)", esp_err_to_name(err));
	                esp_ota_end(update_handle);
	                goto err_handler;
	            }
			}
			
	        //
	        remaining_len -= received_len;
	        
	        //-- Chunkyret - application/macbinary
	        //
			//httpd_resp_set_type(req,"application/macbinary");
			/*httpd_resp_set_type(req,"application/json");
			//httpd_resp_send_chunk(req, buf, received_len);
			char *chunkyret = (char*)malloc(256);
			memset(chunkyret,'\0',256);
			sprintf(chunkyret,"{\"success\":true,\"total\":%d,\"remain\":%d}",total_len,remaining_len-total_len);
			printf("sending chunkyret: %s\n", chunkyret);
			httpd_resp_send_chunk(req, chunkyret, strlen(chunkyret));
			free(chunkyret);*/
		}
		
		//
		time_t donein = ( t3ch_time_ts()-update_ts );
		sprintf(res,"{\"success\":true,\"donein\":%d}",donein);
		httpd_resp_send(req, res, strlen(res));
		printf("ota_update_post_handler() upload done, res: %s\n",res);
	    return ESP_OK;
	}
	
	//--
	//
	return ESP_FAIL;
	
	//--
	//
	err_handler:
	    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed.");
	    printf("ota_update_post_handler() Failed!\n");
	    return ESP_FAIL;
}
//
static const httpd_uri_t ota_update_post = {
    .uri       = "/update/*",
    .method    = HTTP_POST,
    .handler   = ota_update_post_handler,
    //.user_ctx  = &user_ctx,
};

//-- ERROR HANDLER
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
    ESP_LOGI(TAG,"t3ch_httpd => NOT FOUND: %s",req->uri);
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Not found.");
    return ESP_FAIL;
}


//-- WSS THINGS are Separated with macros
#ifdef ENABLE_WSS

//
esp_err_t t3ch_ws_send(httpd_req_t *req, char *data) {
	//
	httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    ws_pkt.len  = strlen(data);
    ws_pkt.payload = data;
	esp_err_t ret = httpd_ws_send_frame(req, &ws_pkt);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "t3ch_ws_send() failed with %d", ret);
    }
    return ret;
}

//
static esp_err_t wss_handler(httpd_req_t *req)
{
	//
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "wss_handler() Handshake done, the new connection was opened");
        return ESP_OK;
    }
    //
    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    // retrive data len
    /* Set max_len = 0 to get the frame len */
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "wss_handler() httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }
    ESP_LOGI(TAG, "wss_handler() frame len is %d", ws_pkt.len);
    
    // retrive data
    if (ws_pkt.len) {
        /* ws_pkt.len + 1 is for NULL termination as we are expecting a string */
        buf = calloc(1, ws_pkt.len + 1);
        if (buf == NULL) {
            ESP_LOGE(TAG, "wss_handler() Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        /* Set max_len = ws_pkt.len to get the frame payload */
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "wss_handler() httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }
        ESP_LOGI(TAG, "wss_handler() Got packet with message: %s", ws_pkt.payload);
    }
    ESP_LOGI(TAG, "wss_handler() Packet type: %d", ws_pkt.type);
    /*if (ws_pkt.type == HTTPD_WS_TYPE_TEXT &&
        strcmp((char*)ws_pkt.payload,"Trigger async") == 0) {
        free(buf);
        return trigger_async_send(req->handle, req);
    }*/
	
	//--
	//
	printf("wss_handler() parsing payload(%i): %s\n",strlen((char*)ws_pkt.payload),(char*)ws_pkt.payload);
	//
	cJSON *json      = cJSON_Parse( (char*)ws_pkt.payload );
	cJSON *objAction = cJSON_GetObjectItemCaseSensitive(json,"action");
	cJSON *objUID    = cJSON_GetObjectItemCaseSensitive(json,"uid");
	char *action = cJSON_Print( objAction );
	char *uid    = cJSON_Print( objUID );
	//
	ltrim(action,1);
	rtrim(action,1);
	//
	ltrim(uid,1);
	rtrim(uid,1);
	
	//
	if (ws_pkt.type == HTTPD_WS_TYPE_TEXT && strcmp(action,"infoLoop") == 0) {
		ESP_LOGI(TAG, "wss_handler() infoLoop STARTED");
		//
		char strftime_buf[64];
		char res[128];
		//
		t3ch_time_get(&strftime_buf);
		sprintf(res,"{\"success\":true,\"action\":\"%s\",\"uid\":\"%s\",\"time\":\"%s\",\"free\":\"%d\"}",
			action, uid, strftime_buf, heap_caps_get_free_size(MALLOC_CAP_8BIT));
		ret = t3ch_ws_send(req,res);
	}
	//
	else if (ws_pkt.type == HTTPD_WS_TYPE_TEXT && strcmp(action,"version") == 0) {
		ESP_LOGI(TAG, "wss_handler() version STARTED");
		char res[128];
		sprintf(res,"{\"success\":true,\"action\":\"%s\",\"uid\":\"%s\",\"version\":\"%s\",\"version_string\":\"%s\"}",
			action, uid, t3ch_version(), t3ch_version_string());
		ret = t3ch_ws_send(req,res);
	}
	//
	else if (ws_pkt.type == HTTPD_WS_TYPE_TEXT && strcmp(action,"log_view_old") == 0) {
		ESP_LOGI(TAG, "wss_handler() log_view_old STARTED");
		//
		char *fromPos = cJSON_Print( objUID );
		// generate log and get size
		int size      = t3ch_log_gen_old(getInt(fromPos));
	
		// No more results or just there is no results.
		if( size<=0 ) {
			ESP_LOGI(TAG, "wss_handler() log_view_old test get log size: %i\n",size);
			char res[128];
			sprintf(res,"{\"success\":false,\"action\":\"%s\",\"uid\":\"%s\"}");
			ESP_LOGI(TAG, "wss_handler() log_view_old res: %s\n",res);
			ret = t3ch_ws_send(req,res);
		}
		else {
			// Looks we got results
			ESP_LOGI(TAG, "wss_handler() log_view_old test get log size: %i\n",size);
			char data[size];
			memset(data,'\0',size);
			// get log
			t3ch_log_get(data);
			ESP_LOGI(TAG, "wss_handler() log_view_old test get log: %s\n",data);
			char res[128+size];
			memset(res,'\0',(128+size));
			//
			sprintf(res,"{\"success\":true,\"action\":\"%s\",\"uid\":\"%s\",\"data\":%s}",
				action, uid, data);
			ret = t3ch_ws_send(req,res);
		}
		free(fromPos);
	}
	//
	else if (ws_pkt.type == HTTPD_WS_TYPE_TEXT && strcmp(action,"log_view_new") == 0) {
		ESP_LOGI(TAG, "wss_handler() log_view_new STARTED");
	}
	//
	else if (ws_pkt.type == HTTPD_WS_TYPE_TEXT && strcmp(action,"wifi_view") == 0) {
		ESP_LOGI(TAG, "wss_handler() wifi_view STARTED");
		//
		char res[256]={0};
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
			ESP_LOGI(TAG, "wss_handler() wifi_view cap.ssid not set!?");
		}
		
		//
	    if (strlen((const char*)csta.ssid)) {
	        strcpy(sta_ssid,(const char*) csta.ssid);
	        strcpy(sta_pwd,(const char*) csta.password);
	    }
	    else {
			ESP_LOGI(TAG, "wss_handler() wifi_view csta.ssid not set!");
			strcpy(sta_ssid,"Not set");
			strcpy(sta_pwd,"Not set");
		}
		
		//
		sprintf(res,"{\"success\":true,\"action\":\"%s\",\"uid\":\"%s\",\"ap_ssid\":\"%s\",\"ap_pwd\":\"%s\",\"sta_ssid\":\"%s\",\"sta_pwd\":\"%s\"}",
		    action,uid, ap_ssid, ap_pwd, sta_ssid, sta_pwd);
		ret = t3ch_ws_send(req,res);
	}
	//
	else {
		ESP_LOGI(TAG, "wss_handler() unknown action! Action: %s, UID: %s",action,uid);
		char res[128]={0};
		sprintf(res,"{\"success\":false,\"action\":\"%s\",\"uid\":\"%s\"}",
			action, uid);
	    ret = t3ch_ws_send(req,res);
	}
	//
	free(action);
	free(uid);
    free(buf);
    cJSON_Delete(json);
    return ret;
}
//
static const httpd_uri_t wss_get = {
        .uri        = "/wss",
        .method     = HTTP_GET,
        .handler    = wss_handler,
        //.user_ctx   = NULL,
        .is_websocket = true
};
#endif

#ifdef ENABLE_HTTPS
//
bool StartHTTPS(void) {
	ESP_LOGI(TAG, "StartHTTPS() STARTING.");
	//
	httpd_ssl_config_t conf     = HTTPD_SSL_CONFIG_DEFAULT();
	conf.httpd.uri_match_fn     = httpd_uri_match_wildcard;
#ifdef ENABLE_WSS
	conf.httpd.max_uri_handlers = 3;
#else
	conf.httpd.max_uri_handlers = 8;
#endif
	//
    conf.servercert = servercert_start;
    conf.servercert_len = servercert_end - servercert_start;
    //
    conf.prvtkey_pem = prvtkey_pem_start;
    conf.prvtkey_len = prvtkey_pem_end - prvtkey_pem_start;

//#if CONFIG_EXAMPLE_ENABLE_HTTPS_USER_CALLBACK
//    conf.user_cb = https_server_user_callback;
//#endif

    esp_err_t ret = httpd_ssl_start(&server, &conf);
    if (ESP_OK != ret) {
        ESP_LOGI(TAG, "Error starting server!");
        return false;
    }
    
    //
    httpd_register_uri_handler(server, &home_get);
	httpd_register_uri_handler(server, &ota_update_post);
#ifdef ENABLE_WSS
	//
	httpd_register_uri_handler(server, &wss_get);
#else
	//
	httpd_register_uri_handler(server, &reset_get);
	httpd_register_uri_handler(server, &free_get);
	httpd_register_uri_handler(server, &time_get);
	httpd_register_uri_handler(server, &timer_get);
	httpd_register_uri_handler(server, &wifi_get);
	httpd_register_uri_handler(server, &log_get);
	//httpd_register_uri_handler(server, &dht_get);
#endif
	//
	httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, httpd_error);
    return true;
}
#endif

#if !defined(ENABLE_HTTPS)
//
bool StartHTTP(void) {
	ESP_LOGI(TAG, "StartHTTP() STARTING.");
	//
	//httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    //config.lru_purge_enable = true;
    config.uri_match_fn = httpd_uri_match_wildcard;
	//--
    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
		//
        httpd_register_uri_handler(server, &ota_update_post);
        //
        httpd_register_uri_handler(server, &home_get);
        //httpd_register_uri_handler(server, &dht_get);
        httpd_register_uri_handler(server, &reset_get);
        httpd_register_uri_handler(server, &free_get);
        httpd_register_uri_handler(server, &time_get);
        httpd_register_uri_handler(server, &timer_get);
        httpd_register_uri_handler(server, &wifi_get);
        httpd_register_uri_handler(server, &log_get);
        //
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
#endif
