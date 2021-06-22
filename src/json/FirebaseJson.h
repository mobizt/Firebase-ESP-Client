/*
 * FirebaseJson, version 2.4.0
 * 
 * The Easiest Arduino library to parse, create and edit JSON object using a relative path.
 * 
 * June 22, 2021
 * 
 * Features
 * - Non-recursive parsing.
 * - Unlimited nested node elements.
 * - Using path to access node element. 
 * - Prettify JSON serialization. 
 * 
 * 
 * The zserge's JSON object parser library used as part of this library
 * 
 * The MIT License (MIT)
 * Copyright (c) 2021 K. Suwatchai (Mobizt)
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

#if defined __has_include
#if __has_include(<wirish.h>)
#include <wirish.h>
#undef min
#undef max
#endif
#endif

#include <Arduino.h>
#include <memory>
#include <vector>
#include <string>
#include <stdio.h>
#include <strings.h>
#include <functional>

static const char fb_json_str_1[] PROGMEM = ",";
static const char fb_json_str_2[] PROGMEM = "\"";
static const char fb_json_str_3[] PROGMEM = ":";
static const char fb_json_str_4[] PROGMEM = "%d";
static const char fb_json_str_5[] PROGMEM = "%f";
static const char fb_json_str_6[] PROGMEM = "false";
static const char fb_json_str_7[] PROGMEM = "true";
static const char fb_json_str_8[] PROGMEM = "{";
static const char fb_json_str_9[] PROGMEM = "}";
static const char fb_json_str_10[] PROGMEM = "[";
static const char fb_json_str_11[] PROGMEM = "]";
static const char fb_json_str_12[] PROGMEM = "string";
static const char fb_json_str_13[] PROGMEM = "int";
static const char fb_json_str_14[] PROGMEM = "double";
static const char fb_json_str_15[] PROGMEM = "bool";
static const char fb_json_str_16[] PROGMEM = "object";
static const char fb_json_str_17[] PROGMEM = "array";
static const char fb_json_str_18[] PROGMEM = "null";
static const char fb_json_str_19[] PROGMEM = "undefined";
static const char fb_json_str_20[] PROGMEM = ".";
static const char fb_json_str_21[] PROGMEM = "{\"root\":";
static const char fb_json_str_22[] PROGMEM = "    ";
static const char fb_json_str_24[] PROGMEM = "\n";
static const char fb_json_str_25[] PROGMEM = ": ";
static const char fb_json_str_26[] PROGMEM = "root";
static const char fb_json_str_27[] PROGMEM = "/";
static const char fb_json_str_28[] PROGMEM = "memory allocation error";
static const char fb_json_str_29[] PROGMEM = "invalid character inside JSON object or array";
static const char fb_json_str_30[] PROGMEM = "incompleted JSON object or array";
static const char fb_json_str_31[] PROGMEM = "token array buffer is to small";
static const char fb_json_str_32[] PROGMEM = "\":}";
static const char fb_json_str_33[] PROGMEM = "\":,";
static const char fb_json_str_34[] PROGMEM = "{}";
static const char fb_json_str_35[] PROGMEM = "[]";

class FirebaseJson;
class FirebaseJsonArray;
class FirebaseJsonData;

/***
    * JSON type identifier. Basic types are:
    * 	o Object
    * 	o Array
    * 	o String
    * 	o Other primitive: number, boolean (true/false) or null
    */
typedef enum
{
  fb_json_generic_type_undefined = 0,
  fb_json_generic_type_object = 1,
  fb_json_generic_type_array = 2,
  fb_json_generic_type_string = 3,
  fb_json_generic_type_primitive = 4
} fb_json_generic_type_t;

enum fb_json_err
{
  /** Not enough tokens were provided */
  fb_json_err_nomem = -1,
  /** Invalid character inside JSON string */
  fb_json_err_invalid = -2,
  /** The string is not a full JSON packet, more bytes expected */
  fb_json_err_part = -3
};

/***
    * JSON parser. Contains an array of token blocks available. Also stores
    * the string being parsed now and current position in that string
    */
typedef struct
{
  unsigned int pos;        /** offset in the JSON string */
  unsigned int next_token; /** next token to allocate */
  int super_token;         /** superior token node, e.g parent object or array */
} fb_json_parser;

/***
    * JSON token description.
    * type		type (object, array, string etc.)
    * start	start position in JSON data string
    * end		end position in JSON data string
    */
typedef struct
{
  fb_json_generic_type_t type;
  int16_t start;
  int16_t end;
  int16_t size;
#ifdef FB_JSON_PARENT_LINKS
  int parent;
#endif
} fb_json_token_t;

enum fb_json_str_type
{
  fb_json_str_type_qt,
  fb_json_str_type_tab,
  fb_json_str_type_brk1,
  fb_json_str_type_brk2,
  fb_json_str_type_brk3,
  fb_json_str_type_brk4,
  fb_json_str_type_cm,
  fb_json_str_type_nl,
  fb_json_str_type_nll,
  fb_json_str_type_pr,
  fb_json_str_type_pr2,
  fb_json_str_type_pd,
  fb_json_str_type_pf,
  fb_json_str_type_fls,
  fb_json_str_type_tr,
  fb_json_str_type_string,
  fb_json_str_type_int,
  fb_json_str_type_dbl,
  fb_json_str_type_bl,
  fb_json_str_type_obj,
  fb_json_str_type_arry,
  fb_json_str_type_undef,
  fb_json_str_type_dot,
  fb_json_str_type_empty_obj,
  fb_json_str_type_empty_arr,
  fb_json_str_type_empty_root
};

typedef struct
{
  int code = 0;
  int line = 0;
  std::string function;
  std::string messagge;
} fb_json_last_error_t;

