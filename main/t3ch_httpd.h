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
#include "t3ch_config.h"
//#include "protocol_examples_common.h"
#include "esp_tls_crypto.h"
#ifdef ENABLE_HTTPS
#include <esp_https_server.h>
#else
#include <esp_http_server.h>
#endif
#include "esp_tls.h"
#include "esp_log.h"

//
#ifdef ENABLE_HTTPS
extern const unsigned char servercert_start[] asm("_binary_servercert_pem_start");
extern const unsigned char servercert_end[]   asm("_binary_servercert_pem_end");
extern const unsigned char prvtkey_pem_start[] asm("_binary_prvtkey_pem_start");
extern const unsigned char prvtkey_pem_end[]   asm("_binary_prvtkey_pem_end");
#endif
//
#ifdef ENABLE_WSS
extern const uint8_t HTTP_PANEL[] asm("_binary_wsspanel_html_start");
#else
extern const uint8_t HTTP_PANEL[] asm("_binary_defaultpanel_html_start");
#endif
