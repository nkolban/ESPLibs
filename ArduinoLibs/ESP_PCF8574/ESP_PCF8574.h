#ifndef ESP_PCF8574_h
#define ESP_PCF8574_h
/**
 * Class definition for the PC8574 device.
 */
class ESP_PCF8574 {
public:
	/**
	 * Constructor for the class
	 */
	ESP_PCF8574();

	/**
	 * Set the address and pins to be used for SDA and CLK in the Two Wire (I2C) protocol.
	 */
	void begin(uint8_t address, uint8_t sda, uint8_t clk);

	/**
	 * Set the bit (range 0-7) of the GPIOs to the supplied value.
	 */
	void setBit(uint8_t bit, bool value);

	/**
	 * Write the value of the byte as the output of the GPIOs.
	 */
	void setByte(uint8_t value);

	/**
	 * Get the value of the byte of the GPIOs as input.
	 */
	uint8_t getByte();

	/**
	 * Get the value of the bit of the GPIO as input.
	 */
	bool getBit(uint8_t bit);

private:
	int m_address;
	uint8_t m_currentOutput;
};
#endif
// Nothing after here
