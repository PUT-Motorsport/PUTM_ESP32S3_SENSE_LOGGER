#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "msc.h"
#include "bufferCode.h"
#include "websiteHandler.h"

String currentFolder;
String path;

const int VT = 1;
const int KWH = 2;
const int VEL = 4;
volatile int activeConfig = 7;

volatile int logFileIndex = 0;

const int SD_CS = 21;
const int SD_SCK = 7;
const int SD_MISO = 8;
const int SD_MOSI = 9;

void listDir(fs::FS &fs, String dirname);
bool removeDir(fs::FS &fs, String path);
void appendFile(fs::FS &fs, String path, String message);
void createDir(fs::FS &fs, String path);
String nextFolder(fs::FS &fs);
String dummy();
void appendDummyData(fs::FS &fs, const String &folder);