typedef struct
{
  int16_t ref_token = -1;
  int16_t current_level = 0;
  uint16_t tokens_count = 0;
  int16_t next_level = 0;
  int16_t next_token = 0;
  int16_t skip_level = -1;
  int16_t parent_index = -1;
  int16_t parsing_completed_count = -1;
  int16_t ref_token_index = -1;
  int16_t removed_token_index = -1;

  bool to_set_ref_token = false;
  bool to_replace_array = false;
  bool to_insert_array = false;
  bool to_remove_first_token = false;
  bool to_remove_last_token = false;
  bool is_token_matches = false;
  bool create_item_list = false;
  bool parsing_success = false;

} fb_json_parser_info_t;

class FirebaseJsonHelper
{
public:
  FirebaseJsonHelper(fb_json_last_error_t *err) { last_err = err; };
  ~FirebaseJsonHelper(){};

  /*** dtostrf function is taken from 
     * https://github.com/stm32duino/Arduino_Core_STM32/blob/master/cores/arduino/avr/dtostrf.c
    */

  /***
     * dtostrf - Emulation for dtostrf function from avr-libc
     * Copyright (c) 2013 Arduino.  All rights reserved.
     * Written by Cristian Maglie <c.maglie@arduino.cc>
     * This library is free software; you can redistribute it and/or
     * modify it under the terms of the GNU Lesser General Public
     * License as published by the Free Software Foundation; either
     * version 2.1 of the License, or (at your option) any later version.
     * This library is distributed in the hope that it will be useful,
     * but WITHOUT ANY WARRANTY; without even the implied warranty of
     * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     * Lesser General Public License for more details.
     * You should have received a copy of the GNU Lesser General Public
     * License along with this library; if not, write to the Free Software
     * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
    */

  char *dtostrf(double val, signed char width, unsigned char prec, char *sout)
  {
    //Commented code is the original version
    /***
          char fmt[20];
          sprintf(fmt, "%%%d.%df", width, prec);
          sprintf(sout, fmt, val);
          return sout;
        */

    // Handle negative numbers
    uint8_t negative = 0;
    if (val < 0.0)
    {
      negative = 1;
      val = -val;
    }

    // Round correctly so that print(1.999, 2) prints as "2.00"
    double rounding = 0.5;
    for (int i = 0; i < prec; ++i)
    {
      rounding /= 10.0;
    }

    val += rounding;

    // Extract the integer part of the number
    unsigned long int_part = (unsigned long)val;
    double remainder = val - (double)int_part;

    if (prec > 0)
    {
      // Extract digits from the remainder
      unsigned long dec_part = 0;
      double decade = 1.0;
      for (int i = 0; i < prec; i++)
      {
        decade *= 10.0;
      }
      remainder *= decade;
      dec_part = (int)remainder;

      if (negative)
      {
        sprintf(sout, "-%ld.%0*ld", int_part, prec, dec_part);
      }
      else
      {
        sprintf(sout, "%ld.%0*ld", int_part, prec, dec_part);
      }
    }
    else
    {
      if (negative)
      {
        sprintf(sout, "-%ld", int_part);
      }
      else
      {
        sprintf(sout, "%ld", int_part);
      }
    }
    // Handle minimum field width of the output string
    // width is signed value, negative for left adjustment.
    // Range -128,127
    char fmt[129] = "";
    unsigned int w = width;
    if (width < 0)
    {
      negative = 1;
      w = -width;
    }
    else
    {
      negative = 0;
    }

    if (strlen(sout) < w)
    {
      memset(fmt, ' ', 128);
      fmt[w - strlen(sout)] = '\0';
      if (negative == 0)
      {
        char *tmp = (char *)malloc(strlen(sout) + 1);
        strcpy(tmp, sout);
        strcpy(sout, fmt);
        strcat(sout, tmp);
        free(tmp);
      }
      else
      {
        // left adjustment
        strcat(sout, fmt);
      }
    }

    return sout;
  }

  char *strP(PGM_P pgm)
  {
    size_t len = strlen_P(pgm) + 1;
    char *buf = newS(len);
    strcpy_P(buf, pgm);
    buf[len - 1] = 0;
    return buf;
  }

  void setLastError(int code, const char *file, int line, PGM_P msg)
  {
    if (last_err)
    {
      last_err->code = code;
      last_err->function = file;
      last_err->line = line;
      char *tmp = strP(msg);
      last_err->messagge = tmp;
      delS(tmp);
    }
  }

  void clearLastError()
  {
    if (last_err)
    {
      last_err->code = 0;
      last_err->function = "";
      last_err->line = 0;
      last_err->messagge = "";
    }
  }

  int strpos(const char *haystack, const char *needle, int offset)
  {
    size_t len = strlen(haystack);
    size_t len2 = strlen(needle);
    if (len == 0 || len < len2 || len2 == 0)
      return -1;
    char *_haystack = newS(len - offset + 1);
    if (!_haystack)
    {
      setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
      return -1;
    }
    _haystack[len - offset] = 0;
    strncpy(_haystack, haystack + offset, len - offset);
    char *p = strstr(_haystack, needle);
    int r = -1;
    if (p)
      r = p - _haystack + offset;
    delS(_haystack);
    return r;
  }

  int rstrpos(const char *haystack, const char *needle, int offset)
  {
    size_t len = strlen(haystack);
    size_t len2 = strlen(needle);
    if (len == 0 || len < len2 || len2 == 0)
      return -1;
    char *_haystack = newS(len - offset + 1);
    if (!_haystack)
    {
      setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
      return -1;
    }
    _haystack[len - offset] = 0;
    strncpy(_haystack, haystack + offset, len - offset);
    char *p = rstrstr(_haystack, needle);
    int r = -1;
    if (p)
      r = p - _haystack + offset;
    delS(_haystack);
    return r;
  }

