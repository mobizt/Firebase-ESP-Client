/*
 * FirebaseJson, version 2.3.10
 * 
 * The Easiest ESP8266/ESP32 Arduino library for parse, create and edit JSON object using a relative path.
 * 
 * December 24, 2020
 * 
 * Features
 * - None recursive operations
 * - Parse and edit JSON object directly with a specified relative path. 
 * - Prettify JSON string 
 * 
 * 
 * The zserge's JSON object parser library used as part of this library
 * 
 * The MIT License (MIT)
 * Copyright (c) 2019 K. Suwatchai (Mobizt)
 * Copyright (c) 2012–2018, Serge Zaitsev, zaitsev.serge@gmail.com
 * 
 * 
 * Permission is hereby granted, free of charge, to any person returning a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef FirebaseJson_H
#define FirebaseJson_H

#include <Arduino.h>
#include <memory>
#include <vector>

static const char FirebaseJson_STR_1[] PROGMEM = ",";
static const char FirebaseJson_STR_2[] PROGMEM = "\"";
static const char FirebaseJson_STR_3[] PROGMEM = ":";
static const char FirebaseJson_STR_4[] PROGMEM = "%d";
static const char FirebaseJson_STR_5[] PROGMEM = "%f";
static const char FirebaseJson_STR_6[] PROGMEM = "false";
static const char FirebaseJson_STR_7[] PROGMEM = "true";
static const char FirebaseJson_STR_8[] PROGMEM = "{";
static const char FirebaseJson_STR_9[] PROGMEM = "}";
static const char FirebaseJson_STR_10[] PROGMEM = "[";
static const char FirebaseJson_STR_11[] PROGMEM = "]";
static const char FirebaseJson_STR_12[] PROGMEM = "string";
static const char FirebaseJson_STR_13[] PROGMEM = "int";
static const char FirebaseJson_STR_14[] PROGMEM = "double";
static const char FirebaseJson_STR_15[] PROGMEM = "bool";
static const char FirebaseJson_STR_16[] PROGMEM = "object";
static const char FirebaseJson_STR_17[] PROGMEM = "array";
static const char FirebaseJson_STR_18[] PROGMEM = "null";
static const char FirebaseJson_STR_19[] PROGMEM = "undefined";
static const char FirebaseJson_STR_20[] PROGMEM = ".";
static const char FirebaseJson_STR_21[] PROGMEM = "\"root\":";
static const char FirebaseJson_STR_22[] PROGMEM = "    ";
static const char FirebaseJson_STR_24[] PROGMEM = "\n";
static const char FirebaseJson_STR_25[] PROGMEM = ": ";
static const char FirebaseJson_STR_26[] PROGMEM = "root";
static const char FirebaseJson_STR_27[] PROGMEM = "/";

class FirebaseJson;
class FirebaseJsonArray;

class FirebaseJsonData
{
    friend class FirebaseJson;
    friend class FirebaseJsonArray;
    friend class FB_CM;
    friend class FirebaseStream;
    friend class FirebaseData;
    friend class GG_CloudStorage;

public:
    FirebaseJsonData();
    ~FirebaseJsonData();

    /*
    Get array data as FirebaseJsonArray object from FirebaseJsonData object.
    
    @param jsonArray - The returning FirebaseJsonArray object.

    @return bool status for successful operation.

    This should call after parse or get function.

   */
    bool getArray(FirebaseJsonArray &jsonArray);

    /*
    Get array data as FirebaseJson object from FirebaseJsonData object.
    
    @param jsonArray - The returning FirebaseJson object.

    @return bool status for successful operation.

    This should call after parse or get function.

   */
    bool getJSON(FirebaseJson &json);

    /*
    The String value of parses data.
   */
    String stringValue = "";

    /*
    The int value of parses data.
   */
    int intValue = 0;

    /*
    The float value of parses data.
   */
    float floatValue = 0.0f;

    /*
    The double value of parses data.
   */
    double doubleValue = 0.0;

    /*
    The bool value of parses data.
   */
    bool boolValue = false;

    /*
    The type String of parses data.
   */
    String type = "";

    /*
    The type (number) of parses data.
   */
    uint8_t typeNum = 0;

    /*
    The success flag of parsing data.
   */
    bool success = false;

