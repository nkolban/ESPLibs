// Open a socket for listening ... when data arrives, callback a callback function

#include "espmissingincludes.h"
#include "getdata.h"

static void (*g_callback)(char *data, uint16 length);
static void recvCB(void *arg, char *pData, unsigned short len) {
  g_callback(pData, len);
}
void getData(struct espconn *pEspconn, void (*callback)(char *data, uint16 length)) {
	g_callback = callback;
	espconn_regist_recvcb(pEspconn, recvCB);
}
