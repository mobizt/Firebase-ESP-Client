/**
 * FireSense v1.0.15
 *
 * The Programmable Data Logging and IO Control library.
 *
 * This library required FirebaseESP32 or FirebaseESP8266 or Firebase ESP Client to be installed.
 *
 * This library supports Espressif ESP8266, ESP32 and RP2040 Pico
 *
 * Created September 10, 2023
 *
 * This work is a part of Firebase ESP Client library
 * Copyright (c) 2023 K. Suwatchai (Mobizt)
 *
 * The MIT License (MIT)
 * Copyright (c) 2023 K. Suwatchai (Mobizt)
 *
 *
 * Permission is hereby granted, free of charge, to any person returning a copy
 * of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of
 * the Software, and to permit persons to whom the Software is furnished to do
 * so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
 * OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <Arduino.h>
#include "mbfs/MB_MCU.h"
#include "FirebaseFS.h"

#if defined(ENABLE_RTDB) || defined(FIREBASE_ENABLE_RTDB)

#ifndef FireSenseClass_H
#define FireSenseClass_H

#include <Firebase.h>

#ifndef FIREBASE_STREAM_CLASS
#if defined(FIREBASE_ESP_CLIENT)
#define FIREBASE_STREAM_CLASS FirebaseStream
#elif defined(FIREBASE_ESP32_CLIENT) || defined(FIREBASE_ESP8266_CLIENT)
#define FIREBASE_STREAM_CLASS StreamData
#endif
#endif

#if defined(FIREBASE_ESP32_CLIENT) || defined(FIREBASE_ESP8266_CLIENT)
#define FBRTDB Firebase
#define EXT *
#define EXT2 
#elif defined(FIREBASE_ESP_CLIENT)
#define FBRTDB Firebase.RTDB
#define EXT
#define EXT2 &
#endif

using namespace mb_string;

class MB_MillisTimer
{
public:
    MB_MillisTimer(){};

    void setTo(uint32_t timeout)
    {
        tmo = timeout;
    }

    void reset()
    {
        running = false;
        startMillis = 0;
    }

    void start()
    {
        running = true;
        startMillis = millis();
    }

    bool isTo()
    {
        if (startMillis > 0)
        {
            if (millis() - startMillis > tmo)
                return true;
        }

        return false;
    }

    bool isStarted()
    {
        return startMillis > 0;
    }

private:
    bool running = false;
    unsigned long startMillis = 0;
    uint32_t tmo = 0;
};

#if defined(__AVR__)
typedef void (*FireSense_Function)(const char *);
typedef void (*callback_function_t)(void);
#else
typedef struct std::function<void(const char *)> FireSense_Function;
typedef std::function<void(void)> callback_function_t;
#endif

class FireSenseClass
{

public:
    enum data_type_t
    {
        data_type_undefined = -1,
        data_type_byte,
        data_type_bool,
        data_type_int,
        data_type_float,
        data_type_expression
    };

    enum firesense_data_type_t
    {
        Byte,
        Boolean,
        Integer,
        Float
    };

    enum channel_type_t
    {
        Undefined = -1,
        Input = 0,
        Output = 1,
        Analog_input,
        Value
    };

    struct data_value_info_t
    {
        int int_data = 0;
        float float_data = 0;
        data_type_t type = data_type_undefined;
    };

    struct data_value_pointer_info_t
    {
        bool *boolPtr = nullptr;
        uint8_t *bytePtr = nullptr;
        int *intPtr = nullptr;
        float *floatPtr = nullptr;
        data_type_t type = data_type_undefined;
    };

    struct firesense_config_t
    {
        String basePath = "/demo";
        String deviceId = "";
        float time_zone = 0;
        float daylight_offset_in_sec = 0;
        unsigned long last_seen_interval = 60 * 1000;
        unsigned long log_interval = 60 * 1000;
        unsigned long condition_process_interval = 500;
        unsigned long dataRetainingPeriod = 5 * 60;
        uint32_t max_node_to_delete = 10;
        FirebaseData *shared_fbdo = nullptr;
        FirebaseData *stream_fbdo = nullptr;
        bool debug = false;
        bool disable_command = false;
        bool close_session = false;
    };

    struct channel_info_t
    {
        String id;
        String name;
        String location;
        String uid;
        int type = -1;
        int gpio = -1;
        struct data_value_info_t current_value;
        struct data_value_info_t last_value;
        int value_index = -1;
        bool status = false;
        bool log = false;
        firesense_data_type_t unbound_type = Integer;
        unsigned long lastPolling = 0;
        uint32_t pollingInterval = 100;
        bool ready = false;
    };

    struct firesense_condition_t
    {
        String IF;
        String THEN;
        String ELSE;
    };

    FireSenseClass();
    ~FireSenseClass();

    FireSenseClass &operator=(FireSenseClass other);
    FireSenseClass(const FireSenseClass &other);

    /** Initiate the FireSense Class.
     *
     * @param config The pointer to Firesense_Config data.
     * @param databaseSecret The database secret.
     *
     * @return Boolean value, indicates the success of the operation.
     *
     * @note Due to query index need to be assign to the database rules, the admin rights access is needed
     * The database secret can be empty string if the sign-in sign-in method is OAuth2.0.
     */
    bool begin(struct firesense_config_t *config, const char *databaseSecret);

    /** Load the device configurations (deprecated).
     *
     * @param defaultDataLoadCallback The callback function that called when no config found in the database.
     *
     * @note The callback function should add the channals or conditions manually or load from the device storage.
     *
     * Due to limited stack memory (4 kB) in ESP8266, this function can cause the stack overflow and should not use.
     *
     * Use the loadConfig function without callback instead.
     */
    void loadConfig(callback_function_t defaultDataLoadCallback);

    /** Load the device configurations and return false when no config loaded.
     *
     * @return boolean status of operation.
     *
     * @note The function returns false when no config found in database and the default config loading function should be called.
     *
     * After the default config loaded, the function updateConfig should be called.
     */
    bool loadConfig();

    /** Update the config after loaded.
     */
    void updateConfig();

    /** Save the current config to the device storage.
     *
     * @param filename The file path includes its name of file that will be saved.
     * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
     *
     * @return Boolean value, indicates the success of the operation.
     *
     */
    bool backupConfig(const String &filename, firebase_mem_storage_type storageType);

    /** Read the config from the device storage.
     *
     * @param filename The file path includes its name of file that will be read.
     * @param storageType The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.
     *
     * @return Boolean value, indicates the success of the operation.
     *
     */
    bool restoreConfig(const String &filename, firebase_mem_storage_type storageType);

    /** Enable (run) or disable (stop) the conditions checking tasks.
     *
     * @param enable The boolean value to enable/disable.
     *
     */
    void enableController(bool enable);

    /** Add a channel to device config.
     *
     * @param channel The FireSense_Channel data to add.
     * @param addToDatabase The boolean option, set to true to add the data to database.
     *
     */
    void addChannel(struct channel_info_t &channel, bool addToDatabase = true);

    /** Add a condition to device config.
     *
     * @param cond The FireSense_Condition data to add.
     * @param addToDatabase The boolean option, set to true to add the data to database.
     *
     * The format of conditions (IF) and its expression.
     *
     * CONDITION1 + && or || LOGICAL OPERATOR + CONDITION2 + LOGICAL OPERATOR + CONDITION3 +...
     *
     * The condition checking and expression evaluation are from left to right
     *
     *
     * The valid left, right operands and syntaxes are
     *
     * Operand and Syntaxes                                  Usages
     *
     * <channel id>                                         LED1 == false && STATUS == LED1
     * values e.g. boolean, integer and float               HUMID1 > 70 || LAMP1 == false
     * millis                                               millis > 200000 + VALUE1
     * micros                                               VALUE1 < micros - 1000000
     * time e.g. hour:min:sec                               time > 12:00:00 && time < 15:30:00
     * date e.g. month/day/year where month start with 0    date == 5/28/2022
     * weekday e.g. 1 for monday and 7 for sunday           weekday == 5
     * day e.g. 1 to 31                                     day > 24
     * month e.g. 0 to 11                                   month < 11
     * year e.g. 2022                                       year == 2022
     * hour e.g. 0 to 23                                    hour == 18
     * min  e.g. 0 to 59                                    min == 30
     * sec  e.g. 0 to 59                                    sec == 20
     * change e.g the value of channel changed              change(VALUE1)
     * ! e.g. the opposite of expresion result              !LED1 || !(time > 15:20:06)
     *
     *
     * The format of statements (THEN and ELSE) and its expression.
     *
     * STATEMENT1 + COMMA + STATEMENT2 +...
     *
     * The statement processing and expression evaluation are from left to right.
     *
     * The valid left, right operands and syntaxes are
     *
     * Operand and Syntaxes                                  Usages
     *
     * <channel id>                                         LED1 = false, STATUS = 5 * 10
     * values e.g. boolean, integer and float               HUMID1 = 70
     * millis                                               VALUE1 += millis
     * micros                                               VALUE1 *= micros
     * delay                                                delay(1000), LED1 = true
     *                                                      ;do non-blocking delay until timed out and set LED1 to true
     *
     * func  e.g. func(x,y,z)                               func(0,10,'hello world')
     * where x is the index of callback function added      ;send the hello world text 10 times to function with index 0
     * with FireSense.addCallbackFunction
     * y is the number of iteration that function will be
     * called as long as the conditions is true
     * z is the message payload sent to the callback.
     * The content of payload other than alphabet (A-Z, a-z
     * and 1-9) should be in ''.
     *
     * Use {CHANNEL_ID} to insert the channel value into
     * the text payload.
     *
     * The supported assignment operators are
     * +=, -=, *=, /=, &=, |=
     *
     *
     * The supported comparision operators are
     * ==, !=, >, <, >=, <=
     *
     */
    void addCondition(struct firesense_condition_t cond, bool addToDatabase = true);

    /** Add a callback function used with func syntax in the conditions.
     *
     * @param func The FireSense_Function callback.
     *
     */
    void addCallbackFunction(FireSense_Function func);

    /** Clear all callback functions used with func syntax in the conditions.
     *
     */
    void clearAllCallbackFunctions();

    /** Add a pointer of uint8_t (byte) variable that bind to the channels.
     *
     * @param value The uint8_t variable.
     *
     */
    void addUserValue(uint8_t *value);

    /** Add a pointer of bool variable that bind to the channel.
     *
     * @param value The bool variable.
     *
     */
    void addUserValue(bool *value);

    /** Add a pointer of int variable that bind to the channel.
     *
     * @param value The int variable.
     *
     */
    void addUserValue(int *value);

    /** Add a pointer of float variable that bind to the channel.
     *
     * @param value The float variable.
     *
     */
    void addUserValue(float *value);

    /** Clear all user variable pointers that bound to the channels.
     *
     */
    void clearAllUserValues();

    /** Get the devivce id string.
     *
     * @return The unique id String of device.
     *
     */
    String getDeviceId();

    /** Run the schedule tasks.
     *
     */
    void run();

    void readStream(FIREBASE_STREAM_CLASS *data);
    void readStream(FirebaseData *fbdo);
    bool configExisted();