  char *rstrstr(const char *haystack, const char *needle)
  {
    size_t needle_sizegth = strlen(needle);
    const char *haystack_end = haystack + strlen(haystack) - needle_sizegth;
    const char *p;
    size_t i;
    for (p = haystack_end; p >= haystack; --p)
    {
      for (i = 0; i < needle_sizegth; ++i)
      {
        if (p[i] != needle[i])
          goto next;
      }
      return (char *)p;
    next:;
    }
    return 0;
  }

  void delS(char *p)
  {
    if (p != nullptr)
      delete[] p;
  }

  char *newS(size_t len)
  {
    char *p = new char[len];
    memset(p, 0, len);
    return p;
  }

  char *floatStr(float value)
  {
    char *buf = newS(36);
    dtostrf(value, 7, 6, buf);
    return buf;
  }

  char *intStr(int value)
  {
    char *buf = newS(36);
    sprintf(buf, "%d", value);
    return buf;
  }

  char *boolStr(bool value)
  {
    char *buf = nullptr;
    value ? buf = strP(fb_json_str_7) : buf = strP(fb_json_str_6);
    return buf;
  }

  char *doubleStr(double value)
  {
    char *buf = newS(36);
    dtostrf(value, 12, 9, buf);
    return buf;
  }

  void trimDouble(char *buf)
  {
    size_t i = strlen(buf) - 1;
    while (buf[i] == '0' && i > 0)
    {
      if (buf[i - 1] == '.')
      {
        i--;
        break;
      }
      if (buf[i - 1] != '0')
        break;
      i--;
    }
    if (i < strlen(buf) - 1)
      buf[i] = '\0';
  }

  char *getStr(fb_json_str_type type)
  {
    switch (type)
    {
    case fb_json_str_type_qt:
      return strP(fb_json_str_2);
    case fb_json_str_type_tab:
      return strP(fb_json_str_22);
    case fb_json_str_type_brk1:
      return strP(fb_json_str_8);
    case fb_json_str_type_brk2:
      return strP(fb_json_str_9);
    case fb_json_str_type_brk3:
      return strP(fb_json_str_10);
    case fb_json_str_type_brk4:
      return strP(fb_json_str_11);
    case fb_json_str_type_cm:
      return strP(fb_json_str_1);
    case fb_json_str_type_pr2:
      return strP(fb_json_str_3);
    case fb_json_str_type_nl:
      return strP(fb_json_str_24);
    case fb_json_str_type_nll:
      return strP(fb_json_str_18);
    case fb_json_str_type_pr:
      return strP(fb_json_str_25);
    case fb_json_str_type_pd:
      return strP(fb_json_str_4);
    case fb_json_str_type_pf:
      return strP(fb_json_str_5);
    case fb_json_str_type_fls:
      return strP(fb_json_str_6);
    case fb_json_str_type_tr:
      return strP(fb_json_str_7);
    case fb_json_str_type_string:
      return strP(fb_json_str_12);
    case fb_json_str_type_int:
      return strP(fb_json_str_13);
    case fb_json_str_type_dbl:
      return strP(fb_json_str_14);
    case fb_json_str_type_bl:
      return strP(fb_json_str_15);
    case fb_json_str_type_obj:
      return strP(fb_json_str_16);
    case fb_json_str_type_arry:
      return strP(fb_json_str_17);
    case fb_json_str_type_undef:
      return strP(fb_json_str_19);
    case fb_json_str_type_dot:
      return strP(fb_json_str_20);
    case fb_json_str_type_empty_obj:
      return strP(fb_json_str_34);
    case fb_json_str_type_empty_arr:
      return strP(fb_json_str_35);
    case fb_json_str_type_empty_root:
      return strP(fb_json_str_21);
    default:
      return nullptr;
    }
    return nullptr;
  }

  void appendS(std::string &s, fb_json_str_type type)
  {
    char *tmp = getStr(type);
    if (tmp)
    {
      s += tmp;
      delS(tmp);
    }
  }

  void storeS(std::string &s, const char *v, bool append)
  {
    if (!append)
      clearS(s);
#if defined(ESP32)
    s += v;
    s.shrink_to_fit();
#else
    if (!append)
      s = v;
    else
    {
      std::string t = s;
      t += v;
      clearS(s);
      s = t;
      clearS(t);
    }
#endif
  }

  void storeS(std::string &s, char v, bool append)
  {
    std::string t;
    t += v;
    storeS(s, t.c_str(), append);
    clearS(t);
  }

  void clearS(std::string &s)
  {
    s.clear();
    std::string().swap(s);
  }

  void shrinkS(std::string &s)
  {
#if defined(ESP32)
    s.shrink_to_fit();
#else
    std::string t = s;
    clearS(s);
    s = t;
    clearS(t);
#endif
  }

  void buildPath(std::string &json_path, const char *node, bool isArray)
  {
    if (isArray)
    {
      char *root = strP(fb_json_str_26);
      char *slash = strP(fb_json_str_27);
      json_path = root;
      json_path += slash;
      json_path += node;
      delS(root);
      delS(slash);
    }
    else
      json_path = node;
  }

private:
  fb_json_last_error_t *last_err = nullptr;
};

class FirebaseJsonData
{
  friend class FirebaseJson;
  friend class FirebaseJsonArray;

public:
  FirebaseJsonData();
  ~FirebaseJsonData();

  /**
     * Get array data as FirebaseJsonArray object from FirebaseJsonData object.
     * 
     * @param jsonArray - The returning FirebaseJsonArray object.
     * @return bool status for successful operation.
     * This should call after parse or get function.
    */
  bool getArray(FirebaseJsonArray &jsonArray);

  /**
     * Get array data as FirebaseJsonArray object from string.
     * @param source - The JSON array string.
     * @param jsonArray - The returning FirebaseJsonArray object.
     * @return bool status for successful operation.
     * This should call after parse or get function.
    */
  bool getArray(const char *source, FirebaseJsonArray &jsonArray);

