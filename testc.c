#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <stdarg.h>
#include <stdbool.h>
//#include <b64/cencode.h>
//#include <b64/cdecode.h>


/* This is the basic CRC-32 calculation with some optimization but no
table lookup. The the byte reversal is avoided by shifting the crc reg
right instead of left and by using a reversed 32-bit word to represent
the polynomial.
   When compiled to Cyclops with GCC, this function executes in 8 + 72n
instructions, where n is the number of bytes in the input message. It
should be doable in 4 + 61n instructions.
   If the inner loop is strung out (approx. 5*8 = 40 instructions),
it would take about 6 + 46n instructions. */
unsigned int crc32b(unsigned char *message) {
   int i, j;
   unsigned int byte, crc, mask;

   i = 0;
   crc = 0xFFFFFFFF;
   while (message[i] != 0) {
      byte = message[i];            // Get next byte.
      crc = crc ^ byte;
      for (j = 7; j >= 0; j--) {    // Do eight times.
         mask = -(crc & 1);
         crc = (crc >> 1) ^ (0xEDB88320 & mask);
      }
      i = i + 1;
   }
   return ~crc;
}

//
int chrat(char *str, char key) {
	char *pch = strchr(str,key);
	return (int)(pch-str);
}

// Stupid helper function that returns the value of a hex char.
static int esp_web_char2hex(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }

    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }

    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }

    return 0;
}

/**
 * @brief Decode a percent-encoded value.
 * Takes the valLen bytes stored in val, and converts it into at most retLen bytes that
 * are stored in the ret buffer. Returns the actual amount of bytes used in ret. Also
 * zero-terminates the ret buffer.
 *
 */
static int esp_web_url_decode(char *val, int valLen, char *ret, int retLen)
{
    int s = 0, d = 0;
    int esced = 0, escVal = 0;

    while (s < valLen && d < retLen) {
        if (esced == 1) {
            escVal = esp_web_char2hex(val[s]) << 4;
            esced = 2;
        } else if (esced == 2) {
            escVal += esp_web_char2hex(val[s]);
            ret[d++] = escVal;
            esced = 0;
        } else if (val[s] == '%') {
            esced = 1;
        } else if (val[s] == '+') {
            ret[d++] = ' ';
        } else {
            ret[d++] = val[s];
        }

        s++;
    }

    if (d < retLen) {
        ret[d] = 0;
    }

    return d;
}

//
//
/*
char tmp1[] = "hello%20World";
char *p, *e;
p = tmp1;
e = p + strlen(p);
char buff[128]={0};
int ret = esp_web_url_decode(p, (e - p), buff, 128);
printf("ret: %i, buf: %s\n",ret,buff);
*/
int t3ch_urldecode(char *in, char *out, int size) {
	char *p, *e;
	p = in;
	e = p + strlen(p);
	int ret = esp_web_url_decode(p, (e-p), out, size);
	printf("ret: %d\n", ret);
	printf("decoded: %s\n", out);
	return ret;
}

/*char rfc3986[256] = {0};
char html5[256] = {0};
void url_encoder_rfc_tables_init(){

    int i;

    for (i = 0; i < 256; i++){

        rfc3986[i] = (isalnum( i) || i == '~' || i == '-' || i == '.' || i == '_' ? i : 0);
        html5[i] = (isalnum( i) || i == '*' || i == '-' || i == '.' || i == '_' ? i : (i == ' ') ? '+' : 0);
    }
}
char *url_encode( char *table, unsigned char *s, char *enc){

    for (; *s; s++){

        if (table[*s]) *enc = table[*s];
        else sprintf( enc, "%%%02X", *s);
        while (*++enc);
    }

    return( enc);
}*/

 int UrlEncode(char* url, char* encode,  char* buffer, unsigned int size)
    {
        char chars[127] = {0};
        unsigned int length = 0;

        if(!url || !encode || !buffer) return 0;

//Create an array to hold ascii chars, loop through encode string
//and assign to place in array. I used this construct instead of a large if statement for speed.
        while(*encode) chars[*encode++] = *encode;

//Loop through url, if we find an encode char, replace with % and add hex
//as ascii chars. Move buffer up by 2 and track the length needed.
//If we reach the query string (?), move to query string encoding
    URLENCODE_BASE_URL:
        while(size && (*buffer = *url)) {
            if(*url == '?') goto URLENCODE_QUERY_STRING;
            if(chars[*url] && size > 2) {
                *buffer++ = '%';
                //itoa(*url, buffer, 16);
                sprintf( buffer, "%02X", *url);
                buffer++; size-=2; length+=2;
            }
            url++, buffer++, size--; length++;  
        }
        goto URLENCODE_RETURN;

//Same as above but on spaces (' '), replace with plus ('+') and convert
//to hex ascii. I moved this out into a separate loop for speed.
    URLENCODE_QUERY_STRING:
        while(size && (*buffer = *url)) {
            if(chars[*url] && size > 2) {
                *buffer++ = '%';
                if(*url == ' ') {
					//itoa('+', buffer, 16);
					sprintf( buffer, "%02X", '+');
                }
                else {
					//itoa(*url, buffer, 16);
					sprintf( buffer, "%02X", *url);
				}
                buffer++; size-=2; length+=2;
            }
            url++, buffer++, size--; length++;
        }

//Terminate the end of the buffer, and if the buffer wasn't large enough
//calc the rest of the url length and return
    URLENCODE_RETURN:
        *buffer = '\0';
        if(*url)
        while(*url) { if(chars[*url]) length+=2; url++; length++; }
        return length;
}

