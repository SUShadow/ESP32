/*
 * HTTPModule.h
 *
 *  Created on: Dec 1, 2019
 *      Author: SU_Shadow
 */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#define WEB_PORT "80"
#define WEB_PATH "/"

#ifndef MAIN_MODULES_HTTPMODULE_H_
#define MAIN_MODULES_HTTPMODULE_H_
static void SendRequestTask(void *pvParameters);
#endif /* MAIN_MODULES_HTTPMODULE_H_ */
