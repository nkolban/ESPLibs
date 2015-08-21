#include "ESP_RESTClient.h"

#define DEBUG
#ifdef DEBUG
#include <Arduino.h>
#include <ESP_Log.h>
#else
#define LOG(message)
#endif

extern "C" {
#include <CommonSDK.h>
}

enum {
	ESP_REST_CLIENT_ERROR_SIG = 0x10001,
	ESP_REST_CLIENT_RESPONSE_SIG = 0x10002
};


#define TASK_QUEUE_LEN 10
#define REST_TASK_PRI USER_TASK_PRIO_1

class URL {
public:
	bool ssl;
	int port;
	String hostname;
	String path;
	void dump() {
		LOG("URL: hostname: %s, port: %d, path: %s, useSSL: %d\n", hostname.c_str(), port, path.c_str(), ssl);
	}
};

void taskHandler(os_event_t *event) {
	ESP_RestClient *pESP_RestClient;
	switch(event->sig) {
	case ESP_REST_CLIENT_ERROR_SIG:
		pESP_RestClient= (ESP_RestClient *)event->par;
		pESP_RestClient->_handleError();
		break;
	case ESP_REST_CLIENT_RESPONSE_SIG:
		pESP_RestClient= (ESP_RestClient *)event->par;
		pESP_RestClient->_handleResponse();
		break;
	}
}

/**
 * Parse a URL string of the format:
 * <http|https>://<hostname>[:<port>]</path>
 *
 */
