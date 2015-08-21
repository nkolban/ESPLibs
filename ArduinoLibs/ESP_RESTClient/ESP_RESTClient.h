#ifndef ESP_RestClient_h
#define ESP_RestClient_h
#include <Arduino.h>
#include <List.h>

extern "C" {
	#include <c_types.h>
	#include <ip_addr.h>
	#include <espconn.h>
}
class ESP_RestClient {
public:
	ESP_RestClient();
	void setURL(const char *server, int port, const char *path, bool useSSL=false);
	void setURL(String url);
	void setHeader(const char *name, const char *value);

	bool get();
	bool put(const char *payload);
	bool post(const char *payload);
	void onResponse(void (*onResponse)(String response));
	void onError(void (*onError)());
	void _send();
	void _close();
	void _handleError();
	void _handleResponse();
	void _connect();
	ip_addr_t m_serverIP;
	String m_response;

private:
	bool m_send(const char *payload);
	void m_resolveHostname();



	uint16 m_port;
	bool m_secure;
	char *m_payload;
	int m_commandType;
	String m_hostname;
	String m_path;
	void (*m_onResponse)(String response);
	void (*m_onError)();
	uint8_t *m_pSendBuffer;
	struct espconn m_conn;
	esp_tcp m_tcp;
	List m_headers;
};
#endif