private:
    // condition's group comparison operator type
    enum next_comp_opr_t
    {
        next_comp_opr_none = -1,
        next_comp_opr_or,
        next_comp_opr_and
    };

    enum statement_type_t
    {
        statement_type_then,
        statement_type_else

    };

    enum assignment_operator_type_t
    {
        assignment_operator_type_undefined = -1,
        assignment_operator_type_assignment,
        assignment_operator_type_add,
        assignment_operator_type_subtract,
        assignment_operator_type_multiply,
        assignment_operator_type_division,
        assignment_operator_type_remainder,
        assignment_operator_type_left_shift,
        assignment_operator_type_right_shift,
        assignment_operator_type_and,
        assignment_operator_type_or,
        assignment_operator_type_logic_and,
        assignment_operator_type_logic_or
    };

    // condition's operand type
    enum cond_operand_type_t
    {
        cond_operand_type_undefined = -1,
        cond_operand_type_date = 0,
        cond_operand_type_time,
        cond_operand_type_day,
        cond_operand_type_month,
        cond_operand_type_year,
        cond_operand_type_weekday,
        cond_operand_type_hour,
        cond_operand_type_min,
        cond_operand_type_sec,
        cond_operand_type_changed,
        cond_operand_type_millis,
        cond_operand_type_micros,
        cond_operand_type_channel,
        cond_operand_type_expression,
    };

    // expression's operand type
    enum expr_operand_type_t
    {
        expr_operand_type_undefined = -1,
        expr_operand_type_channel,
        expr_operand_type_value,
        expr_operand_type_millis,
        expr_operand_type_micros
    };

    // conddition's comparision operator type
    enum cond_comp_opr_type_t
    {
        cond_comp_opr_type_undefined,
        cond_comp_opr_type_lt,
        cond_comp_opr_type_gt,
        cond_comp_opr_type_lteq,
        cond_comp_opr_type_gteq,
        cond_comp_opr_type_eq,
        cond_comp_opr_type_neq
    };

    // statement's operand type
    enum stm_operand_type_t
    {
        stm_operand_type_undefined = -1,
        stm_operand_type_function,
        stm_operand_type_delay,
        stm_operand_type_channel,
        stm_operand_type_expression
    };

    // expression item data
    struct expr_item_data_t
    {
        expr_operand_type_t type = expr_operand_type_undefined;
        struct channel_info_t *channel = nullptr;
        struct data_value_info_t value;
        bool not_op = false;
    };

    struct expression_item_info_t
    {
        struct expr_item_data_t data;
        bool is_nested = false;
        int depth = 0;
        struct data_value_info_t result;
        bool not_op = false;
        assignment_operator_type_t next_ass_opr = assignment_operator_type_undefined;
        MB_VECTOR<struct expression_item_info_t> list;
    };

    struct expressions_info_t
    {
        MB_VECTOR<struct expression_item_info_t> expressions;
        struct data_value_info_t result;
    };

    // condition's left operand
    struct cond_left_oprand_item_t
    {
        struct expressions_info_t exprs;
        cond_operand_type_t type = cond_operand_type_undefined;
        struct channel_info_t *channel = nullptr;
        bool not_op = false;
        struct tm time;
    };

    // condition's right operand
    struct cond_right_oprand_item_t
    {
        struct expressions_info_t exprs;
        cond_operand_type_t type = cond_operand_type_undefined;
        struct channel_info_t *channel = nullptr;
        bool not_op = false;
    };

    // condition item data
    struct cond_item_data_t
    {
        struct cond_left_oprand_item_t left;
        struct cond_right_oprand_item_t right;
        cond_comp_opr_type_t comp = cond_comp_opr_type_undefined;
    };

    struct function_info_t
    {
        FireSense_Function *ptr = nullptr;
        int numArg = 0;
        MB_String payload;
        int iteration_max = 1;
        int iteration_count = 0;
    };

    struct stm_left_operand_item_t
    {
        stm_operand_type_t type = stm_operand_type_undefined;
        struct channel_info_t *channel = nullptr;
        struct function_info_t function;
        int tmo = -1;
        MB_MillisTimer timer;
    };

    struct stm_right_operand_item_t
    {
        struct expressions_info_t exprs;
        stm_operand_type_t type = stm_operand_type_undefined;
        struct channel_info_t *channel = nullptr;
    };

    struct stm_item_t
    {
        struct stm_left_operand_item_t left;
        struct stm_right_operand_item_t right;
        assignment_operator_type_t ass = assignment_operator_type_undefined;
    };

    struct condition_item_info_t
    {
        // MB_String raw;
        struct cond_item_data_t data;
        bool is_nested = false;
        int depth = 0;
        bool result = false;
        bool not_op = false;
        next_comp_opr_t next_comp_opr = next_comp_opr_none;
        MB_VECTOR<struct condition_item_info_t> list;
    };

    struct expression_group_item_t
    {
        struct data_value_info_t result;
        assignment_operator_type_t next_ass_opr = assignment_operator_type_undefined;
    };

    struct statement_item_info_t
    {
        struct stm_item_t data;
        bool done = false;
    };

    struct conditions_info_t
    {
        MB_VECTOR<struct condition_item_info_t> conditions = MB_VECTOR<struct condition_item_info_t>();
        MB_VECTOR<struct statement_item_info_t> thenStatements = MB_VECTOR<struct statement_item_info_t>();
        MB_VECTOR<struct statement_item_info_t> elseStatements = MB_VECTOR<struct statement_item_info_t>();
        bool result = false;
    };

    MB_VECTOR<struct channel_info_t> channelsList = MB_VECTOR<struct channel_info_t>();
    MB_VECTOR<struct data_value_pointer_info_t> userValueList = MB_VECTOR<struct data_value_pointer_info_t>();
    MB_VECTOR<FireSense_Function> functionList = MB_VECTOR<FireSense_Function>();
    MB_VECTOR<struct conditions_info_t> conditionsList = MB_VECTOR<struct conditions_info_t>();

    struct firesense_config_t *config;

    callback_function_t _callback_function = NULL;
    FirebaseJson _json;
    FirebaseJsonData result;

    bool timeReady = false;
    bool initializing = false;
    bool loadingConfig = false;
    bool loadingCondition = false;
    bool conditionsLoaded = false;
    bool loadingStatus = false;
    bool sendingLog = false;
    bool config_existed = false;
    bool initReady = false;
    bool configLoadReady = false;
    bool streamPause = false;
    bool controllerEnable = true;
    const char *databaseSecret = "";

    callback_function_t defaultDataLoadCallback = NULL;

    MB_String streamCmd;
    unsigned long lastSeenMillis = 0;
    unsigned long logMillis = 0;
    unsigned long conditionMillis = 0;
    unsigned long authen_check_millis = 0;
    time_t minTs = FIREBASE_DEFAULT_TS;
    uint64_t maxTs = 32503654800;
    MB_String deviceId;

    MB_String controlPath();
    MB_String channelControlPath();
    MB_String configPath();
    MB_String conditionPath();
    MB_String channelConfigPath();
    MB_String streamCmdPath();
    MB_String statusPath();
    MB_String terminalPath();
    MB_String channelStatusPath();
    MB_String lastSeenPath();
    MB_String logPath();
    bool mBegin();
    void setLogQueryIndex();
    void addDBChannel(struct channel_info_t &channel);
    void updateDBStatus(struct channel_info_t &channel);
    void storeDBStatus();
    void parseCondition(const char *src, MB_VECTOR<struct condition_item_info_t> &conditions, int depth = 0);
    void parseExpression(const char *src, MB_VECTOR<struct expression_item_info_t> &expressions, int depth = 0);
    void parseStatement(const char *src, MB_VECTOR<struct statement_item_info_t> &stm);
    int getCompareConditionType(const char c);
    int getAssignOprType(const char c);
    void loadConditionsList();
    void loadStatus();
    void setChannalValueType();
    void setChannelValue(struct channel_info_t &channel, struct data_value_info_t value);
    void setUserValue(struct channel_info_t *channel, bool fromUserValue, struct data_value_info_t val);
    bool configReady();
    void setClock(float time_zone, float daylight_offset_in_sec);
    void clearLog();
    struct data_value_info_t getChannelValue(struct channel_info_t *channel);
    void resetStatement(struct conditions_info_t *conditionsListItem, statement_type_t type);
    void executeStatement(struct conditions_info_t *conditionsListItem, statement_type_t type);
    void assignDataValue(struct data_value_info_t *lvalue, struct data_value_info_t *rvalue, assignment_operator_type_t ass, bool setType, bool rvalTypeCheck);
    void assignNotValue(struct data_value_info_t *rvalue);
    void evalExpressionsList(struct expressions_info_t *exprList);
    void evalExpressionsItem(struct expression_item_info_t *cond);
    int isDigit(const char *str);
    void testConditionsList();
    void testConditionItem(struct condition_item_info_t *cond);
    void restart();
    void checkCommand();
    void checkInput();
    void sendLog();
    void sendLastSeen();
    void sendReadyStatus();
    void getCondition(const char *s, struct cond_item_data_t &data);
    void getExpression(const char *s, struct expr_item_data_t &data);
    void getStatement(const char *src, struct stm_item_t &data);
    void parseDateTime(const char *str, int type, struct tm &out);
    void getConditionItem(struct cond_item_data_t &data, MB_String &left, MB_String &right);
    void replaceAll(MB_String &str, const char *find, const char *replace);
    void replaceChannelsValues(MB_String &str);
    void trim(const char *s, MB_String &d, bool isExpression, const char beginTrim = '(', const char endTrim = ')');
    void split(MB_VECTOR<MB_String> &out, const char *str, const char delim, const char beginEsc = 0, const char endEsc = 0);
    void getChipId(String &s);
    void randomUid(uint8_t length, String &s);
    void getDateTimeString(MB_String &s);
    void setupStream();
    void mRun();
    void printError(FirebaseData *fbdo);
    void printUpdate(const char *msg, int type, float value = 0);
    void pauseStream();
    void unpauseStream();
    void copyConstructor(const FireSenseClass &other);

    const char *months(int index)
    {
        switch (index)
        {
        case 0:
            return (const char *)FPSTR("Jan");
        case 1:
            return (const char *)FPSTR("Feb");
        case 2:
            return (const char *)FPSTR("Mar");
        case 3:
            return (const char *)FPSTR("Apr");
        case 4:
            return (const char *)FPSTR("May");
        case 5:
            return (const char *)FPSTR("Jun");
        case 6:
            return (const char *)FPSTR("Jul");
        case 7:
            return (const char *)FPSTR("Aug");
        case 8:
            return (const char *)FPSTR("Sep");
        case 9:
            return (const char *)FPSTR("Oct");
        case 10:
            return (const char *)FPSTR("Nov");
        case 11:
            return (const char *)FPSTR("Dec");

        default:
            return (const char *)FPSTR("Dec");
        }
    }

    const char *sdow(int index)
    {
        switch (index)
        {
        case 0:
            return (const char *)FPSTR("Sun");
        case 1:
            return (const char *)FPSTR("Mon");
        case 2:
            return (const char *)FPSTR("Tue");
        case 3:
            return (const char *)FPSTR("Wed");
        case 4:
            return (const char *)FPSTR("Thu");
        case 5:
            return (const char *)FPSTR("Fri");
        case 6:
            return (const char *)FPSTR("Sat");

        default:
            return (const char *)FPSTR("Sat");
        }
    }

#if defined(ESP8266)
    void set_scheduled_callback(callback_function_t callback);
#endif
};

FireSenseClass FireSense;

#if defined(ESP32)
TaskHandle_t firesense_run_task_handle = NULL;
#endif
typedef struct FireSenseClass::channel_info_t FireSense_Channel;
typedef enum FireSenseClass::channel_type_t Firesense_Channel_Type;
typedef struct FireSenseClass::firesense_condition_t FireSense_Condition;
typedef struct FireSenseClass::firesense_config_t Firesense_Config;
typedef enum FireSenseClass::firesense_data_type_t FireSense_Data_Type;

FireSenseClass::FireSenseClass()
{
}

FireSenseClass::~FireSenseClass()
{
}

FireSenseClass &FireSenseClass::operator=(FireSenseClass other)
{
    copyConstructor(other);
    return *this;
}

FireSenseClass::FireSenseClass(const FireSenseClass &other)
{
    copyConstructor(other);
}

void FireSenseClass::copyConstructor(const FireSenseClass &other)
{
    channelsList = other.channelsList;
    userValueList = other.userValueList;
    functionList = other.functionList;
    conditionsList = other.conditionsList;
    config = new struct firesense_config_t();
    *config = *other.config;
    _callback_function = other._callback_function;
    result = other.result;
    timeReady = other.timeReady;
    initializing = other.initializing;
    loadingConfig = other.loadingConfig;
    loadingCondition = other.loadingCondition;
    conditionsLoaded = other.conditionsLoaded;
    loadingStatus = other.loadingStatus;
    sendingLog = other.sendingLog;
    config_existed = other.config_existed;
    initReady = other.initReady;
    configLoadReady = other.configLoadReady;
    streamPause = other.streamPause;
    controllerEnable = other.controllerEnable;
    databaseSecret = other.databaseSecret;
    defaultDataLoadCallback = other.defaultDataLoadCallback;
    streamCmd = other.streamCmd;
    lastSeenMillis = other.lastSeenMillis;
    logMillis = other.logMillis;
    conditionMillis = other.conditionMillis;
    authen_check_millis = other.authen_check_millis;
    deviceId = other.deviceId;
}

void FireSenseClass::sendReadyStatus()
{
    if (!configReady())
        return;

    FBRTDB.setAsync(EXT config->shared_fbdo, streamCmdPath().c_str(), (const char *)FPSTR("Idle"));
    FBRTDB.setAsync(EXT config->shared_fbdo, terminalPath().c_str(), (const char *)FPSTR("Ready"));

    if (config->close_session)
        config->shared_fbdo->clear();
}

bool FireSenseClass::configExisted()
{
    if (!configReady())
        return false;
    config_existed = FBRTDB.pathExisted(EXT config->shared_fbdo, configPath().c_str());

    if (config->close_session)
        config->shared_fbdo->clear();

    return config_existed;
}

__attribute__((__used__)) static void FiresenseStreamCB(FIREBASE_STREAM_CLASS data)
{
    FireSense.readStream(&data);
}

__attribute__((__used__)) static void FiresenseStreamToCB(bool timeout)
{
    if (timeout)
    {
    }
}

bool FireSenseClass::begin(struct firesense_config_t *config, const char *databaseSecret)
{
    initReady = false;

    this->config = config;
    this->databaseSecret = databaseSecret;

    if (!configReady())
    {
        Serial.println(F("No FireSense config assigned."));
        return false;
    }

#ifdef ESP32
    if (!Firebase.ready())
    {
        printUpdate("", 44);
        while (!Firebase.ready())
        {
            delay(0);
        }
    }
#endif

    printUpdate("", 3);

    if (this->config->basePath == "")
        this->config->basePath = (const char *)FPSTR("/demo");

    if (this->config->deviceId == "")
    {
        String s;
        getChipId(s);
        deviceId = s.c_str();
    }
    else
    {
        deviceId = this->config->deviceId.c_str();
        deviceId += (const char *)FPSTR("-");
        String s;
        getChipId(s);
        deviceId += s.c_str();
    }

    if (!config->shared_fbdo)
    {
        printUpdate("", 5);
        return false;
    }

    if (this->config->shared_fbdo)
        this->config->shared_fbdo->setBSSLBufferSize(2048, 512);
    if (this->config->stream_fbdo)
        this->config->stream_fbdo->setBSSLBufferSize(2048, 512);

    this->config->shared_fbdo->setResponseSize(1024);
    if (this->config->stream_fbdo)
        this->config->stream_fbdo->setResponseSize(1024);

    return mBegin();
}

bool FireSenseClass::mBegin()
{
    if (initializing)
        return false;

    initializing = true;
    initReady = false;

    if (!Firebase.ready())
    {
        printUpdate("", 7);
        initializing = false;
        return false;
    }

    if (Firebase.getCurrentTime() < minTs)
    {
        printUpdate("", 14);
        setClock(config->time_zone, config->daylight_offset_in_sec);
    }

    printUpdate("", 15);

    configExisted();

    if (!Firebase.authenticated())
    {
        printUpdate("", 16);
        initializing = false;
        return false;
    }

    if (config_existed)
        printUpdate("", 17);

    setupStream();

    setLogQueryIndex();

    printUpdate("", 18);

    initReady = true;

    initializing = false;

    return true;
}

void FireSenseClass::enableController(bool enable)
{
    controllerEnable = enable;
}

void FireSenseClass::setupStream()
{

    delay(0);

    // No need to run stream check at this time
    pauseStream();

    printUpdate("", 19);
    if (config->stream_fbdo)
    {
        if (!FBRTDB.beginStream(EXT config->stream_fbdo, controlPath().c_str()))
            printError(config->stream_fbdo);
        else
            printUpdate("", 21);
    }
    else
    {

        if (!FBRTDB.beginStream(EXT config->shared_fbdo, controlPath().c_str()))
            printError(config->stream_fbdo);
        else
            printUpdate("", 21);
    }
#if defined(ESP32) || defined(ESP8266)
    if (config->stream_fbdo && !config->disable_command)
        FBRTDB.setStreamCallback(EXT config->stream_fbdo, FiresenseStreamCB, FiresenseStreamToCB);
#endif
}

void FireSenseClass::addCallbackFunction(FireSense_Function func)
{
    functionList.push_back(func);
}

void FireSenseClass::clearAllCallbackFunctions()
{
    functionList.clear();
}

void FireSenseClass::addUserValue(uint8_t *value)
{
    struct data_value_pointer_info_t ptr;
    ptr.bytePtr = value;
    ptr.type = data_type_byte;
    userValueList.push_back(ptr);
}

void FireSenseClass::addUserValue(bool *value)
{
    struct data_value_pointer_info_t ptr;
    ptr.boolPtr = value;
    ptr.type = data_type_bool;
    userValueList.push_back(ptr);
}

void FireSenseClass::addUserValue(int *value)
{
    struct data_value_pointer_info_t ptr;
    ptr.intPtr = value;
    ptr.type = data_type_int;
    userValueList.push_back(ptr);
}

void FireSenseClass::addUserValue(float *value)
{
    struct data_value_pointer_info_t ptr;
    ptr.floatPtr = value;
    ptr.type = data_type_float;
    userValueList.push_back(ptr);
}

void FireSenseClass::clearAllUserValues()
{
    userValueList.clear();
}

void FireSenseClass::clearLog()
{
    if (!configReady())
        return;

    printUpdate("", 22);
    if (!FBRTDB.deleteNode(EXT config->shared_fbdo, logPath().c_str()))
        printError(config->shared_fbdo);

    if (config->close_session)
        config->shared_fbdo->clear();

    sendReadyStatus();
}

void FireSenseClass::setChannalValueType()
{
    for (size_t i = 0; i < channelsList.size(); i++)
    {
        struct channel_info_t *channel = &channelsList[i];

        if (channel->value_index > -1)
        {
            for (size_t j = 0; j < userValueList.size(); j++)
            {
                if (channel->value_index == (int)j)
                {
                    struct data_value_pointer_info_t *valPtr = &userValueList[j];
                    if (valPtr->type == data_type_float)
                    {
                        channel->current_value.type = data_type_float;
                        channel->last_value.type = data_type_float;
                    }
                    else
                    {
                        channel->current_value.type = data_type_int;
                        channel->last_value.type = data_type_int;
                    }

                    break;
                }
            }
        }
        else
        {
            // The type of unbound variable channel
            channel->current_value.type = (data_type_t)channel->unbound_type;
            channel->last_value.type = (data_type_t)data_type_float;
        }
    }
}

void FireSenseClass::resetStatement(struct conditions_info_t *conditionsListItem, statement_type_t type)
{
    MB_VECTOR<statement_item_info_t> *stms = nullptr;
    if (type == statement_type_then)
        stms = &conditionsListItem->thenStatements;
    else if (type == statement_type_else)
        stms = &conditionsListItem->elseStatements;

    for (size_t i = 0; i < stms->size(); i++)
    {

#if defined(MB_USE_STD_VECTOR)
        statement_item_info_t *statement = &(*(stms->begin() + i));
#else
        statement_item_info_t *statement = &((*stms)[i]);
#endif

        if (statement->data.left.type == stm_operand_type_delay)
        {
            statement->done = false;
            statement->data.left.timer.reset();
        }
        else if (statement->data.left.type == stm_operand_type_function)
        {
            statement->data.left.function.iteration_count = 0;
            statement->done = false;
        }
    }
}

struct FireSenseClass::data_value_info_t FireSenseClass::getChannelValue(struct channel_info_t *channel)
{
    struct data_value_info_t v;
    if (!channel->ready)
    {
        if (channel->type == channel_type_t::Input || channel->type == channel_type_t::Output)
        {
            if (channel->type == channel_type_t::Output)
                pinMode(channel->gpio, OUTPUT);
            else
                pinMode(channel->gpio, INPUT);
            channel->ready = true;
        }
    }

    if (channel->type == channel_type_t::Input)
    {
        v.type = channel->current_value.type;
        v.int_data = digitalRead(channel->gpio);
        v.float_data = (float)v.int_data;
    }
    else if (channel->type == channel_type_t::Analog_input)
    {
        v.type = channel->current_value.type;
        v.int_data = analogRead(channel->gpio);
        v.float_data = (float)v.int_data;
    }
    else
        v = channel->current_value;

    return v;
}