private:
    int _type = 0;
    int _k_start = 0;
    int _start = 0;
    int _end = 0;
    int _tokenIndex = 0;
    int _depth = 0;
    int _len = 0;
    std::string _dbuf = "";
};

class FirebaseJson
{
    friend class FirebaseJsonArray;
    friend class FirebaseJsonData;
    friend class Firebase_ESP_Client;
    friend class Firebase_Signer;
    friend class FirebaseData;
    friend class FirebaseStream;
    friend class FB_RTDB;
    friend class FB_CM;
    friend class FB_Firestore;
    friend class FB_Functions;
    friend class PolicyBuilder;
    friend class PolicyInfo;
    friend class GG_CloudStorage;

public:
    typedef enum {
        JSON_UNDEFINED = 0,
        JSON_OBJECT = 1,
        JSON_ARRAY = 2,
        JSON_STRING = 3,
        JSON_INT = 4,
        JSON_FLOAT = 5,
        JSON_DOUBLE = 6,
        JSON_BOOL = 7,
        JSON_NULL = 8
    } jsonDataType;

    typedef enum {
        PRINT_MODE_NONE = -1,
        PRINT_MODE_PLAIN = 0,
        PRINT_MODE_PRETTY = 1
    } PRINT_MODE;

    typedef struct
    {
        bool matched = false;
        std::string tk = "";
    } path_tk_t;

    typedef struct
    {
        int index;
        bool firstTk;
        bool lastTk;
        bool success;
    } single_child_parent_t;

    typedef struct
    {
        uint16_t index;
        uint8_t type;
    } eltk_t;

    typedef struct
    {
        uint16_t index;
        uint8_t type;
        uint16_t olen;
        uint16_t oindex;
        int depth;
        bool omark;
        bool ref;
        bool skip;
    } el_t;

    typedef struct
    {
        int index;
        uint16_t oindex;
        uint16_t olen;
        uint8_t type;
        int depth;
        bool omark;
        bool ref;
        bool skip;
    } tk_index_t;

    /**
    * JSON type identifier. Basic types are:
    * 	o Object
    * 	o Array
    * 	o String
    * 	o Other primitive: number, boolean (true/false) or null
    */
    typedef enum {
        JSMN_UNDEFINED = 0,
        JSMN_OBJECT = 1,
        JSMN_ARRAY = 2,
        JSMN_STRING = 3,
        JSMN_PRIMITIVE = 4
    } fbjs_type_t;

    enum fbjs_err
    {
        /* Not enough tokens were provided */
        JSMN_ERROR_NOMEM = -1,
        /* Invalid character inside JSON string */
        JSMN_ERROR_INVAL = -2,
        /* The string is not a full JSON packet, more bytes expected */
        JSMN_ERROR_PART = -3
    };

    /**
    * JSON token description.
    * type		type (object, array, string etc.)
    * start	start position in JSON data string
    * end		end position in JSON data string
    */
    typedef struct
    {
        fbjs_type_t type;
        int start;
        int end;
        int size;
#ifdef JSMN_PARENT_LINKS
        int parent;
#endif
    } fbjs_tok_t;

    /**
    * JSON parser. Contains an array of token blocks available. Also stores
    * the string being parsed now and current position in that string
    */
    typedef struct
    {
        unsigned int pos;     /* offset in the JSON string */
        unsigned int toknext; /* next token to allocate */
        int toksuper;         /* superior token node, e.g parent object or array */
    } fbjs_parser;

    FirebaseJson();
    FirebaseJson(std::string &data);
    ~FirebaseJson();

    /*
    Clear internal buffer of FirebaseJson object.z
    
    @return instance of an object.

   */
    FirebaseJson &clear();

    /*
    Set JSON data (JSON object string) to FirebaseJson object.
    
    @param data - The JSON object string.

    @return instance of an object.

   */
    FirebaseJson &setJsonData(const String &data);

    /*
    Add null to FirebaseJson object.
    
    @param key - The new key string that null to be added.

    @return instance of an object.

   */
    FirebaseJson &add(const String &key);

    /*
    Add string to FirebaseJson object.
    
    @param key - The new key string that string value to be added.

    @param value - The string value for the new specified key.

    @return instance of an object.

   */
    FirebaseJson &add(const String &key, const String &value);

    /*
    Add string (chars array) to FirebaseJson object.
    
    @param key - The new key string that string (chars array) value to be added.

    @param value - The char array for the new specified key.

    @return instance of an object.

   */
    FirebaseJson &add(const String &key, const char *value);

