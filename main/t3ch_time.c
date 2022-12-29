#include "t3ch_time.h"
#include "driver/gpio.h"
//--
static const char *TAG = "T3CH_TIME";
//
static time_t now;
static struct tm timeinfo;
char strftime_buf[64];

//
static struct mytimer {
    //
    struct tm start_time;
    struct tm end_time;
    //
    bool running;
};
//
struct mytimer myt[2];
int myt_pos = 0;
bool myt_running = false;
TaskHandle_t h_myt;
static uint8_t ucptp_myt;

void time_timer(void * pvp) {
	while(true) {
		for(int i=0; i<time_timer_pos(); i++) {
			//
			if( !myt[i].running && t3ch_time_chk( myt[i].start_time, myt[i].end_time ) ) {
				printf("Temporary timer at %i not running, Starting.\n",i);
				
				bool chk = t3ch_time_chk( myt[i].start_time, myt[i].end_time );
				printf("Temporary timer is true: %s\n", (chk?"YES":"NO") );
				myt[i].running = true;
				gpio_set_level(GPIO_NUM_26,1);
			}
			else if( myt[i].running && !t3ch_time_chk( myt[i].start_time, myt[i].end_time ) ) {
				printf("Temporary timer at %i running, Shutting down.\n",i);
				myt[i].running = false;
				gpio_set_level(GPIO_NUM_26,0);
			}
			else if( myt[i].running ) {
				printf("Temporary timer at %i running...\n",i);
			}
			else {
				printf("Temporary timer at %i not running...\n",i);
			}
		}
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
}

/*
//
myt[0].start_time.tm_hour = 17;
myt[0].start_time.tm_min  = 30;
//
myt[0].end_time.tm_hour   = 23;
myt[0].end_time.tm_min    = 0;
//
myt[0].running            = false;
//
myt[1].start_time.tm_hour = 4;
myt[1].start_time.tm_min  = 0;
//
myt[1].end_time.tm_hour   = 8;
myt[1].end_time.tm_min    = 0;
//
myt[1].running            = false;
*/

//-- TIMER
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
		jsonlen += sprintf(ret+jsonlen,"{\"startHour\":%i,\"startMin\":%i,\"endHour\":%i,\"endMin\":%i,\"running\":%s}",
		    myt[i].start_time.tm_hour, myt[i].start_time.tm_min,myt[i].end_time.tm_hour, myt[i].end_time.tm_min, (myt[i].running?"true":"false") );
	}
	jsonlen += sprintf(ret+jsonlen,"]");
	printf("time_timer_get() done, jsonlen: %i, data: %s\n",jsonlen,ret);
	strcpy(out,ret);
	return jsonlen;
}
//
bool time_timer_add(int startHour, int startMin, int endHour, int endMin) {
	if( myt_pos>=time_timer_size() ) {
		printf("time_timer_add() Failed, timer full.");
		return false;
	}
	//
	myt[myt_pos].start_time.tm_hour = startHour;
	myt[myt_pos].start_time.tm_min  = startMin;
	//
	myt[myt_pos].end_time.tm_hour   = endHour;
	myt[myt_pos].end_time.tm_min    = endMin;
	//
	myt[myt_pos].running            = true;
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