void FireSenseClass::executeStatement(struct conditions_info_t *conditionsListItem, statement_type_t type)
{
    if (!timeReady)
        return;

    if (!Firebase.ready())
        return;

    bool timerDone = false;
    MB_VECTOR<statement_item_info_t> *stms = nullptr;
    if (type == statement_type_then)
        stms = &conditionsListItem->thenStatements;
    else if (type == statement_type_else)
        stms = &conditionsListItem->elseStatements;

    for (size_t i = 0; i < stms->size(); i++)
    {
        if (!timeReady)
            return;

        delay(0);

#if defined(MB_USE_STD_VECTOR)
        statement_item_info_t *statement = &(*(stms->begin() + i));
#else
        statement_item_info_t *statement = &((*stms)[i]);
#endif
        if (statement->data.left.type == stm_operand_type_delay)
        {

            if (!statement->data.left.timer.isStarted())
            {
                statement->data.left.timer.setTo(statement->data.left.tmo);
                statement->data.left.timer.start();
            }

            if (!statement->data.left.timer.isTo())
                break;

            if (!statement->done)
                timerDone = true;

            statement->done = true;
        }
        else if (statement->data.left.type == stm_operand_type_function)
        {
            if (timerDone)
            {
                statement->data.left.function.iteration_count = 0;
                statement->done = false;
            }

            if (statement->done && statement->data.left.function.iteration_max > 0 && statement->data.left.function.iteration_count >= statement->data.left.function.iteration_max)
                continue;

            statement->data.left.function.iteration_count++;

            // call user function
            if (statement->data.left.function.ptr || statement->data.left.function.numArg > 0)
            {
                statement->done = true;
                MB_String s = statement->data.left.function.payload;
                replaceChannelsValues(s);
                replaceAll(s, (const char *)FPSTR("\\n"), (const char *)FPSTR("\n"));

                if (statement->data.left.function.ptr)
                    (*statement->data.left.function.ptr)(s.c_str());
            }
        }
        else if (statement->data.left.type == stm_operand_type_channel)
        {
            // update channel value
            struct data_value_info_t rvalue;

            if (statement->data.right.type == stm_operand_type_channel)
                rvalue = getChannelValue(statement->data.right.channel);
            else if (statement->data.right.type == stm_operand_type_expression)
            {
                evalExpressionsList(&statement->data.right.exprs);
                rvalue = statement->data.right.exprs.result;
            }

            if (statement->data.left.type == stm_operand_type_channel)
            {
                if (statement->data.left.channel->type == channel_type_t::Value)
                {
                    if (statement->data.ass == assignment_operator_type_assignment && statement->data.left.channel->current_value.type == rvalue.type)
                    {
                        if (statement->data.left.channel->current_value.type == data_type_float)
                        {
                            if (statement->data.left.channel->current_value.float_data == rvalue.float_data)
                                continue;
                        }
                        else if (statement->data.left.channel->current_value.int_data == rvalue.int_data)
                            continue;
                    }

                    assignDataValue(&statement->data.left.channel->current_value, &rvalue, statement->data.ass, false, false);

                    if (statement->data.left.channel)
                        setUserValue(statement->data.left.channel, false, statement->data.left.channel->current_value);

                    if (statement->data.left.channel->status)
                    {
                        MB_String path = channelStatusPath();
                        path += (const char *)FPSTR("/");
                        path += statement->data.left.channel->id.c_str();

                        printUpdate(statement->data.left.channel->id.c_str(), 0);
                        if (statement->data.left.channel->current_value.type == data_type_float)
                        {
                            if (!FBRTDB.setAsync(EXT config->shared_fbdo, path.c_str(), statement->data.left.channel->current_value.float_data))
                                printError(config->shared_fbdo);
                        }
                        else
                        {
                            if (!FBRTDB.setAsync(EXT config->shared_fbdo, path.c_str(), statement->data.left.channel->current_value.int_data))
                                printError(config->shared_fbdo);
                        }

                        if (config->close_session)
                            config->shared_fbdo->clear();
                    }
                }
                else if (statement->data.left.channel->type == channel_type_t::Output)
                    setChannelValue(*statement->data.left.channel, rvalue);
            }
        }
    }
}

void FireSenseClass::assignNotValue(struct data_value_info_t *rvalue)
{
    rvalue->int_data = rvalue->int_data > 0 ? 0 : 1;
    rvalue->float_data = (float)rvalue->int_data;
}

void FireSenseClass::assignDataValue(struct data_value_info_t *lvalue, struct data_value_info_t *rvalue, assignment_operator_type_t ass, bool setType, bool rvalTypeCheck)
{
    data_type_t vtype = rvalTypeCheck ? rvalue->type : lvalue->type;

    switch (ass)
    {
    case assignment_operator_type_assignment:

        if (vtype == data_type_float)
        {
            lvalue->float_data = rvalue->float_data;
            lvalue->int_data = (int)lvalue->float_data;
        }
        else
        {
            lvalue->int_data = rvalue->int_data;
            lvalue->float_data = (float)lvalue->int_data;
        }
        if (setType)
            lvalue->type = rvalue->type;
        break;
    case assignment_operator_type_add:
        if (vtype == data_type_float)
        {
            lvalue->float_data += rvalue->float_data;
            lvalue->int_data = (int)lvalue->float_data;
        }
        else
        {
            lvalue->int_data += rvalue->int_data;
            lvalue->float_data = (float)lvalue->int_data;
        }
        if (setType)
            lvalue->type = rvalue->type;
        break;
    case assignment_operator_type_subtract:
        if (vtype == data_type_float)
        {
            lvalue->float_data -= rvalue->float_data;
            lvalue->int_data = (int)lvalue->float_data;
        }
        else
        {
            lvalue->int_data -= rvalue->int_data;
            lvalue->float_data = (float)lvalue->int_data;
        }
        if (setType)
            lvalue->type = rvalue->type;
        break;
    case assignment_operator_type_multiply:
        lvalue->float_data *= rvalue->float_data;
        lvalue->int_data = (int)lvalue->float_data;
        if (setType)
            lvalue->type = data_type_float;
        break;
    case assignment_operator_type_division:
        lvalue->float_data /= rvalue->float_data;
        lvalue->int_data = (int)lvalue->float_data;
        if (setType)
            lvalue->type = data_type_float;
        break;
    case assignment_operator_type_remainder:
        lvalue->int_data = lvalue->int_data % rvalue->int_data;
        lvalue->float_data = (float)lvalue->int_data;
        if (setType)
            lvalue->type = data_type_int;
        break;
    case assignment_operator_type_left_shift:
        lvalue->int_data = lvalue->int_data << rvalue->int_data;
        lvalue->float_data = (float)lvalue->int_data;
        if (setType)
            lvalue->type = data_type_int;
        break;
    case assignment_operator_type_right_shift:
        lvalue->int_data = lvalue->int_data >> rvalue->int_data;
        lvalue->float_data = (float)lvalue->int_data;
        if (setType)
            lvalue->type = data_type_int;
        break;
    case assignment_operator_type_and:
        if (vtype == data_type_float)
        {
            lvalue->float_data = lvalue->float_data && rvalue->float_data;
            lvalue->int_data = (int)lvalue->float_data;
        }
        else
        {
            lvalue->int_data = lvalue->int_data && rvalue->int_data;
            lvalue->float_data = (float)lvalue->int_data;
        }
        break;
    case assignment_operator_type_or:
        if (vtype == data_type_float)
        {
            lvalue->float_data = lvalue->float_data || rvalue->float_data;
            lvalue->int_data = (int)lvalue->float_data;
        }
        else
        {
            lvalue->int_data = lvalue->int_data || rvalue->int_data;
            lvalue->float_data = (float)lvalue->int_data;
        }
        break;
    case assignment_operator_type_logic_and:
        lvalue->int_data = lvalue->int_data & rvalue->int_data;
        lvalue->float_data = (float)lvalue->int_data;
        if (setType)
            lvalue->type = data_type_int;
        break;
    case assignment_operator_type_logic_or:
        lvalue->int_data = lvalue->int_data | rvalue->int_data;
        lvalue->float_data = (float)lvalue->int_data;
        if (setType)
            lvalue->type = data_type_int;
        break;
    default:
        break;
    }
}

void FireSenseClass::testConditionItem(struct condition_item_info_t *cond)
{
    if (!timeReady)
        return;

    if (!Firebase.ready())
        return;

    bool result = false;
    cond->result = false;
    delay(0);

    if (cond->data.left.type == cond_operand_type_date || cond->data.left.type == cond_operand_type_time || cond->data.left.type == cond_operand_type_day || cond->data.left.type == cond_operand_type_weekday || cond->data.left.type == cond_operand_type_year || cond->data.left.type == cond_operand_type_month || cond->data.left.type == cond_operand_type_hour || cond->data.left.type == cond_operand_type_min || cond->data.left.type == cond_operand_type_sec)
    {
        time_t current_ts = Firebase.getCurrentTime();
        time_t target_ts = 0;
        struct tm current_timeinfo;
        localtime_r(&current_ts, &current_timeinfo);

        if (cond->data.left.type == cond_operand_type_day || cond->data.left.type == cond_operand_type_weekday || cond->data.left.type == cond_operand_type_year || cond->data.left.type == cond_operand_type_month || cond->data.left.type == cond_operand_type_hour || cond->data.left.type == cond_operand_type_min || cond->data.left.type == cond_operand_type_sec)
        {
            if (cond->data.left.type == cond_operand_type_day)
            {
                target_ts = cond->data.left.time.tm_mday;
                current_ts = current_timeinfo.tm_mday;
            }
            else if (cond->data.left.type == cond_operand_type_weekday)
            {
                target_ts = cond->data.left.time.tm_wday;
                current_ts = current_timeinfo.tm_wday;
                if (current_ts == 0)
                    current_ts = 7;
            }
            else if (cond->data.left.type == cond_operand_type_year)
            {
                target_ts = cond->data.left.time.tm_year;
                current_ts = current_timeinfo.tm_year;
            }
            else if (cond->data.left.type == cond_operand_type_month)
            {
                target_ts = cond->data.left.time.tm_mon;
                current_ts = current_timeinfo.tm_mon;
            }
            else if (cond->data.left.type == cond_operand_type_hour)
            {
                target_ts = cond->data.left.time.tm_hour;
                current_ts = current_timeinfo.tm_hour;
            }
            else if (cond->data.left.type == cond_operand_type_min)
            {
                target_ts = cond->data.left.time.tm_min;
                current_ts = current_timeinfo.tm_min;
            }
            else if (cond->data.left.type == cond_operand_type_sec)
            {
                target_ts = cond->data.left.time.tm_sec;
                current_ts = current_timeinfo.tm_sec;
            }
        }
        else
        {
            struct tm target_timeinfo = cond->data.left.time;

            if (cond->data.left.time.tm_year == -1)
                target_timeinfo.tm_year = current_timeinfo.tm_year;
            if (cond->data.left.time.tm_mon == -1)
                target_timeinfo.tm_mon = current_timeinfo.tm_mon;
            if (cond->data.left.time.tm_mday == -1)
                target_timeinfo.tm_mday = current_timeinfo.tm_mday;

            if (cond->data.left.time.tm_hour == -1)
                target_timeinfo.tm_hour = current_timeinfo.tm_hour;
            if (cond->data.left.time.tm_min == -1)
                target_timeinfo.tm_min = current_timeinfo.tm_min;
            if (cond->data.left.time.tm_sec == -1)
                target_timeinfo.tm_sec = current_timeinfo.tm_sec;

            target_ts = mktime(&target_timeinfo);
        }

        if (cond->data.left.not_op)
            target_ts = target_ts > 0 ? 0 : 1;

        if (cond->data.comp == cond_comp_opr_type_lt)
            result = current_ts < target_ts;
        else if (cond->data.comp == cond_comp_opr_type_gt)
            result = current_ts > target_ts;
        else if (cond->data.comp == cond_comp_opr_type_lteq)
            result = current_ts <= target_ts;
        else if (cond->data.comp == cond_comp_opr_type_gteq)
            result = current_ts >= target_ts;
        else if (cond->data.comp == cond_comp_opr_type_eq)
            result = current_ts == target_ts;
        else if (cond->data.comp == cond_comp_opr_type_neq)
            result = current_ts != target_ts;

        if (cond->not_op)
            result = !result;
    }
    else
    {
        if (cond->data.left.type == cond_operand_type_millis || cond->data.left.type == cond_operand_type_micros || cond->data.left.type == cond_operand_type_expression || cond->data.left.type == cond_operand_type_channel || cond->data.left.type == cond_operand_type_changed)
        {
            struct data_value_info_t lvalue;
            struct data_value_info_t rvalue;
            if (cond->data.left.type == cond_operand_type_changed && cond->data.left.channel)
            {
                lvalue = cond->data.left.channel->current_value;
                rvalue = cond->data.left.channel->last_value;

                if (cond->data.left.not_op)
                    assignNotValue(&lvalue);

                result = (lvalue.int_data != rvalue.int_data || lvalue.float_data != rvalue.float_data);
            }
            else
            {
                if (cond->data.left.type == cond_operand_type_channel && cond->data.left.channel)
                    lvalue = cond->data.left.channel->current_value;
                else if (cond->data.left.type == cond_operand_type_millis)
                {
                    lvalue.int_data = millis();
                    lvalue.float_data = (float)lvalue.int_data;
                    lvalue.type = data_type_int;
                }
                else if (cond->data.left.type == cond_operand_type_micros)
                {
                    lvalue.int_data = micros();
                    lvalue.float_data = (float)lvalue.int_data;
                    lvalue.type = data_type_int;
                }
                else if (cond->data.left.type == cond_operand_type_expression)
                {
                    evalExpressionsList(&cond->data.left.exprs);
                    lvalue = cond->data.left.exprs.result;
                }

                if (cond->data.left.not_op)
                    assignNotValue(&lvalue);

                if (cond->data.right.type == cond_operand_type_channel && cond->data.right.channel)
                    rvalue = cond->data.right.channel->current_value;
                else if (cond->data.right.type == cond_operand_type_millis)
                {
                    rvalue.int_data = millis();
                    rvalue.float_data = (float)rvalue.int_data;
                }
                else if (cond->data.right.type == cond_operand_type_micros)
                {
                    rvalue.int_data = micros();
                    rvalue.float_data = (float)rvalue.int_data;
                }
                else if (cond->data.right.type == cond_operand_type_expression)
                {
                    evalExpressionsList(&cond->data.right.exprs);
                    rvalue = cond->data.right.exprs.result;
                }
                else if (cond->data.right.type == cond_operand_type_undefined)
                    result = lvalue.int_data > 0;

                if (cond->data.right.not_op)
                    assignNotValue(&rvalue);

                if (cond->data.comp == cond_comp_opr_type_lt)
                {
                    if (lvalue.type == data_type_float)
                        result = (lvalue.float_data < rvalue.float_data);
                    else
                        result = (lvalue.int_data < rvalue.int_data);
                }
                else if (cond->data.comp == cond_comp_opr_type_gt)
                {
                    if (lvalue.type == data_type_float)
                        result = (lvalue.float_data > rvalue.float_data);
                    else
                        result = (lvalue.int_data > rvalue.int_data);
                }
                else if (cond->data.comp == cond_comp_opr_type_lteq)
                {
                    if (lvalue.type == data_type_float)
                        result = (lvalue.float_data <= rvalue.float_data);
                    else
                        result = (lvalue.int_data <= rvalue.int_data);
                }
                else if (cond->data.comp == cond_comp_opr_type_gteq)
                {
                    if (lvalue.type == data_type_float)
                        result = (lvalue.float_data >= rvalue.float_data);
                    else
                        result = (lvalue.int_data >= rvalue.int_data);
                }
                else if (cond->data.comp == cond_comp_opr_type_eq)
                {
                    if (lvalue.type == data_type_float)
                        result = (lvalue.float_data == rvalue.float_data);
                    else
                        result = (lvalue.int_data == rvalue.int_data);
                }
                else if (cond->data.comp == cond_comp_opr_type_neq)
                {
                    if (lvalue.type == data_type_float)
                        result = (lvalue.float_data != rvalue.float_data);
                    else
                        result = (lvalue.int_data != rvalue.int_data);
                }
            }

            if (cond->not_op)
                result = !result;
        }
    }

    if (cond->list.size() > 0)
    {
        bool res = false;

        next_comp_opr_t next_comp_opr = next_comp_opr_none;

        for (size_t i = 0; i < cond->list.size(); i++)
        {
            delay(0);
            struct condition_item_info_t *_cond = &cond->list[i];

            testConditionItem(_cond);

            if (i == 0)
                res = _cond->result;

            if (next_comp_opr == next_comp_opr_or)
            {
                res |= _cond->result;
                if (res)
                    break;
            }
            else if (next_comp_opr == next_comp_opr_and)
                res &= _cond->result;

            next_comp_opr = _cond->next_comp_opr;
            if (next_comp_opr == next_comp_opr_or && res)
                break;
        }

        if (cond->next_comp_opr == next_comp_opr_or)
            result |= res;
        else if (cond->next_comp_opr == next_comp_opr_and)
            result &= res;
        else
            result = res;
    }

    cond->result = result;
}

