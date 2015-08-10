#include <Arduino.h>
#include <Wire.h>
#include "ESP_PCF8574.h"

/**
 * Constructor for the class
 */
ESP_PCF8574::ESP_PCF8574() {
	m_address = 0;
} // End of constructor

/**
 * Set the address and pins to be used for SDA and CLK in the Two Wire (I2C) protocol.
 */
void ESP_PCF8574::begin(uint8_t address, uint8_t sda, uint8_t clk) {
	m_address = address;
	Wire.begin(sda,clk);
} // End of setPins

/**
 * Set the bit (range 0-7) of the GPIOs to the supplied value.
 */
void ESP_PCF8574::setBit(uint8_t bit, bool value) {
	// Check that the bit is in range.  Must be 0-7.
	if (bit < 0 || bit > 7) {
		return;
	}
	if (value == true) {
		setByte(m_currentOutput | (1 << bit));
	} else {
		setByte(m_currentOutput & ~(1 << bit));
	}
} // End of setBit

/**
 * Write the value of the byte as the output of the GPIOs.
 */
void ESP_PCF8574::setByte(uint8_t value) {
	// Guard that we have been initialized
	if (m_address == 0) {
		return;
	}
	m_currentOutput = value;
	Wire.beginTransmission(m_address);
	Wire.write(m_currentOutput);
	Wire.endTransmission();
} // End of setByte

/**
 * Get the value of the byte of the GPIOs as input.
 */
uint8_t ESP_PCF8574::getByte() {
	Wire.requestFrom(m_address, 1);
	return Wire.read();
} // End of getByte

/**
 * Get the value of the bit of the GPIO as input.
 */
bool ESP_PCF8574::getBit(uint8_t bit) {
	if (bit < 0 || bit > 7) {
		return 0;
	}
	uint8_t byteValue = getByte();
	return (byteValue & (1<<bit)) != 0;
} // End of getBit
// End of file