    /*
    Add integer/unsigned short to FirebaseJson object.
    
    @param key - The new key string in which value to be added.

    @param value - The integer/unsigned short value for the new specified key.

    @return instance of an object.

   */
    FirebaseJson &add(const String &key, int value);
    FirebaseJson &add(const String &key, unsigned short value);

    /*
    Add float to FirebaseJson object.
    
    @param key - The new key string that double value to be added.

    @param value - The double value for the new specified key.

    @return instance of an object.

   */

    FirebaseJson &add(const String &key, float value);

    /*
    Add double to FirebaseJson object.
    
    @param key - The new key string that double value to be added.

    @param value - The double value for the new specified key.

    @return instance of an object.

   */
    FirebaseJson &add(const String &key, double value);

    /*
    Add boolean to FirebaseJson object.
    
    @param key - The new key string that bool value to be added.

    @param value - The boolean value for the new specified key.

    @return instance of an object.

   */
    FirebaseJson &add(const String &key, bool value);

    /*
    Add nested FirebaseJson object into FirebaseJson object.
    
    @param key - The new key string that FirebaseJson object to be added.

    @param json - The FirebaseJson object for the new specified key.

    @return instance of an object.

   */
    FirebaseJson &add(const String &key, FirebaseJson &json);

    /*
    Add nested FirebaseJsonArray object into FirebaseJson object.
    
    @param key - The new key string that FirebaseJsonArray object to be added.

    @param arr - The FirebaseJsonArray for the new specified key.

    @return instance of an object.

   */
    FirebaseJson &add(const String &key, FirebaseJsonArray &arr);

    /*
    Get the FirebaseJson object serialized string.

    @param buf - The returning String object. 

    @param prettify - Boolean flag for return the pretty format string i.e. with text indentation and newline. 

   */
    void toString(String &buf, bool prettify = false);

    /*
    Get the value from the specified node path in FirebaseJson object.

    @param jsonData - The returning FirebaseJsonData that holds the returned data.

    @param path - Relative path to the specific node in FirebaseJson object.

    @param prettify - The bool flag for a prettifying string in FirebaseJsonData's stringValue.

    @return boolean status of the operation.

    The FirebaseJsonData object holds the returned data which can be read from the following properties.

    jsonData.stringValue - contains the returned string.

    jsonData.intValue - contains the returned integer value.

    jsonData.floatValue - contains the returned float value.

    jsonData.doubleValue - contains the returned double value.

    jsonData.boolValue - contains the returned boolean value.

    jsonData.success - used to determine the result of the get operation.

    jsonData.type - used to determine the type of returned value in string represent 
    the types of value e.g. string, int, double, boolean, array, object, null and undefined.

    jsonData.typeNum used to determine the type of returned value is an integer as represented by the following value.
    
    FirebaseJson::UNDEFINED = 0
    FirebaseJson::OBJECT = 1
    FirebaseJson::ARRAY = 2
    FirebaseJson::STRING = 3
    FirebaseJson::INT = 4
    FirebaseJson::FLOAT = 5
    FirebaseJson::DOUBLE = 6
    FirebaseJson::BOOL = 7 and
    FirebaseJson::NULL = 8

   */
    bool get(FirebaseJsonData &jsonData, const String &path, bool prettify = false);

    /*
    Parse and collect all node/array elements in FirebaseJson object.

    @param data - The JSON data string to parse (optional to replace the internal buffer with new data).

    @return number of child/array elements in FirebaseJson object.

   */
    size_t iteratorBegin(const char *data = NULL);

    /*
    Get child/array elements from FirebaseJson objects at specified index.
    
    @param index - The element index to get.

    @param type - The integer which holds the type of data i.e. JSON_OBJECT and JSON_ARR

    @param key - The string which holds the key/name of an object, can return empty String if the data type is an array.

    @param value - The string which holds the value for the element key or array.   

   */
    void iteratorGet(size_t index, int &type, String &key, String &value);

    /*
    Clear all iterator buffer (should be called since iteratorBegin was called).

   */
    void iteratorEnd();

    /*
    Set null to FirebaseJson object at the specified node path.
    
    @param path - The relative path that null to be set.

    The relative path can be mixed with array index (number placed inside square brackets) and node names 
    e.g. /myRoot/[2]/Sensor1/myData/[3].

   */
    void set(const String &path);

