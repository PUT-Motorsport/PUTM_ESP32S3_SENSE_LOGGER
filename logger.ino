#include "SDCardManager.h"
#include "HardwareSerial.h"
unsigned long lastLog = 0;
const long interval = 2000;
String command = "";
HardwareSerial SerialUART(8);

SemaphoreHandle_t bufferMutex;

String getResponse() {
    String commandz = "";
    if (SerialUART.available() > 0){
        commandz = SerialUART.readStringUntil('\n');
    }
    else if (Serial.available() > 0){
        commandz = Serial.readStringUntil('\n');
    }
    commandz.trim();
    return commandz;
}


void setup() {
    Serial.begin(115200);
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    if (!SD.begin(SD_CS, SPI, 40000000)) {
        print("CARD WONT INITIALIZE");
        return;
    }
    wifiConfig();
    timeConfig();
    print("Time Synced successfully!"); 

    memset(bufferA, 0, BUFFER_SIZE);
    memset(bufferB, 0, BUFFER_SIZE);

    path = nextFolder(SD);
    createDir(SD, path);
    String *pathg = new String(path);
    xTaskCreatePinnedToCore(
        TaskWriteToSD,
        "WriteSD",
        8192,
        (void*)pathg,
        1,               
        &TaskWriteHandle,
        1
    );
}

void loop() {
    worker();
    WebsiteHandler();
    checkFlush();

    command = getResponse();
    if (command.length() > 0){
        print("CMD: " + command);
        if (mscConfirm) {
            if (command.equalsIgnoreCase("y")) {
                mscConfirm = false;
                if (logFile) logFile.close();
                startMSC();
            } else {
                mscConfirm = false;
                print("MSC Cancelled");
            }
        }
        else if (command.startsWith("AT:SAVE ")){
            appendFile(SD, path, command);
        }
        else if (command.startsWith("AT:STORAGE ")){
            print(getStorage());
        }
        else if (command == "AT:MSC"){
            mscConfirm = true;
            print("Send 'y' to confirm MSC");
        }
        else{
            print("No such command");
        }
        command = "";
    }
    String data = dummy();
    addDataToBuffer((const uint8_t*)data.c_str(), data.length());

    delay(2);

}