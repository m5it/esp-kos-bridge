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
#include "esp_wifi.h"
#include "t3ch_console.h"
#include "t3ch_events.h"
#include "esp_console.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_mac.h"
#include "argtable3/argtable3.h"
#include "ping/ping_sock.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "esp_heap_caps.h"
#include "esp32/himem.h"
#include "dht.h"
//
#include "nvs.h"
#include "nvs_flash.h"
#include "cJSON.h" // https://github.com/DaveGamble/cJSON
//
#include "driver/gpio.h"
#include "driver/ledc.h"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (26) // Define the output GPIO
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (4095) // Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095
#define LEDC_FREQUENCY          (5000) // Frequency in Hertz. Set frequency at 5 kHz

//--
static const char *TAG = "T3CH_CONSOLE";

//--
// nvs variables used in "test" command
static nvs_handle_t nvsh;
static esp_err_t nvse;

//--
// nvs variables used in "ap" command
nvs_handle_t ap_nvsh;
esp_err_t ap_nvse;

//-- free Command aka space, memory
//
static int do_cmd_free(int argc, char **argv) {
	printf("do_cmd_free() started.");
	
	//
	//size_t memcnt=esp_himem_get_phys_size();
	//size_t memfree=esp_himem_get_free_size();
	//
	//printf("do_cmd_free() Himem has %dKiB of memory\n", (int)memcnt/1024);
	//printf("do_cmd_free() Himem has %d\n", (int)memfree/1024);
	printf("do_cmd_free() Heap: %d\n",heap_caps_get_free_size(MALLOC_CAP_8BIT));
	printf("t3ch_events_count_sta_connect: ");
	t3ch_events_get_count_sta_connect();
	printf("do_cmd_free() ended.");
	return 0;
}
// register space
esp_err_t register_free(void) {
    //
	esp_console_cmd_t command = {
        .command = "free",
        .help = "Preview used space & memory",
        .hint = NULL,
        .func = &do_cmd_free,
    };
    return esp_console_cmd_register(&command);
}

//-- RESET Command
//
static int do_cmd_reset(int argc, char **argv) {
	printf("do_cmd_reset() started.");
	
	//-- undefined reference to `esp_mesh_lite_erase_rtc_store'
    //esp_mesh_lite_erase_rtc_store();
    
    //-- lets add option like "hard reset" when erasing flash memory
    //nvs_flash_erase();
    
    //--
    esp_restart();
	printf("do_cmd_reset() ended.");
	return 0;
}
// register reset
esp_err_t register_reset(void) {
    //
	esp_console_cmd_t command = {
        .command = "reset",
        .help = "Reset device to KOS factory settings.",
        .hint = NULL,
        .func = &do_cmd_reset,
        //.argtable = &list_args
    };
    return esp_console_cmd_register(&command);
}


//-- LIST Command
//
static int do_cmd_list(int argc, char **argv) {
	printf("do_cmd_list() started.");
	size_t n = esp_netif_get_nr_of_ifs();
	printf("do_cmd_list() num ifs: %i\n",n);
	esp_netif_t *tmpnif = NULL;
	//
	printf("Number of Interfaces %i\n",n);
	//
	for(int i=0; i<n; i++) {
		tmpnif = esp_netif_next( tmpnif );
		//
		//const char nif_desc = esp_netif_get_desc( tmp );
		// const char *esp_netif_get_ifkey(esp_netif_t *esp_netif)
		int nif_index = esp_netif_get_netif_impl_index(tmpnif);
		bool nif_isup = esp_netif_is_netif_up(tmpnif);
		// esp_netif_get_hostname(esp_netif_t *esp_netif, const char **hostname)
		// int esp_netif_get_route_prio(esp_netif_t *esp_netif)
		uint8_t nif_mac[6];
		ESP_ERROR_CHECK(esp_netif_get_mac(tmpnif, nif_mac));
		char nif_name[6];
		ESP_ERROR_CHECK(esp_netif_get_netif_impl_name(tmpnif,nif_name));
		// get ip, netmask, gw
		esp_netif_ip_info_t nif_info;
		esp_netif_get_ip_info(tmpnif,&nif_info);
		//
		printf("%i.)[ %s ] index: %i - %s - "MACSTR" - "IPSTR"\n",i,(nif_isup?"UP":"DOWN"),nif_index,nif_name,MAC2STR(nif_mac),IP2STR(&nif_info.ip));
	}
	printf("do_cmd_list() ended.");
	return 0;
}
// List interfaces ( esp_netif )
esp_err_t register_list(void) {
    //
	esp_console_cmd_t command = {
        .command = "list",
        .help = "View list of interfaces",
        .hint = NULL,
        .func = &do_cmd_list,
        //.argtable = &list_args
    };
    return esp_console_cmd_register(&command);
}

