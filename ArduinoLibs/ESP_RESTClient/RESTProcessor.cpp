/**
 * RESTProcessor
 * A REST processor for the ESP/Arduino.
 *
 * This class provides a set of routines for manipulating REST requests to be sent
 * or received over a TCP/IP connection.  It does not deal with actual communications
 * but rather concentrates solely on building and parsing REST requests and responses.
 * This makes it a partner to communication routines.
 *
 *
 * Neil Kolban
 * August 2015
 */
#include "RESTProcessor.h"

/**
 * The definition of a header.
 */
class Header {
public:
	void setHeader(String name, String value) {
		m_name = name;
		m_value = value;
	}
	String getName() {
		return m_name;
	}
	String getValue() {
		return m_value;
	}
private:
	String m_name;	// The name of the header
	String m_value;	// The value of the header
};

/**
 * Set the REST/HTTP command type.
 */
void RESTProcessor::setCommandType(HTTPCommand commandType) {
	m_commandType = commandType;
} // End of setCommandType


String RESTProcessor::getHostname() {
	return m_hostname;
} // End of getHostname

int RESTProcessor::getPort() {
	return m_port;
} // End of getPort

String RESTProcessor::getPath() {
	return m_path;
} // End of getPath

bool RESTProcessor::isUseSSL() {
	return m_useSSL;
} // End of isUseSSL


/**
 * Build a REST request.
 * The REST request to be sent over the network is returned in the
 * String returned by this function.
 */
String RESTProcessor::buildRequest(String payload) {
	String data;
	switch (m_commandType) {
	case GET:
		data += "GET";
		break;
	case PUT:
		data += "PUT";
		break;
	case POST:
		data += "POST";
		break;
	}
	// Build the start line of the request.
	data += " " + String(m_path) + String(" HTTP/1.1\r\n");

	// Add the headers to the HTTP request
	int size = m_headers.size();
	for (int i = 0; i < size; i++) {
		Header *pHeader = (Header *) m_headers.get(i);
		data += pHeader->getName() + String(": ") + pHeader->getValue()
				+ String("\r\n");
	}

	// Add the payload length (Content-Length) if the type is PUT or POST.
	if (m_commandType == PUT || m_commandType == POST) {
		data += "Content-Length: " + payload + String("\r\n");
	}

	// Add the data separator line.
	data += String("\r\n");

	// Add the payload if we have any.
	if (payload != NULL) {
		data += payload;
	}

	return data;
} // End of buildRequest

void RESTProcessor::addHeader(const char *name, const char *value) {
	Header *pHeader = new Header();
	pHeader->setHeader((char *) name, (char *) value);
	m_headers.add((void *) pHeader);
} // End of addHeader


/**
 * Parse a URL to extract the host, port, path and whether or not SSL is required.
 */
bool RESTProcessor::parseURL(String url) {

	int i;
	if (url.startsWith("http://")) {
		m_useSSL = false;
		i = 7; // Skip 7 characters
	} else if (url.startsWith("https://")) {
		m_useSSL = true;
		i = 8; // Skip 8 characters
	} else {
		return false;
	}
	// Look for either an end of string, a ':' or a '/'
	//        i
	//        |
	//        V
	// http://<hostname>[:<port>]/path
	//
	int j = i;
	char c = url.charAt(j);
	while (c != 0 && c != ':' && c != '/') {
		j++;
		c = url.charAt(j);
	}
	// url[i] - url[j] = hostname
	m_hostname = url.substring(i, j);

	// Let us now see if we have a port or just a path
	if (c == 0 || c == '/') {
		m_port = 80;
	} else {
		j++;
		int s = j;
		char c = url.charAt(j);
		while (c != 0 && c != '/') {
			j++;
			c = url.charAt(j);
		}
		String portStr = url.substring(s, j);
		m_port = portStr.toInt();
	}
	m_path = url.substring(j);
	// Now we have ssl, host, port and path
	return true;
} // End of parseURL
// End of file