int match(const char *rx, char *buf) {
	regex_t regex;
	int reti;
	//char msgbuf[100];
	//
	reti = regcomp(&regex,rx,0);
	if( reti ) {
		printf("Failed!\n");
		regfree(&regex);
		return 0;
	}
	//
	reti = regexec(&regex,buf,0,NULL,0);
	if(!reti) {
		printf("Match!\n");
		regfree(&regex);
		return 1;
	}
	else if(reti==REG_NOMATCH) {
		printf("No match!\n");
		regfree(&regex);
		return 1;
	}
	else {
		printf("Looks error!\n");
		regfree(&regex);
		return 1;
	}
}

/*void _printf(const char *fmt, FILE *out, va_list ap) {
	char tmp[256]={0};
    //vfprintf(out, fmt, ap);
    sprintf(out, fmt, ap);
    //strcpy(tmp,out);
    //printf("DEBUG xxx tmp: %s\n", tmp);
    //printf("DEBUG xxx out: %s\n", out);
}*/

void mprintf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    //_printf(fmt, stdout, ap);
    char tmp[256]={0};
    //vfprintf(stdout, fmt, ap);
    vsprintf(tmp, fmt, ap);
    printf(tmp);
    va_end(ap);
}

struct MyStruct {
	int id;
	char name[64];
	char desc[256];
};
int pmys=0; // position current
struct MyStruct amys[10]={0};
void addStruct(char *name, char *desc) {
	struct MyStruct mys;
	strcpy(mys.name,name);
	strcpy(mys.desc,desc);
	mys.id = pmys;
	amys[pmys] = mys;
	pmys++;
}
//void delStruct(struct MyStruct[], int at) {
//	printf("delStruct() starting, MyStruct size: %d\n", (sizeof(MyStruct) / sizeof(MyStruct[0]) ) );
//}
void viewStruct() {
	printf("DEBUG MyStruct start, pmys: %i, try get size of amys: %d\n",pmys, (sizeof(amys)/(sizeof(amys[0]))) );
	//
	//for(int i=0; i<pmys; i++) {
	for(int i=0; i<(sizeof(amys)/(sizeof(amys[0]))); i++) {
		printf("DEBUG MyStruct i at: %i\n",i);
		if(strlen(amys[i].name)<=0) break;
		struct MyStruct mys = amys[i];
		printf("DEBUG MyStruct at %i, name: %s, desc: %s\n",mys.id, mys.name, mys.desc);
	}
	printf("DEBUG MyStruct done.\n");
}
//
bool Contain(char inArray[], int inArraySize, char chk) {
	for(int i=0; i<inArraySize; i++) {
		if(inArray[i] == chk) return true;
	}
	return false;
}
//
int myUrlEncodeSize(char *in) {
	int chk=0, cnt=0, size=strlen(in);
	//                                        '
	char chars[] = {'\r','\n',' ','!','"','@',39,'.','+','-','_'};
	// calc new ret size
	while(in[chk]!=NULL) {
		if(Contain(chars,sizeof(chars)/sizeof(chars[0]),in[chk])) cnt+=3;
		chk++;
	}
	return (size+cnt);
}
//
void myUrlEncode(char *in, char *out, int newSize) {
	int chk=0, cnt=0, size=strlen(in);
	//                                        '
	char chars[] = {'\r','\n',' ','!','"','@',39,'.','+','-','_'};
	char ret[newSize];
	memset(ret,'\0',newSize);
	chk=0,cnt=0;
	//
	while(in[chk]!=NULL) {
		//printf("myUrlEncode() debug char at %i - %c - %x\n",in[chk],in[chk],in[chk]);
		if( Contain(chars,sizeof(chars)/sizeof(chars[0]),in[chk]) ) {
			cnt += sprintf(ret+cnt,"%%%s%x",((int)in[chk]<16?"0":""),in[chk]);
		}
		else {
			cnt += sprintf(ret+cnt,"%c",in[chk]);
		}
		chk++;
	}
	strcpy(out,ret);
}

