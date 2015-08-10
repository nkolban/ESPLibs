#include <Common.h>

/**
 * Return a string representation of the IPAddress passed in address.  The format
 * of the string is "xxx.xxx.xxx.xxx".
 */
String Common::ipAddressToString(IPAddress address) {
	return String(address[0]) + "." + String(address[1]) + "." + String(address[2]) + "." + String(address[3]);
} // End of ipAddressToString
