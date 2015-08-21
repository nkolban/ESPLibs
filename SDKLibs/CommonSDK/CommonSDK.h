#ifndef INCLUDE_COMMON_H_
#define INCLUDE_COMMON_H_
#include <user_interface.h>
#include <c_types.h>
#include <ip_addr.h>
#include <espconn.h>
const char *authModeToString(AUTH_MODE mode);
void checkError(sint8 err);
void delayMilliseconds(uint32 milliseconds);
void dumpBSSINFO(struct bss_info *bssInfo);
void dumpEspConn(struct espconn *pEspConn);
void dumpRestart();
void dumpState();
const char *errorToString(sint8 err);
void eventLogger(System_Event_t *event);
const char *eventReasonToString(int reason);
const char *flashSizeAndMapToString(enum flash_size_map sizeMap);
const char *opModeToString(int opMode);
void setAsGpio(uint8 pin);
void setupBlink(uint8 blinkPin);
uint8 *toHex(uint8 *ptr, int size, uint8 *buffer);
#endif /* INCLUDE_COMMON_H_ */