  /**
     * Get JSON data as FirebaseJson object from FirebaseJsonData object.
     * 
     * @param json - The returning FirebaseJson object.
     * @return bool status for successful operation.
     * This should call after parse or get function.
    */
  bool getJSON(FirebaseJson &json);

  /**
     * Get JSON data as FirebaseJson object from string.
     * 
     * @param source - The JSON array string.
     * @param json - The returning FirebaseJson object.
     * @return bool status for successful operation.
     * This should call after parse or get function.
    */
  bool getJSON(const char *source, FirebaseJson &json);

  /**
     * Clear internal buffer.
    */
  void clear();

  /**
     * The String value of parses data.
    */
  String stringValue;

  /**
     * The int value of parses data.
    */
  int intValue = 0;

  /**
     * The float value of parses data.
    */
  float floatValue = 0.0f;

  /**
     * The double value of parses data.
    */
  double doubleValue = 0.0;

  /**
     * The bool value of parses data.
    */
  bool boolValue = false;

  /**
     * The type String of parses data.
    */
  String type;

  /**
     * The type (number) of parses data.
    */
  uint8_t typeNum = 0;

  /**
     * The success flag of parsing data.
    */
  bool success = false;

private:
  int16_t type_num = 0;
  int16_t k_start = 0;
  int16_t start = 0;
  int16_t end = 0;
  int16_t token_index = 0;
  int16_t level = 0;
  int arr_size = 0;
  std::string data_raw;

  void setTokenInfo(fb_json_token_t *token, int16_t index, int16_t level, bool success);
  void shrinkS(std::string &s);
  void clearS(std::string &s);
  char *strP(PGM_P p);
  void delS(char *s);
};

