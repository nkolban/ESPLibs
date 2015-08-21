#include <Arduino.h>
#include <ESPString.h>
ESPString::ESPString(char *data) {
	m_pData = (char *)malloc(strlen(data) + 1);
	strcpy(m_pData, data);
}
