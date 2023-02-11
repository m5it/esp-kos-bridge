#include <stdarg.h>
#include "t3ch_time.h"
#include "t3ch_log.h"
#include "cJSON.h" // https://github.com/DaveGamble/cJSON

#ifdef ENABLE_LOG
	#define LOG_SIZE 100
	#define LOG_TEXT_SIZE 128
	#define LOG_DATA_SIZE 256
#else
	#define LOG_SIZE 0
	#define LOG_TEXT_SIZE 0
	#define LOG_DATA_SIZE 0
#endif
static const char *TAG = "T3CH_LOG";
//
int LOG_POS = 0;              // position of log array
int LOG_CNT = 0;              // increase for id
char LOG_DATA[LOG_DATA_SIZE]; // generated and auto incremented with t3ch_log_gen(..)
//int LAST_ID = 0;
//
struct Log {
	int id;
	char text[LOG_TEXT_SIZE];
};
//
struct Log log[LOG_SIZE]={0};
//
int t3ch_log_gen_new(int lastId) {
	//printf("t3ch_log_gen_new() STARTING, lastId: %i, LOG_POS: %i\n",lastId,LOG_POS);
	//
	int startPos = LOG_POS-1;
	//printf("t3ch_log_gen_new() DEBUG startPos: %i, LOG_POS: %i\n", startPos, LOG_POS);
	//
    //if( log[startPos].id<=lastId ) {
	//	printf("t3ch_log_gen_old() t3ch_log_gen_new() looks there is no new data. lastId: %i vs: %i, using startPos: %i\n",lastId, log[startPos].id, startPos);
	//	return 0;
	//}
	//
	LOG_DATA[LOG_DATA_SIZE];
	memset(LOG_DATA,'\0',LOG_DATA_SIZE);
	int jsonlen=0, cnt=0;
	// generate json array of objects
	jsonlen = sprintf(LOG_DATA+jsonlen,"[");
	for(int i=startPos; i>=0; i--) {
		//printf("t3ch_log_gen_new() DEBUG i: %i, id: %i vs lastId: %i\n",i,log[i].id,lastId);
		//
		if(log[i].id<=lastId) {
			//printf("t3ch_log_gen_new() CONTINuE! id <= lastId!\n");
		//	break;
			continue;
		}
		// encode text
		int tmpsize = myUrlEncodeSize(log[i].text);
		char tmp[tmpsize+1];
		memset(tmp,'\0', tmpsize+1);
		myUrlEncode(log[i].text,tmp,tmpsize);
		
		// prepare id and its length
		char tmpid[12];
		memset(tmpid,'\0',12);
		sprintf(tmpid,"%i",log[i].id);
		
		//
		char tmpresult[LOG_DATA_SIZE];
		int tmplen = sprintf(tmpresult,"{\"id\":%s,\"text\":\"%s\"}",tmpid, tmp);
		
		//
		if( cnt==0 ) {
			jsonlen += sprintf(LOG_DATA+jsonlen,"%s",tmpresult);
			//printf("t3ch_log_gen_old() t3ch_log_gen_new() d0 jsonlen: %i\n",jsonlen);
		}
		/*else if( log[i].id<=lastId ) {
			printf("t3ch_log_gen_old() t3ch_log_gen_new() d3, lastId jsonlen: %i\n",jsonlen);
			break;
		}*/
		else if( (jsonlen+tmplen)>=LOG_DATA_SIZE ) {
			//printf("t3ch_log_gen_old() t3ch_log_gen_new() d1, break jsonlen: %i\n",jsonlen);
			break;
		}
		else {
			jsonlen += sprintf(LOG_DATA+jsonlen,",%s",tmpresult);
			//printf("t3ch_log_gen_old() t3ch_log_gen_new() d2, add jsonlen: %i\n",jsonlen);
		}
		cnt++;
	}
	jsonlen += sprintf(LOG_DATA+jsonlen,"]");
	return jsonlen;
}
//
int t3ch_log_gen_old(int fromPos) {
	//printf("t3ch_log_gen_old() STARTING, fromPos: %i\n",fromPos);
	if(LOG_POS<=0) return 0;
	int startPos = ((LOG_SIZE-(LOG_SIZE-LOG_POS))-fromPos)-1;
	if( startPos<0 ) return 0;
	//printf("t3ch_log_gen_old() STARTING, startPos: %i\n",startPos);
	LOG_DATA[LOG_DATA_SIZE];
	memset(LOG_DATA,'\0',LOG_DATA_SIZE);
	int jsonlen=0, cnt=0;
	// generate json array of objects
	jsonlen = sprintf(LOG_DATA+jsonlen,"[");
	for(int i=startPos; i>=0; i--) {
		// encode text
		int tmpsize = myUrlEncodeSize(log[i].text);
		char tmp[tmpsize+1];
		memset(tmp,'\0', tmpsize+1);
		myUrlEncode(log[i].text,tmp,tmpsize);
		
		// prepare id and its length
		char tmpid[12];
		memset(tmpid,'\0',12);
		sprintf(tmpid,"%i",log[i].id);
		
		//
		char tmpresult[LOG_DATA_SIZE];
		int tmplen = sprintf(tmpresult,"{\"id\":%s,\"text\":\"%s\"}",tmpid, tmp);
		
		//
		if( cnt==0 ) {
			jsonlen += sprintf(LOG_DATA+jsonlen,"%s",tmpresult);
			//printf("t3ch_log_gen_old() t3ch_log_gen_old() d0 jsonlen: %i\n",jsonlen);
		}
		else if( (jsonlen+tmplen)>=LOG_DATA_SIZE ) {
			//printf("t3ch_log_gen_old() t3ch_log_gen_old() d1, break jsonlen: %i\n",jsonlen);
			break;
		}
		else {
			jsonlen += sprintf(LOG_DATA+jsonlen,",%s",tmpresult);
			//printf("t3ch_log_gen_old() t3ch_log_gen_old() d2, add jsonlen: %i\n",jsonlen);
		}
		cnt++;
	}
	jsonlen += sprintf(LOG_DATA+jsonlen,"]");
	//printf("t3ch_log_gen_old() DONE data(%i): %s\n",strlen(LOG_DATA),LOG_DATA);
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
	for(int i=0; i<(LOG_SIZE-1); i++) {
		log[i] = (i>=pos?log[i+1]:log[i]);
	}
	LOG_POS--;
}