class FirebaseJson
{
  friend class FirebaseJsonArray;
  friend class FirebaseJsonData;

public:
  typedef enum
  {
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

  typedef enum
  {
    fb_json_serialize_mode_none = -1,
    fb_json_serialize_mode_plain = 0,
    fb_json_serialize_mode_pretty = 1
  } fb_json_serialize_mode;

  typedef struct
  {
    bool matched = false;
    std::string path;
  } path_tk_t;

  typedef struct
  {
    int16_t index;
    bool first_token;
    bool last_token;
    bool success;
  } single_child_parent_t;

  typedef struct
  {
    uint16_t index;
    uint8_t type;
  } token_item_info_t;

  typedef struct
  {
    int16_t index = -1;
    uint8_t type = fb_json_generic_type_undefined;
    uint16_t node_length = 0;
    uint16_t node_index = 0;
    int level = -1;
    bool node_mark = false;
    bool ref = false;
    bool skip = false;
  } fb_json_token_descriptor_t;

  typedef enum
  {
    fb_json_token_update_mode_parsing,
    fb_json_token_update_mode_compile,
    fb_json_token_update_mode_remove,

  } fb_json_token_update_mode;

  typedef enum
  {
    fb_json_token_descriptor_update_ref,
    fb_json_token_descriptor_update_skip,
    fb_json_token_descriptor_update_mark
  } fb_json_token_descriptor_update_type;

  FirebaseJson();
  FirebaseJson(std::string &data);
  ~FirebaseJson();

  /**
     * Clear internal buffer of FirebaseJson object.
     * 
     * @return instance of an object.
    */
  FirebaseJson &clear();

  /**
     * Set JSON data (JSON object string) to FirebaseJson object.
     * 
     * @param data - The JSON object string.
     * @return instance of an object.
    */
  FirebaseJson &setJsonData(const String &data);

  /**
     * Add null to FirebaseJson object.
     * 
     * @param key - The new key string that null to be added.
     * @return instance of an object.
    */
  FirebaseJson &add(const String &key);

  /**
     * Add string to FirebaseJson object.
     * 
     * @param key - The new key string that string value to be added.
     * @param value - The string value for the new specified key.
     * @return instance of an object.
    */
  FirebaseJson &add(const String &key, const String &value);

  /**
     * Add string (chars array) to FirebaseJson object.
     * 
     * @param key - The new key string that string (chars array) value to be added.
     * @param value - The char array for the new specified key.
     * @return instance of an object.
    */
  FirebaseJson &add(const String &key, const char *value);

  /**
     * Add integer/unsigned short to FirebaseJson object.
     * 
     * @param key - The new key string in which value to be added.
     * @param value - The integer/unsigned short value for the new specified key.
     * @return instance of an object.
    */
  FirebaseJson &add(const String &key, int value);
  FirebaseJson &add(const String &key, unsigned short value);

  /**
     * Add float to FirebaseJson object.
     * 
     * @param key - The new key string that double value to be added.
     * @param value - The double value for the new specified key.
     * @return instance of an object.
    */
  FirebaseJson &add(const String &key, float value);

  /**
     * Add double to FirebaseJson object.
     * 
     * @param key - The new key string that double value to be added.
     * @param value - The double value for the new specified key.
     * @return instance of an object.
    */
  FirebaseJson &add(const String &key, double value);

  /**
     * Add boolean to FirebaseJson object.
     * 
     * @param key - The new key string that bool value to be added.
     * @param value - The boolean value for the new specified key.
     * @return instance of an object.
    */
  FirebaseJson &add(const String &key, bool value);

  /**
     * Add nested FirebaseJson object into FirebaseJson object.
     * 
     * @param key - The new key string that FirebaseJson object to be added.
     * @param json - The FirebaseJson object for the new specified key.
     * @return instance of an object.
    */
  FirebaseJson &add(const String &key, FirebaseJson &json);

  /**
     * Add nested FirebaseJsonArray object into FirebaseJson object.
     * 
     * @param key - The new key string that FirebaseJsonArray object to be added.
     * @param arr - The FirebaseJsonArray for the new specified key.
     * @return instance of an object.
    */
  FirebaseJson &add(const String &key, FirebaseJsonArray &arr);

  /**
     * Get the FirebaseJson object serialized string.
     * 
     * @param buf - The returning String object.
     * @param prettify - Boolean flag for return the pretty format string i.e. with text indentation and newline.
    */
  void toString(String &buf, bool prettify = false);

  /**
     * Get the value from the specified node path in FirebaseJson object.
     * 
     * @param jsonData - The returning FirebaseJsonData that holds the returned data.
     * @param path - Relative path to the specific node in FirebaseJson object.
     * @param prettify - The bool flag for a prettifying string in FirebaseJsonData's stringValue.
     * @return boolean status of the operation.
     * 
     * The FirebaseJsonData object holds the returned data which can be read from the following properties.
     * jsonData.stringValue - contains the returned string.
     * jsonData.intValue - contains the returned integer value.
     * jsonData.floatValue - contains the returned float value.
     * jsonData.doubleValue - contains the returned double value.
     * jsonData.boolValue - contains the returned boolean value.
     * jsonData.success - used to determine the result of the get operation.
     * jsonData.type - used to determine the type of returned value in string represent 
     * the types of value e.g. string, int, double, boolean, array, object, null and undefined.
     * 
     * jsonData.typeNum used to determine the type of returned value is an integer as represented by the following value.
     * FirebaseJson::UNDEFINED = 0
     * FirebaseJson::OBJECT = 1
     * FirebaseJson::ARRAY = 2
     * FirebaseJson::STRING = 3
     * FirebaseJson::INT = 4
     * FirebaseJson::FLOAT = 5
     * FirebaseJson::DOUBLE = 6
     * FirebaseJson::BOOL = 7 and
     * FirebaseJson::NULL = 8
    */
  bool get(FirebaseJsonData &jsonData, const String &path, bool prettify = false);

  /**
     * Parse and collect all node/array elements in FirebaseJson object.
     * 
     * @param data - The JSON data string to parse (optional to replace the internal buffer with new data).
     * @return number of child/array elements in FirebaseJson object.
    */
  size_t iteratorBegin(const char *data = NULL);

  /**
     * Get child/array elements from FirebaseJson objects at specified index.
     * 
     * @param index - The element index to get.
     * @param type - The integer which holds the type of data i.e. JSON_OBJECT and JSON_ARR
     * @param key - The string which holds the key/name of an object, can return empty String if the data type is an array.
     * @param value - The string which holds the value for the element key or array.
     */
  void iteratorGet(size_t index, int &type, String &key, String &value);

  /**
     * Clear all iterator buffer (should be called since iteratorBegin was called).
    */
  void iteratorEnd();

  /**
     * Set null to FirebaseJson object at the specified node path.
     * 
     * @param path - The relative path that null to be set.
     * The relative path can be mixed with array index (number placed inside square brackets) and node names
     * e.g. /myRoot/[2]/Sensor1/myData/[3].
    */
  void set(const String &path);

  /**
     * Set String value to FirebaseJson object at the specified node path.
     * 
     * @param path - The relative path that string value to be set.
     * @param value - The string value to set.
     * 
     * The relative path can be mixed with array index (number placed inside square brackets) and node names
     * e.g. /myRoot/[2]/Sensor1/myData/[3].
    */
  void set(const String &path, const String &value);

  /**
     * Set string (chars array) value to FirebaseJson object at the specified node path.
     * 
     * @param path - The relative path that string (chars array) to be set.
     * @param value - The char array to set.
     * The relative path can be mixed with array index (number placed inside square brackets) and node names
     * e.g. /myRoot/[2]/Sensor1/myData/[3].
    */
  void set(const String &path, const char *value);

  /**
     * Set integer/unsigned short value to FirebaseJson object at specified node path.
     * 
     * @param path - The relative path that int value to be set.
     * @param value - The integer/unsigned short value to set.
     * The relative path can be mixed with array index (number placed inside square brackets) and node names
     * e.g. /myRoot/[2]/Sensor1/myData/[3].
    */
  void set(const String &path, int value);
  void set(const String &path, unsigned short value);

  /**
     * Set the float value to FirebaseJson object at the specified node path.
     * 
     * @param path - The relative path that float value to be set.
     * @param value - The float value to set.
     * The relative path can be mixed with array index (number placed inside square brackets) and node names
     * e.g. /myRoot/[2]/Sensor1/myData/[3].
    */
  void set(const String &path, float value);

  /**
     * Set the double value to FirebaseJson object at the specified node path.
     * 
     * @param path - The relative path that double value to be set.
     * @param value - The double value to set.
     * The relative path can be mixed with array index (number placed inside square brackets) and node names
     * e.g. /myRoot/[2]/Sensor1/myData/[3].
    */
  void set(const String &path, double value);

  /**
     * Set boolean value to FirebaseJson object at the specified node path.
     * 
     * @param path - The relative path that bool value to be set.
     * @param value - The boolean value to set.
     * The relative path can be mixed with array index (number placed inside square brackets) and node names
     * e.g. /myRoot/[2]/Sensor1/myData/[3].
    */
  void set(const String &path, bool value);

  /**
     * Set nested FirebaseJson object to FirebaseJson object at the specified node path.
     * 
     * @param path - The relative path that nested FirebaseJson object to be set.
     * @param json - The FirebaseJson object to set.
     * The relative path can be mixed with array index (number placed inside square brackets) and node names
     * e.g. /myRoot/[2]/Sensor1/myData/[3].
    */
  void set(const String &path, FirebaseJson &json);

  /**
     * Set nested FirebaseJsonAtrray object to FirebaseJson object at specified node path.
     * 
     * @param path - The relative path that nested FirebaseJsonAtrray object to be set.
     * @param arr - The FirebaseJsonAtrray object to set.
     * The relative path can be mixed with array index (number placed inside square brackets) and node names
     * e.g. /myRoot/[2]/Sensor1/myData/[3].
    */
  void set(const String &path, FirebaseJsonArray &arr);

  /**
     * Remove the specified node and its content.
     * 
     * @param path - The relative path to remove its contents/children.
     * @return bool value represents the success operation.
    */
  bool remove(const String &path);

  /**
   * Get raw JSON
   * @return raw JSON string
   */
  const char *raw();

  /**
     * Get last error of operation.
     * 
     * @return fb_json_last_error_t structured data of error
     */
  fb_json_last_error_t getLastError();

  template <typename T>
  FirebaseJson &add(const String &key, T value);

  template <typename T>
  bool set(const String &path, T value);

private:
  fb_json_last_error_t last_error;

  FirebaseJsonHelper *hp = new FirebaseJsonHelper(&last_error);

  fb_json_generic_type_t top_level_token_type = fb_json_generic_type_object;

  fb_json_parser_info_t parser_info;

  std::string raw_buf;
  std::string temp_buf;
  fb_json_token_descriptor_t last_token;
  std::vector<path_tk_t> path_list = std::vector<path_tk_t>();
  std::vector<token_item_info_t> token_item_info_list = std::vector<token_item_info_t>();
  std::vector<fb_json_token_descriptor_t> fb_json_token_descriptor_list = std::vector<fb_json_token_descriptor_t>();
  FirebaseJsonData result;

  std::shared_ptr<fb_json_parser> parser = std::shared_ptr<fb_json_parser>(new fb_json_parser());
  std::shared_ptr<fb_json_token_t> tokens = nullptr;

  FirebaseJson &int_setJsonData(std::string &data);
  FirebaseJson &int_add(const char *key, const char *value, size_t klen, size_t vlen, bool isString = true, bool isJson = true);
  FirebaseJson &int_addArrayStr(const char *value, size_t len, bool isString);
  void int_resetParsserInfo();
  void int_resetParseResult();
  void int_setElementType();
  void int_addString(const std::string &key, const std::string &value);
  void int_addArray(const std::string &key, FirebaseJsonArray *arr);
  void int_addInt(const std::string &key, int value);
  void int_addFloat(const std::string &key, float value);
  void int_addDouble(const std::string &key, double value);
  void int_addBool(const std::string &key, bool value);
  void int_addNull(const std::string &key);
  void int_addJson(const std::string &key, FirebaseJson *json);
  void int_setString(const std::string &path, const std::string &value);
  void int_setInt(const std::string &path, int value);
  void int_setFloat(const std::string &path, float value);
  void int_setDouble(const std::string &path, double value);
  void int_setBool(const std::string &path, bool value);
  void int_setNull(const std::string &path);
  void int_setJson(const std::string &path, FirebaseJson *json);
  void int_setArray(const std::string &path, FirebaseJsonArray *arr);
  void int_set(const char *path, const char *data);
  void int_clearPathList();
  void int_clearTokenList();
  void int_clearTokenDescriptorList();
  void int_parse(const char *path, fb_json_serialize_mode deserializeMode);
  void int_parse(const char *key, int16_t level, int16_t index, fb_json_serialize_mode deserializeMode);
  void int_compile(const char *key, int16_t level, int16_t index, const char *replace, fb_json_serialize_mode deserializeMode, int16_t refTokenIndex = -1, bool isRemove = false);
  void int_remove(const char *key, int16_t level, int16_t index, const char *replace, int16_t refTokenIndex = -1, bool isRemove = false);
  void int_fb_json_parseToken(bool create_item_list = false);
  bool int_updateTokenIndex(fb_json_token_update_mode mode, uint16_t index, int16_t &level, const char *searchKey, int16_t searchIndex, const char *replace, fb_json_serialize_mode deserializeMode, bool advancedCount, std::string &buf);
  void int_getTokenIndex(int level, fb_json_token_descriptor_t &tk);
  void int_updateDescriptor(fb_json_token_descriptor_update_type type, int level, bool value);
  void int_appendTab(std::string &s, int level, bool notEmpty, bool newLine = false);
  void int_insertChilds(const char *data, fb_json_serialize_mode deserializeMode);
  void int_addObjNodes(std::string &str, std::string &str2, int16_t index, const char *data, fb_json_serialize_mode deserializeMode);
  void int_addArrNodes(std::string &str, std::string &str2, int16_t index, const char *data, fb_json_serialize_mode deserializeMode);
  void int_compileToken(uint16_t &i, const char *buf, int16_t &level, const char *searchKey, int16_t searchIndex, fb_json_serialize_mode deserializeMode, const char *replace, int16_t refTokenIndex = -1, bool isRemove = false);
  void int_parseToken(uint16_t &i, const char *buf, int16_t &level, const char *searchKey, int16_t searchIndex, fb_json_serialize_mode deserializeMode);
  void int_removeToken(uint16_t &i, const char *buf, int16_t &level, const char *searchKey, int16_t searchIndex, fb_json_serialize_mode deserializeMode, const char *replace, int16_t refTokenIndex = -1, bool isRemove = false);
  void int_addDescriptorList(fb_json_token_t *token, int16_t index, int16_t level, bool mark, bool ref, fb_json_token_descriptor_t *descr);
  single_child_parent_t int_findSingleChildParent(int level);
  bool int_isArray(int index);
  bool int_isString(int index);
  int int_getArrIndex(int index);
  void int_get(const char *key, int level, int index = -1);
  void int_ltrim(std::string &str, const std::string &chars = " ");
  void int_rtrim(std::string &str, const std::string &chars = " ");
  void int_trim(std::string &str, const std::string &chars = " ");
  void int_splitTokens(const std::string &str, std::vector<path_tk_t> &tk, char delim);

  void int_fb_json_init(fb_json_parser *parser);
  int int_fb_json_parse(fb_json_parser *parser, const char *js, size_t len, fb_json_token_t *tokens, unsigned int num_tokens);
  int int_fb_json_parse_string(fb_json_parser *parser, const char *js, size_t len, fb_json_token_t *tokens, size_t num_tokens);
  int int_fb_json_parse_primitive(fb_json_parser *parser, const char *js, size_t len, fb_json_token_t *tokens, size_t num_tokens);
  void int_fb_json_fill_token(fb_json_token_t *token, fb_json_generic_type_t type, int start, int end);
  fb_json_token_t *int_fb_json_alloc_token(fb_json_parser *parser, fb_json_token_t *tokens, size_t num_tokens);
};

class FirebaseJsonArray
{

