#include "FS.h"
#include "SD.h"
#include "SPI.h"
//ERROR 001 <- nie ma folderu/to nie jest folder
//ERROR 002 <- Błąd przy usuwaniu pliku
//ERROR 003 <- Błąd przy otwieraniu
//ERROR 004 <- Błąd przy zapisywaniu
//ERROR 005 <- Nie można utworzyć folderu
//ERROR 006 <- Błąd karty SD

//Ten kod jest mocno niedoszlifowany, prototyp jak chuy

//###########################ZWIAZANE Z PLIKAMI I FOLDERAMI
void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("LIST: %s\n", dirname);
    File root = fs.open(dirname);
    if(!root){ Serial.println("ERROR 003"); return; }
    if(!root.isDirectory()){ Serial.println("ERROR 001"); return; }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : "); Serial.println(file.name());
            if(levels) listDir(fs, file.path(), levels-1);
        } 
        else 
        {
            Serial.print("  FILE: "); Serial.println(file.name());
        }
        file = root.openNextFile();
    }
}


void browseDir(fs::FS &fs, String currentPath) {
    while(true) {
        Serial.printf("\n--> %s\n", currentPath.c_str());
        listDir(fs, currentPath.c_str(), 0);
        String name = "";
        while(true) {
            if(Serial.available()) {
                char response = Serial.read();
                if(response == '\\'){ 
                    return;
                }
                if(response == '\n' || response == '\r') {
                    if(name.length() > 0) break;
                } 
                else {
                    name += response;
                    Serial.print(response);
                }
            }
        }

        String path = currentPath + "/" + name;
        File f = fs.open(path);

        if(!f) {
            Serial.println("ERROR 003");
        } 
        else if(f.isDirectory()) {
            f.close();
            browseDir(fs, path);
        } 
        else {
            Serial.println("\DATA:");
            while(f.available()) Serial.write(f.read());
            f.close();
            Serial.println("\n--- END OF FILE ---");
        }
    }
}


bool removeDirr(fs::FS &fs, const char *path) {
    Serial.printf(">>> Removing... %s\n", path);
    
    int maxIterations = 100;
    int iteration = 0;
    
    while(iteration < maxIterations) {
        iteration++;
        File dir = fs.open(path);
        if(!dir) {
            Serial.println("ERROR 003");
            break;
        }
        
        if(!dir.isDirectory()) {
            Serial.println("ERROR 005");
            dir.close();
            break;
        }
        
        File file = dir.openNextFile();
        
        if(!file) {
            // Folder pusty
            Serial.println("Removing...");
            dir.close();
            delay(100);
            
            if(fs.rmdir(path)) {
                Serial.printf("Removed %s\n", path);
            } else {
                Serial.println("ERROR 002");
            }
            return true;
        }
        
        // Jest jakiś plik/folder
        char nameBuffer[64];
        strncpy(nameBuffer, file.name(), 63);
        nameBuffer[63] = '\0';
        bool isDir = file.isDirectory();
        
        file.close();
        dir.close();
        
        delay(50);
        yield();
        
        String itemPath = String(path) + "/" + String(nameBuffer);
        Serial.printf("Found: %s (folder: %d)\n", itemPath.c_str(), isDir);
        
        if(isDir) {
            removeDir(fs, itemPath.c_str());
        } else {
            // Usuń plik
            if(fs.remove(itemPath.c_str())) {
                Serial.printf("Removed: %s\n", itemPath.c_str());
            } else {
                Serial.printf("ERROR 002");
            }
        }
        
        delay(50);
        yield();
    }
    
    if(iteration >= maxIterations) {
        Serial.println("!!!");
    }
    
    return false;
}


bool removeDir(fs::FS &fs, const char *path) {
    Serial.printf("Usuwam folder: %s\n", path);
    
    bool foundSomething = true;
    
    while(foundSomething) {
        foundSomething = false;
        
        File dir = fs.open(path);
        if(!dir || !dir.isDirectory()) {
            if(dir) dir.close();
            break;
        }
        
        File file = dir.openNextFile();
        
        if(file) {
            char nameBuffer[64];
            strncpy(nameBuffer, file.name(), 63);
            nameBuffer[63] = '\0';
            
            bool isDir = file.isDirectory();
            
            // Now close everything
            file.close();
            dir.close();

            delay(10);
            yield();
            String fpath = String(path) + "/" + String(nameBuffer);
            
            if(isDir) {
                if(!removeDir(fs, fpath.c_str())) {
                    Serial.printf("ERROR 002: %s\n", fpath.c_str());
                }
            } else {
                if(fs.remove(fpath.c_str())) {
                    Serial.printf("Removed: %s\n", fpath.c_str());
                } else {
                    Serial.printf("ERROR 002");
                }
            }
            
            foundSomething = true;
            delay(10);
        } else {
            dir.close();
        }
    }
    delay(50);
    if(fs.rmdir(path)) {
        Serial.printf("REMOVED: %s\n", path);
        return true;
    } else {
        Serial.printf("ERROR 002");
        return false;
    }
}


