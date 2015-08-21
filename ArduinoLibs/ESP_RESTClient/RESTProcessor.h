#ifndef RESTProcessor_h
#define RESTProcessor_h
#include <Arduino.h>
#include <List.h>
enum HTTPCommand {
	GET, POST, PUT
};

class RESTProcessor {
public:
	bool parseURL(String url);
	String getHostname();
	int getPort();
	String getPath();
	bool isUseSSL();
	void setCommandType(HTTPCommand commandType);
	void addHeader(const char *name, const char *value);
	String buildRequest(String payload);
private:
	String m_hostname;
	HTTPCommand m_commandType;
	int m_port;
	String m_path;
	bool m_useSSL;
	List m_headers;
};
#endif