  friend class FirebaseJson;
  friend class FirebaseJsonData;

public:
  FirebaseJsonArray();
  FirebaseJsonArray(fb_json_last_error_t *lastErr);
  ~FirebaseJsonArray();

  /**
     * Add null to FirebaseJsonArray object.
     * 
     * @return instance of an object.
    */
  FirebaseJsonArray &add();

  /**
     * Add string to FirebaseJsonArray object.
     * 
     * @param value - The string value to add.
     * @return instance of an object.
    */
  FirebaseJsonArray &add(const String &value);

  /**
     * Add string (chars arrar) to FirebaseJsonArray object.
     * 
     * @param value - The char array to add.
     * @return instance of an object.
    */
  FirebaseJsonArray &add(const char *value);

  /**
     * Add integer/unsigned short to FirebaseJsonArray object.
     * 
     * @param value - The integer/unsigned short value to add.
     * @return instance of an object.
    */
  FirebaseJsonArray &add(int value);
  FirebaseJsonArray &add(unsigned short value);

  /**
     * Add float to FirebaseJsonArray object.
     * 
     * @param value - The float value to add.
     * @return instance of an object.
    */
  FirebaseJsonArray &add(float value);

  /**
     * Add double to FirebaseJsonArray object.
     * 
     * @param value - The double value to add.
     * @return instance of an object.
    */
  FirebaseJsonArray &add(double value);

