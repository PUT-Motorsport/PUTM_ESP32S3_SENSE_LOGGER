void wifiConfig(){
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    int tries = 0;
    while (WiFi.status() != WL_CONNECTED && tries < 20){
        print(".");
        delay(500);
        tries++;
    }
    WiFi.setSleep(false);
    print("IP ADDRESS: " + WiFi.localIP().toString());
    server.begin();
}

String listDirJSON(fs::FS &fs, String dirname) {
    String output = "[";
    File root = fs.open(dirname.c_str());
    if (!root) return "[]";
    bool first = true;
    File file = root.openNextFile();
    while (file) {
        String fileName = String(file.name());
        if (fileName.startsWith(".") || fileName.indexOf("System Volume") >= 0) {
            file = root.openNextFile(); continue;
        }
        if (!first) output += ",";
        String type = file.isDirectory() ? "dir" : "file";
        output += "{\"name\":\"" + fileName + "\",\"type\":\"" + type + "\",\"size\":" + String(file.size()) + "}";
        first = false;
        file = root.openNextFile();
    }
    output += "]";
    return output;
}

String shorten(String line, String name) {
    int start = line.indexOf(name + "=");
    if (start == -1) return "";
    start += name.length() + 1;
    int end = line.indexOf(" ", start);
    if (end == -1) end = line.length();
    String val = line.substring(start, end);

    val.replace("%2F", "/"); val.replace("%2f", "/");
    val.replace("%20", " "); val.replace("+", " ");
    val.replace("%3A", ":"); val.replace("%3a", ":");
    return val;
}

void worker() {
    if (workerActive && (millis() - lastWorkerRun > 1000)) {
        lastWorkerRun = millis();
    }
}

void handleFileDownload(WiFiClient &client, String path) {
    if (!path.startsWith("/")) path = "/" + path;
    if (path == "/log.txt") {
        downloadReady = false;
        downloadRequest = true; 
        if (TaskWriteHandle != NULL) xTaskNotifyGive(TaskWriteHandle); 
        
        unsigned long s = millis();
        while (!downloadReady && millis() - s < 2000) delay(10);
        
        if (downloadReady) path = "/dump.csv";
        else {
            client.println("HTTP/1.1 500 Internal Server Error");
            return; 
        }
    }
    spause = false;
    spauseRequest = true; 
    if (TaskWriteHandle != NULL) xTaskNotifyGive(TaskWriteHandle); 
    unsigned long pStart = millis();
    while (!spause && millis() - pStart < 1500) delay(1);

    if (spause) { 
        if (SD.exists(path)) {
            File file = SD.open(path, FILE_READ);
            if (file) {
                String filename = String(file.name());
                if (filename.startsWith("/")) filename = filename.substring(1);

                client.println("HTTP/1.1 200 OK");
                client.println("Content-Type: application/octet-stream");
                client.println("Content-Disposition: attachment; filename=\"" + filename + "\"");
                client.println("Connection: close");
                client.println("Content-Length: " + String(file.size()));
                client.println();

                uint8_t buffer[1024];
                while (file.available()) {
                    size_t len = file.read(buffer, 1024);
                    if (len > 0) client.write(buffer, len);
                    yield(); 
                }
                file.close();
            } else {
                client.println("HTTP/1.1 500 File Open Error");
                client.println();
            }
        } else {
            client.println("HTTP/1.1 404 Not Found");
            client.println();
        }
    } else {
        client.println("HTTP/1.1 503 SD Busy");
        client.println();
    }
    spauseRequest = false; 
}

void handleFileRead(WiFiClient &client, String path) {
    if (SD.exists(path)) {
        File file = SD.open(path, FILE_READ);
        if (file) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/plain");
            client.println("Connection:close");
            client.println();
            uint8_t buffer[1024];
            while (file.available()) {
                size_t len = file.read(buffer, 1024);
                if (len > 0){
                    client.write(buffer, len);
                }
                yield;
            }
            file.close();
        }
        else {
            client.println("HTTP/1.1 500 Error");
            client.println();
        }
    } else {
        client.println("HTTP/1.1 500 Error");
        client.println();
    }
}

