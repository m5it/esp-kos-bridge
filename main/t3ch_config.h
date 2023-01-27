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

//
#define AP_SSID "STARTING_SSID"
#define AP_PWD "STARTING_PWD"

// Uncomment #define ENABLE_LOG to enable WEB log.
#define ENABLE_LOG
// Uncomment #define ENABLE_HTTPS to run HTTPS instead of HTTP
#define ENABLE_HTTPS
// Uncomment to enable. (require ENABLE_HTTPS)
#define ENABLE_WSS

//
#define VERSION_STRING "Rasca Pantalla"
// main/version.txt should be generated with every build. (increased)
// usage ex.: ./build.sh 0.4
extern const uint8_t VERSION[] asm("_binary_version_txt_start");