  /**
     * Add boolean to FirebaseJsonArray object.
     * 
     * @param value - The boolean value to add.
     * @return instance of an object.
    */
  FirebaseJsonArray &add(bool value);

  /**
     * Add nested FirebaseJson object  to FirebaseJsonArray object.
     * 
     * @param json - The FirebaseJson object to add.
     * @return instance of an object.
    */
  FirebaseJsonArray &add(FirebaseJson &json);

  /**
     * Add nested FirebaseJsonArray object  to FirebaseJsonArray object.
     * 
     * @param arr - The FirebaseJsonArray object to add.
     * @return instance of an object.
    */
  FirebaseJsonArray &add(FirebaseJsonArray &arr);

  /**
     * Set JSON array data (JSON array string) to FirebaseJsonArray object.
     * 
     * @param data - The JSON array string.
     * @return instance of an object.
    */
  FirebaseJsonArray &setJsonArrayData(const String &data);

  /**
     * Get the array value at the specified index from the FirebaseJsonArray object.
     * 
     * @param jsonData - The returning FirebaseJsonData object that holds data at the specified index.
     * @param index - Index of data in FirebaseJsonArray object.
     * @return boolean status of the operation.
    */
  bool get(FirebaseJsonData &jsonData, int index);
  bool get(FirebaseJsonData *jsonData, int index);

  /**
     * Get the array value at the specified path from FirebaseJsonArray object.
     * 
     * @param jsonData - The returning FirebaseJsonData object that holds data at the specified path.
     * @param path - Relative path to data in FirebaseJsonArray object.
     * @return boolean status of the operation.
     * The relative path must begin with array index (number placed inside square brackets) followed by
     * other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2
    */
  bool get(FirebaseJsonData &jsonData, const String &path);

  /**
     * Get the length of the array in FirebaseJsonArray object.
     * 
     * @return length of the array.
    */
  size_t size();

  /**
     * Get the FirebaseJsonArray object serialized string.
     * 
     * @param buf - The returning String object.
     * @param prettify - Boolean flag for return the pretty format string i.e. with text indentation and newline.
    */
  void toString(String &buf, bool prettify = false);

  /**
   * Get raw JSON Array
   * @return raw JSON Array string
   */
  const char *raw();

  /**
     * Clear all array in FirebaseJsonArray object.
     * 
     * @return instance of an object.
    */
  FirebaseJsonArray &clear();

  /**
     * Set null to FirebaseJsonArray object at specified index.
     * 
     * @param index - The array index that null to be set.
    */
  void set(int index);

  /**
     * Set String to FirebaseJsonArray object at the specified index.
     * 
     * @param index - The array index that String value to be set.
     * @param value - The String to set.
    */
  void set(int index, const String &value);

  /**
     * Set string (chars array) to FirebaseJsonArray object at specified index.
     * 
     * @param index - The array index that string (chars array) to be set.
     * @param value - The char array to set.
    */
  void set(int index, const char *value);

  /**
     * Set integer/unsigned short value to FirebaseJsonArray object at specified index.
     * 
     * @param index - The array index that int/unsigned short to be set.
     * @param value - The integer/unsigned short value to set.
    */
  void set(int index, int value);
  void set(int index, unsigned short value);

  /**
     * Set float value to FirebaseJsonArray object at specified index.
     * 
     * @param index - The array index that float value to be set.
     * @param value - The float value to set.
    */
  void set(int index, float value);

  /**
     * Set double value to FirebaseJsonArray object at specified index.
     * 
     * @param index - The array index that double value to be set.
     * @param value - The double value to set.
    */
  void set(int index, double value);

