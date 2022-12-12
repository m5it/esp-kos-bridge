#include "argtable3/argtable3.h"
#include "ping/ping_sock.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_console.h"
//
esp_err_t register_reset(void);
esp_err_t register_test(void);
esp_err_t register_free(void);
esp_err_t register_list(void);
esp_err_t register_dht(void);  // DHT sensor, humidity & temperature
