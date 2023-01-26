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

#include <stdio.h>
#include <string.h>
#include <regex.h>
#include "b64/cdecode.h"
#include "cJSON.h"
#include <errno.h>
#include <stdbool.h>

//-- cJSON
//
char *cjson_oget(char *in, const char *key) {
	cJSON *json = cJSON_Parse( in );
	return cJSON_Print(cJSON_GetObjectItemCaseSensitive(json,key));
}

//-- Other
//
int getInt(char *str) {
	return strtol(str, (char**)NULL, 10);
}

//
int getSize(char *buf) {
	return sizeof(buf)/sizeof(buf[0]);
}

//
void rtrim(char inout[], int len) {
	char tmp[strlen(inout)];
	memset(tmp,'\0',strlen(inout));
	memcpy(tmp,inout,strlen(inout)-len);
	strcpy(inout,tmp);
}

//
int chrat(char *str, char key) {
	char *pch = strchr(str,key);
	return (int)(pch-str);
}

/**/
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
		return 0;
	}
	else {
		printf("Looks error!\n");
		regfree(&regex);
		return 0;
	}
}

/**
 * function split() taken from: 
 * https://stackoverflow.com/questions/54261257/splitting-a-string-and-returning-an-array-of-strings
 * Fixed by t3ch for arduino.
 * */
char ** split(const char * str, const char * delim) {
  /* count words */
  char * s = strdup(str);
  
  if (strtok(s, delim) == 0)
  /* no word */
  return NULL;
  
  int nw = 1;
  
  while (strtok(NULL, delim) != 0)
  nw += 1;
  
  strcpy(s, str); /* restore initial string modified by strtok */
  
  /* split */
  //char ** v = malloc((nw + 1) * sizeof(char *));
  char ** v = (char**)malloc((nw + 1) * sizeof(char *));
  int i;
  
  v[0] = strdup(strtok(s, delim));
  //
  for (i = 1; i != nw; ++i) {
    v[i] = strdup( strtok(NULL, delim) );
  }
  v[i] = NULL; /* end mark */
  free(s);
  return v;
}

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
bool Contain(char inArray[], int inArraySize, char chk) {
	for(int i=0; i<inArraySize; i++) {
		if(inArray[i] == chk) return true;
	}
	return false;
}

/**
 * myUrlEncode... by t3ch aka w4d4f4k
 * required:
 * - (function) Contain()
 * - (char array) myUrlEncodeChars
 * - (function) myUrlEncodeSize()
 * - (function) myUrlEncode()
 * */
//
char myUrlEncodeChars[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,' ','!','"','@',39,'.','+','-','_',27,':',';','[',']',',','%','}','{','&'};
//
int myUrlEncodeSize(char *in) {
	int chk=0, cnt=0, size=strlen(in);
	//                                        '
	// calc new ret size
	while(in[chk]!=NULL) {
		if(Contain(myUrlEncodeChars,sizeof(myUrlEncodeChars)/sizeof(myUrlEncodeChars[0]),in[chk])) cnt+=3;
		chk++;
	}
	return (size+cnt);
}
//
void myUrlEncode(char *in, char *out, int newSize) {
	int chk=0, cnt=0, size=strlen(in);
	//                                        '
	char ret[newSize];
	memset(ret,'\0',newSize);
	chk=0,cnt=0;
	//
	while(in[chk]!=NULL) {
		if( Contain(myUrlEncodeChars,sizeof(myUrlEncodeChars)/sizeof(myUrlEncodeChars[0]),in[chk]) ) {
			cnt += sprintf(ret+cnt,"%%%s%x",((int)in[chk]<16?"0":""),in[chk]);
		}
		else {
			cnt += sprintf(ret+cnt,"%c",in[chk]);
		}
		chk++;
	}
	strcpy(out,ret);
}

/**
 * from here:
 * https://stackoverflow.com/questions/5842471/c-url-encoding
 * modified by t3ch for warnings
 * */
// web_server.c => static int esp_web_url_decode()
/*int esp_web_url_encode(char* url, char* encode,  char* buffer, unsigned int size) {
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
}*/

