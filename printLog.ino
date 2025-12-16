void print(String msg) {
    String ts = timeStamp();
    webLog += ts + msg + "\n";
    Serial.println(webLog);
    SerialUART.println(webLog);
    if (webLog.length() > 3000) {
        webLog = webLog.substring(webLog.length() - 3000);
    }
}