//--
// DHT arguments
static struct {
    struct arg_int *set_gpio; // define gpio pin if not defined in t3ch_config.h
    struct arg_lit *view; //
    struct arg_end *end;
} dht_args;
//
static int do_cmd_dht(int argc, char **argv) {
	int nerrors = arg_parse(argc, argv, (void **)&dht_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, dht_args.end, argv[0]);
        return 1;
    }
    if (dht_args.set_gpio->count > 0) {
        int tmp = (uint32_t)(dht_args.set_gpio->ival[0]);
        //setDHTgpio( tmp );
    }
    if (dht_args.view->count > 0) {
        //ESP_LOGI(TAG,"do_cmd_dht() getDHTgpio(): %i",getDHTgpio());
        float temp, humi;
        dht_read_float_data(DHT_TYPE_DHT11,26,&humi,&temp);
        //dht_read_float_data(DHT_TYPE_AM2301,26,&humi,&temp);
        //dht_read_float_data(DHT_TYPE_SI7021,26,&humi,&temp);
		//ESP_LOGI(TAG,"do_cmd_dht() Humidity: %.1f, Temperature: %.1f", humi, temp);
		printf("do_cmd_dht() Humidity: %.1f, Temperature: %.1f\n", humi, temp);
    }
	return 0;
}
//
esp_err_t register_dht(void) {
	//
	dht_args.set_gpio  = arg_int0("s", "set_gpio","<n>","Set gpio pin if not set");
	dht_args.view      = arg_lit0("v","view","View DHT data");
	dht_args.end       = arg_end(1);
	//
	esp_console_cmd_t command = {
        .command = "dht",
        .help = "DHT Sensor",
        .hint = NULL,
        .func = &do_cmd_dht,
        .argtable = &dht_args
    };
    return esp_console_cmd_register(&command);
}

//--
// AP arguments
static struct {
    struct arg_str *set_ssid; //
    struct arg_str *set_password; //
    //struct arg_lit *reset;
    struct arg_end *end;
} ap_args;
//
static int do_cmd_ap(int argc, char **argv) {
	int nerrors = arg_parse(argc, argv, (void **)&ap_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, ap_args.end, argv[0]);
        return 1;
    }
    // set_ssid
    if (ap_args.set_ssid->count > 0) {
        char *tmp = ap_args.set_ssid->sval[0];
        //
        esp_err_t tmperr = nvs_set_str(ap_nvsh,"ssid",tmp);
		if(tmperr!=ESP_OK) printf("nvsWrite set_ssid Failed!\n");
		else printf("nvsWrite set_ssid Success!\n");
		//
		tmperr = nvs_commit( ap_nvsh );
		if(tmperr!=ESP_OK) printf("nvsWrite set_ssid commit Failed!\n");
		else printf("nvsWrite set_ssid commit Success!\n");
    }
    // set_password
    if (ap_args.set_password->count > 0) {
        char *tmp = ap_args.set_password->sval[0];
        //
        esp_err_t tmperr = nvs_set_str(ap_nvsh,"pwd",tmp);
		if(tmperr!=ESP_OK) printf("nvsWrite set_password Failed!\n");
		else printf("nvsWrite set_password Success!\n");
		//
		tmperr = nvs_commit( ap_nvsh );
		if(tmperr!=ESP_OK) printf("nvsWrite set_password commit Failed!\n");
		else printf("nvsWrite set_password commit Success!\n");
    }
	return 0;
}
//
esp_err_t register_ap(void) {
	//
	ap_args.set_ssid     = arg_str1("s", "set_ssid","<n>","Set gpio pin if not set");
	ap_args.set_password = arg_str1("p", "set_password","<n>","Set gpio pin if not set");
	ap_args.end          = arg_end(0);
	//
	esp_console_cmd_t command = {
        .command = "ap",
        .help = "AP Router",
        .hint = NULL,
        .func = &do_cmd_ap,
        .argtable = &ap_args
    };
    // open nvs storage and retrive handle
    ap_nvse = nvs_open("ap_storage",NVS_READWRITE,&ap_nvsh);
	if(nvse!=ESP_OK) {
		printf("nvsOpen failed!\n");
	}
	else {
		printf("nvsOpen success!\n");
	}
	
	// register command
    return esp_console_cmd_register(&command);
}

