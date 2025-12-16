static int32_t onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
    if (!SD.begin(21)) return -1;
    uint8_t* buf = (uint8_t*)buffer;
    uint32_t count = bufsize / sectorSize;
    for (uint32_t i = 0; i < count; i++) {
        if (!SD.readRAW(buf, lba + i)) return -1;
        buf += sectorSize;
    }
    return bufsize;
}

static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
    if (!SD.begin(21)) return -1;
    uint32_t count = bufsize / sectorSize;
    for (uint32_t i = 0; i < count; i++) {
        if (!SD.writeRAW(buffer, lba + i)) return -1;
        buffer += sectorSize;
    }
    return bufsize;
}

void startMSC() {
    if (!SD.begin(21)) {
        print("SD not initialized");
        return;
    }
    uint64_t cardSize = SD.cardSize();
    sectorSize = SD.sectorSize();
    sectorCount = SD.numSectors();
    USB.begin();
    msc.vendorID("Seeed");
    msc.productID("XIAO ESP32S3");
    msc.productRevision("1.0");
    msc.onRead(onRead);
    msc.onWrite(onWrite);
    msc.mediaPresent(true);
    msc.begin(sectorCount, sectorSize);

    delay(500);
    print("MSC MODE ACTIVE");
    while (true) {
        delay(1000);
    }
}