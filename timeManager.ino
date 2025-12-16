const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 0;


void timeConfig(){
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    struct tm timeinfo;
}
String timeStamp(){
    String timeString = "";
    struct timeval tv;
    if (gettimeofday(&tv, NULL) != 0) {
        timeString ="[--:--:--:--] ";
        return timeString;
    }
    struct tm* timeinfo = localtime(&tv.tv_sec);
    if (timeinfo->tm_year < 116) {
      timeString = "[" + String(millis()) + " ms] ";
    } else {
        char timeBuff[20];
        strftime(timeBuff, sizeof(timeBuff), "%H:%M:%S", timeinfo); //stftime Należy do C dlatego chary musza byc
        int millisec = tv.tv_usec / 1000;
        char msBuff[5];
        sprintf(msBuff, "%03d ", millisec);
        timeString = "[" + String(timeBuff) + ":" + String(msBuff) + "] ";
    }
    return timeString;
}