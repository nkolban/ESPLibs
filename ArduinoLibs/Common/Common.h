#ifndef Common_h
#define Common_h

#include <Arduino.h>
#include <ESP8266WiFi.h>

class Common {
public:
	/**
	 * Return a string representation of the IPAddress passed in address.  The format
	 * of the string is "xxx.xxx.xxx.xxx".
	 */
	static String ipAddressToString(IPAddress address);
};
#endif
