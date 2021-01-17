#include <espressif/esp_common.h>
#include <esp8266.h>
#include <esp/uart.h>
#include <string.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
#include <ssid_config.h>
#include <httpd/httpd.h>

const char *uart_cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]){
   for (int i = 0; i < iNumParams; i++) {
      if(strcmp(pcParam[i], "char") == 0) {  
         printf(pcValue[i]);
         printf("\n\r"); 
      } 
   }
   return "/index.ssi";
}

void httpd_task(void *pvParameters){
    tCGI pCGIs[] = {
        {"/uart", (tCGIHandler) uart_cgi_handler},  
    };
 
    http_set_cgi_handlers(pCGIs, sizeof (pCGIs) / sizeof (pCGIs[0])); 
    httpd_init();

    for (;;);
}

void user_init(void){
    uart_set_baud(0, 115200); 

    struct sdk_station_config config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS,
    };
 
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);
    sdk_wifi_station_connect();
 
    xTaskCreate(&httpd_task, "HTTP Daemon", 256, NULL, 2, NULL);
}
