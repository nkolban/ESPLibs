#include <osapi.h>
#include "user_interface.h"

#include "espmissingincludes.h"
#include "startListening.h"

static void (*g_callback)(struct espconn *pEspconn);
static struct espconn conn1;
static esp_tcp tcp1;

static void connectCB(void *arg) {
  g_callback((struct espconn *)arg);
}

void startListening(uint16 port, void (*callback)(struct espconn *pEspconn)) {
  g_callback = callback;
  conn1.proto.tcp = &tcp1;
  conn1.type = ESPCONN_TCP;
  conn1.state = ESPCONN_NONE;
  tcp1.local_port = port;
  espconn_regist_connectcb(&conn1, connectCB);
  sint8 rc = espconn_accept(&conn1);
  if (rc != 0) {
    os_printf("Error from espconn_accept: %d\n", rc);
    return;
  }
}
