
/**
 * Created by K. Suwatchai (Mobizt)
 * 
 * Email: k_suwatchai@hotmail.com
 * 
 * Github: https://github.com/mobizt
 * 
 * Copyright (c) 2021 mobizt
 *
*/

/** 
 * This example shows the basic example for automatic plant watering system.
 * The sketch will set GPIO16 for Pump1, and GPIO12 for Pump2
 * The status of pumps showed at /PlantWatering/status
 * 
 * Two pumps will be set to turn on in the moring and evening for 120 second everyday
 * To manually turn on and off both pumps, change the value under /PlantWatering/control
 * 
 * To control the device, send command at /PlantWatering/control/cmd and the result from process
 * showed at /PlantWatering/status/terminal
 * 
 * The command and its description.
 * 
 * idle: nothing to do
 * get-time: get time from NTP server
 * boot: restart the device
 * load-pump: load pump configuration
 * load-schedule: load schedule configuration
 * pump-count: show the number of pumps at /PlantWatering/status/terminal
 * schedule-count: show the number of schedules at /PlantWatering/status/terminal
*/

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <time.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

/* 2. Define the Firebase project host name and API Key */
#define FIREBASE_HOST "PROJECT_ID.firebaseio.com"
#define API_KEY "API_KEY"

/* 3. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"

struct schedule_info_t
{
    int tm_sec;
    int tm_min;
    int tm_hour;
    int duration;
    int state;
    String pump_uid;
    int active;
    int inactive;
};

struct pump_info_t
{
    String id;
    String name;
    String location;
    String uid;
    int state;
    int gpio;
};

FirebaseData fbdo1;
FirebaseData fbdo2;

FirebaseAuth auth;
FirebaseConfig config;

String path = "/PlantWatering";
String Path;
bool timeReady = false;
//Change to match your time zone
float time_zone = 3;
float daylight_offset_in_sec = 0;
char letters[36] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};

std::vector<schedule_info_t> scheduleInfo;
std::vector<pump_info_t> pumpInfo;

void setPumpState(int pumpIndex, int state);
void streamCallback(FirebaseStream data);
void streamTimeoutCallback(bool timeout);
void setClock(float time_zone, float daylight_offset_in_sec);
void addSchedule(String pumpId, int activeState, int inactiveState, int hour, int min, int sec, int duration_in_sec, FirebaseJsonArray *scheduleConfig);
void runSchedule();
void loadSchedule(FirebaseData &data);
void addPump(String id, String name, String location, int gpio, int state, FirebaseJsonArray *pumpConfig);
void loadPump(FirebaseData &data);
String randomUid(uint8_t length);

void setup()
{

    Serial.begin(115200);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    //get time from NTP server and set up
    Serial.println("Get time from NTP server...");
    setClock(time_zone, daylight_offset_in_sec);

    /* Assign the project host and api key (required) */
    config.host = FIREBASE_HOST;
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);

#ifdef ESP8266
    //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
    fbdo.setBSSLBufferSize(1024, 1024);