//--
// STA arguments
static struct {
    struct arg_lit *isUp; //
    struct arg_lit *up;
    struct arg_lit *scan;
    struct arg_lit *scanView;
    struct arg_end *end;
} sta_args;
//
static int do_cmd_sta(int argc, char **argv) {
	int nerrors = arg_parse(argc, argv, (void **)&sta_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, sta_args.end, argv[0]);
        return 1;
    }
    //
    if (sta_args.isUp->count > 0) {
        printf("DEBUG STA isUp() STARTING.\n");
		if( t3ch_wifi_sta_isup() ) {
			printf("DEBUG STA IS UP.\n");
		}
		else {
			printf("DEBUG STA NOT UP!\n");
		}
    }
    //
    if (sta_args.up->count>0 ) {
		printf("DEBUG STA up() STARTING.\n");
	    if( t3ch_wifi_sta_up() ) {
			printf("DEBUG STA configured succesfully.\n");
		}
		else {
			printf("DEBUG Failed configuring STA!\n");
		}
	}
	//
	if (sta_args.scan->count>0 ) {
	    t3ch_wifi_scan_start();
	}
	//
	if (sta_args.scanView->count>0 ) {
		int size = t3ch_wifi_scan_gen();
		printf("t3ch_console.c => do_cmd_sta() scanView size: %d\n",size);
		char tmp[size+1];
		memset(tmp,'\0',size+1);
		t3ch_wifi_scan_get(tmp);
	    printf("t3ch_console.c => do_cmd_sta() scanView: %s\n",tmp);
	}
	return 0;
}
//
esp_err_t register_sta(void) {
	//
	sta_args.isUp      = arg_lit0("u", "isUp","Check if STA interface is up.");
	sta_args.up        = arg_lit0("U", "up","Put STA interface up.");
	sta_args.scan      = arg_lit0("s", "scan","Scan for wifi.");
	sta_args.scanView  = arg_lit0("S", "scanView","Preview scan results for wifi.");
	sta_args.end       = arg_end(0);
	//
	esp_console_cmd_t command = {
        .command = "sta",
        .help = "STA client",
        .hint = NULL,
        .func = &do_cmd_sta,
        .argtable = &sta_args
    };
	// register command
    return esp_console_cmd_register(&command);
}


//--
// test arguments
static struct {
	//
	struct arg_int *testGpio;
	struct arg_lit *eraseTimer;
	//
	struct arg_str *nvsOpen;
	struct arg_str *nvsRead;
	struct arg_str *nvsWrite;
	//
	struct arg_int *testGetLog;
	struct arg_str *testOtaUpdate;
	struct arg_lit *testOtaPartition;
	struct arg_str *testB64Decode;
	struct arg_lit *testScan;
	struct arg_lit *testTime;
	struct arg_lit *testLedc;
	struct arg_lit *pauseLedc;
    struct arg_int *testInt;
    struct arg_dbl *testDouble;
    struct arg_str *testString;
    struct arg_end *end;
} test_args;

//
static void example_ledc_init(void)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 5 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

