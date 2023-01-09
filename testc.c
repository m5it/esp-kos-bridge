#include <stdio.h>
#include <string.h>
#include <regex.h>
//#include <b64/cencode.h>
//#include <b64/cdecode.h>
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

void main() {
	//
	char xxx[] = "aG9sYQ==";
	printf("xxx: %d\n",strlen(xxx));
	//
	char tmp[] = "test=123";
	int x = chrat(tmp,'=');
	printf("hola! pos: %i\n",x);
	
	//
	char tmp1[] = "hello%20World";
	/*char *p, *e;
	p = tmp1;
	e = p + strlen(p);
	char buff[128]={0};
	int ret = esp_web_url_decode(p, (e - p), buff, 128);
	printf("ret: %i, buf: %s\n",ret,buff);
	*/
	int buffsize = (strlen(tmp1)+1);
	char buff[buffsize];
	memset(buff,'\0',buffsize);
	t3ch_urldecode(tmp1, buff, buffsize);
	//
	char out[128]={0};
	UrlEncode(buff, buff, out, 128);
	printf("Encoded: %s\n",out);
	
	//
	char tmp2[] = "hello\r\n\r\nWorld\r\n";
	printf("seaching World...in: %s\n",tmp2);
	// example retrive of length of string
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
	
}