#endif

    //Set the size of HTTP response buffers in the case where we want to work with large data.
    fbdo2.setResponseSize(1024);
    Path = path + "/control";

    if (!Firebase.RTDB.beginStream(&fbdo1, Path.c_str()))
        Serial.println(fbdo1.errorReason());

    Firebase.RTDB.setStreamCallback(&fbdo1, streamCallback, streamTimeoutCallback);

    Path = path + "/control/cmd";

    Firebase.RTDB.set(&fbdo2, Path.c_str(), "idle");

    Path = path + "/config/pump";

    if (!Firebase.RTDB.pathExisted(&fbdo2, Path.c_str()))
    {
        //Setup initial pump data

        FirebaseJsonArray pumpConfig;

        //pump id, name, location, gpio, state, config
        addPump("P01", "Pump 1", "Garden", 16, 0, &pumpConfig);
        addPump("P02", "Pump 2", "Garden", 12, 0, &pumpConfig);

        Path = path + "/config/pump";

        Firebase.RTDB.set(&fbdo2, Path.c_str(), pumpConfig);
    }
    else
    {
        if (Firebase.RTDB.get(&fbdo2, Path.c_str()))
            loadPump(fbdo2);
    }

    //Set up node for pump if not exist.
    int pumpNum = sizeof(pumpInfo) / sizeof(pumpInfo[0]);
    for (int i = 0; i < pumpNum; i++)
    {
        if (pumpInfo[i].id != "")
        {
            Path = path + "/control/" + pumpInfo[i].id;
            if (!Firebase.RTDB.pathExisted(&fbdo2, Path.c_str()))
            {
                Path = path + "/control/" + pumpInfo[i].id;
                Firebase.RTDB.set(&fbdo2, Path.c_str(), pumpInfo[i].state);
                Firebase.RTDB.set(&fbdo2, Path.c_str(), pumpInfo[i].state);
            }
        }
    }

    if (timeReady)
    {

        Path = path + "/config/schedule";
        if (!Firebase.RTDB.pathExisted(&fbdo2, Path.c_str()))
        {

            Serial.println("Setup schedule...");

            FirebaseJsonArray scheduleConfig;

            //preset index, pump index, activeState, inactiveState, hour, minute, second, duration in second, new setting

            //Set for Pump 1 (P01) to turn on (1) 120 seconds from 7:30:00 then turn off (0)
            addSchedule("P01", 1, 0, 7, 30, 00, 120, &scheduleConfig);
            //Set for Pump1 (P01) to turn on (1) 120 seconds from 17:30:00 then turn off (0)
            addSchedule("P01", 1, 0, 17, 30, 00, 120, &scheduleConfig);

            //Set for Pump2 (P02) to turn on (1) 120 seconds from 7:30:00 then turn off (0)
            addSchedule("P02", 1, 0, 7, 30, 00, 120, &scheduleConfig);
            //Set for Pump2 (P02) to turn on (1) 120 seconds from 17:30:00 then turn off (0)
            addSchedule("P02", 1, 0, 17, 30, 00, 120, &scheduleConfig);
            Path = path + "/config/schedule";
            Firebase.RTDB.set(&fbdo2, Path.c_str(), scheduleConfig);
        }
        else
        {
            Path = path + "/config/schedule";
            if (Firebase.RTDB.get(&fbdo2, Path.c_str()))
                loadSchedule(fbdo2);
        }
    }

    Serial.println("Ready!");
}

void loop()
{
    runSchedule();
}

void setPumpState(int pumpIndex, int state)
{
    int pumpNum = sizeof(pumpInfo) / sizeof(pumpInfo[0]);
    if (pumpIndex < 0 || pumpIndex >= pumpNum)
        return;

    if (pumpInfo[pumpIndex].state == state)
        return;

    digitalWrite(pumpInfo[pumpIndex].gpio, state);
    pumpInfo[pumpIndex].state = state;
    Path = path + "/status/" + pumpInfo[pumpIndex].id;

    Firebase.RTDB.set(&fbdo2, Path.c_str(), state);
    if (state == 0)
        Serial.println(pumpInfo[pumpIndex].id + " OFF");
    else if (state == 1)
        Serial.println(pumpInfo[pumpIndex].id + " ON");
}

void streamCallback(FirebaseStream data)
{

    int pumpNum = sizeof(pumpInfo) / sizeof(pumpInfo[0]);

    if (data.dataType() == "json")
    {
        FirebaseJson *json = data.jsonObjectPtr();
        FirebaseJsonData jsonData;

        for (int i = 0; i < pumpNum; i++)
        {
            //Parse for each pump state
            json->get(jsonData, pumpInfo[i].id);
            setPumpState(i, jsonData.intValue);
        }
    }
    else if (data.dataType() == "int")
    {
        for (int i = 0; i < pumpNum; i++)
        {
            if (data.dataPath() == "/" + pumpInfo[i].id)
            {
                setPumpState(i, data.intData());
                String status = pumpInfo[i].id;
                if (data.intData() == 0)
                    status += " OFF";
                else if (data.intData() == 1)
                    status += " ON";
                Path = path + "/status/terminal";
                Firebase.RTDB.set(&fbdo2, Path.c_str(), status);
            }
        }
    }
    else if (data.dataPath() == "/cmd")
    {
        if (data.stringData() == "get-time")
        {
            Serial.println("cmd: get time from NTP server");
            Path = path + "/status/terminal";
            Firebase.RTDB.set(&fbdo2, Path.c_str(), "get time");
            setClock(time_zone, daylight_offset_in_sec);
        }
        else if (data.stringData() == "load-pump")
        {
            Serial.println("cmd: load pump");
            Path = path + "/status/terminal";
            Firebase.RTDB.set(&fbdo2, Path.c_str(), "load pump");
            Path = path + "/config/pump";
            if (Firebase.RTDB.get(&fbdo2, Path.c_str()))
                loadPump(fbdo2);
        }
        else if (data.stringData() == "load-schedule")
        {
            Serial.println("cmd: load schedule");
            Path = path + "/status/terminal";
            Firebase.RTDB.set(&fbdo2, Path.c_str(), "load schedule");
            Path = path + "/config/schedule";
            if (Firebase.RTDB.get(&fbdo2, Path.c_str()))
                loadSchedule(fbdo2);
        }
        if (data.stringData() == "schedule-count")
        {
            Serial.println("cmd: schedule-count");
            Path = path + "/status/terminal";
            Firebase.RTDB.set(&fbdo2, Path.c_str(), String(scheduleInfo.size()));
        }
        if (data.stringData() == "pump-count")
        {
            Serial.println("cmd: pump-count");
            Path = path + "/status/terminal";
            Firebase.RTDB.set(&fbdo2, Path.c_str(), String(pumpInfo.size()));
        }
        else if (data.stringData() == "boot")
        {
            Serial.println("cmd: reboot device");
            Path = path + "/status/terminal";
            Firebase.RTDB.set(&fbdo2, Path.c_str(), "restart device");
            ESP.restart();
        }
    }
}