void WebsiteHandler(){
    WiFiClient client = server.available();
    if (!client) return;

    String requestLine = client.readStringUntil('\r');

    while (client.connected() && client.available()) {
        if (client.readStringUntil('\n') == "\r") break; 
    }

    if (requestLine.startsWith("POST /save")) {
        String path = shorten(requestLine, "path");
        File file = SD.open(path, FILE_WRITE);
        if (file) {
            while (client.available()) file.write(client.read());
            file.close();
            print("Saved: " + path);
            client.println("HTTP/1.1 200 OK\r\nConnection: close\r\n\r\n");
        } else {
            client.println("HTTP/1.1 500 Error\r\nConnection: close\r\n\r\n");
        }
    }
    else if (requestLine.indexOf("/set_cfg") >= 0) {
        String maskStr = shorten(requestLine, "mask");
        int newCfg = maskStr.toInt();
        if (newCfg > 0) {
            activeConfig = newCfg;
            bufferIdx = 0; //RESETUJE TUTAJ DANE!!!
            bufferSizes[0] = 0;
            bufferSizes[1] = 0;
            
            rotateRequest = true;
            if (TaskWriteHandle != NULL) xTaskNotifyGive(TaskWriteHandle);
            
            client.println("HTTP/1.1 200 OK\r\nConnection: close\r\n\r\nOK");
        } else {
            client.println("HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\nInvalid");
        }
    }

    else if (requestLine.indexOf("/get_cfg") >= 0) {
        client.println("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n" + String(activeConfig));
    }
    else if (requestLine.indexOf("/toggle") >= 0) {
        workerActive = !workerActive;
        client.println("HTTP/1.1 200 OK\r\nConnection: close\r\n\r\n");
    }

    else if (requestLine.indexOf("/logs") >= 0) {
        client.println("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n" + webLog);
    }
    else if (requestLine.indexOf("/storage") >= 0) { // chuj wie ocb ale buja?
        String json = "{\"used\":\"" + String((uint64_t)SD.usedBytes()) + "\",\"total\":\"" + String((uint64_t)SD.totalBytes()) + "\"}";
        client.println("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n" + json);
    }
    else if (requestLine.indexOf("/list") >= 0) {
        client.println("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n" + listDirJSON(SD, shorten(requestLine, "dir")));
    }

    else if (requestLine.indexOf("/download") >= 0) {
        handleFileDownload(client, shorten(requestLine, "path"));
    }

    else if (requestLine.indexOf("/read") >= 0) {
        handleFileRead(client, shorten(requestLine, "path"));
    }

    else if (requestLine.indexOf("/delete") >= 0) {
        String path = shorten(requestLine, "path");
        if (SD.exists(path)) {
            File temp = SD.open(path);
            bool isDir = temp.isDirectory();
            temp.close();
            if (isDir ? removeDir(SD, path) : SD.remove(path)) print("Deleted: " + path);
        }
        client.println("HTTP/1.1 200 OK\r\nConnection: close\r\n\r\n");
    }
    else if (requestLine.indexOf("/set_cfg") >= 0) {
        String maskStr = shorten(requestLine, "mask");
        int newCfg = maskStr.toInt();
        if (newCfg > 0) {
            activeConfig = newCfg;
            rotateRequest = true;
            if (TaskWriteHandle != NULL) {
                xTaskNotifyGive(TaskWriteHandle);
            }
            Serial.println("Web Config change: " + String(newCfg));
            client.println("HTTP/1.1 200 OK\r\nConnection: close\r\n\r\nConfig Saved");
        } else {
            client.println("HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\nInvalid Mask");
        }
    }

    else if (requestLine.indexOf("/cmd") >= 0) {
        String val = shorten(requestLine, "val");
        print("> " + val);
        if (mscConfirm && val.equalsIgnoreCase("y")) {
          mscConfirm = false;
          startMSC(); 
        }
        else if (val.startsWith("AT:SAVE")) {
        }
        else if (val == "AT:MSC") {
          mscConfirm = true;
          print("Send 'y' to confirm MSC");
        }
        client.println("HTTP/1.1 200 OK\r\nConnection: close\r\n\r\n");
    }
    else if (requestLine.indexOf("/time") >= 0) {
        struct tm t; 
        if(getLocalTime(&t)) {
            char s[10]; strftime(s, 10, "%H:%M:%S", &t);
            client.println("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"time\":\"" + String(s) + "\"}");
        } else client.println("HTTP/1.1 500 Error\r\n\r\n");
    }
    else {
        client.println("HTTP/1.1 200 OK\r\nContent-type:text/html\r\n\r\n");
        client.write(index_html, strlen(index_html));
    }
    
    client.stop();
}