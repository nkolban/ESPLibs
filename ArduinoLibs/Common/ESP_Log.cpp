/**
 * Logging function
 */

#include <stdarg.h>
#include <Arduino.h> //To allow function to run from any file in a project
#include "ESP_Log.h"

#define ARDBUFFER 16 //Buffer for storing intermediate strings. Performance may vary depending on size.

/**
 * Perform a "printf" to the Arduino Serial port.  The supported
 * printf flags are:
 * - %d - Print an integer
 * - %l - Print a long integer
 * - %c - Print a character
 * - %s - Print a string
 * - %x - Print an integer in hex
 * - %i - Print an IP address
 */
void ESPLog::ardprintf(const char *str, ...) {
	int i, j;
	char temp[ARDBUFFER + 1];

	va_list vargs;
	va_start(vargs, str);
	//Iterate over formatting string
	for (i = 0, j = 0; str[i] != '\0'; i++) {
		if (str[i] == '%') {
			//Clear buffer
			temp[j] = '\0';
			Serial1.print(temp);
			j = 0;
			temp[0] = '\0';

			//Process argument
			switch (str[++i]) {
			case 'd':
				Serial1.print(va_arg(vargs, int));
				break;
			case 'l':
				Serial1.print(va_arg(vargs, long));
				break;
			case 'f':
				Serial1.print(va_arg(vargs, double));
				break;
			case 'c':
				Serial1.print((char) va_arg(vargs, int));
				break;
			case 's':
				Serial1.print(va_arg(vargs, char *));
				break;
			case 'x':
				Serial1.print(va_arg(vargs, int), HEX);
				break;
			case 'i':
				char *p;
				p = va_arg(vargs, char *);
				Serial1.print(String((int)p[0]) + "." + String((int)p[1]) + "." + String((int)p[2]) + "." + String((int)p[3]));
				break;
			default:
				;
			};
		} else {
			//Add to buffer
			temp[j] = str[i];
			j = (j + 1) % ARDBUFFER;
			if (j == 0)  //If buffer is full, empty buffer.
					{
				temp[ARDBUFFER] = '\0';
				Serial1.print(temp);
				temp[0] = '\0';
			}
		}
	};
	// If there are any output characters not yet sent, send them now.
	if (j != 0) {
		temp[j] = '\0';
		Serial1.print(temp);
	}

	//Serial1.println(); //Print trailing newline
	va_end(vargs);
} // End of ardprintf


/**
 * Create and dump a hex array of the content of memory.
 */
void ESPLog::dumpHex(const char *buf, int size) {
	ESPLog::ardprintf("Dump hex of address: 0x%x for %d\n", buf, size);
	int diff = ((int)buf)% 16;
	int lineAddr = (int)buf - diff;

	ESPLog::ardprintf("%x ", (int)lineAddr);
	for (int i=0; i<diff; i++) {
		ESPLog::ardprintf("   ");
	}
	size += diff;
	buf = buf - diff;
	for (int i=diff; i<size; i++) {
		if (i>0 && i%16==0) {
			ESPLog::ardprintf("\n");
			lineAddr+=16;
			ESPLog::ardprintf("%x ", (int)lineAddr);
		}
		char c = buf[i];
		if (c<=0xf) {
			ESPLog::ardprintf("0");
		}
		ESPLog::ardprintf("%x ", (int)c);
	}
	ESPLog::ardprintf("\n");
} // End of dumpHex

/**
 * Create and dump a hex array of the content of memory.
 */
void ESPLog::dumpHex(const char *from, const char *to) {
	int size = to - from;
	if (size <= 0) {
		ESPLog::ardprintf("ESPLog::dumpHex: Error end  (0x%x) is < start (0x%x)\n", from, to);
		return;
	}
	ESPLog::dumpHex(from, size);
} // End of dumpHex
// End of file