void streamTimeoutCallback(bool timeout)
{
    if (timeout)
    {
        //Timeout occurred
    }
}

void setClock(float time_zone, float daylight_offset_in_sec)
{
    configTime(time_zone * 3600, daylight_offset_in_sec, "pool.ntp.org", "time.nist.gov", NULL);
    time_t now = time(nullptr);
    int cnt = 0;

    while (now < 8 * 3600 * 2 && cnt < 20)
    {
        delay(50);
        now = time(nullptr);
        cnt++;
    }

    timeReady = now > 8 * 3600 * 2;
    Path = path + "/status/terminal";
    if (timeReady)
        Firebase.RTDB.set(&fbdo2, Path.c_str(), "idle");
    else
        Firebase.RTDB.set(&fbdo2, Path.c_str(), "cannot get time");
}

void addPump(String id, String name, String location, int gpio, int state, FirebaseJsonArray *pumpConfig)
{
    pump_info_t pinfo;
    pinfo.id = id;
    pinfo.uid = randomUid(10);
    pinfo.name = name;
    pinfo.location = location;
    pinfo.gpio = gpio;
    pinfo.state = state;
    pinMode(gpio, OUTPUT);
    pumpInfo.push_back(pinfo);
    if (pumpConfig)
    {
        FirebaseJson json;
        json.add("id", id);
        json.add("name", name);
        json.add("location", location);
        json.add("uid", pinfo.uid);
        json.add("gpio", gpio);
        json.add("state", state);
        pumpConfig->add(json);
    }
}

void addSchedule(String pumpId, int activeState, int inactiveState, int hour, int min, int sec, int duration_in_sec, FirebaseJsonArray *scheduleConfig)
{
    for (size_t i = 0; i < pumpInfo.size(); i++)
    {
        if (pumpInfo[i].id == pumpId && pumpId != "")
        {
            schedule_info_t tinfo;
            tinfo.tm_hour = hour;
            tinfo.tm_min = min;
            tinfo.tm_sec = sec;
            tinfo.state = 1;
            tinfo.duration = duration_in_sec;
            tinfo.pump_uid = pumpInfo[i].uid;
            tinfo.active = activeState;
            tinfo.inactive = inactiveState;
            scheduleInfo.push_back(tinfo);

            if (scheduleConfig)
            {
                FirebaseJson json;
                json.add("hour", hour);
                json.add("min", min);
                json.add("sec", sec);
                json.add("duration", duration_in_sec);
                json.add("uid", pumpInfo[i].uid);
                json.add("active", activeState);
                json.add("inactive", inactiveState);
                scheduleConfig->add(json);
            }

            break;
        }
    }
}

