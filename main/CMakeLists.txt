idf_component_register(SRCS "app_main.c" "t3ch_console.c" "dht.c" "t3ch_httpd.c" "t3ch_events.c" "t3ch_time.c" "t3ch_wifi.c" "t3ch_nvs.c" "t3ch_functions.c" "t3ch_gpio.c" "t3ch_ota.c" "t3ch_log.c" "cdecode.c" 
                       INCLUDE_DIRS "." 
                       EMBED_TXTFILES "defaultpanel.html" "wsspanel.html" "version.txt" "servercert.pem" "prvtkey.pem" 
                       )
