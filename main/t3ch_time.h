#include "time.h"
#include "esp_sntp.h"
#include "esp_log.h"
void t3ch_time_sntp_init(void);
void t3ch_time_sntp_update(void);
bool t3ch_time_sntp_updated(void);
bool t3ch_time_chk( struct tm starttime, struct tm endtime );
void t3ch_time_get(char * buf);
void t3ch_time_tm( struct tm * TimeInfo );