    /*
    Set String value to FirebaseJson object at the specified node path.
    
    @param path - The relative path that string value to be set.

    @param value - The string value to set.

    The relative path can be mixed with array index (number placed inside square brackets) and node names 
    e.g. /myRoot/[2]/Sensor1/myData/[3].

   */
    void set(const String &path, const String &value);

    /*
    Set string (chars array) value to FirebaseJson object at the specified node path.
    
    @param path - The relative path that string (chars array) to be set.

    @param value - The char array to set.

    The relative path can be mixed with array index (number placed inside square brackets) and node names 
    e.g. /myRoot/[2]/Sensor1/myData/[3].

   */
    void set(const String &path, const char *value);

    /*
    Set integer/unsigned short value to FirebaseJson object at specified node path.
    
    @param path - The relative path that int value to be set.

    @param value - The integer/unsigned short value to set.

    The relative path can be mixed with array index (number placed inside square brackets) and node names 
    e.g. /myRoot/[2]/Sensor1/myData/[3].

   */
    void set(const String &path, int value);
    void set(const String &path, unsigned short value);

    /*
    Set the float value to FirebaseJson object at the specified node path.
    
    @param path - The relative path that float value to be set.

    @param value - The float value to set.

    The relative path can be mixed with array index (number placed inside square brackets) and node names 
    e.g. /myRoot/[2]/Sensor1/myData/[3].

   */
    void set(const String &path, float value);

    /*
    Set the double value to FirebaseJson object at the specified node path.
    
    @param path - The relative path that double value to be set.

    @param value - The double value to set.

    The relative path can be mixed with array index (number placed inside square brackets) and node names 
    e.g. /myRoot/[2]/Sensor1/myData/[3].

   */
    void set(const String &path, double value);

    /*
    Set boolean value to FirebaseJson object at the specified node path.
    
    @param path - The relative path that bool value to be set.

    @param value - The boolean value to set.


    The relative path can be mixed with array index (number placed inside square brackets) and node names 
    e.g. /myRoot/[2]/Sensor1/myData/[3].

   */
    void set(const String &path, bool value);

    /*
    Set nested FirebaseJson object to FirebaseJson object at the specified node path.
    
    @param path - The relative path that nested FirebaseJson object to be set.

    @param json - The FirebaseJson object to set.

    The relative path can be mixed with array index (number placed inside square brackets) and node names 
    e.g. /myRoot/[2]/Sensor1/myData/[3].

   */
    void set(const String &path, FirebaseJson &json);

    /*
    Set nested FirebaseJsonArray object to FirebaseJson object at specified node path.
    
    @param path - The relative path that nested FirebaseJsonArray object to be set.

    @param arr - The FirebaseJsonArray object to set.


    The relative path can be mixed with array index (number placed inside square brackets) and node names 
    e.g. /myRoot/[2]/Sensor1/myData/[3].

   */
    void set(const String &path, FirebaseJsonArray &arr);

    /*
    Remove the specified node and its content.

    @param path - The relative path to remove its contents/children.

    @return bool value represents the success operation.
    */
    bool remove(const String &path);

    template <typename T>
    FirebaseJson &add(const String &key, T value);
    template <typename T>
    bool set(const String &path, T value);

private:
    int _nextToken = 0;
    int _refToken = -1;
    int _nextDepth = 0;
    int _parentIndex = -1;
    int _parseDepth = 0;
    int _skipDepth = -1;
    int _parseCompleted = -1;
    int _refTkIndex = -1;
    int _remTkIndex = -1;
    int _tokenCount = 0;
    bool _TkRefOk = false;
    bool _tokenMatch = false;
    bool _remFirstTk = false;
    bool _remLastTk = false;
    bool _collectTk = false;
    bool _paresRes = false;
    bool _arrReplaced = false;
    bool _arrInserted = false;
    fbjs_type_t _topLevelTkType = JSMN_OBJECT;