int FireSenseClass::isDigit(const char *str)
{
    int dot = 0;
    for (size_t i = 0; i < strlen(str); i++)
    {
        if (i == 0 && str[i] == '-')
            continue;

        if (str[i] == '.')
            dot++;

        if (dot > 1 || (!isdigit(str[i] && str[i] != '-')))
            return -1;
    }

    return dot;
}

void FireSenseClass::evalExpressionsList(struct expressions_info_t *exprList)
{
    if (!timeReady)
        return;

    if (!Firebase.ready())
        return;

    MB_VECTOR<struct expression_group_item_t> resList;

    struct expression_group_item_t r;

    for (size_t i = 0; i < exprList->expressions.size(); i++)
    {
        if (!timeReady)
            break;

        delay(0);
        struct expression_item_info_t *expr = &exprList->expressions[i];

        evalExpressionsItem(expr);

        if (expr->not_op)
            assignNotValue(&expr->result);

        if (i == 0)
            assignDataValue(&r.result, &expr->result, assignment_operator_type_assignment, true, true);
        else
        {
            if (r.next_ass_opr == assignment_operator_type_add || r.next_ass_opr == assignment_operator_type_subtract)
            {
                resList.push_back(r);
                assignDataValue(&r.result, &expr->result, assignment_operator_type_assignment, true, true);
            }
            else
                assignDataValue(&r.result, &expr->result, r.next_ass_opr, true, true);
        }

        r.next_ass_opr = expr->next_ass_opr;
    }

    if (resList.size() > 0)
    {
        assignment_operator_type_t next_ass_opr = assignment_operator_type_undefined;

        for (size_t i = 0; i < resList.size(); i++)
        {
            if (i == 0)
                assignDataValue(&exprList->result, &resList[i].result, assignment_operator_type_assignment, true, true);
            else
            {
                if (next_ass_opr == assignment_operator_type_add || next_ass_opr == assignment_operator_type_subtract)
                    assignDataValue(&exprList->result, &resList[i].result, next_ass_opr, true, true);
            }
            next_ass_opr = resList[i].next_ass_opr;
        }

        if (next_ass_opr == assignment_operator_type_add || next_ass_opr == assignment_operator_type_subtract)
            assignDataValue(&exprList->result, &r.result, next_ass_opr, true, true);
    }
    else
        assignDataValue(&exprList->result, &r.result, assignment_operator_type_assignment, true, true);
}

void FireSenseClass::evalExpressionsItem(struct expression_item_info_t *expr)
{
    if (!timeReady)
        return;

    if (!Firebase.ready())
        return;

    delay(0);

    if (expr->list.size() == 0)
    {
        if (expr->data.type == expr_operand_type_channel && expr->data.channel)
            assignDataValue(&expr->result, &expr->data.channel->current_value, assignment_operator_type_assignment, true, true);
        else if (expr->data.type == expr_operand_type_millis)
        {
            expr->result.int_data = millis();
            expr->result.type = data_type_int;
            expr->result.float_data = (float)expr->result.int_data;
        }
        else if (expr->data.type == expr_operand_type_micros)
        {
            expr->result.int_data = micros();
            expr->result.type = data_type_int;
            expr->result.float_data = (float)expr->result.int_data;
        }
        else if (expr->data.type == expr_operand_type_value)
            assignDataValue(&expr->result, &expr->data.value, assignment_operator_type_assignment, true, true);

        if (expr->data.not_op)
        {
            expr->result.int_data = expr->result.int_data > 0 ? 0 : 1;
            expr->result.float_data = (float)expr->result.int_data;
        }
    }
    else
    {
        MB_VECTOR<struct expression_group_item_t> resList;

        struct expression_group_item_t r;

        for (size_t i = 0; i < expr->list.size(); i++)
        {
            delay(0);
            struct expression_item_info_t *_expr = &expr->list[i];
            evalExpressionsItem(_expr);
            if (i == 0)
                assignDataValue(&r.result, &_expr->result, assignment_operator_type_assignment, true, true);
            else
            {
                if (r.next_ass_opr == assignment_operator_type_add || r.next_ass_opr == assignment_operator_type_subtract)
                {
                    resList.push_back(r);
                    assignDataValue(&r.result, &_expr->result, assignment_operator_type_assignment, true, true);
                }
                else
                    assignDataValue(&r.result, &_expr->result, r.next_ass_opr, true, true);
            }
            r.next_ass_opr = _expr->next_ass_opr;
        }

        if (resList.size() > 0)
        {
            assignment_operator_type_t next_ass_opr = assignment_operator_type_undefined;

            for (size_t i = 0; i < resList.size(); i++)
            {
                if (i == 0)
                    assignDataValue(&expr->result, &resList[i].result, assignment_operator_type_assignment, true, true);
                else if (next_ass_opr == assignment_operator_type_add || next_ass_opr == assignment_operator_type_subtract)
                    assignDataValue(&expr->result, &resList[i].result, next_ass_opr, true, true);

                next_ass_opr = resList[i].next_ass_opr;
            }

            if (next_ass_opr == assignment_operator_type_add || next_ass_opr == assignment_operator_type_subtract)
                assignDataValue(&expr->result, &r.result, next_ass_opr, true, true);
        }
        else
            assignDataValue(&expr->result, &r.result, assignment_operator_type_assignment, true, true);
    }
}

void FireSenseClass::testConditionsList()
{
    if (!configReady())
        return;

    if (!Firebase.ready())
        return;

    if (!timeReady || conditionsList.size() == 0 || loadingCondition)
        return;

    if (millis() - conditionMillis > config->condition_process_interval || conditionMillis == 0)
    {
        conditionMillis = millis();

        for (size_t i = 0; i < conditionsList.size(); i++)
        {
            if (!timeReady)
                break;
            delay(0);
            struct conditions_info_t *listItem = &conditionsList[i];
            listItem->result = false;

            next_comp_opr_t next_comp_opr = next_comp_opr_none;

            for (size_t j = 0; j < listItem->conditions.size(); j++)
            {
                if (!timeReady)
                    break;

                delay(0);
                struct condition_item_info_t *condition = &listItem->conditions[j];

                testConditionItem(condition);

                if (j == 0)
                    listItem->result = condition->result;

                if (next_comp_opr == next_comp_opr_or)
                    listItem->result |= condition->result;
                else if (next_comp_opr == next_comp_opr_and)
                    listItem->result &= condition->result;
                else
                    listItem->result = condition->result;

                next_comp_opr = condition->next_comp_opr;
                if (next_comp_opr == next_comp_opr_or && listItem->result)
                    break;
            }

            if (listItem->result)
            {
                resetStatement(listItem, statement_type_else);
                executeStatement(listItem, statement_type_then);
            }
            else
            {
                resetStatement(listItem, statement_type_then);
                executeStatement(listItem, statement_type_else);
            }
        }

        for (size_t i = 0; i < channelsList.size(); i++)
            channelsList[i].last_value = channelsList[i].current_value;
    }
}
void FireSenseClass::pauseStream()
{
    if (!configReady() || !config->stream_fbdo)
        return;

    if (!config->stream_fbdo->isPause())
    {
        streamPause = true;
        config->stream_fbdo->pauseFirebase(true);
    }
}

void FireSenseClass::unpauseStream()
{
    if (!configReady() || !config->stream_fbdo)
        return;

    if (config->stream_fbdo->isPause())
    {
        streamPause = false;
        config->stream_fbdo->pauseFirebase(false);
    }
}

void FireSenseClass::mRun()
{
    delay(0);
    if (configReady())
    {
        time_t now = Firebase.getCurrentTime();

        timeReady = now > minTs && Firebase.ready();

        if (Firebase.ready())
        {

            if (!config_existed && timeReady && initReady && configLoadReady && millis() - authen_check_millis > 10000)
            {
                authen_check_millis = millis();
                configExisted();
            }

            if (Firebase.authenticated() && timeReady && initReady && configLoadReady)
            {
                if (streamPause)
                    unpauseStream();

#if !defined(ESP32)
                checkCommand();
#endif
                checkInput();

                if (controllerEnable)
                {

                    if (!conditionsLoaded)
                        loadConditionsList();
                    testConditionsList();
                }

                sendLog();
                sendLastSeen();

                if (!config->stream_fbdo && !config->disable_command)
                    readStream(config->shared_fbdo);
            }
        }
    }
}

void FireSenseClass::run()
{
    delay(0);

#if defined(FB_ENABLE_EXTERNAL_CLIENT)
    mRun();
#elif defined(ESP32)

    if (firesense_run_task_handle)
        return;

    mRun();

    static FireSenseClass *_this = this;

    TaskFunction_t taskCode = [](void *param)
    {
        for (;;)
        {
            _this->mRun();
            yield();
            vTaskDelay(5 / portTICK_PERIOD_MS);
        }

        firesense_run_task_handle = NULL;
        vTaskDelete(NULL);
    };

    xTaskCreatePinnedToCore(taskCode, "firesense_run_task", 8192, NULL, 10, &firesense_run_task_handle, 1);

#elif defined(ESP8266)
    mRun();
    set_scheduled_callback(std::bind(&FireSenseClass::run, this));
#endif
}

void FireSenseClass::restart()
{
    if (!configReady())
        return;
    if (config->debug)
        FBRTDB.setAsync(EXT config->shared_fbdo, terminalPath().c_str(), "restarting device...");
#if defined(ESP32) || defined(ESP8266)
    ESP.restart();
#endif
}

void FireSenseClass::checkCommand()
{
    if (!configReady())
        return;

    if (streamCmd.length() == 0 || streamCmd == (const char *)FPSTR("Idle"))
        return;

    // get NTP time
    if (streamCmd == (const char *)FPSTR("time"))
        setClock(config->time_zone, config->daylight_offset_in_sec);
    else if (streamCmd.find("time ") != MB_String::npos)
    {
        // set system time with timestamp
        int t = atoi(streamCmd.substr(9).c_str());
        if (t > (int)minTs && t < (int)maxTs)
        {
            timeReady = false;
            Firebase.setSystemTime(t);

            if (Firebase.getCurrentTime() > minTs && Firebase.ready())
                timeReady = true;
        }
    }
    else if (streamCmd == (const char *)FPSTR("config"))
        // load config
        configLoadReady = false;
    else if (streamCmd == (const char *)FPSTR("condition"))
        // load conditions
        conditionsLoaded = false;
    else if (streamCmd == (const char *)FPSTR("run"))
        // run conditions test
        controllerEnable = true;
    else if (streamCmd == (const char *)FPSTR("stop"))
        // stop conditions test
        controllerEnable = false;

    else if (streamCmd == (const char *)FPSTR("store"))
        // store the channels status
        storeDBStatus();
    else if (streamCmd == (const char *)FPSTR("restore"))
        // restore the channels status
        loadStatus();
    else if (streamCmd == (const char *)FPSTR("clear"))
        // clear log
        clearLog();
    else if (streamCmd == (const char *)FPSTR("ping"))
    {
        // ping back
        MB_String s = (const char *)FPSTR("reply");

        s += (uint64_t)Firebase.getCurrentTime();

        FBRTDB.setAsync(EXT config->shared_fbdo, terminalPath().c_str(), s.c_str());
        if (config->close_session)
            config->shared_fbdo->clear();
    }
    else if (streamCmd == (const char *)FPSTR("restart"))
        // restart device
        restart();

    else if (config->debug)
    {
        FBRTDB.setAsync(EXT config->shared_fbdo, terminalPath().c_str(), "Unknown command!");

        if (config->close_session)
            config->shared_fbdo->clear();
    }

    FBRTDB.setAsync(EXT config->shared_fbdo, streamCmdPath().c_str(), (const char *)FPSTR("Idle"));
    if (config->close_session)
        config->shared_fbdo->clear();
    streamCmd.clear();
}

void FireSenseClass::checkInput()
{
    if (loadingConfig || loadingCondition || loadingStatus || sendingLog)
        return;

    for (size_t i = 0; i < channelsList.size(); i++)
    {
        if (channelsList[i].type == channel_type_t::Input || channelsList[i].type == channel_type_t::Analog_input || channelsList[i].type == channel_type_t::Value)
            setChannelValue(channelsList[i], channelsList[i].current_value);
    }
}

void FireSenseClass::sendLog()
{

    if (!configReady())
        return;

    if (loadingConfig || loadingCondition || loadingStatus || sendingLog)
        return;

    if (millis() - logMillis > config->log_interval || logMillis == 0)
    {
        sendingLog = true;

        logMillis = millis();
        bool logEnable = false;
        for (size_t i = 0; i < channelsList.size(); i++)
        {
            if (channelsList[i].log)
            {
                logEnable = true;
                break;
            }
        }

        if (logEnable)
        {
            printUpdate("", 20);
            if (!FBRTDB.deleteNodesByTimestamp(EXT config->shared_fbdo, logPath().c_str(), (const char *)FPSTR("time"), config->max_node_to_delete, config->dataRetainingPeriod))
                printError(config->shared_fbdo);

            if (config->close_session)
                config->shared_fbdo->clear();

            _json.clear();
            time_t ts = 0;

            ts = Firebase.getCurrentTime();

            size_t count = 0;
            for (size_t i = 0; i < channelsList.size(); i++)
            {
                if (channelsList[i].log)
                {
                    count++;
                    struct data_value_info_t val = getChannelValue(&channelsList[i]);
                    if (val.type == data_type_float)
                        _json.add(channelsList[i].id, val.float_data);
                    else
                        _json.add(channelsList[i].id, val.int_data);
                }
            }

            if (count > 0)
            {
                _json.add((const char *)FPSTR("time"), (int)ts);
                MB_String path = logPath();
                path += (const char *)FPSTR("/");
                path += (uint64_t)ts;

                printUpdate("", 1);
                if (!FBRTDB.updateNode(EXT config->shared_fbdo, path.c_str(), EXT2 _json))
                    printError(config->shared_fbdo);

                if (config->close_session)
                    config->shared_fbdo->clear();
            }

            _json.clear();
        }
        sendingLog = false;
    }
}

void FireSenseClass::sendLastSeen()
{
    if (millis() - lastSeenMillis > config->last_seen_interval || lastSeenMillis == 0)
    {

        lastSeenMillis = millis();

        if (timeReady)
        {
            _json.clear();
            MB_String s;
            getDateTimeString(s);
            _json.add((const char *)FPSTR("date"), s.c_str());

            _json.add((const char *)FPSTR("ts"), (uint64_t)Firebase.getCurrentTime());

            printUpdate("", 2);
            if (!FBRTDB.updateNode(EXT config->shared_fbdo, lastSeenPath().c_str(), EXT2 _json))
                printError(config->shared_fbdo);

            if (config->close_session)
                config->shared_fbdo->clear();
            sendReadyStatus();
        }
    }
}

bool FireSenseClass::configReady()
{
    if (!config)
        return false;

    if (!config->shared_fbdo)
        return false;

    return true;
}

void dummy()
{
}

void FireSenseClass::loadConfig(callback_function_t defaultDataLoadCallback)
{
    Serial.println("******************************************************************************");
    Serial.println("**** This function was deprecated, please check the new examples updates. *****");
    Serial.println("******************************************************************************");

    this->defaultDataLoadCallback = defaultDataLoadCallback;
    loadConfig();
}

void FireSenseClass::updateConfig()
{
    printUpdate("", 23);
    setChannalValueType();
    loadStatus();
    conditionsLoaded = true;
    loadingConfig = false;
    printUpdate("", 24);
    configLoadReady = true;
    run();
}

