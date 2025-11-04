#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <HardwareSerial.h>
#include "USB.h"
#include "USBMSC.h"
#include "esp_task_wdt.h"

String command = "";
bool busy = false;
String currentFolder;
String path;
HardwareSerial SerialUART(2);
USBMSC msc;
bool mscMode = false;
uint32_t sectorCount = 0;
uint32_t sectorSize = 512;
bool sdInitialized = false;

String getResponse() {
    String commandz = "";
    if (Serial.available()) {
        commandz = Serial.readStringUntil('\n');
    } else if (SerialUART.available()) {
        commandz = SerialUART.readStringUntil('\n');
    }
    commandz.trim();
    return commandz;
}

void listDir(fs::FS &fs, String dirname) {
    Serial.printf("LIST: %s\n", dirname.c_str());
    File root = fs.open(dirname.c_str());
    if (!root) {
        Serial.println("ERROR 003");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println("ERROR 001");
        return;
    }
    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.print(" DIR : ");
            Serial.println(file.name());
        } else {
            Serial.print(" FILE: ");
            Serial.println(file.name());
        }
        file = root.openNextFile();
    }
}

bool removeDirr(fs::FS &fs, String path) {
    int maxIterations = 100;
    int iteration = 0;
    while (iteration < maxIterations) {
        iteration++;
        File dir = fs.open(path.c_str());
        if (!dir) {
            Serial.println("ERROR 003");
            break;
        }
        if (!dir.isDirectory()) {
            Serial.println("ERROR 005");
            dir.close();
            break;
        }
        File file = dir.openNextFile();
        if (!file) {
            dir.close();
            delay(100);
            if (fs.rmdir(path)) {
                Serial.printf("Removed %s\n", path.c_str());
            } else {
                Serial.println("ERROR 002");
            }
            return true;
        }
        char nameBuffer[64];
        strncpy(nameBuffer, file.name(), 63);
        nameBuffer[63] = '\0';
        bool isDir = file.isDirectory();
        file.close();
        dir.close();
        delay(50);
        yield();
        String itemPath = String(path) + "/" + String(nameBuffer);
        if (isDir) {
            removeDir(fs, itemPath);
        } else {
            if (!fs.remove(itemPath.c_str())) {
                Serial.printf("ERROR 102");
            }
        }
        delay(50);
        yield();
    }
    return false;
}

bool removeDir(fs::FS &fs, String path) {
    bool foundSomething = true;
    while (foundSomething) {
        foundSomething = false;
        File dir = fs.open(path.c_str());
        if (!dir || !dir.isDirectory()) {
            if (dir) dir.close();
            break;
        }
        File file = dir.openNextFile();
        if (file) {
            char nameBuffer[64];
            strncpy(nameBuffer, file.name(), 63);
            nameBuffer[63] = '\0';
            bool isDir = file.isDirectory();
            file.close();
            dir.close();
            delay(10);
            yield();
            String fpath = String(path) + "/" + String(nameBuffer);
            if (isDir) {
                if (!removeDir(fs, fpath.c_str())) {
                    Serial.printf("ERROR 202: %s\n", fpath.c_str());
                }
            } else {
                if (!fs.remove(fpath.c_str())) {
                    Serial.printf("ERROR 302");
                }
            }
            foundSomething = true;
            delay(10);
        } else {
            dir.close();
        }
    }
    delay(50);
    if (fs.rmdir(path)) {
        Serial.printf("REMOVED: %s\n", path.c_str());
        return true;
    } else {
        Serial.printf("ERROR 402");
        return false;
    }
}

void DEV(fs::FS &fs, String currentPath) {
    bool browsing = true;
    Serial.printf("\nDEV MODE%s\n", currentPath.c_str());
    listDir(fs, currentPath);

    while (browsing) {
        String response = getResponse();
        if (response.length() == 0) continue;
        response.trim();

        if (response.equalsIgnoreCase("AT:EXIT")) {
            Serial.println("Exiting dev mode...");
            browsing = false;
        } else if (response.equalsIgnoreCase("AT:\\")) {
            if (currentPath != "/") {
                int lastSlash = currentPath.lastIndexOf('/');
                if (lastSlash > 0) currentPath = currentPath.substring(0, lastSlash);
                else currentPath = "/";
                listDir(fs, currentPath);
            } else {
                Serial.println("Already at root folder.");
            }
        } else if (response.startsWith("AT:CD ")) {
            String folder = response.substring(6);
            folder.trim();
            String newPath = currentPath + "/" + folder;
            File f = fs.open(newPath);
            if (!f || !f.isDirectory()) {
                Serial.println("ERROR 101.");
            } else {
                currentPath = newPath;
                listDir(fs, currentPath);
            }
            if (f) f.close();
        } else if (response.startsWith("AT:R ")) {
            String filename = response.substring(5);
            filename.trim();
            String filePath = currentPath + "/" + filename;
            File f = fs.open(filePath);
            if (!f) {
                Serial.println("ERROR 201");
            } else if (f.isDirectory()) {
                Serial.println(".");
            } else {
                Serial.printf("--- %s ---\n", filePath.c_str());
                while (f.available()) Serial.write(f.read());
                Serial.println("\n--- END OF FILE ---");
            }
            if (f) f.close();
        } else if (response.startsWith("AT:RMALL ")) {
            String folder = response.substring(9);
            folder.trim();
            String folderPath = currentPath + "/" + folder;
            if (removeDirr(fs, folderPath)) Serial.printf("Removed: %s\n", folderPath.c_str());
            else Serial.println("ERROR 301");
            listDir(fs, currentPath);
        } else if (response.equalsIgnoreCase("AT:MSC")) {
            mscMode = true;
            browsing = false;
        } else {
            Serial.println("Unknown command in DEV mode.");
        }
    }
}

