#include <stdarg.h>
#include "t3ch_time.h"
#include "t3ch_log.h"
#include "cJSON.h" // https://github.com/DaveGamble/cJSON

#ifdef ENABLE_LOG
	#define LOG_SIZE 50
	#define LOG_TEXT_SIZE 128
	#define LOG_DATA_SIZE 256
#else
	#define LOG_SIZE 0
	#define LOG_TEXT_SIZE 0
	#define LOG_DATA_SIZE 0
#endif
static const char *TAG = "T3CH_LOG";
//
int LOG_CNT_DEL=0;
//
int LOG_POS = 0;              // position of log array
int LOG_CNT = 0;              // increase for id
char LOG_DATA[LOG_DATA_SIZE+1]; // generated and auto incremented with t3ch_log_gen(..)
//int LAST_ID = 0;
QueueHandle_t log_qh;
//
struct Log {
	int id;
	char text[LOG_TEXT_SIZE];
};
//
struct Log log[LOG_SIZE]={0};
//char *log_text_arg;
char log_text_arg[LOG_TEXT_SIZE]={0};
//SemaphoreHandle_t xSemaphore;
//
int t3ch_log_gen_new(int lastId) {
	//
	int startPos = LOG_POS-1;
	//
    if( log[startPos].id<=lastId ) {
		//printf("t3ch_log_gen_old() t3ch_log_gen_new() looks there is no new data. lastId: %i vs: %i, using startPos: %i\n",lastId, log[startPos].id, startPos);
		return 0;
	}
	//
	LOG_DATA[LOG_DATA_SIZE+1];
	memset(LOG_DATA,'\0',LOG_DATA_SIZE+1);
	int jsonlen=0, cnt=0;
	// generate json array of objects
	jsonlen = sprintf(LOG_DATA+jsonlen,"[");
	for(int i=startPos; i>=0; i--) {
		if((i-1)>=0 && log[i-1].id>lastId) {
			//printf("t3ch_log_gen_new() continue at: %i, id: %i, id-1: %i, lastId: %i\n",i,log[i].id,log[i-1].id,lastId);
			continue;
		//	continue;
		}
		/*else if(log[i].id<=lastId) {
			printf("t3ch_log_gen_new() BREAK! id <= lastId: %i\n",lastId);
			break;
		//	continue;
		}*/
		// encode text
		int tmpsize = myUrlEncodeSize(log[i].text);
		char tmp[tmpsize+1];
		memset(tmp,'\0', tmpsize+1);
		myUrlEncode(log[i].text,tmp,tmpsize);
		
		//
		char tmpresult[LOG_DATA_SIZE+1]={0};
		int tmplen = sprintf(tmpresult,"{\"id\":%i,\"text\":\"%s\"}",log[i].id, tmp);
		
		//
		if( cnt==0 ) {
			jsonlen += sprintf(LOG_DATA+jsonlen,"%s",tmpresult);
			//printf("t3ch_log_gen_new() d0 at: %i, id: %i, jsonlen: %i, lastId: %i\n",i,log[i].id,jsonlen,lastId);
		}
		else if( log[i].id<=lastId ) {
			//printf("t3ch_log_gen_new() d3, lastId jsonlen: %i\n",jsonlen);
			break;
		}
		else if( (jsonlen+tmplen)>=LOG_DATA_SIZE ) {
			//printf("t3ch_log_gen_new() d1 at: %i, id: %i, break jsonlen: %i, lastId: %i\n",i,log[i].id,jsonlen,lastId);
			break;
		}
		else {
			jsonlen += sprintf(LOG_DATA+jsonlen,",%s",tmpresult);
			//printf("t3ch_log_gen_new() d2, add jsonlen: %i\n",jsonlen);
		}
		cnt++;
	}
	jsonlen += sprintf(LOG_DATA+jsonlen,"]");
	return jsonlen;
}
//
int t3ch_log_gen_old(int fromPos) {
	printf("t3ch_log_gen_old() STARTING, fromPos: %i\n",fromPos);
	if(LOG_POS<=0) return 0;
	int startPos = ((LOG_SIZE-(LOG_SIZE-LOG_POS))-fromPos)-1;
	if( startPos<0 ) return 0;
	if( log[startPos].id<=0 ) {
		printf("t3ch_log_gen_old() Ending d3\n");
		return 0;
	}
	printf("t3ch_log_gen_old() STARTING, startPos: %i\n",startPos);
	LOG_DATA[LOG_DATA_SIZE+1];
	memset(LOG_DATA,'\0',LOG_DATA_SIZE+1);
	int jsonlen=0, cnt=0;
	// generate json array of objects
	jsonlen = sprintf(LOG_DATA+jsonlen,"[");
	for(int i=startPos; i>=0; i--) {
		// encode text
		int tmpsize = myUrlEncodeSize(log[i].text);
		char tmp[tmpsize+1];
		memset(tmp,'\0', tmpsize+1);
		myUrlEncode(log[i].text,tmp,tmpsize);
		
		//
		char tmpresult[LOG_DATA_SIZE+1]={0};
		int tmplen = sprintf(tmpresult,"{\"id\":%i,\"text\":\"%s\"}",log[i].id, tmp);
		printf("t3ch_log_gen_old() t3ch_log_gen_old() at: %i, cnt: %i tmpresult( %i ): %s\n", i, cnt, strlen(tmpresult), tmpresult);
		
		//
		if( cnt==0 ) {
			jsonlen += sprintf(LOG_DATA+jsonlen,"%s",tmpresult);
			printf("t3ch_log_gen_old() t3ch_log_gen_old() d0 at: %i, id: %i, jsonlen: %i, fromPos: %i\n",i,log[i].id,jsonlen,fromPos);
		}
		else if( (jsonlen+tmplen)>=LOG_DATA_SIZE ) {
			printf("t3ch_log_gen_old() t3ch_log_gen_old() d1 at: %i, id: %i, jsonlen: %i, fromPos: %i\n",i,log[i].id,jsonlen,fromPos);
			break;
		}
		else {
			jsonlen += sprintf(LOG_DATA+jsonlen,",%s",tmpresult);
			printf("t3ch_log_gen_old() t3ch_log_gen_old() d2 at: %i, id: %i, jsonlen: %i, fromPos: %i\n",i,log[i].id,jsonlen,fromPos);
		}
		cnt++;
	}
	jsonlen += sprintf(LOG_DATA+jsonlen,"]");
	printf("t3ch_log_gen_old() DONE data(%i): %s\n",strlen(LOG_DATA),LOG_DATA);
	return jsonlen;
}