void rtrim(char inout[], int len) {
	char tmp[strlen(inout)];
	memset(tmp,'\0',strlen(inout));
	memcpy(tmp,inout,strlen(inout)-len);
	strcpy(inout,tmp);
}

void ltrim(char *inout, int len) {
	char *a = (char*)malloc(strlen(inout));
	memset(a,'\0',strlen(inout));
	strcpy(a,inout);
	strcpy(inout,(a+=len));
}

void main() {
	//
	printf("test clock: %i\n",time(NULL));
	//
	char hash[8]={0};
	char *ihash = crc32b("holaa");
	printf("ihash: %x\n",ihash);
	sprintf(hash,"%x",ihash);
	printf("hash: %s\n",hash);
	
	//for(int i=0; i<30; i++) {
	//	printf("DEBUG hex at: %i - %x\n",i,i);
	//}
	/*//
	addStruct("test 1", "test 1 description...");
	addStruct("test 2", "test 2 description...");
	viewStruct();
	//delStruct(amys, 1);
	//viewStruct();
	//
	char xxx[] = "aG9sYQ==";
	mprintf("xxx: %d\n",strlen(xxx));
	//
	char tmp[] = "test=123";
	int x = chrat(tmp,'=');
	mprintf("hola! pos: %i\n",x);
	
	//
	char tmp1[] = "hello%20World";
	int buffsize = (strlen(tmp1)+1);
	char buff[buffsize];
	memset(buff,'\0',buffsize);
	t3ch_urldecode(tmp1, buff, buffsize);
	//
	char out[128]={0};
	UrlEncode(buff, buff, out, 128);
	printf("Encoded: %s\n",out);
	*/
	//
	char tmp2[] = "hello\r\n\r\nWorld\r\nAa Hmm test@\"'-. Done!+";
	printf("seaching World...in: %s\n",tmp2);
	//
	int newSize = myUrlEncodeSize(tmp2);
	//
	//char tmp5[newSize];
	//myUrlEncode(tmp2,tmp5,newSize);
	//printf("tmp3 d0: %s\n", tmp5);
	//
	char tmp3[newSize];
	myUrlEncode(tmp2,tmp3,newSize);
	printf("tmp3 d1: %s\n", tmp3);
	rtrim(tmp3,3);
	printf("tmp3 d2: %s\n", tmp3);
	//char *tmpx = fuck(tmp3);
	ltrim(tmp3,1);
	printf("tmp4 d0: %s\n",tmp3);
	
	/*// example retrive of length of string
	printf("tmp2 size: %d, sizeof: %d\n", ( sizeof(tmp2)/sizeof(tmp2[0])), sizeof(tmp2));
	
	char * pch = strstr(tmp2,"\r\n\r\n");
	if( pch==NULL ) {
		printf("Can not find what we are searching\n");
	}
	else {
		printf("Looks we found what we search..: \n%s\n",pch);
		char xx[strlen(pch)];
		memset(xx,'\0',strlen(pch));
		strcpy(xx,pch+4);
		printf("fixing...: %s\n",xx);
	}
	
	//
	if( match("^-.*WebKitFormBoundary.*","a------WebKitFormBoundary5ing3KxzYPLdymXH--") ) {
		printf("Looks we got comething... :)!\n");
	}
	else {
		printf("Looks didnt match nothing!\n");
	}
	*/
}