//
static int do_cmd_test(int argc, char **argv) {
	ESP_LOGI(TAG,"do_cmd_test() START Hello world by test!!! :)\n");
	
	int nerrors = arg_parse(argc, argv, (void **)&test_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, test_args.end, argv[0]);
        return 1;
    }
    
    //--
    //
    if( test_args.testGpio->count>0 ) {
		int tmp = (uint32_t)(test_args.testGpio->ival[0]);
		printf("testGpio tmp %i\n",tmp);
		gpio_set_level(GPIO_NUM_26,tmp);
	}
	//
	if( test_args.eraseTimer->count>0 ) {
		nvs_handle_t nvsh;
		esp_err_t err = nvs_open("timer_storage",NVS_READWRITE,&nvsh);
		nvs_erase_key(nvsh,"json");
		nvs_close(nvsh);
	}
    //--
    //
    if( test_args.nvsOpen->count>0 ) {
		char *tmp = test_args.nvsOpen->sval[0];
		printf("nvsOpen starting, tmp: %s\n",tmp);
		nvse = nvs_open("storage",NVS_READWRITE,&nvsh);
		if(nvse!=ESP_OK) {
			printf("nvsOpen failed!\n");
		}
		else {
			printf("nvsOpen success!\n");
		}
	}
	//
    if( test_args.nvsRead->count>0 ) {
		char *tmp = test_args.nvsRead->sval[0];
		printf("nvsRead starting, tmp: %s\n",tmp);
		char nvsout[256];
		size_t rs;
		nvs_get_str(nvsh,"testnvs",NULL,&rs);
		esp_err_t tmperr = nvs_get_str(nvsh,"testnvs",nvsout,&rs);
		
		if(tmperr!=ESP_OK) printf("nvsRead Failed!\n");
		else printf("nvsRead Success, data: %s\n",nvsout);
		//printf("nvsRead data: %s\n",nvsout);
	}
	//
    if( test_args.nvsWrite->count>0 ) {
		char *tmp = test_args.nvsWrite->sval[0];
		printf("nvsWrite starting, tmp: %s\n",tmp);
		esp_err_t tmperr = nvs_set_str(nvsh,"testnvs",tmp);
		if(tmperr!=ESP_OK) printf("nvsWrite Failed!\n");
		else printf("nvsWrite Success!\n");
		tmperr = nvs_commit( nvsh );
		if(tmperr!=ESP_OK) printf("nvsWrite commit Failed!\n");
		else printf("nvsWrite commit Success!\n");
	}
    //--
    //
    if (test_args.testGetLog->count > 0) {
		int fromPos = (uint32_t)(test_args.testGetLog->ival[0]);
		ESP_LOGI(TAG,"test get log starting, fromPos: %i\n",fromPos);
		
		/*int size = t3ch_log_gen_old(fromPos);
		printf("test get log size: %i\n",size);
		char out[size];
		t3ch_log_get(out);
		printf("test get log: %s\n",out);
		cJSON *ary = cJSON_Parse( out );
		size = cJSON_GetArraySize( ary );
		printf("test get log json size: %i\n",size);
		free(ary);*/
		t3ch_log_test();
	}
    //
    if (test_args.testOtaPartition->count > 0) {
		ESP_LOGI(TAG,"test ota partition starting.\n");
		const esp_partition_t *partition = esp_web_get_ota_update_partition();
		printf("test ota partition size: %d\n",partition->size);
	}
	//
    if (test_args.testOtaUpdate->count > 0) {
		char *url = test_args.testOtaUpdate->sval[0];
		printf("test ota update starting, from url: %s\n",url);
		// Check if url starts with http
		if( match("^http\:\/\/.*",url)==0 ) {
			printf("test ota update Failed. Url should start with http://...\n");
			return 0;
		}
		
		if( t3ch_ota_download(url) ) {
			//esp_restart();
		}
	}
    //
    if (test_args.testB64Decode->count > 0) {
		char *tmp = test_args.testB64Decode->sval[0];
		ESP_LOGI(TAG,"B64Decode STARTED, trying to decode(%d): %s\n",strlen(tmp),tmp);
		tmp = b64decode( tmp );
		ESP_LOGI(TAG,"B64Decode DECODED(%d): %s\n",strlen(tmp),tmp);
		free(tmp);
	}
	//
    if (test_args.testScan->count > 0) {
		ESP_LOGI(TAG,"test scan starting.\n");
		StartScan();
	}
    //--
    if (test_args.testLedc->count > 0) {
		ESP_LOGI(TAG,"test ledc starting.");
	    example_ledc_init();
	    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY));
	    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
	}
	if (test_args.pauseLedc->count > 0) {
		ESP_LOGI(TAG,"pause ledc starting.");
	    //ESP_ERROR_CHECK(ledc_timer_pause(LEDC_MODE, LEDC_TIMER));
	    ESP_ERROR_CHECK(ledc_stop(LEDC_MODE, LEDC_CHANNEL, 0));
	    //ESP_ERROR_CHECK(gpio_reset_pin( LEDC_OUTPUT_IO ));
	}
    if (test_args.testTime->count > 0) {
		ESP_LOGI(TAG,"test time starting...");
		/*time_t now;
		struct tm timeinfo;
		char strftime_buf[64];
		time(&now);
	    localtime_r(&now, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);*/
        //char *strftime_buf = get_time_str();
        char strftime_buf[64];
        t3ch_time_get(&strftime_buf);
	    ESP_LOGI(TAG, "The current date/time is: %s", strftime_buf);
    }
    //
    if (test_args.testInt->count > 0) {
        int tmp = (uint32_t)(test_args.testInt->ival[0]);
        ESP_LOGI(TAG,"do_cmd_test() debug argument testInt: %i\n", tmp);
    }
    if (test_args.testDouble->count > 0) {
        double tmp = test_args.testDouble->dval[0];
        printf("do_cmd_test() debug argument testInt: %f\n",tmp);
    }
    if (test_args.testString->count > 0) {
        char *tmp = test_args.testString->sval[0];
        printf("do_cmd_test() debug argument testString: %s\n",tmp);
    }
    
    ESP_LOGI(TAG,"do_cmd_test() END.");
    
	return 0;
}
//
esp_err_t register_test(void) {
	//
	test_args.testGpio    = arg_int0("t","testGpio","<i>","Test gpio");
	test_args.eraseTimer  = arg_lit0("e","eraseTimer","Erase timer");
	// nvs options
	test_args.nvsOpen    = arg_str1("o","nvsOpen","<s>","Open nvs");
	test_args.nvsRead    = arg_str1("r","nvsRead","<s>","Read from nvs");
	test_args.nvsWrite   = arg_str1("w","nvsWrite","<s>","Write to nvs");
	// default options
	test_args.testGetLog       = arg_int0("l", "testGetLog","<i>", "Test log...");
	test_args.testOtaPartition = arg_lit0("p", "testOtaPartition", "Test ota partition size..");
	test_args.testOtaUpdate    = arg_str1("u", "testOtaUpdate", "<s>", "Test ota update");
	test_args.testB64Decode    = arg_str1("D", "testB64Decode","<s>", "Test base64 decode.");
	test_args.testScan   = arg_lit0("S", "testScan", "Test wifi scan");
	test_args.testTime   = arg_lit0("T", "testTime", "Test time");
	test_args.testLedc   = arg_lit0("L", "testLedc", "Test ledc");
	test_args.pauseLedc  = arg_lit0("P", "pauseLedc", "Pause ledc");
	test_args.testInt    = arg_int0("i", "testInteger", "<n>", "Test integer argument");
	test_args.testDouble = arg_dbl0("d", "testDouble", "<n>", "Test double argument");
	test_args.testString = arg_str1("s", "testString", "<s>", "Test string argument");
	test_args.end     = arg_end(0);
	//
	gpio_set_direction(GPIO_NUM_26, GPIO_MODE_OUTPUT);
	//
	esp_console_cmd_t command = {
        .command = "test",
        .help = "This is test command",
        .hint = NULL,
        .func = &do_cmd_test,
        .argtable = &test_args
    };
    return esp_console_cmd_register(&command);
}