//
int t3ch_log(const char *text, va_list args) {
	//ESP_LOGI(TAG,"t3ch_log_gen_old() t3ch_log() STARTING!!! LOG_POS: %i\n",LOG_POS);
	//
	if( LOG_POS>=(LOG_SIZE-1) ) {
		t3ch_log_del(0);
	}
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
    //
    //hexDump(tmp,tmp,strlen(tmp),20);
	//
	//strcpy(log[LOG_POS].tag,tag);
	if( strlen(tmp)>(LOG_TEXT_SIZE-4) ) {
		//ESP_LOGI(TAG,"t3ch_log_gen_old() t3ch_log() text too long: %i, shortening...\n",strlen(tmp));
		memcpy(log[LOG_POS].text,tmp,(LOG_TEXT_SIZE-4));
		strcpy(log[LOG_POS].text+(LOG_TEXT_SIZE-4),"...");
	}
	else {
		//ESP_LOGI(TAG,"t3ch_log_gen_old() t3ch_log() text ok: %i\n",strlen(tmp));
		strcpy(log[LOG_POS].text,tmp);
	}
	log[LOG_POS].id = LOG_CNT;
	//
	free(tmp);
	LOG_POS++;
	LOG_CNT++;
	return ret;
}

//
void t3ch_log_test() {
	printf("t3ch_log_test() STARTING!!!\n");
	for(int i=0; i<LOG_POS; i++) {
		printf("t3ch_log_test() at %i, test( %i ): %s\n",i,log[i].id,log[i].text);
		hexDump(log[i].text,log[i].text,strlen(log[i].text),20);
	}
	printf("t3ch_log_test() DONE!!!\n");
}