    char *_qt = nullptr;
    char *_tab = nullptr;
    char *_brk1 = nullptr;
    char *_brk2 = nullptr;
    char *_brk3 = nullptr;
    char *_brk4 = nullptr;
    char *_cm = nullptr;
    char *_nl = nullptr;
    char *_nll = nullptr;
    char *_pr = nullptr;
    char *_pr2 = nullptr;
    char *_pd = nullptr;
    char *_pf = nullptr;
    char *_fls = nullptr;
    char *_tr = nullptr;
    char *_string = nullptr;
    char *_int = nullptr;
    char *_dbl = nullptr;
    char *_bl = nullptr;
    char *_obj = nullptr;
    char *_arry = nullptr;
    char *_undef = nullptr;
    char *_dot = nullptr;

    std::string _rawbuf = "";
    std::string _tbuf = "";
    tk_index_t _lastTk;
    std::vector<path_tk_t> _pathTk = std::vector<path_tk_t>();
    std::vector<eltk_t> _eltk = std::vector<eltk_t>();
    std::vector<el_t> _el = std::vector<el_t>();
    FirebaseJsonData _jsonData;

    std::shared_ptr<fbjs_parser> _parser = std::shared_ptr<fbjs_parser>(new fbjs_parser());
    std::shared_ptr<fbjs_tok_t> _tokens = nullptr;

    void _init();
    void _finalize();
    FirebaseJson &_setJsonData(std::string &data);
    FirebaseJson &_add(const char *key, const char *value, size_t klen, size_t vlen, bool isString = true, bool isJson = true);
    FirebaseJson &_addArrayStr(const char *value, size_t len, bool isString);
    void _resetParseResult();
    void _setElementType();
    void _addString(const std::string &key, const std::string &value);
    void _addArray(const std::string &key, FirebaseJsonArray *arr);
    void _addInt(const std::string &key, int value);
    void _addFloat(const std::string &key, float value);
    void _addDouble(const std::string &key, double value);
    void _addBool(const std::string &key, bool value);
    void _addNull(const std::string &key);
    void _addJson(const std::string &key, FirebaseJson *json);
    void _setString(const std::string &path, const std::string &value);
    void _setInt(const std::string &path, int value);
    void _setFloat(const std::string &path, float value);
    void _setDouble(const std::string &path, double value);
    void _setBool(const std::string &path, bool value);
    void _setNull(const std::string &path);
    void _setJson(const std::string &path, FirebaseJson *json);
    void _setArray(const std::string &path, FirebaseJsonArray *arr);
    void _set(const char *path, const char *data);
    void clearPathTk();
    void _parse(const char *path, PRINT_MODE printMode);
    void _parse(const char *key, int depth, int index, PRINT_MODE printMode);
    void _compile(const char *key, int depth, int index, const char *replace, PRINT_MODE printMode, int refTokenIndex = -1, bool removeTk = false);
    void _remove(const char *key, int depth, int index, const char *replace, int refTokenIndex = -1, bool removeTk = false);
    void _fbjs_parse(bool collectTk = false);
    bool _updateTkIndex(uint16_t index, int &depth, char *searchKey, int searchIndex, char *replace, PRINT_MODE printMode, bool advanceCount);
    bool _updateTkIndex2(std::string &str, uint16_t index, int &depth, char *searchKey, int searchIndex, char *replace, PRINT_MODE printMode);
    bool _updateTkIndex3(uint16_t index, int &depth, char *searchKey, int searchIndex, PRINT_MODE printMode);
    void _getTkIndex(int depth, tk_index_t &tk);
    void _setMark(int depth, bool mark);
    void _setSkip(int depth, bool skip);
    void _setRef(int depth, bool ref);
    void _insertChilds(char *data, PRINT_MODE printMode);
    void _addObjNodes(std::string &str, std::string &str2, int index, char *data, PRINT_MODE printMode);
    void _addArrNodes(std::string &str, std::string &str2, int index, char *data, PRINT_MODE printMode);
    void _compileToken(uint16_t &i, char *buf, int &depth, char *searchKey, int searchIndex, PRINT_MODE printMode, char *replace, int refTokenIndex = -1, bool removeTk = false);
    void _parseToken(uint16_t &i, char *buf, int &depth, char *searchKey, int searchIndex, PRINT_MODE printMode);
    void _removeToken(uint16_t &i, char *buf, int &depth, char *searchKey, int searchIndex, PRINT_MODE printMode, char *replace, int refTokenIndex = -1, bool removeTk = false);
    single_child_parent_t _findSCParent(int depth);
    bool _isArrTk(int index);
    bool _isStrTk(int index);
    int _getArrIndex(int index);
    char *getFloatString(float value);
    char *getDoubleString(double value);
    char *getIntString(int value);
    char *getBoolString(bool value);
    char *getPGMString(PGM_P pgm);
    void _trimDouble(char *buf);
    void _get(const char *key, int depth, int index = -1);
    void _ltrim(std::string &str, const std::string &chars = " ");
    void _rtrim(std::string &str, const std::string &chars = " ");
    void _trim(std::string &str, const std::string &chars = " ");
    void _toStdString(std::string &s, bool isJson = true);
    void _tostr(std::string &s, bool prettify = false);
    void _strToTk(const std::string &str, std::vector<path_tk_t> &tk, char delim);
    int _strpos(const char *haystack, const char *needle, int offset);
    int _rstrpos(const char *haystack, const char *needle, int offset);
    char *_rstrstr(const char *haystack, const char *needle);
    void _delPtr(char *p);
    char *_newPtr(size_t len);
    char *_newPtr(char *p, size_t len);
    char *_newPtr(char *p, size_t len, char *d);
    char *_getPGMString(PGM_P pgm);