// Stupid helper function that returns the value of a hex char.
int esp_web_char2hex(char c)
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

//
int esp_web_url_decode(char *val, int valLen, char *ret, int retLen)
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
int t3ch_urldecode(char *in, char *out, int size) {
	char *p, *e;
	p = in;
	e = p + strlen(p);
	int ret = esp_web_url_decode(p, (e-p), out, size);
	//printf("ret: %d\n", ret);
	//printf("decoded: %s\n", out);
	return ret;
}

//
char* b64decode(const char* input)
{
	/* set up a destination buffer large enough to hold the encoded data */
	char* output = (char*)malloc(strlen(input));
	memset(output,'\0',strlen(input));
	/* keep track of our decoded position */
	char* c = output;
	/* store the number of bytes decoded by a single call */
	int cnt = 0;
	/* we need a decoder state */
	base64_decodestate s;
	
	/*---------- START DECODING ----------*/
	/* initialise the decoder state */
	base64_init_decodestate(&s);
	/* decode the input data */
	cnt = base64_decode_block(input, strlen(input), c, &s);
	c += cnt;
	/* note: there is no base64_decode_blockend! */
	/*---------- STOP DECODING  ----------*/
	
	/* we want to print the decoded data, so null-terminate it: */
	*c = 0;
	
	return output;
}

// Usage:
//     hexDump(desc, addr, len, perLine);
//         desc:    if non-NULL, printed as a description before hex dump.
//         addr:    the address to start dumping from.
//         len:     the number of bytes to dump.
//         perLine: number of bytes on each output line.