void runSchedule()
{
    struct tm current_timeinfo;
    struct tm target_timeinfo;
    time_t current_ts = time(nullptr);
    time_t target_ts;
    gmtime_r(&current_ts, &current_timeinfo);
    target_timeinfo.tm_year = current_timeinfo.tm_year;
    target_timeinfo.tm_mon = current_timeinfo.tm_mon;
    target_timeinfo.tm_mday = current_timeinfo.tm_mday;

    for (size_t i = 0; i < scheduleInfo.size(); i++)
    {
        if (scheduleInfo[i].state > 0)
        {
            target_timeinfo.tm_hour = scheduleInfo[i].tm_hour;
            target_timeinfo.tm_min = scheduleInfo[i].tm_min;
            target_timeinfo.tm_sec = scheduleInfo[i].tm_sec;

            target_ts = mktime(&target_timeinfo);

            if (current_ts >= target_ts && current_ts <= target_ts + scheduleInfo[i].duration && scheduleInfo[i].state == 1)
            {
                int index = -1;
                for (size_t j = 0; j < pumpInfo.size(); j++)
                    if (scheduleInfo[i].pump_uid == pumpInfo[j].uid)
                        index = j;

                if (index > -1)
                {
                    if (scheduleInfo[i].active != pumpInfo[index].state)
                    {
                        String status = pumpInfo[index].id + " ON - " + String(scheduleInfo[i].duration) + " sec from ";

                        if (target_timeinfo.tm_hour < 10)
                            status += "0";
                        status + String(target_timeinfo.tm_hour);
                        status += ":";
                        if (target_timeinfo.tm_min < 10)
                            status + "0";
                        status + String(target_timeinfo.tm_min);
                        status += ":";
                        if (target_timeinfo.tm_sec < 10)
                            status += "0";
                        status + String(target_timeinfo.tm_sec);

                        Serial.println(status);

                        scheduleInfo[i].state = 2;
                        Path = path + "/control/" + pumpInfo[index].id;
                        Firebase.RTDB.set(&fbdo2, Path.c_str(), scheduleInfo[i].active);
                        setPumpState(index, scheduleInfo[i].active);
                        Path = path + "/status/terminal";
                        Firebase.RTDB.set(&fbdo2, Path.c_str(), status);
                    }
                }
            }
            else if (current_ts > target_ts + scheduleInfo[i].duration && scheduleInfo[i].state == 2)
            {
                int index = -1;
                for (size_t j = 0; j < pumpInfo.size(); j++)
                    if (scheduleInfo[i].pump_uid == pumpInfo[j].uid)
                        index = j;

                if (index > -1)
                {
                    if (scheduleInfo[i].inactive != pumpInfo[index].state)
                    {
                        scheduleInfo[i].state = 1;
                        Path = path + "/control/" + pumpInfo[index].id;
                        Firebase.RTDB.set(&fbdo2, Path.c_str(), scheduleInfo[i].inactive);
                        setPumpState(index, scheduleInfo[i].inactive);
                        String status = pumpInfo[index].id + " OFF";
                        Path = path + "/status/terminal";
                        Firebase.RTDB.set(&fbdo2, Path.c_str(), status);
                    }
                }
            }
        }
    }
}

void loadSchedule(FirebaseData &data)
{

    scheduleInfo.clear();

    FirebaseJsonArray *scheduleConfig = data.jsonArrayPtr();
    FirebaseJson json;

    Serial.println("Schedule config:");

    for (size_t i = 0; i < scheduleConfig->size(); i++)
    {
        FirebaseJsonData &jsonData = data.jsonData();
        scheduleConfig->get(jsonData, i);
        Serial.println(jsonData.stringValue);
        json.setJsonData(jsonData.stringValue);
        json.get(jsonData, "hour");
        int hour = jsonData.intValue;
        json.get(jsonData, "min");
        int min = jsonData.intValue;
        json.get(jsonData, "sec");
        int sec = jsonData.intValue;
        json.get(jsonData, "duration");
        int duration = jsonData.intValue;
        json.get(jsonData, "uid");
        String uid = jsonData.stringValue;
        json.get(jsonData, "active");
        int act = jsonData.intValue;
        json.get(jsonData, "inactive");
        int ina = jsonData.intValue;
        String id;
        for (size_t i = 0; i < pumpInfo.size(); i++)
            if (uid == pumpInfo[i].uid)
                id = pumpInfo[i].id;

        addSchedule(id, act, ina, hour, min, sec, duration, nullptr);
    }
}

void loadPump(FirebaseData &data)
{

    int pumpNum = sizeof(pumpInfo) / sizeof(pumpInfo[0]);
    for (int i = 0; i < pumpNum; i++)
        pumpInfo[i].id = "";

    FirebaseJsonArray *pumpConfig = data.jsonArrayPtr();
    FirebaseJson json;

    Serial.println("Pump config:");

    for (size_t i = 0; i < pumpConfig->size(); i++)
    {
        FirebaseJsonData &jsonData = data.jsonData();
        pumpConfig->get(jsonData, i);
        Serial.println(jsonData.stringValue);
        json.setJsonData(jsonData.stringValue);
        json.get(jsonData, "id");
        String id = jsonData.stringValue;
        json.get(jsonData, "uid");
        String uid = jsonData.stringValue;
        json.get(jsonData, "name");
        String name = jsonData.stringValue;
        json.get(jsonData, "location");
        String location = jsonData.stringValue;
        json.get(jsonData, "state");
        int state = jsonData.intValue;
        json.get(jsonData, "gpio");
        int gpio = jsonData.intValue;
        addPump(id, name, location, gpio, state, nullptr);
    }
}

String randomUid(uint8_t length)
{
    String randString = "";
    for (uint8_t i = 0; i < length; i++)
        randString = randString + letters[random(0, 36)];
    return randString;
}
