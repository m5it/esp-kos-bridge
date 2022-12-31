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

//--
//
#define VERSION_STRING "Conejo blanco"
#define VERSION "0.2"

//--
// Default AP SSID & PWD
// Then is saved in nvs flash
char AP_SSID[]  = "STARTING_SSID";
char AP_PWD[]   = "STARTING_PWD";//"JWm4k8Ko6W24G87Z24uv";

//--
// Force extend from this STA
// If STA_SSID & STA_PWD are empty then only AP is started. You can configure STA from AP panel.
char STA_SSID[]  = "MOVISTAR_8CE2";
char STA_PWD[]   = "helloworld";//"JWm4k8Ko6W24G87Z24uv";
char STA_MAC[]   = ""; // connect to this/specific mac

//--
//
int DHT_gpio    = 25; // jet just for testing
// Start Light On GPIO 26 (TIMER) :)
int LED_gpio    = 26;
