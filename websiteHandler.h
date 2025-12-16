#include "WiFi.h"
#include "webInterface.h"
#include "printLog.h"

WiFiServer server(80);
bool workerActive = true;
String ssid = "xxxxx";
String password = "xxxxxxx"; // nie dowiecie sie 
String webLog;
unsigned long lastWorkerRun = 0;
int lastWifiCheck = 0;
volatile bool downloadRequest = false;
volatile bool downloadReady = false;
String downloadPath = "/dump.csv";
const int DCS = 4096; //Download Chunk Size

String listDirJSON(fs::FS &fs, String dirname);
void worker();
void handleFileDownload(WiFiClient &client, String path);
void handleFileRead(WiFiClient &client, String path);
void WebsiteHandler();