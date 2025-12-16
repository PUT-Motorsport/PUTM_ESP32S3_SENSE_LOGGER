void listDir(fs::FS &fs, String dirname) {
    print("LIST: " + dirname);
    File root = fs.open(dirname.c_str());
    if (!root) {
        print("Root file doesnt exist");
        return;
    }
    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            print(" DIR : " + String(file.name()));
        } else {
            print(" FILE: " + String(file.name()));
        }
        file = root.openNextFile();
    }
}

bool removeDir(fs::FS &fs, String path) {
    File dir = fs.open(path.c_str());
    if (!dir) {
        print("Failed to open directory " + path);
        return false;
    }
    if (!dir.isDirectory()) {
        print("Not a directory " + path);
        return false;
    }

    File file = dir.openNextFile();
    while (file) {
        String fileName = String(file.name());
        String fullPath = path + "/" + fileName;
        
        if (file.isDirectory()) {
            file.close(); 
            if (!removeDir(fs, fullPath)) {
                return false;
            }
        } else {
            file.close();
            if (!fs.remove(fullPath.c_str())) {
                print("Delete failed " + fullPath);
                return false;
            }
        }
        file = dir.openNextFile();
    }
    dir.close();
    
    if (fs.rmdir(path.c_str())) {
        print("REMOVED DIR: " + path);
        return true;
    } else {
        print("Removing failed" + path);
        return false;
    }
}

void appendFile(fs::FS &fs, String path, String message) {
    if (message.startsWith("AT:SAVE ")){
      String text = message.substring(8);
      text.trim();
    }
    File file = fs.open(path.c_str(), FILE_APPEND);
    if (!file) {
        print("Cant open file or file doesnt exist");
        return;
    }
    if (!file.print(message.c_str())) {
        print("Cant append to file");
    }
    file.close();
}


void createDir(fs::FS &fs, String path) {
    if (!fs.mkdir(path.c_str())) {
        print("Cant create directory");
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

String dummy(){
    String t = timeStamp();
    t.replace("[", ""); 
    t.replace("]", "");
    t.trim();
    
    String str = t + ";";
    if (activeConfig & 1) {
        float V = random(2200, 2400) / 10.0;
        str += String(V, 1) + ";";
    }
    if (activeConfig & 2) {
        float kWh = random(0, 101) / 10.0;
        str += String(kWh, 2) + ";";
    }
    if (activeConfig & 4) {
        int velocity = random(0, 100);
        str += String(velocity) + ";";
    }
    if (str.endsWith(";")) str.remove(str.length() - 1);
    str += "\r\n";
    return str;
}

String formatBytes(uint64_t bytes) { // prawie to samo co w js
    if (bytes == 0) return "0 B";
    const char* sizes[] = {"B", "KB", "MB", "GB", "TB"};
    int i = 0;
    double dblBytes = bytes;
    while (dblBytes >= 1024 && i < 4) {
        dblBytes /= 1024;
        i++;
    }
    char buf[20];
    sprintf(buf, "%.2f %s", dblBytes, sizes[i]);
    return String(buf);
}

String getStorage() {
    uint64_t total = SD.totalBytes();
    uint64_t used = SD.usedBytes();
    return "Storage: " + formatBytes(used) + " / " + formatBytes(total);
}

String getHeader() {
    String h = "Time;";
    if (activeConfig & 1) h += "VOLTAGE;";
    if (activeConfig & 2) h += "KWH;";
    if (activeConfig & 4) h += "VELOCITY;";
    if (h.endsWith(";")) h.remove(h.length() - 1);
    return h + "\r\n";
}