//
void t3ch_log_get(char *out) {
	//ESP_LOGI(TAG,"t3ch_log_gen_old() t3ch_log_get() STARTED. LOG_POS: %d, LOG_CNT: %d\n", LOG_POS, LOG_CNT);
	strcpy(out,LOG_DATA);
}

//
void t3ch_log_del(int pos) {
	//printf("t3ch_log_del() starting at pos %i\n",pos);
	for(int i=0; i<=(LOG_SIZE-1); i++) {
		log[i] = (i>=pos?log[i+1]:log[i]);
	}
	LOG_CNT_DEL++;
	LOG_POS--;
}

//
void t3ch_log_task(void *pt) {
	ESP_LOGI(TAG, "t3ch_log_task() STARTING");
	int log_cnt=1;
	//
	while( true ) {
		//
		//char *text = (char*)malloc( LOG_TEXT_SIZE+1 );
		char text[LOG_TEXT_SIZE];
		//
		//if( xSemaphoreTake( xSemaphore, 10 ) == pdTRUE ) {
			if( xQueueReceive( log_qh, &( text ), ( TickType_t ) 500 ) ) {
				//
				if( log_cnt<LOG_POS ) {
					printf("t3ch_log_task() d4 %i - %i\n",log_cnt,LOG_POS);
				} else {
					//
					if( LOG_POS>=(LOG_SIZE-1) ) {
						t3ch_log_del(0);
					}
					//
					strcpy(log[LOG_POS].text,text);
					log[LOG_POS].id = log_cnt;
					LOG_POS++;
					log_cnt++;
				}
			}
		//	xSemaphoreGive( xSemaphore );
		//}
		//free( text );
	}
}