    void fbjs_init(fbjs_parser *parser);
    int fbjs_parse(fbjs_parser *parser, const char *js, size_t len,
                   fbjs_tok_t *tokens, unsigned int num_tokens);
    int fbjs_parse_string(fbjs_parser *parser, const char *js,
                          size_t len, fbjs_tok_t *tokens, size_t num_tokens);
    int fbjs_parse_primitive(fbjs_parser *parser, const char *js,
                             size_t len, fbjs_tok_t *tokens, size_t num_tokens);
    void fbjs_fill_token(fbjs_tok_t *token, fbjs_type_t type,
                         int start, int end);
    fbjs_tok_t *fbjs_alloc_token(fbjs_parser *parser,
                                 fbjs_tok_t *tokens, size_t num_tokens);
};

class FirebaseJsonArray
{

    friend class FirebaseJson;
    friend class FirebaseJsonData;
    friend class Firebase_ESP_Client;
    friend class Firebase_Signer;
    friend class FirebaseData;
    friend class FirebaseStream;
    friend class FB_RTDB;
    friend class FB_CM;

public:
    FirebaseJsonArray();
    ~FirebaseJsonArray();
    void _init();
    void _finalize();

    /*
    Add null to FirebaseJsonArray object.

    @return instance of an object.

   */
    FirebaseJsonArray &add();

    /*
    Add string to FirebaseJsonArray object.

    @param value - The string value to add.

    @return instance of an object.

   */
    FirebaseJsonArray &add(const String &value);

    /*
    Add string (chars arrar) to FirebaseJsonArray object.

    @param value - The char array to add.

    @return instance of an object.

   */
    FirebaseJsonArray &add(const char *value);

    /*
    Add integer/unsigned short to FirebaseJsonArray object.

    @param value - The integer/unsigned short value to add.

    @return instance of an object.

   */
    FirebaseJsonArray &add(int value);
    FirebaseJsonArray &add(unsigned short value);

    /*
    Add float to FirebaseJsonArray object.

    @param value - The float value to add.

    @return instance of an object.

   */
    FirebaseJsonArray &add(float value);

    /*
    Add double to FirebaseJsonArray object.

    @param value - The double value to add.

    @return instance of an object.

   */
    FirebaseJsonArray &add(double value);

    /*
    Add boolean to FirebaseJsonArray object.

    @param value - The boolean value to add.

    @return instance of an object.

   */
    FirebaseJsonArray &add(bool value);

    /*
    Add nested FirebaseJson object  to FirebaseJsonArray object.

    @param json - The FirebaseJson object to add.

    @return instance of an object.

   */
    FirebaseJsonArray &add(FirebaseJson &json);

    /*
    Add nested FirebaseJsonArray object  to FirebaseJsonArray object.

    @param arr - The FirebaseJsonArray object to add.

    @return instance of an object.

   */
    FirebaseJsonArray &add(FirebaseJsonArray &arr);

    /*
    Set JSON array data (JSON array string) to FirebaseJsonArray object.
    
    @param data - The JSON array string.

    @return instance of an object.

   */
    FirebaseJsonArray &setJsonArrayData(const String &data);