bool FireSenseClass::loadConfig()
{
    if (!configReady() || !Firebase.ready() || !Firebase.authenticated())
    {
        run();
        return true;
    }

    if (loadingConfig)
        return false;

    loadingConfig = true;

    if (!configExisted())
    {
        if (defaultDataLoadCallback)
        {
            printUpdate("", 23);
            defaultDataLoadCallback();
            setChannalValueType();
            loadStatus();
            conditionsLoaded = true;
            loadingConfig = false;
            printUpdate("", 24);
            configLoadReady = true;
            run();
            return true;
        }
        loadingConfig = false;
        return false;
    }

    if (!Firebase.authenticated())
    {
        loadingConfig = false;
        return false;
    }

    printUpdate("", 25);

    if (config->debug)
        FBRTDB.setAsync(EXT config->shared_fbdo, terminalPath().c_str(), "Loading database config...");

    if (!FBRTDB.getShallowData(EXT config->shared_fbdo, channelConfigPath().c_str()))
    {
        loadingConfig = false;
        printUpdate("", 26);

        if (config->close_session)
            config->shared_fbdo->clear();
        return false;
    }

    FirebaseJson *js = config->shared_fbdo->to<FirebaseJson *>();
    MB_VECTOR<String> channelIdxs;

    if (js)
    {
        size_t len = js->iteratorBegin();
        FirebaseJson::IteratorValue value;
        for (size_t i = 0; i < len; i++)
        {
            value = js->valueAt(i);
            channelIdxs.push_back(value.key);
        }
        js->iteratorEnd();
    }

    channelsList.clear();

    for (size_t i = 0; i < channelIdxs.size(); i++)
    {
        MB_String path = channelConfigPath();
        path += (const char *)FPSTR("/");
        path += channelIdxs[i].c_str();
        printUpdate("", 27);
        if (!FBRTDB.get(EXT config->shared_fbdo, path.c_str()))
        {
            printError(config->shared_fbdo);
            loadingConfig = false;

            if (config->close_session)
                config->shared_fbdo->clear();
            return false;
        }

        struct channel_info_t channel;

        FirebaseJson *json = config->shared_fbdo->to<FirebaseJson *>();
        json->get(result, (const char *)FPSTR("id"));
        channel.id = result.to<String>();
        json->get(result, (const char *)FPSTR("uid"));
        channel.uid = result.to<String>();
        json->get(result, (const char *)FPSTR("name"));
        channel.name = result.to<String>();
        json->get(result, (const char *)FPSTR("location"));
        channel.location = result.to<String>();
        json->get(result, (const char *)FPSTR("gpio"));
        channel.gpio = result.to<int>();
        json->get(result, (const char *)FPSTR("type"));
        channel.type = result.to<int>();
        json->get(result, (const char *)FPSTR("utype"));
        channel.unbound_type = (firesense_data_type_t)result.to<int>();
        json->get(result, (const char *)FPSTR("vIndex"));
        channel.value_index = result.to<int>();
        json->get(result, (const char *)FPSTR("status"));
        channel.status = result.to<bool>();
        json->get(result, (const char *)FPSTR("log"));
        channel.log = result.to<bool>();
        delay(0);

        if (config->close_session)
            config->shared_fbdo->clear();
        addChannel(channel, false);
    }

    setChannalValueType();
    loadStatus();

    if (controllerEnable)
        loadConditionsList();

    printUpdate("", 24);

    loadingConfig = false;
    configLoadReady = true;
    run();

    return true;
}

bool FireSenseClass::backupConfig(const String &filename, firebase_mem_storage_type storageType)
{
    if (!configReady())
        return false;

    if (loadingConfig || loadingCondition || loadingStatus || sendingLog)
        return false;

    printUpdate("", 40);
    delay(0);
    if (!FBRTDB.backup(EXT config->shared_fbdo, storageType, configPath().c_str(), filename.c_str()))
    {
        printError(config->shared_fbdo);

        if (config->close_session)
            config->shared_fbdo->clear();
        return false;
    }
    return true;
}

bool FireSenseClass::restoreConfig(const String &filename, firebase_mem_storage_type storageType)
{
    if (!configReady())
        return false;

    if (loadingConfig || loadingCondition || loadingStatus || sendingLog)
        return false;

    delay(0);
    printUpdate("", 41);
    if (!FBRTDB.restore(EXT config->shared_fbdo, storageType, configPath().c_str(), filename.c_str()))
    {
        printError(config->shared_fbdo);

        if (config->close_session)
            config->shared_fbdo->clear();
        return false;
    }
    return true;
}

void FireSenseClass::setClock(float time_zone, float daylight_offset_in_sec)
{
    if (!configReady())
        return;

    if (loadingConfig || loadingCondition || loadingStatus || sendingLog)
        return;

    time_t now = Firebase.getCurrentTime();
    timeReady = now > minTs && Firebase.ready();
#if defined(ESP32) || defined(ESP8266)
    if (now < minTs)
        configTime(time_zone * 3600, config->daylight_offset_in_sec, (const char *)FPSTR("pool.ntp.org"), (const char *)FPSTR("time.nist.gov"), NULL);
#endif
    now = Firebase.getCurrentTime();

    timeReady = now > minTs && Firebase.ready();

    if (timeReady)
    {
        _json.clear();
        MB_String s;
        getDateTimeString(s);
        _json.add((const char *)FPSTR("date"), s.c_str());

        _json.add((const char *)FPSTR("ts"), (uint64_t)Firebase.getCurrentTime());

        printUpdate("", 2);
        if (!FBRTDB.updateNode(EXT config->shared_fbdo, lastSeenPath().c_str(), EXT2 _json) || !FBRTDB.set(EXT config->shared_fbdo, terminalPath().c_str(), (int)now))
            printError(config->shared_fbdo);
        _json.clear();
    }
    else
    {
        if (config->debug)
            FBRTDB.setAsync(EXT config->shared_fbdo, terminalPath().c_str(), "Acquiring NTP time...");
    }

    if (config->close_session)
        config->shared_fbdo->clear();
}

void FireSenseClass::addCondition(struct firesense_condition_t cond, bool addToDatabase)
{
    if (!configReady())
        return;

    struct conditions_info_t conds;
    delay(0);

    if (cond.IF.length() > 0 && cond.THEN.length() > 0)
    {
        printUpdate("", 28);
        parseCondition(cond.IF.c_str(), conds.conditions);
        printUpdate("", 29);
        parseStatement(cond.THEN.c_str(), conds.thenStatements);
    }

    if (cond.IF.length() > 0 && cond.THEN.length() > 0 && cond.ELSE.length() > 0)
    {
        printUpdate("", 30);
        parseStatement(cond.ELSE.c_str(), conds.elseStatements);
    }

    if (cond.IF.length() > 0)
        conditionsList.push_back(conds);

    delay(0);
    if (addToDatabase)
    {
        _json.clear();
        if (cond.IF.length() > 0)
            _json.add((const char *)FPSTR("if"), cond.IF);
        if (cond.THEN.length() > 0)
            _json.add((const char *)FPSTR("then"), cond.THEN);
        if (cond.ELSE.length() > 0)
            _json.add((const char *)FPSTR("else"), cond.ELSE);
        MB_String path = conditionPath();
        path += (const char *)FPSTR("/");
        path += String(conditionsList.size() - 1).c_str();
        delay(0);

        printUpdate("", 31);
        if (!FBRTDB.updateNode(EXT config->shared_fbdo, path.c_str(), EXT2 _json))
            printError(config->shared_fbdo);

        if (config->close_session)
            config->shared_fbdo->clear();

        cond.IF.remove(1, cond.IF.length());
        cond.THEN.remove(1, cond.THEN.length());
        cond.ELSE.remove(1, cond.ELSE.length());
    }
}

void FireSenseClass::loadConditionsList()
{
    if (!configReady())
        return;

    if (loadingCondition)
        return;

    loadingCondition = true;

    printUpdate("", 32);

    conditionsList.clear();

    if (config->debug)
        FBRTDB.setAsync(EXT config->shared_fbdo, terminalPath().c_str(), "Loading conditions...");

    printUpdate("", 42);
    if (!FBRTDB.getShallowData(EXT config->shared_fbdo, conditionPath().c_str()))
    {
        printError(config->shared_fbdo);
        loadingCondition = false;

        if (config->close_session)
            config->shared_fbdo->clear();
        return;
    }

    FirebaseJson *js = config->shared_fbdo->to<FirebaseJson *>();
    MB_VECTOR<String> conditionIds;

    if (js)
    {
        size_t len = js->iteratorBegin();
        FirebaseJson::IteratorValue value;
        for (size_t i = 0; i < len; i++)
        {
            value = js->valueAt(i);
            conditionIds.push_back(value.key);
        }
        js->iteratorEnd();
    }

    if (conditionIds.size() == 0)
    {
        printUpdate("", 33);
        loadingCondition = false;
        conditionsLoaded = true;
        return;
    }

    for (size_t i = 0; i < conditionIds.size(); i++)
    {
        MB_String path = conditionPath();
        path += (const char *)FPSTR("/");
        path += conditionIds[i].c_str();

        printUpdate(String(i).c_str(), 34);
        if (!FBRTDB.get(EXT config->shared_fbdo, path.c_str()))
        {
            printError(config->shared_fbdo);

            if (config->close_session)
                config->shared_fbdo->clear();
            loadingCondition = false;
            return;
        }

        struct firesense_condition_t cond;
        FirebaseJson *json = config->shared_fbdo->to<FirebaseJson *>();
        json->get(result, (const char *)FPSTR("if"));
        cond.IF = result.to<String>();
        json->get(result, (const char *)FPSTR("then"));
        cond.THEN = result.to<String>();
        json->get(result, (const char *)FPSTR("else"));
        cond.ELSE = result.to<String>();

        if (config->close_session)
            config->shared_fbdo->clear();
        addCondition(cond, false);
    }

    sendReadyStatus();
    printUpdate("", 35);
    loadingCondition = false;
    conditionsLoaded = true;
}

#if defined(ESP8266)
void FireSenseClass::set_scheduled_callback(callback_function_t callback)
{
    _callback_function = std::move([callback]()
                                   { schedule_function(callback); });
    _callback_function();
}
#endif

void FireSenseClass::printError(FirebaseData *fbdo)
{
    if (!configReady())
        return;

    if (!config->debug)
        return;

    Serial.print(F("[DEBUG] Failed, "));
    Serial.println(fbdo->errorReason());
}

void FireSenseClass::printUpdate(const char *msg, int type, float value)
{
    if (!configReady())
        return;

    if (!config->debug)
        return;

    switch (type)
    {
    case 0:
        Serial.print(F("[DEBUG] Updating "));
        Serial.print(msg);
        Serial.println(F(" status..."));
        break;
    case 1:
        Serial.println(F("[DEBUG] Updating log..."));
        break;
    case 2:
        Serial.println(F("[DEBUG] Updating lastSeen..."));
        break;
    case 3:
        Serial.println(F("[DEBUG] Initializing..."));
        break;
    case 4:
        Serial.println(F("[DEBUG] Failed, no FireSense config assigned.\n"));
        break;
    case 5:
        Serial.println(F("[DEBUG] Failed, no Firebase Data objects assigned.\n"));
        break;
    case 6:
        Serial.println(F("[DEBUG] Loading channels status from database..."));
        break;
    case 7:
        Serial.println(F("[DEBUG] Failed, Firebase authentication token is not ready."));
        break;
    case 8:
        Serial.println(F("[DEBUG] Updating channel data..."));
        break;
    case 9:
        Serial.println(F("[DEBUG] No channels status found."));
        break;
    case 10:
        Serial.println(F("[DEBUG] Channels status loaded."));
        break;
    case 11:
        Serial.println(F("[DEBUG] Update database channel status..."));
        break;
    case 12:
        Serial.println(F("[DEBUG] Setup the database rules query index..."));
        break;
    case 13:
        Serial.println(F("[DEBUG] Query index added."));
        break;
    case 14:
        Serial.println(F("[DEBUG] Acquiring NTP time..."));
        break;
    case 15:
        Serial.println(F("[DEBUG] Checking databasse config and authentication..."));
        break;
    case 16:
        Serial.println(F("[DEBUG] Failed, access denied, check the authentication.\n"));
        break;
    case 17:
        Serial.println(F("[DEBUG] Config found."));
        break;
    case 18:
        Serial.println(F("[DEBUG] Initialization completed."));
        break;
    case 19:
        Serial.println(F("[DEBUG] Setup stream..."));
        break;
    case 20:
        Serial.println(F("[DEBUG] Deleting old log data..."));
        break;
    case 21:
        Serial.println(F("[DEBUG] Stream setup completed"));
        break;
    case 22:
        Serial.println(F("[DEBUG] Clear log..."));
        break;
    case 23:
        Serial.println(F("[DEBUG] Loading default config..."));
        break;
    case 24:
        Serial.println(F("[DEBUG] Config loaded."));
        Serial.println(F("[DEBUG] Ready\n"));
        break;
    case 25:
        Serial.println(F("[DEBUG] Loading database config..."));
        break;
    case 26:
        Serial.println(F("[DEBUG] No config found."));
        break;
    case 27:
        Serial.println(F("[DEBUG] Loading channel item..."));
        break;
    case 28:
        Serial.println(F("[DEBUG] Parsing IF conditions..."));
        break;
    case 29:
        Serial.println(F("[DEBUG] Parsing THEN conditions..."));
        break;
    case 30:
        Serial.println(F("[DEBUG] Parsing ELSE conditions..."));
        break;
    case 31:
        Serial.println(F("[DEBUG] Adding condition to database..."));
        break;
    case 32:
        Serial.println(F("[DEBUG] Loading conditions from database..."));
        break;
    case 33:
        Serial.println(F("[DEBUG] No conditions found."));
        break;
    case 34:
        Serial.print(F("[DEBUG] Loading condition ["));
        Serial.print(msg);
        Serial.println(F("]..."));
        break;
    case 35:
        Serial.println(F("[DEBUG] Conditions loaded."));
        break;
    case 36:
        Serial.print(F("[DEBUG] "));
        Serial.print(msg);
        Serial.println(F(" -> true"));
        break;
    case 37:
        Serial.print(F("[DEBUG] "));
        Serial.print(msg);
        Serial.println(F(" -> false"));
        break;
    case 38:
        Serial.print(F("[DEBUG] "));
        Serial.print(msg);
        Serial.print(F(" -> "));
        Serial.println(value);
        break;
    case 39:
        Serial.print(F("[DEBUG] Adding "));
        Serial.print(msg);
        Serial.println(F(" channel..."));
        break;
    case 40:
        Serial.println(F("[DEBUG] Backup config..."));
        break;
    case 41:
        Serial.println(F("[DEBUG] Restore config..."));
        break;
    case 42:
        Serial.println(F("[DEBUG] Get conditions list..."));
        break;
    case 43:
        Serial.print(F("[DEBUG] Free Heap "));
#if defined(ESP32) || defined(ESP8266)
        Serial.println(ESP.getFreeHeap());
#endif
        break;
    case 44:
        Serial.println(F("[DEBUG] Waiting for authentication to be completed..."));
        break;
    default:
        break;
    }
}

void FireSenseClass::setChannelValue(struct channel_info_t &channel, struct data_value_info_t value)
{
    if (!configReady())
        return;

    if (!timeReady)
        return;

    delay(0);

    if (channel.type == channel_type_t::Input || channel.type == channel_type_t::Output || channel.type == channel_type_t::Analog_input || channel.type == channel_type_t::Value)
    {

        if (channel.type == channel_type_t::Output)
        {
            if (channel.current_value.int_data == value.int_data)
                return;
        }
        else
        {

            if (millis() - channel.lastPolling < channel.pollingInterval)
                return;
            channel.lastPolling = millis();
        }

        MB_String path = channelStatusPath();
        path += (const char *)FPSTR("/");
        path += channel.id.c_str();

        if (!channel.ready)
        {
            if (channel.type == channel_type_t::Input || channel.type == channel_type_t::Output)
            {
                if (channel.type == channel_type_t::Output)
                    pinMode(channel.gpio, OUTPUT);
                else
                    pinMode(channel.gpio, INPUT);
                channel.ready = true;
            }
        }

        if (channel.type == channel_type_t::Output)
        {
            digitalWrite(channel.gpio, channel.current_value.int_data > 0);
            channel.current_value.int_data = value.int_data;
            channel.current_value.float_data = (float)channel.current_value.int_data;
            if (channel.current_value.int_data > 0)
                printUpdate(channel.id.c_str(), 36);
            else
                printUpdate(channel.id.c_str(), 37);

            if (channel.status)
            {
                printUpdate(channel.id.c_str(), 0);
                if (!FBRTDB.setAsync(EXT config->shared_fbdo, path.c_str(), channel.current_value.int_data > 0))
                    printError(config->shared_fbdo);

                if (config->close_session)
                    config->shared_fbdo->clear();
                printUpdate("", 43);
            }
        }
        else if (channel.type == channel_type_t::Input)
        {

            int v = digitalRead(channel.gpio);

            if (v == channel.current_value.int_data)
                return;

            channel.current_value.int_data = v;
            channel.current_value.float_data = (float)channel.current_value.int_data;
            if (channel.status)
            {
                printUpdate(channel.id.c_str(), 0);
                if (!FBRTDB.setAsync(EXT config->shared_fbdo, path.c_str(), v))
                    printError(config->shared_fbdo);

                if (config->close_session)
                    config->shared_fbdo->clear();
                printUpdate("", 43);
            }
        }
        else if (channel.type == channel_type_t::Analog_input)
        {
            int v = analogRead(channel.gpio);

            if (v == channel.current_value.int_data)
                return;

            channel.current_value.int_data = v;
            channel.current_value.float_data = (float)channel.current_value.int_data;
            if (channel.status)
            {
                printUpdate(channel.id.c_str(), 0);
                if (!FBRTDB.setAsync(EXT config->shared_fbdo, path.c_str(), v))
                    printError(config->shared_fbdo);

                if (config->close_session)
                    config->shared_fbdo->clear();
                printUpdate("", 43);
            }
        }
        else if (channel.type == channel_type_t::Value)
            setUserValue(&channel, true, value);
    }
}

