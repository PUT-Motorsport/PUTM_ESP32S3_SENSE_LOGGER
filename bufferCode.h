const int BUFFER_SIZE = 4096; 

uint8_t bufferA[BUFFER_SIZE];
uint8_t bufferB[BUFFER_SIZE];

volatile uint32_t bufferIdx = 0;
volatile size_t bufferSizes[2] = {0, 0};
volatile bool useBufferA = true;
TaskHandle_t TaskWriteHandle = NULL;
File logFile;
unsigned long lastFlushTime = 0;
const long flushInterval = 4000;
const long MFS = 100000; //MaxFileSize
volatile bool rotateRequest = false;
extern volatile bool spauseRequest = false; //dowiedzialem sie dopiero ze takie cos jak extern istnieje xd
extern volatile bool spause = false;

void TaskWriteToSD(void *pvParameters);
void addDataToBuffer(const uint8_t* data, size_t len);
void checkFlush();