#include <Arduino.h>
#include "Debug.h"

extern "C" {
	#include "user_interface.h"
}

void Debug::lastException() {
	struct rst_info *pRstInfo;
	pRstInfo = system_get_rst_info();
	Serial1.println("reason=" + String(pRstInfo->reason) + ", exccause=0x" + String(pRstInfo->exccause, HEX) + ", epc1=0x" + String(pRstInfo->epc1, HEX) + ", epc2=0x" + String(pRstInfo->epc2,HEX) + ", epc3=0x" + String(pRstInfo->epc3, HEX) + ", excvaddr=" + String(pRstInfo->excvaddr, HEX) + ", depc=0x" + String(pRstInfo->depc, HEX));
}