    /*
    Get the array value at the specified index from the FirebaseJsonArray object.

    @param jsonData - The returning FirebaseJsonData object that holds data at the specified index.

    @param index - Index of data in FirebaseJsonArray object.    

    @return boolean status of the operation.

   */
    bool get(FirebaseJsonData &jsonData, int index);
    bool get(FirebaseJsonData *jsonData, int index);

    /*
    Get the array value at the specified path from FirebaseJsonArray object.

    @param jsonData - The returning FirebaseJsonData object that holds data at the specified path.

    @param path - Relative path to data in FirebaseJsonArray object.    

    @return boolean status of the operation.

    The relative path must begin with array index (number placed inside square brackets) followed by 
    other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2

   */
    bool get(FirebaseJsonData &jsonData, const String &path);

    /*
    Get the length of the array in FirebaseJsonArray object.  

    @return length of the array.

   */
    size_t size();

    /*
    Get the FirebaseJsonArray object serialized string.

    @param buf - The returning String object. 

    @param prettify - Boolean flag for return the pretty format string i.e. with text indentation and newline. 

   */
    void toString(String &buf, bool prettify = false);

    /*
    Clear all array in FirebaseJsonArray object.

    @return instance of an object.

   */
    FirebaseJsonArray &clear();

    /*
    Set null to FirebaseJsonArray object at specified index.
    
    @param index - The array index that null to be set.

   */
    void set(int index);

    /*
    Set String to FirebaseJsonArray object at the specified index.
    
    @param index - The array index that String value to be set.

    @param value - The String to set.

   */
    void set(int index, const String &value);

    /*
    Set string (chars array) to FirebaseJsonArray object at specified index.
    
    @param index - The array index that string (chars array) to be set.

    @param value - The char array to set.

   */
    void set(int index, const char *value);

    /*
    Set integer/unsigned short value to FirebaseJsonArray object at specified index.
    
    @param index - The array index that int/unsigned short to be set.

    @param value - The integer/unsigned short value to set.

   */
    void set(int index, int value);
    void set(int index, unsigned short value);

    /*
    Set float value to FirebaseJsonArray object at specified index.
    
    @param index - The array index that float value to be set.

    @param value - The float value to set.

   */
    void set(int index, float value);

    /*
    Set double value to FirebaseJsonArray object at specified index.
    
    @param index - The array index that double value to be set.

    @param value - The double value to set.

   */
    void set(int index, double value);

    /*
    Set boolean value to FirebaseJsonArray object at specified index.
    
    @param index - The array index that bool value to be set.

    @param value - The boolean value to set.

   */
    void set(int index, bool value);

    /*
    Set nested FirebaseJson object to FirebaseJsonArray object at specified index.
    
    @param index - The array index that nested FirebaseJson object to be set.

    @param value - The FirebaseJson object to set.

   */
    void set(int index, FirebaseJson &json);

    /*
    Set nested FirebaseJsonArray object to FirebaseJsonArray object at specified index.
    
    @param index - The array index that nested FirebaseJsonArray object to be set.

    @param value - The FirebaseJsonArray object to set.

   */
    void set(int index, FirebaseJsonArray &arr);

    /*
    Set null to FirebaseJson object at the specified path.
    
    @param path - The relative path that null to be set.

    The relative path must begin with array index (number placed inside square brackets) followed by 
    other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

   */
    void set(const String &path);

    /*
    Set String to FirebaseJsonArray object at the specified path.
    
    @param path - The relative path that string value to be set.

    @param value - The String to set.

    The relative path must begin with array index (number placed inside square brackets) followed by 
    other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

   */
    void set(const String &path, const String &value);

    /*
    Set string (chars array) to FirebaseJsonArray object at the specified path.
    
    @param path - The relative path that string (chars array) value to be set.

    @param value - The char array to set.

    The relative path must begin with array index (number places inside square brackets) followed by 
    other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

   */
    void set(const String &path, const char *value);

    /*
    Set integer/unsigned short value to FirebaseJsonArray object at specified path.
    
    @param path - The relative path that integer/unsigned short value to be set.

    @param value - The integer value to set.

    The relative path must begin with array index (number placed inside square brackets) followed by 
    other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

   */
    void set(const String &path, int value);
    void set(const String &path, unsigned short value);

    /*
    Set float value to FirebaseJsonArray object at specified path.
    
    @param path - The relative path that float value to be set.

    @param value - The float to set.

    The relative path must begin with array index (number placed inside square brackets) followed by 
    other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

   */
    void set(const String &path, float value);