void FireSenseClass::setUserValue(struct channel_info_t *channel, bool fromUserValue, struct data_value_info_t val)
{
    if (channel->value_index > -1 && channel->value_index < (int)userValueList.size())
    {
        if (fromUserValue)
        {
            if (userValueList[channel->value_index].type == data_type_byte)
            {
                val.int_data = *userValueList[channel->value_index].bytePtr;
                val.float_data = (float)val.int_data;
            }
            else if (userValueList[channel->value_index].type == data_type_bool)
            {
                val.int_data = *userValueList[channel->value_index].boolPtr;
                val.float_data = (float)val.int_data;
            }
            else if (userValueList[channel->value_index].type == data_type_int)
            {
                val.int_data = *userValueList[channel->value_index].intPtr;
                val.float_data = (float)val.int_data;
            }
            else if (userValueList[channel->value_index].type == data_type_float)
            {
                val.float_data = *userValueList[channel->value_index].floatPtr;
                val.int_data = (int)val.float_data;
            }
        }
        else
        {
            if (userValueList[channel->value_index].type == data_type_byte)
                *userValueList[channel->value_index].bytePtr = (uint8_t)val.int_data;
            else if (userValueList[channel->value_index].type == data_type_bool)
                *userValueList[channel->value_index].boolPtr = (bool)val.int_data;
            else if (userValueList[channel->value_index].type == data_type_int)
                *userValueList[channel->value_index].intPtr = val.int_data;
            else if (userValueList[channel->value_index].type == data_type_float)
                *userValueList[channel->value_index].floatPtr = val.float_data;
        }
    }

    if (val.int_data != channel->current_value.int_data || val.float_data != channel->current_value.float_data)
    {
        if (val.type == data_type_float)
        {
            printUpdate(channel->id.c_str(), 38, val.float_data);
            channel->current_value.float_data = val.float_data;
            channel->current_value.int_data = (int)channel->current_value.float_data;
        }
        else
        {
            printUpdate(channel->id.c_str(), 38, val.int_data);
            channel->current_value.int_data = val.int_data;
            channel->current_value.float_data = (float)channel->current_value.int_data;
        }

        if (channel->status)
        {
            MB_String path = channelStatusPath();
            path += (const char *)FPSTR("/");
            path += channel->id.c_str();
            printUpdate(channel->id.c_str(), 0);
            if (val.type == data_type_float)
            {
                if (!FBRTDB.setAsync(EXT config->shared_fbdo, path.c_str(), val.float_data))
                    printError(config->shared_fbdo);
            }
            else
            {
                if (!FBRTDB.setAsync(EXT config->shared_fbdo, path.c_str(), val.int_data))
                    printError(config->shared_fbdo);
            }

            if (config->close_session)
                config->shared_fbdo->clear();
        }
    }
}

void FireSenseClass::loadStatus()
{
    if (!configReady())
        return;

    if (loadingStatus)
        return;

    loadingStatus = true;

    printUpdate("", 6);

    if (config->debug)
        FBRTDB.setAsync(EXT config->shared_fbdo, terminalPath().c_str(), "Loading channels status...");

    if (FBRTDB.getJSON(EXT config->shared_fbdo, channelStatusPath().c_str()))
    {

        FirebaseJson *json = config->shared_fbdo->to<FirebaseJson *>();

        for (size_t i = 0; i < channelsList.size(); i++)
        {
            // Parse for each channel state
            json->get(result, channelsList[i].id);

            if (result.success)
            {
                channel_info_t *channel = &channelsList[i];

                if (channel->type == channel_type_t::Output)
                {
                    digitalWrite(channel->gpio, result.to<bool>());
                    channel->current_value.int_data = result.to<bool>();
                }
                else if (channel->type == channel_type_t::Value)
                {
                    struct data_value_info_t v;
                    v.type = channel->current_value.type;
                    if (result.type == (const char *)FPSTR("float") || result.type == (const char *)FPSTR("double"))
                    {
                        v.float_data = result.to<float>();
                        v.int_data = (int)v.float_data;
                    }
                    else if (result.type == (const char *)FPSTR("boolean") || result.type == (const char *)FPSTR("int"))
                    {
                        v.int_data = result.to<int>();
                        v.float_data = (float)v.int_data;
                    }
                    setUserValue(channel, false, v);
                }
            }
        }

        if (config->close_session)
            config->shared_fbdo->clear();
    }
    else
        printError(config->shared_fbdo);

    if (channelsList.size() == 0)
        printUpdate("", 9);
    else
        printUpdate("", 10);

    loadingStatus = false;
}

void FireSenseClass::readStream(FIREBASE_STREAM_CLASS *data)
{
    if (!configReady())
        return;

    if (data->dataType() == (const char *)FPSTR("int") || data->dataType() == (const char *)FPSTR("boolean") || data->dataType() == (const char *)FPSTR("float") || data->dataType() == (const char *)FPSTR("double"))
    {
        for (size_t i = 0; i < channelsList.size(); i++)
        {
            if (!timeReady)
                break;
            String path = "/channels/" + channelsList[i].id;
            if (data->dataPath() == path)
            {
                if (channelsList[i].type == channel_type_t::Output || channelsList[i].type == channel_type_t::Value)
                {
                    if (channelsList[i].type == channel_type_t::Output)
                    {
                        struct data_value_info_t v;
                        if (data->dataType() == (const char *)FPSTR("int") || data->dataType() == (const char *)FPSTR("boolean"))
                        {
                            v.int_data = data->to<int>();
                            v.type = data_type_int;
                            v.float_data = (float)v.int_data;
                            setChannelValue(channelsList[i], v);
                        }
                    }
                    else if (data->dataType() == (const char *)FPSTR("boolean") || data->dataType() == (const char *)FPSTR("int") || data->dataType() == (const char *)FPSTR("float") || data->dataType() == (const char *)FPSTR("double"))
                    {
                        struct data_value_info_t v;
                        if (data->dataType() == (const char *)FPSTR("float") || data->dataType() == (const char *)FPSTR("double"))
                        {
                            v.float_data = data->to<float>();
                            v.type = data_type_float;
                            v.int_data = (int)v.float_data;
                        }
                        else
                        {
                            v.int_data = data->to<int>();
                            v.type = data_type_int;
                            v.float_data = (float)v.int_data;
                        }
                        setUserValue(&channelsList[i], false, v);
                    }
                }
            }
        }
    }
    else if (data->dataPath() == "/cmd")
    {
        streamCmd = data->to<const char *>();
#if defined(ESP32)
        checkCommand();
#endif
    }
}

void FireSenseClass::readStream(FirebaseData *fbdo)
{
    if (!configReady())
        return;

    FBRTDB.readStream(EXT fbdo);

    if (fbdo->streamAvailable())
    {
        if (fbdo->dataType() == (const char *)FPSTR("int") || fbdo->dataType() == (const char *)FPSTR("boolean") || fbdo->dataType() == (const char *)FPSTR("float") || fbdo->dataType() == (const char *)FPSTR("double"))
        {
            for (size_t i = 0; i < channelsList.size(); i++)
            {
                if (!timeReady)
                    break;
                String path = "/channels/" + channelsList[i].id;
                if (fbdo->dataPath() == path)
                {
                    if (channelsList[i].type == channel_type_t::Output || channelsList[i].type == channel_type_t::Value)
                    {
                        if (channelsList[i].type == channel_type_t::Output)
                        {
                            struct data_value_info_t v;
                            if (fbdo->dataType() == (const char *)FPSTR("int") || fbdo->dataType() == (const char *)FPSTR("boolean"))
                            {
                                v.int_data = fbdo->to<int>();
                                v.type = data_type_int;
                                v.float_data = (float)v.int_data;
                                setChannelValue(channelsList[i], v);
                            }
                        }
                        else if (fbdo->dataType() == (const char *)FPSTR("boolean") || fbdo->dataType() == (const char *)FPSTR("int") || fbdo->dataType() == (const char *)FPSTR("float") || fbdo->dataType() == (const char *)FPSTR("double"))
                        {
                            struct data_value_info_t v;
                            if (fbdo->dataType() == (const char *)FPSTR("float") || fbdo->dataType() == (const char *)FPSTR("double"))
                            {
                                v.float_data = fbdo->to<float>();
                                v.type = data_type_float;
                                v.int_data = (int)v.float_data;
                            }
                            else
                            {
                                v.int_data = fbdo->to<int>();
                                v.type = data_type_int;
                                v.float_data = (float)v.int_data;
                            }
                            setUserValue(&channelsList[i], false, v);
                        }
                    }
                }
            }
        }
        else if (fbdo->dataPath() == "/cmd")
        {
            streamCmd = fbdo->to<const char *>();
#if defined(ESP32)
            checkCommand();
#endif
        }
    }
}

void FireSenseClass::addChannel(struct channel_info_t &channel, bool addToDatabase)
{
    if (!configReady())
        return;
    printUpdate(channel.id.c_str(), 39);
    randomUid(10, channel.uid);

    if (channel.type == channel_type_t::Analog_input || channel.type == channel_type_t::Input)
    {
        channel.current_value.type = data_type_int;
        channel.last_value.type = data_type_int;
    }
    if (channel.type == channel_type_t::Output)
    {
        channel.current_value.type = data_type_bool;
        channel.last_value.type = data_type_bool;
    }

    channelsList.push_back(channel);
    if (addToDatabase)
        addDBChannel(channel);
}

void FireSenseClass::addDBChannel(struct channel_info_t &channel)
{
    if (!configReady())
        return;

    printUpdate("", 8);
    delay(0);
    MB_String path;
    _json.clear();
    _json.add((const char *)FPSTR("id"), channel.id);
    _json.add((const char *)FPSTR("name"), channel.name);
    _json.add((const char *)FPSTR("location"), channel.location);
    _json.add((const char *)FPSTR("uid"), channel.uid);
    _json.add((const char *)FPSTR("gpio"), channel.gpio);
    _json.add((const char *)FPSTR("type"), channel.type);
    _json.add((const char *)FPSTR("utype"), (int)channel.unbound_type);
    _json.add((const char *)FPSTR("vIndex"), channel.value_index);
    _json.add((const char *)FPSTR("status"), channel.status);
    _json.add((const char *)FPSTR("log"), channel.log);
    path = channelConfigPath();
    path += (const char *)FPSTR("/");
    path += channelsList.size() - 1;
    if (!FBRTDB.updateNode(EXT config->shared_fbdo, path.c_str(), EXT2 _json))
    {
        printError(config->shared_fbdo);

        if (config->close_session)
            config->shared_fbdo->clear();
        return;
    }
    _json.clear();
    delay(0);

    updateDBStatus(channel);
}

void FireSenseClass::updateDBStatus(struct channel_info_t &channel)
{
    if (!configReady())
        return;

    printUpdate("", 11);

    MB_String path;

    if (channel.type == channel_type_t::Output)
    {
        path = channelStatusPath();
        path += (const char *)FPSTR("/");
        path += channel.id.c_str();
        if (channel.status)
        {
            printUpdate(channel.id.c_str(), 0);
            if (channel.current_value.type == data_type_float)
            {
                delay(0);
                if (!FBRTDB.set(EXT config->shared_fbdo, path.c_str(), channel.current_value.float_data))
                {
                    printError(config->shared_fbdo);

                    if (config->close_session)
                        config->shared_fbdo->clear();
                    return;
                }
            }
            else
            {
                delay(0);
                if (!FBRTDB.set(EXT config->shared_fbdo, path.c_str(), channel.current_value.int_data))
                {
                    printError(config->shared_fbdo);

                    if (config->close_session)
                        config->shared_fbdo->clear();
                    return;
                }
            }
        }

        path = channelControlPath();
        path += (const char *)FPSTR("/");
        path += channel.id.c_str();
        delay(0);
        FBRTDB.set(EXT config->shared_fbdo, path.c_str(), 0);
    }
    else if (channel.type == channel_type_t::Input)
    {
        /*
        path = channelControlPath();
        path += (const char *)FPSTR("/");
        path += channel.id.c_str();
        delay(0);
        FBRTDB.deleteNode(config->shared_fbdo, path.c_str());
        */

        path = channelStatusPath();
        path += (const char *)FPSTR("/");
        path += channel.id.c_str();
        if (channel.status)
        {
            printUpdate(channel.id.c_str(), 0);

            if (channel.current_value.type == data_type_float)
            {
                delay(0);
                if (!FBRTDB.set(EXT config->shared_fbdo, path.c_str(), channel.current_value.float_data))
                {
                    printError(config->shared_fbdo);

                    if (config->close_session)
                        config->shared_fbdo->clear();
                    return;
                }
            }
            else
            {
                delay(0);
                if (!FBRTDB.set(EXT config->shared_fbdo, path.c_str(), channel.current_value.int_data))
                {
                    printError(config->shared_fbdo);
                    return;
                }
            }
        }
    }
    else if (channel.type == channel_type_t::Analog_input)
    {
        /*
        path = channelControlPath();
        path += (const char *)FPSTR("/");
        path += channel.id.c_str();
        delay(0);
        FBRTDB.deleteNode(config->shared_fbdo, path.c_str());
        */

        path = channelStatusPath();
        path += (const char *)FPSTR("/");
        path += channel.id.c_str();
        if (channel.status)
        {
            printUpdate(channel.id.c_str(), 0);

            if (channel.current_value.type == data_type_float)
            {
                delay(0);
                if (!FBRTDB.set(EXT config->shared_fbdo, path.c_str(), channel.current_value.float_data))
                    printError(config->shared_fbdo);
            }
            else
            {
                delay(0);
                if (!FBRTDB.set(EXT config->shared_fbdo, path.c_str(), channel.current_value.int_data))
                    printError(config->shared_fbdo);
            }
        }
    }
    else if (channel.type == channel_type_t::Value)
    {
        path = channelControlPath();
        path += (const char *)FPSTR("/");
        path += channel.id.c_str();
        delay(0);
        FBRTDB.set(EXT config->shared_fbdo, path.c_str(), 0);

        if ((int)userValueList.size() > channel.value_index && channel.value_index > -1 && channel.status)
        {
            path = channelStatusPath();
            path += (const char *)FPSTR("/");
            path += channel.id.c_str();
            struct data_value_info_t val;
            if (channel.value_index != -1 && channel.value_index < (int)userValueList.size())
            {
                if (userValueList[channel.value_index].type == data_type_bool)
                {
                    val.int_data = *userValueList[channel.value_index].boolPtr;
                    val.type = data_type_int;
                    val.float_data = (float)val.int_data;
                }
                else if (userValueList[channel.value_index].type == data_type_byte)
                {
                    val.int_data = *userValueList[channel.value_index].bytePtr;
                    val.type = data_type_int;
                    val.float_data = (float)val.int_data;
                }
                else if (userValueList[channel.value_index].type == data_type_int)
                {
                    val.int_data = *userValueList[channel.value_index].intPtr;
                    val.type = data_type_int;
                    val.float_data = (float)val.int_data;
                }
                else if (userValueList[channel.value_index].type == data_type_float)
                {
                    val.float_data = *userValueList[channel.value_index].floatPtr;
                    val.type = data_type_float;
                    val.int_data = (int)val.float_data;
                }
            }

            printUpdate(channel.id.c_str(), 0);
            if (val.type == data_type_float)
            {
                delay(0);
                if (!FBRTDB.set(EXT config->shared_fbdo, path.c_str(), val.float_data))
                    printError(config->shared_fbdo);
            }
            else
            {
                delay(0);
                if (!FBRTDB.set(EXT config->shared_fbdo, path.c_str(), val.int_data))
                    printError(config->shared_fbdo);
            }
        }
    }

    if (config->close_session)
        config->shared_fbdo->clear();
}

void FireSenseClass::storeDBStatus()
{
    if (!configReady())
        return;

    for (size_t i = 0; i < channelsList.size(); i++)
        updateDBStatus(channelsList[i]);
}

