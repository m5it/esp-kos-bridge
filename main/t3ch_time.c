#include "t3ch_time.h"

//--
static const char *TAG = "T3CH_TIME";
//
static time_t now;
static struct tm timeinfo;
char strftime_buf[64];

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
