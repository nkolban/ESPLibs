/*
 * CommonSDK.c
 *
 *  Created on: Jul 6, 2015
 *      Author: kolban
 */
#include <Arduino.h>
#include "ESP_Log.h"

extern "C" {
#include <c_types.h>
#include <ip_addr.h>
#include <espconn.h>
#include <os_type.h>
#include <osapi.h>
#include <gpio.h>
#include <ip_addr.h>
#include "CommonSDK.h"

void ets_delay_us(unsigned int del);

#define DELAY 1000
LOCAL os_timer_t blink_timer;
LOCAL uint8_t led_state=0;
LOCAL int _blinkPin;
LOCAL int pinMapping[] = {
	PERIPHS_IO_MUX_GPIO0_U, 	// GPIO0
	PERIPHS_IO_MUX_U0TXD_U, 	// GPIO1
	PERIPHS_IO_MUX_GPIO2_U,		  // GPIO2
	PERIPHS_IO_MUX_U0RXD_U,		  // GPIO3
	PERIPHS_IO_MUX_GPIO4_U,		  // GPIO4
	PERIPHS_IO_MUX_GPIO5_U,		  // GPIO5
	PERIPHS_IO_MUX_SD_CLK_U,	  // GPIO6
	PERIPHS_IO_MUX_SD_DATA0_U, // GPIO7
	PERIPHS_IO_MUX_SD_DATA1_U, // GPIO8
	PERIPHS_IO_MUX_SD_DATA2_U, // GPIO9
	PERIPHS_IO_MUX_SD_DATA3_U, // GPIO10
	PERIPHS_IO_MUX_SD_CMD_U,   // GPIO11
	PERIPHS_IO_MUX_MTDI_U,     // GPIO12
	PERIPHS_IO_MUX_MTCK_U,		 // GPIO13
	PERIPHS_IO_MUX_MTMS_U,		 // GPIO14
	PERIPHS_IO_MUX_MTDO_U      // GPIO15
};

LOCAL int funcMapping[] = {
	FUNC_GPIO0,
	FUNC_GPIO1,
	FUNC_GPIO2,
	FUNC_GPIO3,
	FUNC_GPIO4,
	FUNC_GPIO5,
	-1,
	-1,
	-1,
	FUNC_GPIO9,
	FUNC_GPIO10,
	-1,
	FUNC_GPIO12,
	FUNC_GPIO13,
	FUNC_GPIO14,
	FUNC_GPIO15
};

void setAsGpio(uint8 pin) {
	if (pin < 0 || pin > 15) {
		os_printf("bad pin: %d\n", pin);
		return;
	}
	PIN_FUNC_SELECT(pinMapping[pin], funcMapping[pin]);
} // End of setAsGpio

LOCAL void ICACHE_FLASH_ATTR blink_cb(void *arg)
{
	GPIO_OUTPUT_SET(_blinkPin, led_state);
	led_state = !led_state;
}

const char *stringFromIP(uint8_t ip[4]) {
	static char str[20];
	sprintf(str, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	return str;
}

const char *eventReasonToString(int reason) {
	switch(reason) {
	case REASON_UNSPECIFIED:
		return "REASON_UNSPECIFIED";
	case REASON_AUTH_EXPIRE:
		return "REASON_AUTH_EXPIRE";
	case REASON_AUTH_LEAVE:
		return "REASON_AUTH_LEAVE";
	case REASON_ASSOC_EXPIRE:
		return "REASON_ASSOC_EXPIRE";
	case REASON_ASSOC_TOOMANY:
		return "REASON_ASSOC_TOOMANY";
	case REASON_NOT_AUTHED:
		return "REASON_NOT_AUTHED";
	case REASON_NOT_ASSOCED:
		return "REASON_NOT_ASSOCED";
	case REASON_ASSOC_LEAVE:
		return "REASON_ASSOC_LEAVE";
	case REASON_ASSOC_NOT_AUTHED:
		return "REASON_ASSOC_NOT_AUTHED";
	case REASON_DISASSOC_PWRCAP_BAD:
		return "REASON_DISASSOC_PWRCAP_BAD";
	case REASON_DISASSOC_SUPCHAN_BAD:
		return "REASON_DISASSOC_SUPCHAN_BAD";
	case REASON_IE_INVALID:
		return "REASON_IE_INVALID";
	case REASON_MIC_FAILURE:
		return "REASON_MIC_FAILURE";
	case REASON_4WAY_HANDSHAKE_TIMEOUT:
		return "REASON_4WAY_HANDSHAKE_TIMEOUT";
	case REASON_GROUP_KEY_UPDATE_TIMEOUT:
		return "REASON_GROUP_KEY_UPDATE_TIMEOUT";
	case REASON_IE_IN_4WAY_DIFFERS:
		return "REASON_IE_IN_4WAY_DIFFERS";
	case REASON_GROUP_CIPHER_INVALID:
		return "REASON_GROUP_CIPHER_INVALID";
	case REASON_PAIRWISE_CIPHER_INVALID:
		return "REASON_PAIRWISE_CIPHER_INVALID";
	case REASON_AKMP_INVALID:
		return "REASON_AKMP_INVALID";
	case REASON_UNSUPP_RSN_IE_VERSION:
		return "REASON_UNSUPP_RSN_IE_VERSION";
	case REASON_INVALID_RSN_IE_CAP:
		return "REASON_INVALID_RSN_IE_CAP";
	case REASON_802_1X_AUTH_FAILED:
		return "REASON_802_1X_AUTH_FAILED";
	case REASON_CIPHER_SUITE_REJECTED:
		return "REASON_CIPHER_SUITE_REJECTED";
	case REASON_BEACON_TIMEOUT:
		return "REASON_BEACON_TIMEOUT";
	case REASON_NO_AP_FOUND:
		return "REASON_NO_AP_FOUND";
	default:
		return "*** Unknown reason ***";
	}
}

void eventLogger(System_Event_t *event) {
	switch(event->event) {
	case EVENT_STAMODE_CONNECTED:
		os_printf("Event: EVENT_STAMODE_CONNECTED\n");
		break;
	case EVENT_STAMODE_DISCONNECTED:
		os_printf("Event: EVENT_STAMODE_DISCONNECTED - reason: %d (%s)\n", event->event_info.disconnected.reason, eventReasonToString(event->event_info.disconnected.reason));
		break;
	case EVENT_STAMODE_AUTHMODE_CHANGE:
		os_printf("Event: EVENT_STAMODE_AUTHMODE_CHANGE\n");
		break;
	case EVENT_STAMODE_GOT_IP:
		os_printf("Event: EVENT_STAMODE_GOT_IP\n");
		break;
	case EVENT_SOFTAPMODE_STACONNECTED:
		os_printf("Event: EVENT_SOFTAPMODE_STACONNECTED - channel: %d\n", event->event_info.connected.channel);
		break;
	case EVENT_SOFTAPMODE_STADISCONNECTED:
		os_printf("Event: EVENT_SOFTAPMODE_STADISCONNECTED\n");
		break;
	default:
		os_printf("Unexpected event: %d\n", event->event);
		break;
	}
}

void setupBlink(uint8 blinkPin) {
	_blinkPin = blinkPin;
	setAsGpio(blinkPin);
	os_timer_disarm(&blink_timer);
	os_timer_setfn(&blink_timer, (os_timer_func_t *)blink_cb, (void *)0);
	os_timer_arm(&blink_timer, DELAY, 1);
} // End of setupBlink

void dumpRestart() {
	struct rst_info *rstInfo = system_get_rst_info();
	os_printf("Restart info:\n");
	os_printf("  reason: %d\n", rstInfo->reason);
	os_printf("  exccause: %x\n", rstInfo->exccause);
	os_printf("  epc1: %x\n", rstInfo->epc1);
	os_printf("  epc2: %x\n", rstInfo->epc2);
	os_printf("  epc3: %x\n", rstInfo->epc3);
	os_printf("  excvaddr: %x\n", rstInfo->excvaddr);
	os_printf("  depc: %x\n", rstInfo->depc);
} // End of dump_restart

const char *flashSizeAndMapToString(enum flash_size_map sizeMap) {
	switch(sizeMap) {
	case FLASH_SIZE_4M_MAP_256_256:
		return "Size: 4M, Map: 256 256";
	case FLASH_SIZE_2M:
		return "Size: 2M";
	case FLASH_SIZE_8M_MAP_512_512:
		return "Size: 8M, Map: 512 512";
	case FLASH_SIZE_16M_MAP_512_512:
		return "Size: 16M, Map: 512 512";
	case FLASH_SIZE_32M_MAP_512_512:
		return "Size: 32M, Map: 512 512";
	case FLASH_SIZE_16M_MAP_1024_1024:
		return "Size: 16M, Map: 1024 1024";
	case FLASH_SIZE_32M_MAP_1024_1024:
		return "Size: 32M, Map: 1024 1024";
	}
	return "Unknown size and map";
} // End of flashSizeAndMapToString

const char *opModeToString(int opMode) {
	switch(opMode) {
	case STATION_MODE:
		return "STATION_MODE";
	case SOFTAP_MODE:
		return "SOFTAP_MODE";
	case STATIONAP_MODE:
		return "STATIONAP_MODE";
	default:
		return "*** Unknown Mode ***";
	}
} // End of opModeToString

void dumpState() {
	uint8 opMode = wifi_get_opmode();
	os_printf("Op Mode: %s\n", opModeToString(opMode));
	struct station_config stationConfig;
	wifi_station_get_config(&stationConfig);
	os_printf("Current Station Config\r\n");
	os_printf("SSID: %s, password: %s\r\n",
			stationConfig.ssid,
			stationConfig.password);
	struct ip_info info;
	wifi_get_ip_info(SOFTAP_IF, &info);
	//os_printf("AP:  ip: " IPSTR ", gw: " IPSTR "\n", IP2STR(&info.ip), IP2STR(&info.gw));
	wifi_get_ip_info(STATION_IF, &info);
	//os_printf("STA: ip: " IPSTR ", gw: " IPSTR "\n", IP2STR(&info.ip), IP2STR(&info.gw));
	uint8 macaddr[6];
	wifi_get_macaddr(SOFTAP_IF, macaddr);
	os_printf("AP MAC Addr:  " MACSTR "\n", MAC2STR(macaddr));
	wifi_get_macaddr(STATION_IF, macaddr);
	os_printf("STA MAC Addr: " MACSTR "\n", MAC2STR(macaddr));
	os_printf("Free heap size: %d bytes\n", system_get_free_heap_size());
	os_printf("Boot version: %d\n", system_get_boot_version());
	os_printf("Boot mode: %d\n", system_get_boot_mode());
	os_printf("Chip Id: %x\n", system_get_chip_id());
	os_printf("UserBin addr: %x\n", system_get_userbin_addr());
	os_printf("CPU frequency: %dMHz\n", system_get_cpu_freq());
	os_printf("Flash size map: %s\n", flashSizeAndMapToString(system_get_flash_size_map()));
	os_printf("SDK version: %s\n", system_get_sdk_version());
	system_print_meminfo();
} // End of dumpState

const char *authModeToString(AUTH_MODE mode) {
	switch (mode) {
	case AUTH_OPEN:
		return "OPEN";
	case AUTH_WEP:
		return "WEP";
	case AUTH_WPA_PSK:
		return "WPA PSK";
	case AUTH_WPA2_PSK:
		return "WPA2 PSK";
	case AUTH_WPA_WPA2_PSK:
		return "WPA/WPA2 PSK";
	case AUTH_MAX:
		break;
	}
	return "Unknown auth mode!!";
} // End of authModeToString

const char *errorToString(sint8 err) {
	switch(err) {
	case ESPCONN_MEM:
		return "ESPCONN_MEM";
	case ESPCONN_TIMEOUT:
		return "ESPCONN_TIMEOUT";
	case ESPCONN_RTE:
		return "ESPCONN_RTE";
	case ESPCONN_INPROGRESS:
		return "ESPCONN_INPROGRESS";
	case ESPCONN_ABRT:
		return "ESPCONN_ABRT";
	case ESPCONN_RST:
		return "ESPCONN_RST";
	case ESPCONN_CLSD:
		return "ESPCONN_CLSD";
	case ESPCONN_CONN:
		return "ESPCONN_CONN";
	case ESPCONN_ARG:
		return "ESPCONN_ARG";
	case ESPCONN_ISCONN:
		return "ESPCONN_ISCONN";
	case ESPCONN_HANDSHAKE:
		return "ESPCONN_HANDSHAKE";
	case ESPCONN_PROTO_MSG:
		return "ESPCONN_PROTO_MSG";
	}
	return "Unknown error";
} // End of errorToString

void scanCB(void *arg, STATUS status) {
	if (status == OK) {
		struct bss_info *bssInfo = (struct bss_info *) arg;
		bssInfo = STAILQ_NEXT(bssInfo, next);
		while (bssInfo != NULL) {
			dumpBSSINFO(bssInfo);
			bssInfo = STAILQ_NEXT(bssInfo, next);
		}
	} else {
		os_printf("Scan CB reported error: %d\n", status);
	}
} // End of scanCB

void dumpBSSINFO(struct bss_info *bssInfo) {
	uint8 bssidString[13];
	os_printf("ssid: %s, bssid: %s, rssi: %d, authMode: %s\n",
			bssInfo->ssid,
			toHex(bssInfo->bssid, 6, bssidString),
			bssInfo->rssi,
			authModeToString(bssInfo->authmode));
}

void dumpHex(uint8 *data, unsigned int length) {
	uint32_t i;
	for (i=0; i<length; i++) {
		os_printf("%x", data[i]);
	}
}

void dumpEspConn(struct espconn *pEspConn) {
	LOG("Dump of espconn: %x\n", pEspConn);
	if (pEspConn == NULL) {
		return;
	}
	switch(pEspConn->type) {
	case ESPCONN_TCP:
		LOG("type = TCP\n");
		LOG(" - local_port = %d\n", pEspConn->proto.tcp->local_port);
		LOG(" - local_ip = %s\n", stringFromIP(pEspConn->proto.tcp->local_ip));
		LOG(" - remote_port = %d\n", pEspConn->proto.tcp->remote_port);
		LOG(" - remote_ip = %s\n", stringFromIP(pEspConn->proto.tcp->remote_ip));
		break;
	case ESPCONN_UDP:
		os_printf("type = UDP\n");
		os_printf(" - local_port = %d\n", pEspConn->proto.udp->local_port);
		os_printf(" - local_ip = %s\n", stringFromIP(pEspConn->proto.udp->local_ip));
		os_printf(" - remote_port = %d\n", pEspConn->proto.udp->remote_port);
		os_printf(" - remote_ip = %s\n", stringFromIP(pEspConn->proto.udp->remote_ip));
		break;
	default:
		LOG("type = Unknown!! 0x%x\n", pEspConn->type);
	}
	switch(pEspConn->state) {
	case ESPCONN_NONE:
		LOG("state = NONE\n");
		break;
	case ESPCONN_WAIT:
		LOG("state = WAIT\n");
		break;
	case ESPCONN_LISTEN:
		LOG("state = LISTEN\n");
		break;
	case ESPCONN_CONNECT:
		LOG("state = CONNECT\n");
		break;
	case ESPCONN_WRITE:
		LOG("state = WRITE\n");
		break;
	case ESPCONN_READ:
		LOG("state = READ\n");
		break;
	case ESPCONN_CLOSE:
		LOG("state = CLOSE\n");
		break;
	default:
		LOG("state = unknown!!\n");
		break;
	}
	LOG("link_cnt = %d\n", pEspConn->link_cnt);
	LOG("reverse = %x\n", (unsigned int)pEspConn->reverse);
} // End of dumpEspConn

void delayMilliseconds(uint32 milliseconds) {
	while(milliseconds > 0) {
		os_delay_us(1000);
		milliseconds--;
	}
} // End of delayMilliseconds

void checkError(sint8 err) {
	if (err == ESPCONN_OK ) {
		return;
	}
	os_printf("Error: %s\n", errorToString(err));
} // End of checkError

LOCAL char nibbleToHex(int n) {
	if (n < 10) {
		return '0' + n;
	}
	return 'A' + n - 10;
} // End of nibbleToHex

/**
 * Format an array of bytes pointed to by 'ptr' that is 'size' bytes long.
 * Convert them to hex characters and store them in buffer.  Buffer will then
 * be NULL terminated.  Buffer should be at least 2 * size + 1 bytes in length.
 * Return a pointer to the start of the buffer.
 */
uint8 *toHex(uint8 *ptr, int size, uint8 *buffer) {
	uint8 *output = buffer;
	int i;
	for (i=0; i<size; i++) {
		*output = nibbleToHex((*ptr) >> 4);
		output++;
		*output = nibbleToHex((*ptr) & 0x0f);
		output++;
		ptr++;
	}
	*output = 0;
	return buffer;
}
}
// End of file