// http://a.b.c.com[:<port>]/<path>
static bool parseURL(String url, URL *pURL) {

	int i;
	if (url.startsWith("http://")) {
		pURL->ssl = false;
		i = 7; // Skip 7 characters
	} else if (url.startsWith("https://")) {
		pURL->ssl = true;
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
	pURL->hostname = url.substring(i, j);

	// Let us now see if we have a port or just a path
	if (c == 0 || c == '/') {
		pURL->port = 80;
	} else {
		j++;
		int s = j;
		char c = url.charAt(j);
		while (c != 0 && c != '/') {
			j++;
			c = url.charAt(j);
		}
		String portStr = url.substring(s, j);
		pURL->port = portStr.toInt();
	}
	pURL->path = url.substring(j);
	// Now we have ssl, host, port and path
	return true;
} // End of parseURL

/**
 * The possible types of HTTP commands that can be sent by REST.
 */
enum HTTPCommand {
	GET, POST, PUT
};

/**
 * A representation of an HTTP header.
 */
class Header {
public:
	void setHeader(char *name, char *value) {
		m_name = name;
		m_value = value;
	}
	char *getName() {
		return m_name;
	}
	char *getValue() {
		return m_value;
	}
private:
	char *m_name;	// The name of the header
	char *m_value;	// The value of the header
};
// End of Header

static void dnsFoundCallback(const char *hostname, ip_addr_t *pIpAddr,
		void *arg) {
	struct espconn *pEspconn = (struct espconn *) arg;
	ESP_RestClient *pESP_RestClient = (ESP_RestClient *) pEspconn->reverse;
	if (pIpAddr == 0 || pIpAddr->addr == 0) {
		pESP_RestClient->_handleError();
		return;
	}
	LOG("gethostbyname callback ... hostname %s, IP: %i\n", hostname, pIpAddr);
	pESP_RestClient->m_serverIP = *pIpAddr;
	pESP_RestClient->_connect();
	//dumpEspConn(pEspconn);
} // End of dnsFoundCallback

/**
 * The receive callback called when data is received.
 */
static void recvCB(void *arg, char *pData, unsigned short len) {
	LOG("recvCB\n");
	struct espconn *pEspconn;
	pEspconn = (struct espconn *) arg;
	ESP_RestClient *pESP_RestClient = (ESP_RestClient *) pEspconn->reverse;
	LOG("Data: length received = %d\n", len);
	char *pBuf = (char *)malloc(len + 1);
	memcpy(pBuf, pData, len);
	pBuf[len] = '\0';
	pESP_RestClient->m_response = String(pBuf);
	free(pBuf);
	// Don't try and print the data here ... too much...
	pESP_RestClient->_close();
	system_os_post(REST_TASK_PRI, ESP_REST_CLIENT_RESPONSE_SIG, (os_param_t)pESP_RestClient);
} // End of recvCB

/**
 * The connect callback when a connection to a partner is made.
 */
static void connectCB(void *arg) {
	LOG("connectCB\n");
	//ESPLog::dumpHex((const char *)s1, (const char *)e1);
	struct espconn *pEspconn;
	pEspconn = (struct espconn *) arg;
	ESP_RestClient *pESP_RestClient = (ESP_RestClient *) pEspconn->reverse;
	//LOG("Payload in connectCB: %x", pESP_RestClient->m_payload);
	LOG("Address of ESP_RestClient = 0x%x\n", pESP_RestClient);
	pESP_RestClient->_send();
} // End of connectCB

static void sendCB(void *arg) {
	LOG("sendCB\n");
	/*
	 struct espconn *pEspconn;
	 pEspconn = (struct espconn *) arg;
	 ESP_RestClient *pESP_RestClient = (ESP_RestClient *) pEspconn->reverse;
	 */
	//pESP_RestClient->_send();
} // End of sendCB

static void reconnectCB(void *arg, sint8 err) {
	LOG("reconnectCB\n");
	struct espconn *pEspconn;
	pEspconn = (struct espconn *) arg;
	ESP_RestClient *pESP_RestClient = (ESP_RestClient *) pEspconn->reverse;
	LOG("Error: %d: %s\n", err, errorToString(err));
	// We had detected an error.
	pESP_RestClient->_handleError();
} // End of reconnectCB

static void disconnectCB(void *arg) {
	LOG("disconnectCB\n");
	struct espconn *pEspconn;
	pEspconn = (struct espconn *) arg;
	ESP_RestClient *pESP_RestClient = (ESP_RestClient *) pEspconn->reverse;
	pESP_RestClient->_close();
} // End of disconnectCB

void ESP_RestClient::_close() {
	LOG("_close\n");
	if (m_pSendBuffer != NULL) {
		free(m_pSendBuffer);
		m_pSendBuffer = NULL;
	}
	espconn_disconnect(&m_conn);
} // End of __close

void ESP_RestClient::_send() {
	String data;
	switch (m_commandType) {
	case GET:
		data += "GET ";
		break;
	case PUT:
		data += "PUT ";
		break;
	case POST:
		data += "POST ";
		break;
	}
	// Build the start line of the request.
	data += String(m_path) + String(" HTTP/1.1\r\n");

	// Add the headers to the HTTP request
	int size = m_headers.size();
	for (int i = 0; i < size; i++) {
		Header *pHeader = (Header *) m_headers.get(i);
		data += pHeader->getName() + String(": ") + pHeader->getValue()
				+ String("\r\n");
	}

	// Add the payload length if the type is PUT or POST.
	if (m_commandType == PUT || m_commandType == POST) {
		data += "Content-Length: " + String(strlen(m_payload)) + String("\r\n");
	}
	// Add the data separator line.
	data += String("\r\n");

	// Add the payload if we have any.
	if (m_payload != NULL) {
		data += m_payload;
	}

	// Send the buffer.
	int length = data.length() + 1;
	m_pSendBuffer = (uint8_t *) malloc(length);
	data.getBytes(m_pSendBuffer, length, 0);
	int rc = espconn_sent(&m_conn, m_pSendBuffer, length);
	if (rc != 0) {
		// Error
		LOG("Error with send: %d\n", rc);
		return;
	}
} // End of _send

ESP_RestClient::ESP_RestClient() {
	m_conn.type = ESPCONN_TCP;
	m_conn.state = ESPCONN_NONE;
	m_conn.proto.tcp = &m_tcp;
	m_conn.reverse = this;
	m_serverIP.addr = 0; // Initially, no IP address of server

	os_event_t *taskQueue;
	taskQueue = (os_event_t *)malloc(sizeof(os_event_t) * TASK_QUEUE_LEN);
	system_os_task(taskHandler, REST_TASK_PRI, taskQueue, TASK_QUEUE_LEN);

	// Define the callbacks to be called when ESP events are detected.
	espconn_regist_recvcb(&m_conn, recvCB);
	espconn_regist_connectcb(&m_conn, connectCB);
	espconn_regist_sentcb(&m_conn, sendCB);
	espconn_regist_disconcb(&m_conn, disconnectCB);
	espconn_regist_reconcb(&m_conn, reconnectCB);
} // End of constructor

/**
 * Set the URL to be used when sending requests.
 * o server - The IP address of the server
 * o port - The port number of the server
 * o path - The URL path part of the REST request
 */
void ESP_RestClient::setURL(const char *hostname, int port, const char *path,
bool useSSL) {
	m_hostname = (char *)hostname;
	m_port = port;
	m_path = (char *) path;
	m_secure = useSSL;
	m_serverIP.addr = 0;
} // End of setURL

void ESP_RestClient::setURL(String url) {
	URL urlClass;
	parseURL(url, &urlClass);
	urlClass.dump();
	m_hostname = urlClass.hostname;
	m_port = urlClass.port;
	m_path = urlClass.path;
	m_secure = urlClass.ssl;
	m_serverIP.addr = 0;
}

/**
 * Set a header to be sent.
 */
void ESP_RestClient::setHeader(const char *name, const char *value) {
	Header *pHeader = new Header();
	pHeader->setHeader((char *) name, (char *) value);
	m_headers.add((void *) pHeader);
} // End of setHeader

/**
 * Send a GET request.
 */
bool ESP_RestClient::get() {
	m_commandType = GET;
	return m_send(NULL);
} // End of get

/**
 * Send a PUT request.
 */
bool ESP_RestClient::put(const char *payload) {
	m_commandType = PUT;
	return m_send(payload);
} // End of put

/**
 * Send a POST request.
 */
bool ESP_RestClient::post(const char *payload) {
	m_commandType = POST;
	return m_send(payload);
} // End of post

/**
 * Register a callback function to be called when a response is detected.
 */
void ESP_RestClient::onResponse(void (*onResponse)(String response)) {
	m_onResponse = onResponse;
} // End of onResponse

/**
 * Record a callback function to be called if an error is detected.
 */
void ESP_RestClient::onError(void (*onError)()) {
	m_onError = onError;
} // End of onError

/**
 * Handle an error having been detected.
 */
void ESP_RestClient::_handleError() {
	// Close the connection to the partner and then invoke the callback function
	// if one exists of the error callback.
	//LOG("_handleError");
	_close();
	if (m_onError != NULL) {
		//LOG("Calling error handler");
		m_onError();
	}
} // End of _handleError

/**
 * Handle a response.
 */
void ESP_RestClient::_handleResponse() {
	_close();
	if (m_onResponse != NULL) {
		m_onResponse(m_response);
	}
} // End of _handleResponse



/**
 * Build and send a REST request
 */
bool ESP_RestClient::m_send(const char *payload) {
	m_payload = (char *) payload;
	/*
	if (m_secure == false) {
		int rc = espconn_connect(&m_conn);
		if (rc != 0) {
			// Error
			LOG("Error with connect\n");
			_handleError();
			return false;
		}
	} else {
		//int rc = espconn_secure_connect(&m_conn);
	}
	*/
	m_resolveHostname();
	return true;
} // End of send

void ESP_RestClient::m_resolveHostname() {
	LOG("About to resolve hostname of: %s\n", m_hostname.c_str());
	if (m_serverIP.addr != 0) {
		_connect();
		return;
	}
	int rc = espconn_gethostbyname(&m_conn, m_hostname.c_str(), &m_serverIP,
			dnsFoundCallback);
	if (rc == ESPCONN_OK) {
		LOG("We immediately have an IP address\n");
		_connect();
		return;
	}
	if (rc != ESPCONN_INPROGRESS) {
		_handleError();
		return;
	}
	return;
} // End of m_resolveHostname

void ESP_RestClient::_connect() {
	LOG("_connect: %i\n", &m_serverIP);
	memcpy(m_conn.proto.tcp->remote_ip, &m_serverIP, 4);
	m_conn.proto.tcp->remote_port = m_port;
	dumpEspConn(&m_conn);
	if (m_secure == false) {
		int rc = espconn_connect(&m_conn);
		if (rc != 0) {
			// Error
			LOG("Error with connect\n");
			system_os_post(REST_TASK_PRI, ESP_REST_CLIENT_ERROR_SIG, (os_param_t)this);
			return;
		}
	} else {
		//int rc = espconn_secure_connect(&m_conn);
	}
} // End of m_connect

// End of file
