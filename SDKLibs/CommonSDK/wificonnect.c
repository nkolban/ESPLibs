/*
 * wificonnect.c
 *
 *  Created on: Nov 16, 2015
 *      Author: kolban
 */

#include <osapi.h>
#include "user_interface.h"
#include "espmissingincludes.h"
#include "wificonnect.h"

static void (*g_callback)();

static void eventHandler(System_Event_t *event) {
  switch(event->event) {
    case EVENT_STAMODE_CONNECTED:
      os_printf("Event: EVENT_STAMODE_CONNECTED\n");
      break;
    case EVENT_STAMODE_DISCONNECTED:
      os_printf("Event: EVENT_STAMODE_DISCONNECTED\n");
      break;
    case EVENT_STAMODE_AUTHMODE_CHANGE:
      os_printf("Event: EVENT_STAMODE_AUTHMODE_CHANGE\n");
      break;
    case EVENT_STAMODE_GOT_IP:
      os_printf("Event: EVENT_STAMODE_GOT_IP\n");
      if (g_callback != NULL) {
        g_callback();
      }
      break;
    case EVENT_SOFTAPMODE_STACONNECTED:
      os_printf("Event: EVENT_SOFTAPMODE_STACONNECTED\n");
      break;
    case EVENT_SOFTAPMODE_STADISCONNECTED:
      os_printf("Event: EVENT_SOFTAPMODE_STADISCONNECTED\n");
      break;
    default:
      os_printf("Unexpected event: %d\n", event->event);
    break;
  }
}

void wifiConnect(char *ssid, char *password, void (*callback)()) {
  /*
  assert(ssid != NULL);
  assert(password != NULL);
  assert(callback != NULL);
  */

  g_callback = callback;
  wifi_set_opmode_current(STATION_MODE);
  struct station_config stationConfig;
  os_strncpy(stationConfig.ssid, ssid, 32);
  os_strncpy(stationConfig.password, password, 64);
  wifi_station_set_config(&stationConfig);
  wifi_set_event_handler_cb(eventHandler);
  wifi_station_connect();
}
