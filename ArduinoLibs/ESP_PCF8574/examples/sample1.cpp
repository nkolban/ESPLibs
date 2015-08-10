#include <Arduino.h>
#include <Ticker.h>
#include <ESP_PCF8574.h>
// SDA - Yellow - 4
// CLK - White - 5

#define SDA_PIN 4
#define CLK_PIN 5
#define PCF8574_ADDRESS (0x20)

Ticker ticker;
ESP_PCF8574 myPCF8574;
int counter = 0;
int dir = 1;

void timerCB() {
	//myPCF8574.setByte(~((uint8_t)1<<counter));
	myPCF8574.setBit(counter, dir > 0);
	counter += dir;
	if (counter == 8) {
		counter = 7;
		dir = -1;
	} else if (counter == -1) {
		counter = 0;
		dir = 1;
	}
}


void setup()
{
	Serial1.begin(115200);
	myPCF8574.begin(PCF8574_ADDRESS, SDA_PIN, CLK_PIN);
	ticker.attach(0.1, timerCB);
}

// The loop function is called in an endless loop
void loop()
{
}
