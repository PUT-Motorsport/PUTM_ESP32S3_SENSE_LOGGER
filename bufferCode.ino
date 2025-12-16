void addDataToBuffer(const uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        uint8_t* currentBuffer = useBufferA ? bufferA : bufferB;
        currentBuffer[bufferIdx++] = data[i];

        if (bufferIdx >= BUFFER_SIZE) {
            int currentBufIndex = useBufferA ? 0 : 1;
            bufferSizes[currentBufIndex] = BUFFER_SIZE;

            useBufferA = !useBufferA; 
            bufferIdx = 0;            
            
            if (TaskWriteHandle != NULL) {
                xTaskNotifyGive(TaskWriteHandle);
            }
            lastFlushTime = millis();
        }
    }
}

void checkFlush() {
    if (bufferIdx > 0 && (millis() - lastFlushTime > flushInterval)) {
        int currentBufIndex = useBufferA ? 0 : 1;
        bufferSizes[currentBufIndex] = bufferIdx; 

        useBufferA = !useBufferA;
        bufferIdx = 0;

        if (TaskWriteHandle != NULL) {
            xTaskNotifyGive(TaskWriteHandle);
        }
        lastFlushTime = millis();
        print("Auto-flush triggered");
    }
}

void TaskWriteToSD(void *pvParameters) {
    int logIndex = 1;
    String folderPath = *((String*)pvParameters);
    delete (String*)pvParameters;
    
    String currentLogName = folderPath + "/log_" + String(logIndex) + ".txt";
    bool skipGarbage = false; 
    unsigned long lastFlushTime = 0;

    logFile = SD.open(currentLogName, FILE_WRITE);
    if(logFile) {
        logFile.print(getHeader());
        logFile.flush();
    }

    while (true) {
        if (spauseRequest) {
            if(logFile) logFile.flush(); 
            spause = true;             
            
            while (spauseRequest) {       
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
            spause = false;            
        }
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (spauseRequest) {
            if(logFile) logFile.flush();
            spause = true;
            while (spauseRequest) vTaskDelay(10);
            spause = false;
        }

        uint8_t* bufferToSave = useBufferA ? bufferB : bufferA;
        int bufferIndex = useBufferA ? 1 : 0;
        size_t bytesToWrite = bufferSizes[bufferIndex];

        if (downloadRequest) { 
            if (logFile) logFile.close();
            SD.remove("/dump.csv");
            SD.rename(currentLogName, "/dump.csv");

            logIndex++;
            currentLogName = folderPath + "/log_" + String(logIndex) + ".txt";
            logFile = SD.open(currentLogName, FILE_WRITE);
            if (logFile) {
                logFile.print(getHeader());
                logFile.flush();
                skipGarbage = true;
            }
            downloadReady = true;
            downloadRequest = false;
        }

        if (rotateRequest) {
            rotateRequest = false;
            if (logFile) logFile.close();
            logIndex++;
            currentLogName = folderPath + "/log_" + String(logIndex) + ".txt";
            logFile = SD.open(currentLogName, FILE_WRITE);
            if (logFile) {
                logFile.print(getHeader());
                logFile.flush();
                skipGarbage = true;
            }
        }

        if (!logFile) {
            logFile = SD.open(currentLogName, FILE_APPEND);
            if (!logFile) logFile = SD.open(currentLogName, FILE_WRITE);
        }

        if (logFile) {
            if (logFile.size() >= 100000) {
                logFile.close();
                logIndex++;
                currentLogName = folderPath + "/log_" + String(logIndex) + ".txt";
                logFile = SD.open(currentLogName, FILE_WRITE);
                if (logFile) {
                    logFile.print(getHeader());
                    logFile.flush();
                    skipGarbage = true;
                }
            }

            if (bytesToWrite > 0) {
                size_t offset = 0;
                if (skipGarbage) {
                    for (size_t i = 0; i < bytesToWrite; i++) {
                        if (bufferToSave[i] == '\n') {
                            offset = i + 1;
                            skipGarbage = false;
                            break;
                        }
                    }
                    if (skipGarbage) offset = bytesToWrite; 
                }

                if (bytesToWrite > offset) {
                    logFile.write(bufferToSave + offset, bytesToWrite - offset);
                    if (millis() - lastFlushTime > 2000 || logFile.size() < 1024) {
                        logFile.flush();
                        lastFlushTime = millis();
                    }
                }
            }
        }
    }
}