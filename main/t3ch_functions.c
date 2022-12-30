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

//
int getInt(char *str) {
	return strtol(str, (char**)NULL, 10);
}

//
int chrat(char *str, char key) {
	char *pch = strchr(str,key);
	return (int)(pch-str);
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