void appendFile(fs::FS &fs, String path, String message) {
    File file = fs.open(path.c_str(), FILE_APPEND);
    if (!file) {
        Serial.println("ERROR 503");
        return;
    }
    if (!file.print(message.c_str())) {
        Serial.println("ERROR 004");
    }
    file.close();
}

void createDir(fs::FS &fs, String path) {
    if (!fs.mkdir(path.c_str())) {
        Serial.println("ERROR 105");
    }
}

String nextFolder(fs::FS &fs) {
    int counter = 0;
    while (true) {
        String folderName = "/" + String(counter);
        File f = fs.open(folderName);
        if (!f || !f.isDirectory()) {
            return folderName;
        }
        counter++;
        f.close();
    }
}

String dummy() {
    unsigned long czas = millis() / 1000;
    float V = random(0, 101) / 10.0;
    float kWh = random(0, 101) / 10.0;
    int velocity = random(0, 101);
    return String(czas) + ";" + String(V) + ";" + String(kWh) + ";" + String(velocity) + "\n";
}

void appendDummyData(fs::FS &fs, const String &folder) {
    String filePath = folder + "/log.txt";
    File f = fs.open(filePath);
    if (!f) {
        appendFile(fs, filePath.c_str(), "TIME ; V; KWH; VELOCITY\n");
    } else {
        f.close();
    }
    appendFile(fs, filePath.c_str(), dummy().c_str());
}

static int32_t onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
    if (!sdInitialized) return -1;
    uint8_t* buf = (uint8_t*)buffer;
    uint32_t count = bufsize / sectorSize;
    for (uint32_t i = 0; i < count; i++) {
        if (!SD.readRAW(buf, lba + i)) return -1;
        buf += sectorSize;
    }
    return bufsize;
}

static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
    if (!sdInitialized) return -1;
    uint32_t count = bufsize / sectorSize;
    for (uint32_t i = 0; i < count; i++) {
        if (!SD.writeRAW(buffer, lba + i)) return -1;
        buffer += sectorSize;
    }
    return bufsize;
}

void startMSC() {
    if (!SD.begin(21)) {
        Serial.println("SD not initialized");
        return;
    }
    sdInitialized = true; // <--- bardzo ważne
    sectorSize = SD.sectorSize();
    sectorCount = SD.numSectors();

    USB.begin(); // USB trzeba uruchomić najpierw

    msc.vendorID("Seeed");
    msc.productID("XIAO ESP32S3");
    msc.productRevision("1.0");
    msc.onRead(onRead);
    msc.onWrite(onWrite);
    msc.mediaPresent(true);
    msc.begin(sectorCount, sectorSize);

    Serial.println("MSC MODE ACTIVE");

    while (true) {
    }
}

void setup() {
    Serial.begin(115200);
    SerialUART.begin(9600, SERIAL_8N1, 44, 43);

    if (!SD.begin(21)) {
        Serial.println("ERROR 006");
        return;
    }

    Serial.println("DEBUG MODE");
    currentFolder = nextFolder(SD);
    createDir(SD, currentFolder);
    appendDummyData(SD, currentFolder);
}

void loop() {
    while (busy == false) {
        if (Serial.available() || SerialUART.available()) {
            command = getResponse();
            if (!busy && command.length() > 0) {
                busy = true;
            }
        }
    }

    if (busy) {
        if (command.startsWith("AT:SAVE ")) {
            String text = command.substring(8);
            text.trim();
            path = currentFolder + "/" + "log.txt";
            appendFile(SD, path, text);
            Serial.println("\nADDED");
            SerialUART.println("PING");
            nucleoConnected = true;
            lastPing = millis();
        } else if (command == "AT:DEV") {
            DEV(SD, "/");
        } else if (command == "AT:MSC") {
            mscMode = true;
            startMSC();
        } else {
            Serial.println("Unknown command.");
        }
        busy = false;
        path = "";
        command = "";
    }
}