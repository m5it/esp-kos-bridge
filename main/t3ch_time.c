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

#include "t3ch_time.h"
#include "driver/gpio.h"
#include "nvs.h"
#include "cJSON.h" // https://github.com/DaveGamble/cJSON

//--
static const char *TAG = "T3CH_TIME";
//
static time_t now;
static struct tm timeinfo;
char strftime_buf[64];

//
static struct mytimer {
	//
	//char *idCRC;
    //
    struct tm start_time;
    struct tm end_time;
    //
    bool running;
    int gpio;
};
//
struct mytimer myt[2];
int myt_pos = 0;
bool myt_running = false;
TaskHandle_t h_myt;
static uint8_t ucptp_myt;

//-- TIMER
//
void time_timer(void * pvp) {
	while(true) {
		for(int i=0; i<time_timer_pos(); i++) {
			//
			//printf("time_timer t3ch_gpio_state for 26: %i\n", t3ch_gpio_state(26));
			//
			if( !myt[i].running && t3ch_time_chk( myt[i].start_time, myt[i].end_time ) ) {
				printf("Temporary timer at %i not running, STARTING now on gpio: %i!\n", i, myt[i].gpio);
				myt[i].running = true;
				gpio_set_level(myt[i].gpio,1);
			}
			else if( myt[i].running && !t3ch_time_chk( myt[i].start_time, myt[i].end_time ) ) {
				printf("Temporary timer at %i running, Shutting down on gpio: %i.\n", i, myt[i].gpio);
				myt[i].running = false;
				gpio_set_level(myt[i].gpio,0);
			}
			else if( myt[i].running ) {
				printf("Temporary timer at %i running on gpio: %i...\n", i, myt[i].gpio);
			}
			else {
				printf("Temporary timer at %i not running for gpio: %i...\n", i, myt[i].gpio);
			}
		}
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
}
//
int time_timer_size(void) {
	return sizeof(myt)/sizeof(myt[0]);
}
//
int time_timer_pos(void) {
	return myt_pos;
}
//
int time_timer_get(char *out) {
	printf("time_timer_get() STARTING, myt_pos: %i\n",myt_pos);
	char ret[256]={0};
	int jsonlen=0;
	jsonlen = sprintf(ret+jsonlen,"[");
	for(int i=0; i<myt_pos; i++) {
		printf("time_timer_get() at: %i, jsonlen: %i\n",i,jsonlen);
		jsonlen += sprintf(ret+jsonlen,"{\"gpio\":%i,\"startHour\":%i,\"startMin\":%i,\"endHour\":%i,\"endMin\":%i,\"running\":%s}%s",
		    myt[i].gpio, myt[i].start_time.tm_hour, myt[i].start_time.tm_min,myt[i].end_time.tm_hour, myt[i].end_time.tm_min, (myt[i].running?"true":"false"), (i>=(myt_pos-1)?"":",") );
	}
	jsonlen += sprintf(ret+jsonlen,"]");
	printf("time_timer_get() done, jsonlen: %i, data: %s\n",jsonlen,ret);
	strcpy(out,ret);
	return jsonlen;
}
//
void time_timer_del(int pos) {
	struct mytimer tmp[2];
	for(int i=0; i<time_timer_pos(); i++) {
		if(i!=pos) {
			tmp[i] = myt[i];
		}
		else {
			if( myt[i].running ) {
				printf("time_timer_del() at %i, stopping gpio.\n",i);
				gpio_set_level(GPIO_NUM_26,0);
			}
			myt_pos--;
		}
	}
	myt[2];
	for(int i=0; i<myt_pos; i++) {
		myt[i] = tmp[i];
	}
	time_timer_save();
}

//
bool time_timer_exists(struct tm starttime, struct tm endtime) {
	printf("time_timer_exists() STARTING\n");
	//
	int startsec = ((starttime.tm_hour*60)*60) + (starttime.tm_min*60) + starttime.tm_sec;
	int endsec   = ((endtime.tm_hour*60)*60) + (endtime.tm_min*60) + endtime.tm_sec;
	for(int i=0; i<myt_pos; i++) {
		int chkstartsec = ((myt[i].start_time.tm_hour*60)*60) + (myt[i].start_time.tm_min*60) + myt[i].start_time.tm_sec;
		int chkendsec   = ((myt[i].end_time.tm_hour*60)*60) + (myt[i].end_time.tm_min*60) + myt[i].end_time.tm_sec;
		if( startsec <= chkstartsec && endsec >= chkendsec ) {
			printf("time_timer_exists() looks exists!\n");
			return true;
		}
	}
	printf("time_timer_exists() DONE\n");
	return false;
}

//
bool time_timer_add(int startHour, int startMin, int endHour, int endMin, int gpio) {
	printf("time_timer_add() STARTING, startHour: %i, startMin: %i, endHour: %i, endMin: %i\n",
	    startHour, startMin, endHour, endMin);
	//
	if( myt_pos>=time_timer_size() ) {
		printf("time_timer_add() Failed, timer full.");
		return false;
	}
	//
	struct tm tmp_start;
	struct tm tmp_end;
	tmp_start.tm_hour = startHour;
	tmp_start.tm_min  = startMin;
	tmp_end.tm_hour   = endHour;
	tmp_end.tm_min    = endMin;
	// Check if already exists
	if( time_timer_exists(tmp_start, tmp_end) ) {
		printf("time_timer_add() Failed exists!\n");
		return false;
	}
	myt[myt_pos].start_time = tmp_start;
	myt[myt_pos].end_time   = tmp_end;
	myt[myt_pos].running    = false;
	myt[myt_pos].gpio       = gpio;
	// generate idCRC
	/*char tmpstr[32];
	printf("time_timer_add() ascing time...\n");
	sprintf(tmpstr,"%s",asctime(&myt[myt_pos].start_time));
	char tmpcrc[8];
	sprintf(tmpcrc,"%x",crc32b(tmpstr));
	myt[myt_pos].idCRC     = tmpcrc;
	printf("time_timer_add() generated idCRC: %s from %s\n",myt[myt_pos].idCRC,tmpstr);
	*/
	
	myt_pos++;
	return true;
}

//
bool time_timer_running(void) { return myt_running; }
//
bool time_timer_start(void) {
	if( myt_running ) return false;
	myt_running = true;
	// START TIMER
	xTaskCreate(
		time_timer,
		"timetimer",
		3000,
		&ucptp_myt,
		tskIDLE_PRIORITY, 
		&h_myt
	);
	return true;
}
//
bool time_timer_stop(void) {
	if( !myt_running ) return false;
	vTaskDelete( h_myt );
	myt_running = false;
	return true;
}

//
void time_timer_save(void) {
	// Generate cJSON
	cJSON *myt_ary;
    myt_ary  = cJSON_CreateArray();
    for(int i=0; i<myt_pos; i++) {
	    cJSON *myt_obj  = cJSON_CreateObject();
	    //
	    cJSON_AddItemToObject(myt_obj,"startHour", cJSON_CreateNumber( myt[i].start_time.tm_hour )); 
	    cJSON_AddItemToObject(myt_obj,"startMin", cJSON_CreateNumber( myt[i].start_time.tm_min ));
	    //
	    cJSON_AddItemToObject(myt_obj,"endHour", cJSON_CreateNumber( myt[i].end_time.tm_hour )); 
	    cJSON_AddItemToObject(myt_obj,"endMin", cJSON_CreateNumber( myt[i].end_time.tm_min ));
	    //
	    cJSON_AddItemToObject(myt_obj,"gpio", cJSON_CreateNumber( myt[i].gpio ));
	    cJSON_AddItemToArray(myt_ary,myt_obj);
	}
	char *out = cJSON_Print( myt_ary );
    printf("time_timer_add() cJSON out: %s\n",out);
    // Save to nvs
    nvs_handle_t nvsh;
	esp_err_t err = nvs_open("timer_storage",NVS_READWRITE,&nvsh);
	//
	nvs_erase_key(nvsh,"json");
	//
	nvs_set_str(nvsh, "json", out);
	nvs_commit( nvsh );
	nvs_close( nvsh );
}
// retrive timer settings if exists. (timer_storage)
void time_timer_init(void) {
	char tmpout[1024];
	cJSON *myt_ary;
	nvs_handle_t nvsh;
	esp_err_t err = nvs_open("timer_storage",NVS_READWRITE,&nvsh);
	err = t3ch_nvs_get_str(nvsh,"json",&tmpout);
	nvs_close(nvsh);
	if( strlen(tmpout)>0 ) {
		printf("time_timer_init() tmpout: %s\n", tmpout);
		myt_ary = cJSON_Parse( tmpout );
		int myt_size = cJSON_GetArraySize( myt_ary );
		for(int i=0; i<myt_size; i++) {
			cJSON *myt_obj    = cJSON_GetArrayItem(myt_ary,i);
			cJSON *startHour = cJSON_GetObjectItemCaseSensitive(myt_obj,"startHour");
			printf("time_timer_init() startHour: %s\n",cJSON_Print(startHour));
			//
			time_timer_add(
			    getInt( cJSON_Print(cJSON_GetObjectItemCaseSensitive(myt_obj,"startHour")) ),
			    getInt( cJSON_Print(cJSON_GetObjectItemCaseSensitive(myt_obj,"startMin")) ),
			    getInt( cJSON_Print(cJSON_GetObjectItemCaseSensitive(myt_obj,"endHour")) ),
			    getInt( cJSON_Print(cJSON_GetObjectItemCaseSensitive(myt_obj,"endMin")) ),
			    getInt( cJSON_Print(cJSON_GetObjectItemCaseSensitive(myt_obj,"gpio")) )
			);
		}
	}
	else {
		printf("time_timer_init() tmpout empty!\n");
	}
}
//-- UPDATE TIME TROUGH NET
//
void time_sync_notification_cb(struct timeval *tv) {
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

void t3ch_time_sntp_init(void) {
	ESP_LOGI(TAG, "t3ch_time_sntp_init() starting.");
	setenv("TZ", "CST-0", 1);
    tzset();
    time(&now);
    localtime_r(&now, &timeinfo);
    //
	sntp_servermode_dhcp(1);      // accept NTP offers from DHCP server, if any
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(1, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
	sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
	sntp_init();
}

void t3ch_time_sntp_update(void) {
	ESP_LOGI(TAG, "t3ch_time_sntp_update() starting.");
	int retry = 0;
    const int retry_count = 15;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d/%d)", retry, retry_count,timeinfo.tm_year);
        time(&now);
	    localtime_r(&now, &timeinfo);
	    //ti = localtime(&now);
	    if( t3ch_time_sntp_updated() ) break;
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    time(&now);
    localtime_r(&now, &timeinfo);
}

bool t3ch_time_sntp_updated(void) {
	ESP_LOGI(TAG, "t3ch_time_sntp_updated() starting.");
	if (timeinfo.tm_year < (2022 - 1900)) {
		return false;
	}
	return true;
}

//-- SET TIME MANUALLY
//
void t3ch_time_get_tm( struct tm TimeInfo ) {
	time(&now);
    localtime_r(&now, &TimeInfo);
}
//
void t3ch_time_set_tm( struct tm TimeInfo ) {
    time_t t = mktime(&TimeInfo);
    struct timeval tnow = { .tv_sec = t };
    settimeofday(&tnow, NULL);
    localtime_r(&t, &timeinfo);
}

//
bool t3ch_time_chk( struct tm starttime, struct tm endtime ) {
    //
	//ESP_LOGI(TAG, "t3ch_time_chk() starting, starttime %i:%i, endtime %i:%i",
	//    starttime.tm_hour, starttime.tm_min, endtime.tm_hour, endtime.tm_min );
	//
	int startsec = ((starttime.tm_hour*60)*60) + (starttime.tm_min*60) + starttime.tm_sec;
	int endsec   = ((endtime.tm_hour*60)*60) + (endtime.tm_min*60) + endtime.tm_sec;
	int cursec   = ((timeinfo.tm_hour*60)*60) + (timeinfo.tm_min*60) + timeinfo.tm_sec;
	//ESP_LOGI(TAG, "t3ch_time_chk() startsec: %i, endsec: %i, cursec: %i", startsec, endsec, cursec);
	if( startsec<=cursec && endsec>=cursec ) return true;
	return false;
}
//
void t3ch_time_get(char * buf) {
	//ESP_LOGI(TAG, "t3ch_time_get() starting.");
	//printf("t3ch_time.c -> t3ch_time_get() d1 timeinfo: %s\n", asctime(&timeinfo));
	//printf("t3ch_time.c -> t3ch_time_get() d1 ti: %s\n", asctime(ti));
	time(&now);
    localtime_r(&now, &timeinfo);
	strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
	strcpy(buf,strftime_buf);
}