MB_String FireSenseClass::controlPath()
{
    if (!configReady())
        return "";

    MB_String s = config->basePath.c_str();
    s += (const char *)FPSTR("/controls/");
    s += deviceId;
    return s;
}

MB_String FireSenseClass::channelControlPath()
{
    MB_String s = controlPath();
    s += (const char *)FPSTR("/channels");
    return s;
}

MB_String FireSenseClass::configPath()
{
    if (!configReady())
        return "";

    MB_String s = config->basePath.c_str();
    s += (const char *)FPSTR("/config/devices/");
    s += deviceId;
    return s;
}

MB_String FireSenseClass::conditionPath()
{
    MB_String s = configPath();
    s += (const char *)FPSTR("/conditions");
    return s;
}

MB_String FireSenseClass::channelConfigPath()
{
    MB_String s = configPath();
    s += (const char *)FPSTR("/channels");
    return s;
}

MB_String FireSenseClass::streamCmdPath()
{
    MB_String s = controlPath();
    s += (const char *)FPSTR("/cmd");
    return s;
}

MB_String FireSenseClass::statusPath()
{
    if (!configReady())
        return "";

    MB_String s = config->basePath.c_str();
    s += (const char *)FPSTR("/status/");
    s += deviceId;
    return s;
}

MB_String FireSenseClass::terminalPath()
{
    MB_String s = statusPath();
    s += (const char *)FPSTR("/terminal");
    return s;
}

MB_String FireSenseClass::channelStatusPath()
{
    MB_String s = statusPath();
    s += (const char *)FPSTR("/channels");
    return s;
}

MB_String FireSenseClass::lastSeenPath()
{
    MB_String s = statusPath();
    s += (const char *)FPSTR("/lastSeen");
    return s;
}

MB_String FireSenseClass::logPath()
{
    if (!configReady())
        return "";

    MB_String s = config->basePath.c_str();
    s += (const char *)FPSTR("/log/");
    s += deviceId;
    return s;
}

String FireSenseClass::getDeviceId()
{
    String s;
    getChipId(s);
    return s;
}

void FireSenseClass::getChipId(String &s)
{
#if defined(ESP32)
    uint32_t chipId = 0;
    for (int i = 0; i < 17; i = i + 8)
        chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    s = (const char *)FPSTR("ESP-");
    s += String(chipId, HEX);
    s.toUpperCase();
#elif defined(ESP8266)
    s = (const char *)FPSTR("ESP-");
    s += String(ESP.getChipId(), HEX);
    s.toUpperCase();
#endif
}

void FireSenseClass::randomUid(uint8_t length, String &s)
{
    for (uint8_t i = 0; i < length; i++)
        s += (char)random(97, 122);
}

void FireSenseClass::getDateTimeString(MB_String &s)
{
    if (!configReady())
        return;

    struct tm timeinfo;
    time_t now = Firebase.getCurrentTime();

    localtime_r(&now, &timeinfo);

    s = sdow(timeinfo.tm_wday);
    s += (const char *)FPSTR(", ");
    s += timeinfo.tm_mday;
    s += (const char *)FPSTR(" ");
    s += months(timeinfo.tm_mon);

    s += (const char *)FPSTR(" ");
    s += timeinfo.tm_year + 1900;

    s += (const char *)FPSTR(" ");
    if (timeinfo.tm_hour < 10)
        s += (const char *)FPSTR("0");
    s += timeinfo.tm_hour;
    s += (const char *)FPSTR(":");
    if (timeinfo.tm_min < 10)
        s += (const char *)FPSTR("0");
    s += timeinfo.tm_min;

    s += (const char *)FPSTR(":");
    if (timeinfo.tm_sec < 10)
        s += (const char *)FPSTR("0");
    s += timeinfo.tm_sec;

    int p = 1;
    if (config->time_zone < 0)
        p = -1;
    int tz = config->time_zone;
    float dif = (p * (config->time_zone - tz)) * 60.0;
    if (config->time_zone < 0)
        s += (const char *)FPSTR(" -");
    else
        s += (const char *)FPSTR(" +");

    if (tz < 10)
        s += (const char *)FPSTR("0");
    s += tz;

    if (dif < 10)
        s += (const char *)FPSTR("0");
    s += (int)dif;
}

void FireSenseClass::replaceAll(MB_String &str, const char *find, const char *replace)
{
    size_t pos = 0;
    while ((pos = str.find(find, pos)) != MB_String::npos)
    {
        str.erase(pos, strlen(find));
        str.insert(pos, replace);
        pos += strlen(replace);
    }
}

void FireSenseClass::replaceChannelsValues(MB_String &str)
{
    String s, t;
    for (size_t i = 0; i < channelsList.size(); i++)
    {
        s = (const char *)FPSTR("{");
        s += channelsList[i].id.c_str();
        s += (const char *)FPSTR("}");
        struct data_value_info_t val = getChannelValue(&channelsList[i]);

        if (val.type == data_type_float)
            replaceAll(str, s.c_str(), num2Str(val.float_data, -1));
        else
            replaceAll(str, s.c_str(), num2Str(val.int_data, -1));
    }
}

void FireSenseClass::trim(const char *s, MB_String &d, bool isExpression, const char beginTrim, const char endTrim)
{
    int p1 = -1, p2 = -1, p3 = -1, p4 = -1;

    if (isExpression)
    {
        p1 = 0;
        p2 = 0;
        p3 = 0;
        p4 = 0;
        for (size_t i = 0; i < strlen(s); i++)
        {
            if (s[i] == beginTrim)
            {
                p1 = i;
                p3++;
            }
            else if (s[i] == endTrim)
            {
                p2 = i;
                p4++;
            }
        }

        if (p3 != p4 || p1 > p2)
        {
            d.clear();
            return;
        }

        p1 = -1;
        p2 = -1;
        p3 = -1;
        p4 = -1;
    }

    if (p3 != p4 || p1 > p2)
        d = s;

    for (size_t i = 0; i < strlen(s); i++)
    {
        if (s[i] == ' ')
            continue;
        else if (s[i] != beginTrim)
        {
            p3 = i;
            break;
        }
        else
        {
            p1 = i + 1;
            break;
        }
    }

    for (int i = (int)strlen(s) - 1; i >= 0; i--)
    {
        if (s[i] == ' ')
            continue;
        else if (s[i] != endTrim)
        {
            p4 = i;
            break;
        }
        else
        {
            p2 = i - 1;
            break;
        }
    }

    if ((p1 == -1 && p2 == -1))
    {
        p1 = p3;
        p2 = p4;
    }
    else if ((p1 != -1 && p2 == -1))
    {
        p1--;
        p2 = p4;
    }
    else if (p2 != -1 && p1 == -1)
    {
        p1 = p3;
        p2++;
    }

    int i = 0;
    while (i < p2 - p1 + 1)
    {
        d += s[p1 + i];
        i++;
    }
    p1 = 0;
    p2 = 0;
    p3 = 0;
    p4 = 0;
    for (size_t i = 0; i < d.length(); i++)
    {
        if (d[i] == beginTrim)
        {
            p1 = i;
            p3++;
        }
        else if (d[i] == endTrim)
        {
            p2 = i;
            p4++;
        }
    }

    if (p3 != p4 || p1 > p2)
        d = s;
}

void FireSenseClass::getCondition(const char *s, struct cond_item_data_t &data)
{
    if (!Firebase.ready())
        return;

    cond_comp_opr_type_t comp_type = cond_comp_opr_type_undefined;
    int p1 = -1, p2 = -1;
    MB_String t, left, right;

    if (strlen(s) > 7)
    {
        t = s;
        if (t.substr(0, 6) == (const char *)FPSTR("change"))
        {
            left = s;
            size_t p1 = left.find((const char *)FPSTR("("), 6);
            if (p1 != MB_String::npos)
            {
                p1++;
                size_t p2 = left.find((const char *)FPSTR(")"), p1 + 1);
                if (p2 != MB_String::npos)
                {
                    trim(left.substr(p1, p2 - p1).c_str(), right, false);
                    for (size_t j = 0; j < channelsList.size(); j++)
                    {
                        if (strcmp(right.c_str(), channelsList[j].id.c_str()) == 0)
                        {
                            data.left.type = cond_operand_type_changed;
                            data.right.type = cond_operand_type_changed;
                            data.left.channel = &channelsList[j];
                            return;
                        }
                    }
                }
            }
        }
    }

    bool hasOp = false;
    for (size_t i = 0; i < strlen(s); i++)
    {
        if (s[i] == '<' || s[i] == '>' || s[i] == '=')
            hasOp = true;

        if (i + 1 < strlen(s))
        {
            if (s[i] == '!' && s[i + 1] == '=')
                hasOp = true;
        }
    }

    if (!hasOp)
    {
        left = s;
        getConditionItem(data, left, right);
        return;
    }

    for (size_t i = 0; i < strlen(s); i++)
    {
        delay(0);
        if (comp_type == cond_comp_opr_type_undefined)
        {
            if (s[i] == '<')
                comp_type = cond_comp_opr_type_lt;
            else if (s[i] == '>')
                comp_type = cond_comp_opr_type_gt;
            else if (s[i] == '!')
            {
                if (i + 1 < strlen(s))
                {
                    if (s[i + 1] == '=')
                        comp_type = cond_comp_opr_type_neq;
                }
            }
            else if (s[i] == '=')
                comp_type = cond_comp_opr_type_eq;

            if (comp_type != cond_comp_opr_type_undefined)
            {
                p1 = i;
                p2 = i + 1;
            }

            continue;
        }
        else
        {
            if (s[i] == '=' && comp_type == cond_comp_opr_type_lt)
            {
                comp_type = cond_comp_opr_type_lteq;
                p2 = i + 1;
            }
            else if (s[i] == '=' && comp_type == cond_comp_opr_type_gt)
            {
                comp_type = cond_comp_opr_type_gteq;
                p2 = i + 1;
            }
            else if (s[i] == '=' && (comp_type == cond_comp_opr_type_neq || comp_type == cond_comp_opr_type_eq))
                p2 = i + 1;
            else if (s[i] == '=' && comp_type != cond_comp_opr_type_neq && comp_type != cond_comp_opr_type_eq)
                comp_type = cond_comp_opr_type_undefined;
        }

        if (comp_type != cond_comp_opr_type_undefined)
        {
            int i = 0;
            MB_String r, l;
            while (i < p1)
            {
                l += s[i];
                i++;
            }

            i = 0;
            while (i < (int)strlen(s) - p2)
            {
                r += s[p2 + i];
                i++;
            }

            if (l.length() > 0)
                trim(l.c_str(), left, false);
            if (r.length() > 0)
                trim(r.c_str(), right, false);

            getConditionItem(data, left, right);

            data.comp = comp_type;

            return;
        }
    }
}

void FireSenseClass::getConditionItem(struct cond_item_data_t &data, MB_String &left, MB_String &right)
{
    if (left.length() == 0)
        return;

    if (left[0] == '!')
        data.left.not_op = true;
    if (left.length() > 1)
        if (left[1] == ' ')
            data.left.not_op = false;

    if (data.left.not_op)
        left = left.substr(1, left.length() - 1);

    if (left == (const char *)FPSTR("millis"))
        data.left.type = cond_operand_type_millis;
    else if (left == (const char *)FPSTR("micros"))
        data.left.type = cond_operand_type_micros;
    else if (left == (const char *)FPSTR("date") || left == (const char *)FPSTR("time") || left == (const char *)FPSTR("day") || left == (const char *)FPSTR("weekday") || left == (const char *)FPSTR("hour") || left == (const char *)FPSTR("hour") || left == (const char *)FPSTR("sec") || left == (const char *)FPSTR("year") || left == (const char *)FPSTR("month"))
    {
        if (left == (const char *)FPSTR("date"))
        {
            data.left.type = cond_operand_type_date;
            data.right.type = cond_operand_type_date;
        }
        else if (left == (const char *)FPSTR("time"))
        {
            data.left.type = cond_operand_type_time;
            data.right.type = cond_operand_type_time;
        }
        else if (left == (const char *)FPSTR("year"))
        {
            data.left.type = cond_operand_type_year;
            data.right.type = cond_operand_type_year;
        }
        else if (left == (const char *)FPSTR("month"))
        {
            data.left.type = cond_operand_type_month;
            data.right.type = cond_operand_type_month;
        }
        else if (left == (const char *)FPSTR("day"))
        {
            data.left.type = cond_operand_type_day;
            data.right.type = cond_operand_type_day;
        }
        else if (left == (const char *)FPSTR("weekday"))
        {
            data.left.type = cond_operand_type_weekday;
            data.right.type = cond_operand_type_weekday;
        }
        else if (left == (const char *)FPSTR("hour"))
        {
            data.left.type = cond_operand_type_hour;
            data.right.type = cond_operand_type_hour;
        }
        else if (left == (const char *)FPSTR("hour"))
        {
            data.left.type = cond_operand_type_min;
            data.right.type = cond_operand_type_min;
        }
        else if (left == (const char *)FPSTR("sec"))
        {
            data.left.type = cond_operand_type_sec;
            data.right.type = cond_operand_type_sec;
        }

        parseDateTime(right.c_str(), data.left.type, data.left.time);
    }
    else
    {

        if (data.left.type == cond_operand_type_undefined)
        {
            for (size_t j = 0; j < channelsList.size(); j++)
            {
                if (strcmp(left.c_str(), channelsList[j].id.c_str()) == 0)
                {
                    data.left.type = cond_operand_type_channel;
                    data.left.channel = &channelsList[j];
                    break;
                }
            }
        }
    }

    if (data.left.type == cond_operand_type_undefined)
    {
        parseExpression(left.c_str(), data.left.exprs.expressions);
        data.left.type = cond_operand_type_expression;
    }

    if (right.length() == 0)
        return;

    if (left[0] == '!')
        data.right.not_op = true;
    if (left.length() > 1)
        if (left[1] == ' ')
            data.right.not_op = false;

    if (data.right.not_op)
        right = right.substr(1, right.length() - 1);

    if (right == (const char *)FPSTR("millis"))
        data.right.type = cond_operand_type_millis;
    else if (right == (const char *)FPSTR("micros"))
        data.right.type = cond_operand_type_micros;
    else
    {
        for (size_t j = 0; j < channelsList.size(); j++)
        {
            if (strcmp(right.c_str(), channelsList[j].id.c_str()) == 0)
            {
                data.right.type = cond_operand_type_channel;
                data.right.channel = &channelsList[j];
                break;
            }
        }
    }

    if (data.right.type == cond_operand_type_undefined)
    {
        parseExpression(right.c_str(), data.right.exprs.expressions);
        data.right.type = cond_operand_type_expression;
    }
}

void FireSenseClass::parseDateTime(const char *str, int type, struct tm &out)
{
    out.tm_sec = -1;
    out.tm_min = -1;
    out.tm_hour = -1;
    out.tm_mday = -1;
    out.tm_mon = -1;
    out.tm_year = -1;
    out.tm_wday = -1;
    MB_VECTOR<MB_String> tmtk;

    if (type == cond_operand_type_day)
        out.tm_mday = atoi(str);
    else if (type == cond_operand_type_month)
        out.tm_mon = atoi(str);
    else if (type == cond_operand_type_year)
        out.tm_year = atoi(str);
    else if (type == cond_operand_type_weekday)
        out.tm_wday = atoi(str);
    else if (type == cond_operand_type_hour)
        out.tm_hour = atoi(str);
    else if (type == cond_operand_type_min)
        out.tm_min = atoi(str);
    else if (type == cond_operand_type_sec)
        out.tm_sec = atoi(str);
    else if (type == cond_operand_type_time)
    {
        split(tmtk, str, ':');
        if (tmtk.size() == 1)
            out.tm_sec = atoi(tmtk[0].c_str());
        else if (tmtk.size() == 2)
        {
            out.tm_sec = atoi(tmtk[1].c_str());
            out.tm_min = atoi(tmtk[0].c_str());
        }
        else if (tmtk.size() == 3)
        {
            out.tm_sec = atoi(tmtk[2].c_str());
            out.tm_min = atoi(tmtk[1].c_str());
            out.tm_hour = atoi(tmtk[0].c_str());
        }
    }
    else if (type == cond_operand_type_date)
    {
        split(tmtk, str, '/');
        if (tmtk.size() == 1)
            out.tm_mday = atoi(tmtk[0].c_str());
        else if (tmtk.size() == 2)
        {
            out.tm_mday = atoi(tmtk[1].c_str());
            out.tm_mon = atoi(tmtk[0].c_str());
        }
        else if (tmtk.size() == 3)
        {
            out.tm_year = atoi(tmtk[2].c_str());
            out.tm_mday = atoi(tmtk[1].c_str());
            out.tm_mon = atoi(tmtk[0].c_str());
        }
    }
}