  /**
     * Set boolean value to FirebaseJsonArray object at specified index.
     * 
     * @param index - The array index that bool value to be set.
     * @param value - The boolean value to set.
    */
  void set(int index, bool value);

  /**
     * Set nested FirebaseJson object to FirebaseJsonArray object at specified index.
     * 
     * @param index - The array index that nested FirebaseJson object to be set.
     * @param value - The FirebaseJson object to set.
    */
  void set(int index, FirebaseJson &json);

  /**
     * Set nested FirebaseJsonArray object to FirebaseJsonArray object at specified index.
     * 
     * @param index - The array index that nested FirebaseJsonArray object to be set.
     * @param value - The FirebaseJsonArray object to set.
    */
  void set(int index, FirebaseJsonArray &arr);

  /**
     * Set null to FirebaseJson object at the specified path.
     * 
     * @param path - The relative path that null to be set.
     * The relative path must begin with array index (number placed inside square brackets) followed by
     * other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.
    */
  void set(const String &path);

  /**
     * Set String to FirebaseJsonArray object at the specified path.
     * 
     * @param path - The relative path that string value to be set.
     * @param value - The String to set.
     * The relative path must begin with array index (number placed inside square brackets) followed by
     * other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.
    */
  void set(const String &path, const String &value);

  /**
     * Set string (chars array) to FirebaseJsonArray object at the specified path.
     * 
     * @param path - The relative path that string (chars array) value to be set.
     * @param value - The char array to set.
     * The relative path must begin with array index (number places inside square brackets) followed by
     * other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.
    */
  void set(const String &path, const char *value);

  /**
     * Set integer/unsigned short value to FirebaseJsonArray object at specified path.
     * 
     * @param path - The relative path that integer/unsigned short value to be set.
     * @param value - The integer value to set.
     * The relative path must begin with array index (number placed inside square brackets) followed by
     * other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.
    */
  void set(const String &path, int value);
  void set(const String &path, unsigned short value);

  /**
     * Set float value to FirebaseJsonArray object at specified path.
     * 
     * @param path - The relative path that float value to be set.
     * @param value - The float to set.
     * The relative path must begin with array index (number placed inside square brackets) followed by
     * other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.
    */
  void set(const String &path, float value);

  /**
     * Set double value to FirebaseJsonArray object at specified path.
     * 
     * @param path - The relative path that double value to be set.
     * @param value - The double to set.
     * The relative path must begin with array index (number placed inside square brackets) followed by
     * other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.
    */
  void set(const String &path, double value);

  /**
     * Set boolean value to FirebaseJsonArray object at specified path.
     * 
     * @param path - The relative path that bool value to be set.
     * @param value - The boolean value to set.
     * The relative path must begin with array index (number placed inside square brackets) followed by
     * other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.
    */
  void set(const String &path, bool value);

  /**
     * Set the nested FirebaseJson object to FirebaseJsonArray object at the specified path.
     * 
     * @param path - The relative path that nested FirebaseJson object to be set.
     * @param value - The FirebaseJson object to set.
     * The relative path must begin with array index (number placed inside square brackets) followed by
     * other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.
    */
  void set(const String &path, FirebaseJson &json);

  /**
     * Set the nested FirebaseJsonArray object to FirebaseJsonArray object at specified path.
     * 
     * @param path - The relative path that nested FirebaseJsonArray object to be set.
     * @param value - The FirebaseJsonArray object to set.
     * The relative path must begin with array index (number placed inside square brackets) followed by
     * other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.
    */
  void set(const String &path, FirebaseJsonArray &arr);

  /**
     * Remove the array value at the specified index from the FirebaseJsonArray object.
     * 
     * @param index - The array index to be removed.
     * @return bool value represents the successful operation.
    */
  bool remove(int index);

  /**
     * Remove the array value at the specified path from FirebaseJsonArray object.
     * 
     * @param path - The relative path to array in FirebaseJsonArray object to be removed.
     * @return bool value represents the successful operation.
     * The relative path must begin with array index (number placed inside square brackets) followed by
     * other array indexes or node names e.g. /[2]/myData would remove the data of myData key inside the array indexes 2.
    */
  bool remove(const String &path);

  template <typename T>
  void set(int index, T value);
  template <typename T>
  void set(const String &path, T value);
  template <typename T>
  FirebaseJsonArray &add(T value);

private:
  fb_json_last_error_t *last_error = nullptr;
  FirebaseJsonHelper *hp = new FirebaseJsonHelper(last_error);
  FirebaseJson js;
  size_t arr_size = 0;

  void int_addString(const std::string &value);
  void int_addInt(int value);
  void int_addFloat(float value);
  void int_addDouble(double value);
  void int_addBool(bool value);
  void int_addNull();
  void int_addJson(FirebaseJson *json);
  void int_addArray(FirebaseJsonArray *arr);
  void int_setString(int index, const std::string &value);
  void int_setString(const String &path, const std::string &value);
  void int_setInt(int index, int value);
  void int_setInt(const String &path, int value);
  void int_setFloat(int index, float value);
  void int_setFloat(const String &path, float value);
  void int_setDouble(int index, double value);
  void int_setDouble(const String &path, double value);
  void int_setBool(int index, bool value);
  void int_setBool(const String &path, bool value);
  void int_setNull(int index);
  void int_setNull(const String &path);
  void int_setJson(int index, FirebaseJson *json);
  void int_setJson(const String &path, FirebaseJson *json);
  void int_setArray(int index, FirebaseJsonArray *arr);
  void int_setArray(const String &path, FirebaseJsonArray *arr);
  void int_setByIndex(int index, const char *value, bool isStr = true);
  void int_set(const char *path, const char *value, bool isStr = true);
  bool int_get(FirebaseJsonData &jsonData, const char *path);
  bool int_remove(const char *path);
  const char *int_raw(FirebaseJson::fb_json_serialize_mode mode);
};

#endif