//
void t3ch_log_init() {
	//
	/*xSemaphore = xSemaphoreCreateBinary();
	if( xSemaphore!=NULL ) {
		ESP_LOGI(TAG,"t3ch_log_init() xSemaphor created succesfully!\n");
		if( xSemaphoreGive( xSemaphore ) !=pdTRUE ) {
			ESP_LOGI(TAG,"t3ch_log_init() xSemaphor xSemaphorGive() Failed!\n");
		}
	}*/
	//
	//log_qh = xQueueCreate( 10, sizeof( struct AMessage * ) );
	log_qh = xQueueCreate( 10, sizeof( log_text_arg ));
	if( log_qh == 0 ) {
	    ESP_LOGW(TAG, "t3ch_log_init() Failed creating log_qh!");
	}
	//
	ESP_LOGI(TAG, "t3ch_log_init() STARTING t3ch_log_task()...");
	xTaskCreate(t3ch_log_task, "t3ch_log_task", 4048, NULL, 1, NULL);
}

//
int t3ch_log(const char *text, va_list args) {
	//printf("t3ch_log() STARTING!!! LOG_POS: %i\n",LOG_POS);
	//
	/*if( LOG_POS>=(LOG_SIZE-1) ) {
		t3ch_log_del(0);
	}*/

	//
	//va_list ap;
    //va_start(ap, text);
    /**
    size_t size_string=snprintf(NULL,0,format,arguments_list);//Calculating the size of the formed string 
	string=(char *)malloc(size_string+4);//Initialising the string 
	vsnprintf(string,size_string,format,arguments_list);//Storing the outptut
	*/
    //
    //int ret = snprintf(NULL, 0, text, ap);
    //int ret = vfprintf(stdout, text, ap);
    int ret = vfprintf(stdout, text, args);
    
    //char tmp[ret];
    char *tmp = (char*)malloc(ret+1);
    memset(tmp,'\0',ret+1);
    vsprintf(tmp, text, args);
    //va_end(ap);
    
    //-- Lets try manage async way of logging with xTaskCreate, xQueueCreate etc...
    // using queue handle: log_qh
    // using pointer structure to pass data with queue to task: *log_arg
    //log_text_arg = (char*)malloc( LOG_TEXT_SIZE+1 );
    //memset(log_text_arg,'\0',LOG_TEXT_SIZE+1);
    
    
    //-- Without async this kind of logging works fine.
    //
	if( strlen(tmp)>(LOG_TEXT_SIZE-4) ) {
		//ESP_LOGI(TAG,"t3ch_log_gen_old() t3ch_log() text too long: %i, shortening...\n",strlen(tmp));
		//memcpy(log[LOG_POS].text,tmp,(LOG_TEXT_SIZE-4));
		//strcpy(log[LOG_POS].text+(LOG_TEXT_SIZE-4),"...");
		memcpy(log_text_arg,tmp,(LOG_TEXT_SIZE-4));
		strcpy(log_text_arg+(LOG_TEXT_SIZE-4),"...");
	}
	//
	else {
		//ESP_LOGI(TAG,"t3ch_log_gen_old() t3ch_log() text ok: %i\n",strlen(tmp));
		//strcpy(log[LOG_POS].text,tmp);
		strcpy(log_text_arg,tmp);
	}
	//log[LOG_POS].id = LOG_CNT;
	//##
	
	//--
	// pass data with queue to log_task
	xQueueSend( log_qh, (void*)&log_text_arg, (TickType_t)0 );
	
	//
	//free( log_text_arg );
	free( tmp );
	/*LOG_POS++;
	LOG_CNT++;*/
	return ret;
}

//
void t3ch_log_test() {
	ESP_LOGI(TAG,"t3ch_log_test() STARTING!!!\n");
	for(int i=0; i<LOG_POS; i++) {
		//ESP_LOGI(TAG,"t3ch_log_test() at %i, test( %i ): %s\n",i,log[i].id,log[i].text);
		hexDump(log[i].text,log[i].text,strlen(log[i].text),20);
	}
	ESP_LOGI(TAG,"t3ch_log_test() DONE!!!\n");
}
