#ifndef ESP_Log_h
#define ESP_Log_h
class ESPLog {
public:
	static void ardprintf(const char *str, ...);
	static void dumpHex(const char *buf, int size);
	static void dumpHex(const char *from, const char *to);
};
#define LOG ESPLog::ardprintf
#endif
// End of file