void FireSenseClass::getExpression(const char *s, struct expr_item_data_t &data)
{
    MB_String str = s;

    if (strlen(s) > 0)
    {
        if (s[0] == '!')
        {
            data.not_op = true;
            str = str.substr(1, str.length() - 1);
        }
    }

    if (str == (const char *)FPSTR("millis"))
    {
        data.type = expr_operand_type_millis;
        data.value.type = data_type_int;
        data.value.int_data = millis();
        data.value.float_data = (float)data.value.int_data;
        return;
    }
    else if (str == (const char *)FPSTR("micros"))
    {
        data.type = expr_operand_type_micros;
        data.value.type = data_type_int;
        data.value.int_data = micros();
        data.value.float_data = (float)data.value.int_data;
        return;
    }
    else if (str == (const char *)FPSTR("true") || str == (const char *)FPSTR("false"))
    {
        data.type = expr_operand_type_value;
        data.value.type = data_type_bool;
        data.value.int_data = str == (const char *)FPSTR("true") ? 1 : 0;
        data.value.float_data = (float)data.value.int_data;
        return;
    }
    else
    {
        for (size_t i = 0; i < channelsList.size(); i++)
        {
            if (strcmp(s, channelsList[i].id.c_str()) == 0)
            {
                data.type = expr_operand_type_channel;
                data.channel = &channelsList[i];
                data.value.type = data.channel->current_value.type;
                return;
            }
        }
    }

    data.type = expr_operand_type_value;
    if (str.find((const char *)FPSTR(".")) != MB_String::npos)
    {
        data.value.type = data_type_float;
        data.value.float_data = atof(s);
        data.value.int_data = (int)data.value.float_data;
    }
    else
    {
        data.value.type = data_type_int;
        data.value.int_data = atoi(s);
        data.value.float_data = (float)data.value.int_data;
    }
}

void FireSenseClass::parseExpression(const char *src, MB_VECTOR<struct expression_item_info_t> &expressions, int depth)
{
    if (!Firebase.ready())
        return;

    int orbk = 0, crbk = 0;
    bool nested = false;
    int aType = assignment_operator_type_undefined;

    MB_String s, buf, t;
    trim(src, s, true);
    int dp = depth + 1;
    bool not_op = false;

    for (size_t i = 0; i < s.length(); i++)
    {
        if (i + 1 < s.length())
        {
            if (s[i] == '!' && s[i + 1] == '(')
            {
                not_op = true;
                continue;
            }
        }
        if (s[i] == '(')
            orbk++;
        else if (s[i] == ')')
            crbk++;
        else if (orbk - crbk == 0)
            aType = getAssignOprType(s[i]);

        if (i + 1 < s.length())
        {
            if ((aType == assignment_operator_type_left_shift && s[i + 1] != '<') || (aType == assignment_operator_type_right_shift && s[i] != '>'))
                aType = assignment_operator_type_undefined;

            if (aType == assignment_operator_type_logic_and && s[i + 1] == '&')
                aType = assignment_operator_type_and;

            if (aType == assignment_operator_type_logic_or && s[i + 1] == '|')
                aType = assignment_operator_type_or;
        }

        if ((aType == assignment_operator_type_undefined && (assignment_operator_type_t)getAssignOprType(s[i]) == assignment_operator_type_undefined) || orbk - crbk != 0)
        {
            if ((assignment_operator_type_t)getAssignOprType(s[i]) != assignment_operator_type_undefined)
                nested = true;
            buf += s[i];
        }

        if (aType != assignment_operator_type_undefined)
        {
            t.clear();
            trim(buf.c_str(), t, true);
            if (t.length() > 0)
            {
                struct expression_item_info_t e;
                e.is_nested = nested;
                e.depth = depth;
                e.next_ass_opr = (assignment_operator_type_t)aType;
                e.not_op = not_op;
                expressions.push_back(e);
                if (nested)
                    parseExpression(t.c_str(), expressions[expressions.size() - 1].list, dp);
                else
                    getExpression(t.c_str(), expressions[expressions.size() - 1].data);
            }
            aType = assignment_operator_type_undefined;
            nested = false;
            not_op = false;
            buf.clear();
            continue;
        }
    }

    if (buf.length() > 0)
    {
        t.clear();
        trim(buf.c_str(), t, true);
        if (t.length() > 0)
        {
            struct expression_item_info_t e;
            e.is_nested = nested;
            e.depth = depth;
            e.next_ass_opr = (assignment_operator_type_t)aType;
            e.not_op = not_op;
            expressions.push_back(e);
            if (nested)
                parseExpression(t.c_str(), expressions[expressions.size() - 1].list, dp);
            else
                getExpression(t.c_str(), expressions[expressions.size() - 1].data);
        }
    }
}

int FireSenseClass::getAssignOprType(const char c)
{
    assignment_operator_type_t ass = assignment_operator_type_undefined;
    if (c == '+')
        ass = assignment_operator_type_add;
    else if (c == '-')
        ass = assignment_operator_type_subtract;
    else if (c == '*')
        ass = assignment_operator_type_multiply;
    else if (c == '/')
        ass = assignment_operator_type_division;
    else if (c == '%')
        ass = assignment_operator_type_remainder;
    else if (c == '<')
        ass = assignment_operator_type_left_shift;
    else if (c == '>')
        ass = assignment_operator_type_right_shift;
    else if (c == '|')
        ass = assignment_operator_type_logic_or;
    else if (c == '&')
        ass = assignment_operator_type_logic_and;
    return ass;
}

int FireSenseClass::getCompareConditionType(const char c)
{
    next_comp_opr_t compr = next_comp_opr_none;
    if (c == '|')
        compr = next_comp_opr_or;
    else if (c == '&')
        compr = next_comp_opr_and;
    return compr;
}

void FireSenseClass::parseCondition(const char *src, MB_VECTOR<struct condition_item_info_t> &conditions, int depth)
{
    if (!Firebase.ready())
        return;

    int orbk = 0, crbk = 0;
    bool nested = false;
    MB_String s, buf, t;
    trim(src, s, false);
    int dp = depth + 1;
    bool not_op = false;

    next_comp_opr_t compr = next_comp_opr_none;

    for (size_t i = 0; i < s.length(); i++)
    {
        delay(0);
        if (i + 1 < s.length())
        {
            if (s[i] == '!' && s[i + 1] == '(')
            {
                not_op = true;
                continue;
            }
        }

        if (s[i] == '(')
            orbk++;
        else if (s[i] == ')')
            crbk++;
        else if (orbk - crbk == 0)
            compr = (next_comp_opr_t)getCompareConditionType(s[i]);

        if (i + 1 < s.length())
        {
            if ((compr == next_comp_opr_or && s[i + 1] != '|') || (compr == next_comp_opr_and && s[i + 1] != '&'))
                compr = next_comp_opr_none;
        }

        if ((compr == next_comp_opr_none && (next_comp_opr_t)getCompareConditionType(s[i]) == next_comp_opr_none) || orbk - crbk != 0)
        {
            if ((next_comp_opr_t)getCompareConditionType(s[i]) != next_comp_opr_none)
                nested = true;
            buf += s[i];
        }

        if (compr != next_comp_opr_none)
        {
            t.clear();
            trim(buf.c_str(), t, false);
            if (t.length() > 0)
            {
                struct condition_item_info_t e;
                e.is_nested = nested;
                e.depth = depth;
                e.next_comp_opr = compr;
                e.not_op = not_op;
                // e.raw = t;
                conditions.push_back(e);
                if (nested)
                    parseCondition(t.c_str(), conditions[conditions.size() - 1].list, dp);
                else
                    getCondition(t.c_str(), conditions[conditions.size() - 1].data);
            }
            nested = false;
            not_op = false;
            buf.clear();
            continue;
        }
    }

    if (buf.length() > 0)
    {
        t.clear();
        trim(buf.c_str(), t, false);
        if (t.length() > 0)
        {
            struct condition_item_info_t e;
            e.is_nested = nested;
            e.depth = depth;
            e.next_comp_opr = compr;
            e.not_op = not_op;
            // e.raw = t;
            conditions.push_back(e);

            if (nested)
                parseCondition(t.c_str(), conditions[conditions.size() - 1].list, dp);
            else
                getCondition(t.c_str(), conditions[conditions.size() - 1].data);
        }
    }
}

void FireSenseClass::parseStatement(const char *src, MB_VECTOR<struct statement_item_info_t> &stm)
{
    if (!Firebase.ready())
        return;

    MB_String s, t, left, right;
    trim(src, s, false);

    MB_VECTOR<MB_String> stmTk;
    split(stmTk, s.c_str(), ',', '(', ')');

    for (size_t i = 0; i < stmTk.size(); i++)
    {
        delay(0);
        t.clear();
        trim(stmTk[i].c_str(), t, false);

        struct statement_item_info_t item;
        getStatement(t.c_str(), item.data);

        stm.push_back(item);
    }
}

void FireSenseClass::getStatement(const char *src, struct stm_item_t &data)
{
    if (!Firebase.ready())
        return;

    MB_String s, t, left, right;
    trim(src, s, false);

    int pos = 0;
    size_t p1 = 0, p2 = 0;
    delay(0);

    if (s.length() > 6)
    {
        if (s.substr(0, 5) == (const char *)FPSTR("delay"))
            pos = 5;
        else if (s.substr(0, 4) == (const char *)FPSTR("func"))
            pos = 4;

        if (pos > 0)
        {
            p1 = s.find((const char *)FPSTR("("), pos);
            if (p1 != MB_String::npos)
            {
                p1++;
                p2 = s.rfind((const char *)FPSTR(")"), s.length() - 1);
                if (p2 != MB_String::npos && p2 > p1)
                {
                    if (pos == 4)
                    {
                        data.left.type = stm_operand_type_function;
                        MB_VECTOR<MB_String> params;
                        split(params, s.substr(p1, p2 - p1).c_str(), ',', '\'', '\'');
                        int func_idx = -1;

                        if (params.size() > 0)
                        {
                            func_idx = atoi(params[0].c_str());
                        }

                        if (params.size() > 1)
                        {
                            data.left.function.iteration_max = atoi(params[1].c_str());
                        }
                        if (params.size() > 2)
                        {
                            MB_String s;
                            trim(params[2].c_str(), s, false, '\'', '\'');
                            data.left.function.payload = s;
                        }

                        if (func_idx > -1 && func_idx < (int)functionList.size())
                            data.left.function.ptr = &functionList[func_idx];
                    }
                    else if (pos == 5)
                    {
                        data.left.type = stm_operand_type_delay;
                        data.left.tmo = atoi(s.substr(p1, p2 - p1).c_str());
                    }
                    return;
                }
            }
        }
    }

    assignment_operator_type_t ass_type = assignment_operator_type_undefined;

    for (size_t i = 0; i < s.length(); i++)
    {
        delay(0);
        if (ass_type == assignment_operator_type_undefined)
        {
            if (s[i] == '=')
            {
                p2 = i + 1;
                ass_type = assignment_operator_type_assignment;
            }
            else if (s[i] == '+')
                ass_type = assignment_operator_type_add;
            else if (s[i] == '-')
                ass_type = assignment_operator_type_subtract;
            else if (s[i] == '*')
                ass_type = assignment_operator_type_multiply;
            else if (s[i] == '/')
                ass_type = assignment_operator_type_division;
            else if (s[i] == '%')
                ass_type = assignment_operator_type_remainder;
            else if (s[i] == '<')
                ass_type = assignment_operator_type_left_shift;
            else if (s[i] == '>')
                ass_type = assignment_operator_type_right_shift;
            else if (s[i] == '&')
                ass_type = assignment_operator_type_and;
            else if (s[i] == '|')
                ass_type = assignment_operator_type_or;

            if (ass_type != assignment_operator_type_undefined)
                p1 = i;

            continue;
        }
        else
        {

            bool valid = false;

            if (ass_type == assignment_operator_type_assignment)
                valid = true;
            else
            {
                if (s[i] == '=' && (ass_type == assignment_operator_type_add || ass_type == assignment_operator_type_subtract || ass_type == assignment_operator_type_multiply || ass_type == assignment_operator_type_division || ass_type == assignment_operator_type_remainder))
                {
                    valid = true;
                    p2 = i + 1;
                }
                else if (s[i] == '<' || s[i] == '>' || s[i] == '&' || s[i] == '|')
                {
                    if (i + 1 < s.length())
                    {

                        if (s[i] == '<' && s[i + 1] == '=' && ass_type == assignment_operator_type_left_shift)
                            valid = true;
                        else if (s[i] == '>' && s[i + 1] == '=' && ass_type == assignment_operator_type_right_shift)
                            valid = true;
                        else if (s[i] == '|' && s[i + 1] == '=' && ass_type == assignment_operator_type_or)
                            valid = true;
                        else if (s[i] == '&' && s[i + 1] == '=' && ass_type == assignment_operator_type_and)
                            valid = true;

                        if (valid)
                            p2 = i + 2;
                    }
                }
            }

            if (!valid)
                ass_type = assignment_operator_type_undefined;
        }

        if (ass_type != assignment_operator_type_undefined)
        {

            int m = 0;
            MB_String r, l;
            while (m < (int)p1)
            {
                delay(0);
                l += s[m];
                m++;
            }

            m = 0;
            while (m < (int)s.length() - (int)p2)
            {
                delay(0);
                r += s[p2 + m];
                m++;
            }

            if (l.length() > 0)
                trim(l.c_str(), left, false);
            if (r.length() > 0)
                trim(r.c_str(), right, false);

            for (size_t j = 0; j < channelsList.size(); j++)
            {
                if (strcmp(left.c_str(), channelsList[j].id.c_str()) == 0)
                {
                    data.left.type = stm_operand_type_channel;
                    data.left.channel = &channelsList[j];
                    break;
                }
            }

            for (size_t j = 0; j < channelsList.size(); j++)
            {
                if (strcmp(right.c_str(), channelsList[j].id.c_str()) == 0)
                {
                    data.right.type = stm_operand_type_channel;
                    data.right.channel = &channelsList[j];
                    break;
                }
            }

            if (data.right.type == stm_operand_type_undefined)
            {
                parseExpression(right.c_str(), data.right.exprs.expressions);
                data.right.type = stm_operand_type_expression;
            }

            data.ass = ass_type;

            return;
        }
    }
}

void FireSenseClass::split(MB_VECTOR<MB_String> &out, const char *str, const char delim, const char beginEsc, const char endEsc)
{
    uint16_t index = 0;
    uint16_t len = strlen(str);
    size_t sz = len * 2;
    char *buf = new char[sz];

    if (!buf)
        return;

    size_t esc1 = 0, esc2 = 0;

    int next_stage = -1;

    for (uint16_t i = 0; i < len; i++)
    {
        if (beginEsc != endEsc)
        {
            if (str[i] == beginEsc)
                esc1++;
            else if (str[i] == endEsc)
                esc2++;
        }
        else
        {
            if (str[i] == beginEsc && (next_stage == -1 || next_stage == 0))
            {
                next_stage = 1;
                esc1++;
            }
            else if (str[i] == endEsc && (next_stage == -1 || next_stage == 1))
            {
                next_stage = 0;
                esc2++;
            }
        }

        if (str[i] == delim && esc1 == esc2)
        {
            memset(buf, 0, sz);
            strncpy(buf, (char *)str + index, i - index);
            index = i + 1;
            if (strlen(buf) > 0)
            {
                MB_String t = buf;
                out.push_back(t);
            }
        }
    }

    if (index < len + 1 && esc1 == esc2)
    {
        memset(buf, 0, sz);
        strncpy(buf, (char *)str + index, len - index);
        if (strlen(buf) > 0)
        {
            MB_String t = buf;
            out.push_back(t);
        }
    }

    delete[] buf;
}

void FireSenseClass::setLogQueryIndex()
{

    if (!configReady())
        return;

    printUpdate("", 12);

    delay(0);

    if (!FBRTDB.setQueryIndex(EXT config->shared_fbdo, logPath().c_str(), (const char *)FPSTR("time"), databaseSecret))
    {
        printError(config->shared_fbdo);
        if (config->debug)
            Serial.println();
    }
    else
        printUpdate("", 13);

    if (config->close_session)
        config->shared_fbdo->clear();
}

#endif

#endif // ENABLE