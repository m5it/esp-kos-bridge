// Default AP SSID & PWD
// Then is saved in nvs flash
char AP_SSID[]  = "STARTING_SSID";
char AP_PWD[]   = "STARTING_PWD";//"JWm4k8Ko6W24G87Z24uv";

// Force extend from this STA
// If STA_SSID & STA_PWD are empty then only AP is started. You can configure STA from AP panel.
char STA_SSID[]  = "MOVISTAR_8CE2";
char STA_PWD[]   = "helloworld";//"JWm4k8Ko6W24G87Z24uv";
char STA_MAC[]   = ""; // connect to this/specific mac

// Start Light On GPIO 26 (TIMER) :)
int DHT_gpio    = 26;