    /*
    Set double value to FirebaseJsonArray object at specified path.
    
    @param path - The relative path that double value to be set.

    @param value - The double to set.

    The relative path must begin with array index (number placed inside square brackets) followed by 
    other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

   */
    void set(const String &path, double value);

    /*
    Set boolean value to FirebaseJsonArray object at specified path.
    
    @param path - The relative path that bool value to be set.

    @param value - The boolean value to set.

    The relative path must begin with array index (number placed inside square brackets) followed by 
    other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

   */
    void set(const String &path, bool value);

    /*
    Set the nested FirebaseJson object to FirebaseJsonArray object at the specified path.
    
    @param path - The relative path that nested FirebaseJson object to be set.

    @param value - The FirebaseJson object to set.

    The relative path must begin with array index (number placed inside square brackets) followed by 
    other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

   */
    void set(const String &path, FirebaseJson &json);

    /*
    Set the nested FirebaseJsonArray object to FirebaseJsonArray object at specified path.
    
    @param path - The relative path that nested FirebaseJsonArray object to be set.

    @param value - The FirebaseJsonArray object to set.

    The relative path must begin with array index (number placed inside square brackets) followed by 
    other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

   */
    void set(const String &path, FirebaseJsonArray &arr);

    /*
    Remove the array value at the specified index from the FirebaseJsonArray object.

    @param index - The array index to be removed.

    @return bool value represents the successful operation.

    */
    bool remove(int index);

    /*
    Remove the array value at the specified path from FirebaseJsonArray object.

    @param path - The relative path to array in FirebaseJsonArray object to be removed.

    @return bool value represents the successful operation.

    The relative path must begin with array index (number placed inside square brackets) followed by 
    other array indexes or node names e.g. /[2]/myData would remove the data of myData key inside the array indexes 2.
    
    */
    bool remove(const String &path);

    template <typename T>
    void set(int index, T value);
    template <typename T>
    void set(const String &path, T value);
    template <typename T>
    FirebaseJsonArray &add(T value);

private:
    std::string _jbuf = "";
    FirebaseJson _json;
    size_t _arrLen = 0;
    char *_pd = nullptr;
    char *_pf = nullptr;
    char *_fls = nullptr;
    char *_tr = nullptr;
    char *_brk3 = nullptr;
    char *_brk4 = nullptr;
    char *_nll = nullptr;
    char *_root = nullptr;
    char *_root2 = nullptr;
    char *_qt = nullptr;
    char *_slash = nullptr;

    void _addString(const std::string &value);
    void _addInt(int value);
    void _addFloat(float value);
    void _addDouble(double value);
    void _addBool(bool value);
    void _addNull();
    void _addJson(FirebaseJson *json);
    void _addArray(FirebaseJsonArray *arr);
    void _setString(int index, const std::string &value);
    void _setString(const String &path, const std::string &value);
    void _setInt(int index, int value);
    void _setInt(const String &path, int value);
    void _setFloat(int index, float value);
    void _setFloat(const String &path, float value);
    void _setDouble(int index, double value);
    void _setDouble(const String &path, double value);
    void _setBool(int index, bool value);
    void _setBool(const String &path, bool value);
    void _setNull(int index);
    void _setNull(const String &path);
    void _setJson(int index, FirebaseJson *json);
    void _setJson(const String &path, FirebaseJson *json);
    void _setArray(int index, FirebaseJsonArray *arr);
    void _setArray(const String &path, FirebaseJsonArray *arr);
    void _toStdString(std::string &s);
    void _set2(int index, const char *value, bool isStr = true);
    void _set(const char *path, const char *value, bool isStr = true);
    bool _get(FirebaseJsonData &jsonData, const char *path);
    bool _remove(const char *path);
    void _trimDouble(char *buf);
    char *getFloatString(float value);
    char *getDoubleString(double value);
    char *getIntString(int value);
    char *getBoolString(bool value);
    char *_getPGMString(PGM_P pgm);
    int _strpos(const char *haystack, const char *needle, int offset);
    int _rstrpos(const char *haystack, const char *needle, int offset);
    char *_rstrstr(const char *haystack, const char *needle);
    void _delPtr(char *p);
    char *_newPtr(size_t len);
    char *_newPtr(char *p, size_t len);
    char *_newPtr(char *p, size_t len, char *d);
};

#endif