void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.print(".");
    File file = fs.open(path, FILE_APPEND); 
    if(!file){ 
        Serial.println("ERROR 003"); 
        return; 
    } 
    if(file.print(message)){ 
    } 
    else { 
        Serial.println("ERROR 004"); 
    } 
        file.close(); 
}


void createDir(fs::FS &fs, const char *path){
    if(fs.mkdir(path)){
        Serial.printf("Created folder: %s\n", path);
    } 
    else {
        Serial.println("ERROR 005");
    }
}


bool deleteFile(fs::FS &fs, const char * path){
    if(fs.remove(path)){
        Serial.println("Wyjebany."); // bawi mnie wiec zostawilem
        return true;
    } 
    else {
        Serial.println("ERROR 002");
        return false;
    }
}

String nextFolder(fs::FS &fs){
    int counter = 0;
    while(true) {
        String folderName = "/" + String(counter);
        File f = fs.open(folderName);
        if(!f || !f.isDirectory()){
            return folderName;
        }
        counter++;
        f.close();
    }
}

//###########################ZWIAZANE Z DANYMI (TYMCZASOWKA)
String dummy() {
    unsigned long czas = millis()/1000;
    float V = random(0,101)/10.0;
    float kWh = random(0,101)/10.0;
    int velocity = random(0,101);
    return String(czas) + ";" + String(V) + ";" + String(kWh) + ";" + String(velocity) + "\n";
}

void appendDummyData(fs::FS &fs, const String &folder){
    String filePath = folder + "/log.txt";
    File f = fs.open(filePath);
    if(!f){
        appendFile(fs, filePath.c_str(), "TIME ; V; KWH; VELOCITY\n");
    } 
    else { 
        f.close(); 
    }
    appendFile(fs, filePath.c_str(), dummy().c_str());
}

//########################### KOMENDY
char getResponse(char response){
    while(Serial.available()){
        response = Serial.read();
        if(response == '\n' || response == '\r' || response == ' ') continue;
        return response;
    }
    return '\n';
}

char isResponse = 0;
char command = 0;
bool busy = false;
String currentFolder;
unsigned long lastLog = 0;


void setup(){
    Serial.begin(115200);
    while(!Serial);

    if(!SD.begin(21)){
        Serial.println("ERROR 006!");
        return;
    }

    Serial.println("\n\nPROGRAM START\n");
    listDir(SD, "/", 0);

    currentFolder = nextFolder(SD);
    createDir(SD, currentFolder.c_str());
}


void loop(){
    while (busy == false){
        unsigned long now = millis();
        if(now - lastLog >= 2000){
            lastLog = now;
            appendDummyData(SD, currentFolder);
        }
        isResponse = getResponse(isResponse);
        if(!busy && isResponse != '\n' && isResponse != 0){
            command = isResponse;
            busy = true;
        }
    }

    if(busy){
        switch(command){
            case 'd':
                listDir(SD, "/", 0);
                busy = false;
                break;

            case 'c': {
                String folderName = "";
                while(folderName.length() == 0){
                    folderName = getResponse(0);
                }
                String folderPath = "/" + folderName;
                createDir(SD, folderPath.c_str());
                busy = false;
                break;
            }

            case 'r': {
                browseDir(SD, "/");
                busy = false;
                break;
            }

            case 'x': {
                Serial.println("which file you want to remove?");
                String fileName = "";
                while(fileName.length() == 0){
                    fileName = getResponse(0);
                }
                String filePath = "/" + fileName + ".txt";
                deleteFile(SD, filePath.c_str());
                busy = false;
                break;
            }

            case 'f': {
                Serial.println("which folder you want to remove? ");
                while(true){
                    String folderName = "";
                    while(Serial.available()) {
                        char response = Serial.read();
                        
                        if(response == '\n' || response == '\r') { 
                            if(folderName.length() > 0) break; 
                        } 
                        else{
                            folderName += response;
                            Serial.print(response); 
                        }
                    }
                    delay(10);
                    Serial.println();
                    if(folderName.length() > 0) {
                        String folderPath = "/" + folderName;
                        Serial.printf("\nREMOVING... %s\n", folderPath.c_str());
                        removeDirr(SD, folderPath.c_str());
                        break;
                    }
                }
                isResponse = 0;
                busy = false;
                break;
                
            }
                default:
                    busy = true;
                    break;
            
        }
    }
}