void hexDump (
    const char * desc,
    const void * addr,
    const int len,
    int perLine
) {
    // Silently ignore silly per-line values.

    if (perLine < 4 || perLine > 64) perLine = 16;

    int i;
    unsigned char buff[perLine+1];
    const unsigned char * pc = (const unsigned char *)addr;

    // Output description if given.

    if (desc != NULL) printf ("%s:\n", desc);

    // Length checks.

    if (len == 0) {
        printf("  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        printf("  NEGATIVE LENGTH: %d\n", len);
        return;
    }

    // Process every byte in the data.

    for (i = 0; i < len; i++) {
        // Multiple of perLine means new or first line (with line offset).

        if ((i % perLine) == 0) {
            // Only print previous-line ASCII buffer for lines beyond first.

            if (i != 0) printf ("  %s\n", buff);

            // Output the offset of current line.

            printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.

        printf (" %02x", pc[i]);

        // And buffer a printable ASCII character for later.

        if ((pc[i] < 0x20) || (pc[i] > 0x7e)) // isprint() may be better.
            buff[i % perLine] = '.';
        else
            buff[i % perLine] = pc[i];
        buff[(i % perLine) + 1] = '\0';
    }

    // Pad out last line if not exactly perLine characters.

    while ((i % perLine) != 0) {
        printf ("   ");
        i++;
    }

    // And print the final ASCII buffer.

    printf ("  %s\n", buff);
}

/*int GetUtf8CharacterLength( unsigned char utf8Char )
{
    if ( utf8Char < 0x80 ) return 1;
    else if ( ( utf8Char & 0x20 ) == 0 ) return 2;
    else if ( ( utf8Char & 0x10 ) == 0 ) return 3;
    else if ( ( utf8Char & 0x08 ) == 0 ) return 4;
    else if ( ( utf8Char & 0x04 ) == 0 ) return 5;

    return 6;
}

char Utf8ToLatin1Character( char *s, int *readIndex )
{
    int len = GetUtf8CharacterLength( static_cast<unsigned char>( s[ *readIndex ] ) );
    if ( len == 1 )
    {
        char c = s[ *readIndex ];
        (*readIndex)++;

        return c;
    }

    unsigned int v = ( s[ *readIndex ] & ( 0xff >> ( len + 1 ) ) ) << ( ( len - 1 ) * 6 );
    (*readIndex)++;
    for ( len-- ; len > 0 ; len-- )
    {
        v |= ( static_cast<unsigned char>( s[ *readIndex ] ) - 0x80 ) << ( ( len - 1 ) * 6 );
        (*readIndex)++;
    }

    return ( v > 0xff ) ? 0 : (char)v;
}

// overwrites s in place
char *Utf8ToLatin1String( char *s )
{
    for ( int readIndex = 0, writeIndex = 0 ; ; writeIndex++ )
    {
        if ( s[ readIndex ] == 0 )
        {
            s[ writeIndex ] = 0;
            break;
        }

        char c = Utf8ToLatin1Character( s, &readIndex );
        if ( c == 0 )
        {
            c = '_';
        }

        s[ writeIndex ] = c;
    }

    return s;
}*/
/*char *utf8_to_latin9(const char *const string)
{
    size_t         size = 0;
    size_t         used = 0;
    unsigned char *result = NULL;

    if (string) {
        const unsigned char  *s = (const unsigned char *)string;

        while (*s) {

            if (used >= size) {
                void *const old = result;

                size = (used | 255) + 257;
                result = realloc(result, size);
                if (!result) {
                    if (old)
                        free(old);
                    errno = ENOMEM;
                    return NULL;
                }
            }

            if (*s < 128) {
                result[used++] = *(s++);
                continue;

            } else
            if (s[0] == 226 && s[1] == 130 && s[2] == 172) {
                result[used++] = 164;
                s += 3;
                continue;

            } else
            if (s[0] == 194 && s[1] >= 128 && s[1] <= 191) {
                result[used++] = s[1];
                s += 2;
                continue;

            } else
            if (s[0] == 195 && s[1] >= 128 && s[1] <= 191) {
                result[used++] = s[1] + 64;
                s += 2;
                continue;

            } else
            if (s[0] == 197 && s[1] == 160) {
                result[used++] = 166;
                s += 2;
                continue;

            } else
            if (s[0] == 197 && s[1] == 161) {
                result[used++] = 168;
                s += 2;
                continue;

            } else
            if (s[0] == 197 && s[1] == 189) {
                result[used++] = 180;
                s += 2;
                continue;

            } else
            if (s[0] == 197 && s[1] == 190) {
                result[used++] = 184;
                s += 2;
                continue;

            } else
            if (s[0] == 197 && s[1] == 146) {
                result[used++] = 188;
                s += 2;
                continue;

            } else
            if (s[0] == 197 && s[1] == 147) {
                result[used++] = 189;
                s += 2;
                continue;

            } else
            if (s[0] == 197 && s[1] == 184) {
                result[used++] = 190;
                s += 2;
                continue;

            }

            if (s[0] >= 192 && s[0] < 224 &&
                s[1] >= 128 && s[1] < 192) {
                s += 2;
                continue;
            } else
            if (s[0] >= 224 && s[0] < 240 &&
                s[1] >= 128 && s[1] < 192 &&
                s[2] >= 128 && s[2] < 192) {
                s += 3;
                continue;
            } else
            if (s[0] >= 240 && s[0] < 248 &&
                s[1] >= 128 && s[1] < 192 &&
                s[2] >= 128 && s[2] < 192 &&
                s[3] >= 128 && s[3] < 192) {
                s += 4;
                continue;
            } else
            if (s[0] >= 248 && s[0] < 252 &&
                s[1] >= 128 && s[1] < 192 &&
                s[2] >= 128 && s[2] < 192 &&
                s[3] >= 128 && s[3] < 192 &&
                s[4] >= 128 && s[4] < 192) {
                s += 5;
                continue;
            } else
            if (s[0] >= 252 && s[0] < 254 &&
                s[1] >= 128 && s[1] < 192 &&
                s[2] >= 128 && s[2] < 192 &&
                s[3] >= 128 && s[3] < 192 &&
                s[4] >= 128 && s[4] < 192 &&
                s[5] >= 128 && s[5] < 192) {
                s += 6;
                continue;
            }

            s++;
        }
    }

    {
        void *const old = result;

        size = (used | 7) + 1;

        result = realloc(result, size);
        if (!result) {
            if (old)
                free(old);
            errno = ENOMEM;
            return NULL;
        }

        memset(result + used, 0, size - used);
    }

    return (char *)result;
}*/
