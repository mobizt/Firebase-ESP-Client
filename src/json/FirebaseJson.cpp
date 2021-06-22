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

#ifndef FirebaseJson_CPP
#define FirebaseJson_CPP
#define FB_JSON_STRICT

#include "FirebaseJson.h"

//Teensy 3.0, 3.2,3.5,3.6, 4.0 and 4.1
#if defined(__arm__) && defined(TEENSYDUINO) && (defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__) || defined(__IMXRT1062__))
extern "C"
{
  int __exidx_start() { return -1; }
  int __exidx_end() { return -1; }
}

#endif

FirebaseJson::FirebaseJson()
{
  hp->appendS(raw_buf, fb_json_str_type_empty_obj);
}

FirebaseJson::FirebaseJson(std::string &data)
{
  int_setJsonData(data);
}

FirebaseJson::~FirebaseJson()
{
  clear();
  top_level_token_type = fb_json_generic_type_object;
  parser.reset();
  parser = nullptr;
  int_clearPathList();
  int_clearTokenDescriptorList();
  int_clearTokenList();
  delete hp;
}

FirebaseJson &FirebaseJson::int_setJsonData(std::string &data)
{
  return setJsonData(data.c_str());
}

FirebaseJson &FirebaseJson::setJsonData(const String &data)
{
  top_level_token_type = fb_json_generic_type_object;
  hp->clearS(raw_buf);

  if (data.length() > 0)
  {
    int p1 = data.indexOf('{');
    int p2 = data.lastIndexOf('}');
    if (p1 != -1 && p2 != -1)
      raw_buf = data.substring(p1, p2 + 1).c_str();
    else
    {
      p1 = data.indexOf('[');
      p2 = data.indexOf(']');
      if (p1 != -1)
        p1 += 1;
      if (p1 != -1 && p2 != -1)
      {
        hp->appendS(raw_buf, fb_json_str_type_empty_root);
        hp->storeS(raw_buf, data.c_str(), false);
        hp->appendS(raw_buf, fb_json_str_type_brk4);
        top_level_token_type = fb_json_generic_type_array;
      }
      else
      {
        raw_buf = data.c_str();
        top_level_token_type = fb_json_generic_type_primitive;
      }
    }
  }
  else
    hp->appendS(raw_buf, fb_json_str_type_empty_obj);

  int_parse("/", fb_json_serialize_mode_plain);

  if (result.success)
    hp->storeS(raw_buf, result.data_raw.c_str(), false);

  result.clear();
  hp->clearS(temp_buf);

  int_clearPathList();
  int_clearTokenList();
  int_clearTokenDescriptorList();

  tokens.reset();
  tokens = nullptr;
  return *this;
}

FirebaseJson &FirebaseJson::clear()
{
  hp->clearS(raw_buf);
  hp->appendS(raw_buf, fb_json_str_type_empty_obj);
  result.clear();
  hp->clearS(temp_buf);
  int_clearPathList();
  int_clearTokenList();
  int_clearTokenDescriptorList();
  tokens.reset();
  tokens = nullptr;
  top_level_token_type = fb_json_generic_type_object;
  return *this;
}

FirebaseJson &FirebaseJson::add(const String &key)
{
  int_addNull(key.c_str());
  return *this;
}

FirebaseJson &FirebaseJson::add(const String &key, const String &value)
{
  int_addString(key.c_str(), value.c_str());
  return *this;
}

FirebaseJson &FirebaseJson::add(const String &key, const char *value)
{
  int_addString(key.c_str(), value);
  return *this;
}

FirebaseJson &FirebaseJson::add(const String &key, int value)
{
  int_addInt(key.c_str(), value);
  return *this;
}

FirebaseJson &FirebaseJson::add(const String &key, unsigned short value)
{
  int_addInt(key.c_str(), value);
  return *this;
}

FirebaseJson &FirebaseJson::add(const String &key, float value)
{
  int_addFloat(key.c_str(), value);
  return *this;
}

FirebaseJson &FirebaseJson::add(const String &key, double value)
{
  int_addDouble(key.c_str(), value);
  return *this;
}

FirebaseJson &FirebaseJson::add(const String &key, bool value)
{
  int_addBool(key.c_str(), value);
  return *this;
}

FirebaseJson &FirebaseJson::add(const String &key, FirebaseJson &json)
{
  int_addJson(key.c_str(), &json);
  return *this;
}

FirebaseJson &FirebaseJson::add(const String &key, FirebaseJsonArray &arr)
{
  arr.last_error = &last_error;
  int_addArray(key.c_str(), &arr);
  return *this;
}

template <typename T>
FirebaseJson &FirebaseJson::add(const String &key, T value)
{
  if (std::is_same<T, int>::value)
    int_addInt(key, value);
  else if (std::is_same<T, float>::value)
    int_addFloat(key, value);
  else if (std::is_same<T, double>::value)
    int_addDouble(key, value);
  else if (std::is_same<T, bool>::value)
    int_addBool(key, value);
  else if (std::is_same<T, const char *>::value)
    int_addString(key, value);
  else if (std::is_same<T, FirebaseJson &>::value)
    int_addJson(key, &value);
  else if (std::is_same<T, FirebaseJsonArray &>::value)
    int_addArray(key, &value);
  return *this;
}

void FirebaseJson::int_addString(const std::string &key, const std::string &value)
{
  hp->clearLastError();
  int_add(key.c_str(), value.c_str(), key.length(), value.length(), true, true);
}

void FirebaseJson::int_addInt(const std::string &key, int value)
{
  hp->clearLastError();
  char *buf = hp->intStr(value);
  int_add(key.c_str(), buf, key.length(), 60, false, true);
  hp->delS(buf);
}

void FirebaseJson::int_addFloat(const std::string &key, float value)
{
  hp->clearLastError();
  char *buf = hp->floatStr(value);
  hp->trimDouble(buf);
  int_add(key.c_str(), buf, key.length(), 60, false, true);
  hp->delS(buf);
}

void FirebaseJson::int_addDouble(const std::string &key, double value)
{
  hp->clearLastError();
  char *buf = hp->doubleStr(value);
  hp->trimDouble(buf);
  int_add(key.c_str(), buf, key.length(), 60, false, true);
  hp->delS(buf);
}

void FirebaseJson::int_addBool(const std::string &key, bool value)
{
  hp->clearLastError();
  char *tr = hp->getStr(fb_json_str_type_tr);
  char *fls = hp->getStr(fb_json_str_type_fls);
  if (value)
    int_add(key.c_str(), tr, key.length(), 6, false, true);
  else
    int_add(key.c_str(), fls, key.length(), 7, false, true);
  hp->delS(tr);
  hp->delS(fls);
}

void FirebaseJson::int_addNull(const std::string &key)
{
  hp->clearLastError();
  char *nll = hp->getStr(fb_json_str_type_nll);
  int_add(key.c_str(), nll, key.length(), 6, false, true);
  hp->delS(nll);
}

void FirebaseJson::int_addJson(const std::string &key, FirebaseJson *json)
{
  hp->clearLastError();
  int_add(key.c_str(), json->raw(), key.length(), strlen(json->raw()), false, true);
}

void FirebaseJson::int_addArray(const std::string &key, FirebaseJsonArray *arr)
{
  hp->clearLastError();
  arr->last_error = &last_error;
  int_add(key.c_str(), arr->raw(), key.length(), strlen(arr->raw()), false, true);
}

void FirebaseJson::toString(String &buf, bool prettify)
{
  if (prettify)
    int_parse("", fb_json_serialize_mode_pretty);
  else
    int_parse("", fb_json_serialize_mode_plain);
  buf = result.data_raw.c_str();
  hp->clearS(result.data_raw);
}

const char *FirebaseJson::raw()
{
  return raw_buf.c_str();
}

FirebaseJson &FirebaseJson::int_add(const char *key, const char *value, size_t klen, size_t vlen, bool isString, bool isJson)
{
  hp->clearLastError();
  size_t bufSize = klen + vlen + 32;
  char *buf = hp->newS(bufSize);
  if (!buf)
  {
    hp->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
    return *this;
  }

  if (raw_buf.length() > 2)
    strcpy_P(buf, fb_json_str_1);

  char *qt = hp->getStr(fb_json_str_type_qt);
  char *pr2 = hp->getStr(fb_json_str_type_pr2);

  if (isJson)
  {
    strcat(buf, qt);
    strcat(buf, key);
    strcat(buf, qt);
    strcat_P(buf, pr2);
  }

  if (isString)
    strcat(buf, qt);
  strcat(buf, value);
  if (isString)
    strcat(buf, qt);

  if (raw_buf.length() <= 2)
  {
    hp->clearS(raw_buf);
    isJson ? hp->appendS(raw_buf, fb_json_str_type_empty_obj) : hp->appendS(raw_buf, fb_json_str_type_empty_arr);
  }

  char c = raw_buf[raw_buf.length() - 1];
  raw_buf.erase(raw_buf.length() - 1, 1);
  hp->storeS(raw_buf, buf, true);
  hp->storeS(raw_buf, c, true);

  hp->delS(buf);
  hp->delS(qt);
  hp->delS(pr2);
  return *this;
}

FirebaseJson &FirebaseJson::int_addArrayStr(const char *value, size_t len, bool isString)
{
  hp->clearLastError();
  int_add("", value, 0, len, isString, false);
  return *this;
}

bool FirebaseJson::get(FirebaseJsonData &jsonData, const String &path, bool prettify)
{
  hp->clearLastError();
  int_clearPathList();
  int_splitTokens(path.c_str(), path_list, '/');
  hp->clearS(result.data_raw);
  hp->clearS(temp_buf);
  if (prettify)
    int_parse(path.c_str(), fb_json_serialize_mode_pretty);
  else
    int_parse(path.c_str(), fb_json_serialize_mode_plain);
  if (result.success)
  {
    if (result.type_num == fb_json_generic_type_string && result.data_raw.c_str()[0] == '"' && result.data_raw.c_str()[result.data_raw.length() - 1] == '"')
      result.stringValue = result.data_raw.substr(1, result.data_raw.length() - 2).c_str();
    else
      result.stringValue = result.data_raw.c_str();
  }
  jsonData = result;
  hp->clearS(result.data_raw);
  hp->clearS(temp_buf);
  int_clearPathList();
  tokens.reset();
  tokens = nullptr;
  return result.success;
}

size_t FirebaseJson::iteratorBegin(const char *data)
{
  hp->clearLastError();
  if (data)
    setJsonData(data);
  int_fb_json_parseToken(true);

  int16_t level = -1;
  parser_info.create_item_list = true;
  int_clearTokenList();
  for (uint16_t i = 0; i < parser_info.tokens_count; i++)
    int_parseToken(i, (char *)raw_buf.c_str(), level, "", -2, fb_json_serialize_mode_none);
  int_clearTokenDescriptorList();
  return token_item_info_list.size();
}

void FirebaseJson::iteratorEnd()
{
  hp->clearLastError();
  int_clearTokenList();
  int_clearPathList();
  result.clear();
  hp->clearS(temp_buf);
  int_clearPathList();
  tokens.reset();
  tokens = nullptr;
}

void FirebaseJson::iteratorGet(size_t index, int &type, String &key, String &value)
{
  hp->clearLastError();
  if (token_item_info_list.size() < index + 1)
    return;

  if (token_item_info_list[index].type == 0)
  {
    fb_json_token_t *token = &tokens.get()[token_item_info_list[index].index];
    size_t len = token->end - token->start + 3;
    char *k = hp->newS(len);
    if (!k)
    {
      hp->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
      return;
    }
    strncpy(k, raw_buf.c_str() + token->start, token->end - token->start);
    fb_json_token_t *g = &tokens.get()[token_item_info_list[index].index + 1];
    size_t len2 = g->end - g->start + 3;
    char *v = hp->newS(len2);
    if (!v)
    {
      hp->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
      return;
    }
    strncpy(v, raw_buf.c_str() + g->start, g->end - g->start);
    key = k;
    value = v;
    type = JSON_OBJECT;
    hp->delS(k);
    hp->delS(v);
  }
  else if (token_item_info_list[index].type == 1)
  {
    fb_json_token_t *g = &tokens.get()[token_item_info_list[index].index];
    size_t len2 = g->end - g->start + 3;
    char *v = hp->newS(len2);
    if (!v)
    {
      hp->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
      return;
    }
    strncpy(v, raw_buf.c_str() + g->start, g->end - g->start);
    value = v;
    key = "";
    type = JSON_ARRAY;
    hp->delS(v);
  }
}

void FirebaseJson::int_fb_json_parseToken(bool create_item_list)
{
  hp->clearLastError();

  tokens.reset();
  parser_info.create_item_list = create_item_list;
  int_clearTokenList();

  int nvPair = 0;
  int ao = 0;

  for (size_t i = 0; i < raw_buf.length(); i++)
  {
    if (raw_buf[i] == ',')
      nvPair++;
    else if (raw_buf[i] == '[' || raw_buf[i] == '{')
      ao++;
  }

  int num_token = (2 * (nvPair + 1)) + 2 * ao;

  tokens = std::shared_ptr<fb_json_token_t>(new fb_json_token_t[num_token]);
  int_fb_json_init(parser.get());
  parser_info.tokens_count = int_fb_json_parse(parser.get(), raw_buf.c_str(), raw_buf.length(), tokens.get(), num_token);

  if (parser_info.tokens_count < 0)
  {
    // Invalid character inside JSON string
    if (parser_info.tokens_count == fb_json_err_invalid)
      hp->setLastError(-3, __FILE__, __LINE__, fb_json_str_29);

    // The string is not a full JSON packet, more bytes expected
    else if (parser_info.tokens_count == fb_json_err_part)
      hp->setLastError(-4, __FILE__, __LINE__, fb_json_str_30);

    // Not enough tokens were provided
    else if (parser_info.tokens_count == fb_json_err_nomem)
      hp->setLastError(-2, __FILE__, __LINE__, fb_json_str_31);

    parser_info.tokens_count = 0;
    parser_info.parsing_success = false;
    tokens.reset();

    int_clearTokenList();

    return;
  }

  if (num_token > parser_info.tokens_count + 2)
  {
    tokens.reset();
    int_clearTokenList();

    tokens = std::shared_ptr<fb_json_token_t>(new fb_json_token_t[parser_info.tokens_count]);
    int_fb_json_init(parser.get());
    int_fb_json_parse(parser.get(), raw_buf.c_str(), raw_buf.length(), tokens.get(), parser_info.tokens_count);
  }

  parser_info.parsing_success = true;
  if (tokens.get()[0].type != fb_json_generic_type_object)
    parser_info.parsing_success = false;

  result.success = parser_info.parsing_success;
  parser_info.next_token = 0;
  parser_info.next_level = 0;
  parser_info.is_token_matches = false;
  parser_info.ref_token = -1;
  int_resetParseResult();
  int_setElementType();
}

void FirebaseJson::int_updateDescriptor(fb_json_token_descriptor_update_type type, int level, bool value)
{
  for (size_t i = 0; i < fb_json_token_descriptor_list.size(); i++)
  {
    if (fb_json_token_descriptor_list[i].level == level - 1)
    {
      if (type == fb_json_token_descriptor_update_mark)
      {
        fb_json_token_descriptor_list[i].node_mark = value;
        break;
      }
      else if (type == fb_json_token_descriptor_update_skip)
      {
        fb_json_token_descriptor_list[i].skip = value;
        break;
      }
      else if (type == fb_json_token_descriptor_update_ref && value)
      {
        fb_json_token_descriptor_list[i].ref = value;
        break;
      }
    }

    if (type == fb_json_token_descriptor_update_ref && !value)
      fb_json_token_descriptor_list[i].ref = false;
  }
}

void FirebaseJson::int_getTokenIndex(int level, fb_json_token_descriptor_t &tk)
{
  for (size_t i = 0; i < fb_json_token_descriptor_list.size(); i++)
  {
    if (fb_json_token_descriptor_list[i].level == level - 1)
    {
      tk = fb_json_token_descriptor_list[i];
      break;
    }
  }
}

bool FirebaseJson::int_updateTokenIndex(fb_json_token_update_mode mode, uint16_t index, int16_t &level, const char *searchKey, int16_t searchIndex, const char *replace, fb_json_serialize_mode serializeMode, bool advancedCount, std::string &buf)
{
  int len = -1;
  bool skip = false;
  bool ref = false;
  for (size_t i = 0; i < fb_json_token_descriptor_list.size(); i++)
  {
    if (fb_json_token_descriptor_list[i].level == level - 1)
    {
      if (fb_json_token_descriptor_list[i].type == fb_json_generic_type_object || fb_json_token_descriptor_list[i].type == fb_json_generic_type_array)
      {
        fb_json_token_descriptor_list[i].node_index++;
        if (fb_json_token_descriptor_list[i].node_index >= fb_json_token_descriptor_list[i].node_length)
        {
          level = fb_json_token_descriptor_list[i].level;
          len = fb_json_token_descriptor_list[i].node_length;
          skip = fb_json_token_descriptor_list[i].skip;

          if (!parser_info.to_set_ref_token && (fb_json_token_descriptor_list[i].type == fb_json_generic_type_object || (fb_json_token_descriptor_list[i].type == fb_json_generic_type_array && searchIndex > -1)))
            ref = fb_json_token_descriptor_list[i].ref;

          fb_json_token_descriptor_list.erase(fb_json_token_descriptor_list.begin() + i);

          if (mode == fb_json_token_update_mode_parsing)
          {
            if (level < parser_info.skip_level)
              return false;
            if (serializeMode != fb_json_serialize_mode_none && skip)
            {
              if (len > 0 && serializeMode == fb_json_serialize_mode_pretty)
                fb_json_token_descriptor_list[i].type == fb_json_generic_type_object ? int_appendTab(result.data_raw, level - parser_info.skip_level, !ref, true) : int_appendTab(result.data_raw, level - parser_info.skip_level, true, true);

              if (ref)
                int_updateDescriptor(fb_json_token_descriptor_update_ref, level, false);
              fb_json_token_descriptor_list[i].type == fb_json_generic_type_object ? hp->appendS(result.data_raw, fb_json_str_type_brk2) : hp->appendS(result.data_raw, fb_json_str_type_brk4);
            }
          }

          if (serializeMode != fb_json_serialize_mode_none && !skip)
          {

            if (mode == fb_json_token_update_mode_compile)
            {

              if (len > 0 && !parser_info.to_replace_array)
              {
                if (ref)
                  hp->appendS(result.data_raw, fb_json_str_type_cm);
                if (fb_json_token_descriptor_list[i].type == fb_json_generic_type_object && serializeMode == fb_json_serialize_mode_pretty)
                  int_appendTab(result.data_raw, level + 1, !ref, true);
              }

              if (ref)
              {
                if (!advancedCount)
                  parser_info.parsing_completed_count++;

                if (!parser_info.to_replace_array)
                {
                  if (fb_json_token_descriptor_list[i].type == fb_json_generic_type_object)
                  {
                    if (serializeMode == fb_json_serialize_mode_pretty)
                      int_appendTab(result.data_raw, level + 2, true);

                    char *qt = hp->getStr(fb_json_str_type_qt);
                    result.data_raw += qt;
                    result.data_raw += searchKey;
                    result.data_raw += qt;
                    hp->delS(qt);

                    serializeMode == fb_json_serialize_mode_pretty ? hp->appendS(result.data_raw, fb_json_str_type_pr) : hp->appendS(result.data_raw, fb_json_str_type_pr2);
                    if (parser_info.parsing_completed_count == (int)path_list.size())
                      result.data_raw += replace;
                    else
                      int_insertChilds(replace, serializeMode);
                    parser_info.to_replace_array = true;
                    if (serializeMode == fb_json_serialize_mode_pretty)
                      int_appendTab(result.data_raw, level + 1, true, true);
                  }
                  else
                  {
                    char *cm = hp->getStr(fb_json_str_type_cm);
                    char *nll = hp->getStr(fb_json_str_type_nll);
                    for (int k = fb_json_token_descriptor_list[i].node_index - 1; k < searchIndex; k++)
                    {
                      if (serializeMode == fb_json_serialize_mode_pretty)
                        int_appendTab(result.data_raw, level + 2, true, true);
                      if (k == searchIndex - 1)
                      {
                        if (parser_info.parsing_completed_count == (int)path_list.size())
                          result.data_raw += replace;
                        else
                          int_insertChilds(replace, serializeMode);
                        parser_info.to_replace_array = true;
                      }
                      else
                      {
                        result.data_raw += nll;
                        result.data_raw += cm;
                      }
                    }
                    hp->delS(cm);
                    hp->delS(nll);
                  }
                }
                int_updateDescriptor(fb_json_token_descriptor_update_ref, level, false);
                if (!advancedCount)
                  parser_info.parsing_completed_count = path_list.size();
              }

              if (fb_json_token_descriptor_list[i].type == fb_json_generic_type_object)
                hp->appendS(result.data_raw, fb_json_str_type_brk2);
              else
              {
                if (len > 0 && serializeMode == fb_json_serialize_mode_pretty)
                  int_appendTab(result.data_raw, level + 1, true, true);
                hp->appendS(result.data_raw, fb_json_str_type_brk4);
              }
            }
            else if (mode == fb_json_token_update_mode_remove)
            {
              if (len > 0 && serializeMode == fb_json_serialize_mode_pretty)
                fb_json_token_descriptor_list[i].type == fb_json_generic_type_object ? int_appendTab(buf, level + 1, !ref, true) : int_appendTab(buf, level + 1, true, true);

              if (ref)
                int_updateDescriptor(fb_json_token_descriptor_update_ref, level, false);
              fb_json_token_descriptor_list[i].type == fb_json_generic_type_object ? hp->appendS(buf, fb_json_str_type_brk2) : hp->appendS(buf, fb_json_str_type_brk4);
            }
          }

          if (mode == fb_json_token_update_mode_compile)
            hp->shrinkS(result.data_raw);

          return true;
        }
      }
      break;
    }
  }
  return false;
}

void FirebaseJson::int_appendTab(std::string &s, int level, bool notEmpty, bool newLine)
{
  if (newLine)
    hp->appendS(s, fb_json_str_type_nl);
  if (notEmpty)
  {
    char *tab = hp->getStr(fb_json_str_type_tab);
    for (int i = 0; i < level; i++)
      s += tab;
    hp->delS(tab);
  }

  hp->shrinkS(s);
}

void FirebaseJson::int_insertChilds(const char *data, fb_json_serialize_mode serializeMode)
{
  std::string str = "";
  for (int i = path_list.size() - 1; i > parser_info.parsing_completed_count - 1; i--)
  {
    if (int_isArray(i))
    {
      std::string _str;
      int_addArrNodes(_str, str, i, data, serializeMode);
      str = _str;
      hp->clearS(_str);
    }
    else
    {
      std::string _str;
      int_addObjNodes(_str, str, i, data, serializeMode);
      str = _str;
      hp->clearS(_str);
    }
  }
  if ((int)path_list.size() == parser_info.parsing_completed_count)
    str = data;
  result.data_raw += str;
  hp->clearS(str);
  hp->shrinkS(result.data_raw);
}

void FirebaseJson::int_addArrNodes(std::string &str, std::string &str2, int16_t index, const char *data, fb_json_serialize_mode serializeMode)
{

  int i = int_getArrIndex(index);
  hp->appendS(str, fb_json_str_type_brk3);
  char *cm = hp->getStr(fb_json_str_type_cm);
  char *nll = hp->getStr(fb_json_str_type_nll);

  if (serializeMode == fb_json_serialize_mode_pretty)
    hp->appendS(str, fb_json_str_type_nl);

  for (int k = 0; k <= i; k++)
  {
    if (serializeMode == fb_json_serialize_mode_pretty)
      int_appendTab(str, index + 1, true);

    if (k == i)
    {
      if (index == (int)path_list.size() - 1)
        str += data;
      else
        str += str2;
    }
    else
    {
      str += nll;
      str += cm;
    }

    if (serializeMode == fb_json_serialize_mode_pretty)
      hp->appendS(str, fb_json_str_type_nl);
  }
  hp->delS(cm);
  hp->delS(nll);

  if (serializeMode == fb_json_serialize_mode_pretty)
    int_appendTab(str, index, true);
  hp->appendS(str, fb_json_str_type_brk4);

  hp->shrinkS(str);
}

void FirebaseJson::int_addObjNodes(std::string &str, std::string &str2, int16_t index, const char *data, fb_json_serialize_mode serializeMode)
{
  hp->appendS(str, fb_json_str_type_brk1);

  if (serializeMode == fb_json_serialize_mode_pretty)
    int_appendTab(str, index + 1, true, true);

  char *qt = hp->getStr(fb_json_str_type_qt);
  str += qt;
  str += path_list[index].path.c_str();
  str += qt;
  hp->delS(qt);

  serializeMode == fb_json_serialize_mode_pretty ? hp->appendS(str, fb_json_str_type_pr) : hp->appendS(str, fb_json_str_type_pr2);
  index == (int)path_list.size() - 1 ? str += data : str += str2;
  if (serializeMode == fb_json_serialize_mode_pretty)
    int_appendTab(str, index, true, true);
  hp->appendS(str, fb_json_str_type_brk2);

  hp->shrinkS(str);
}

void FirebaseJson::int_parseToken(uint16_t &i, const char *buf, int16_t &level, const char *searchKey, int16_t searchIndex, fb_json_serialize_mode serializeMode)
{
  hp->clearLastError();
  fb_json_token_descriptor_t tok_descr;
  int_getTokenIndex(level, tok_descr);
  fb_json_token_t *token = &tokens.get()[i];
  bool oskip = false;
  bool ex = false;
  size_t resLen = result.data_raw.length();
  if (searchIndex == -2)
    tok_descr.skip = true;
  delay(0);
  if (searchIndex > -1)
  {
    fb_json_token_descriptor_t tk2;
    int16_t level2 = level - 1;
    int_getTokenIndex(level2, tk2);
    if (tok_descr.type == fb_json_generic_type_array && parser_info.current_level == level && tk2.node_index == parser_info.parent_index)
    {
      if (tok_descr.node_index == searchIndex)
      {
        parser_info.next_token = i;
        parser_info.next_level = level;
        parser_info.parent_index = tok_descr.node_index;

        if ((int)path_list.size() != parser_info.current_level + 1)
        {
          parser_info.is_token_matches = true;
          parser_info.parsing_completed_count++;
        }
        else
        {
          if (!parser_info.to_set_ref_token)
          {
            parser_info.parsing_completed_count++;
            parser_info.ref_token_index = i + 1;
            parser_info.ref_token = i + 1;
            parser_info.to_set_ref_token = true;
            char *dat1 = hp->newS(token->end - token->start + 10);
            if (!dat1)
            {
              hp->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
              return;
            }
            strncpy(dat1, buf + token->start, token->end - token->start);
            result.stringValue = dat1;
            hp->delS(dat1);
            result.setTokenInfo(token, i, level, true);
            int_setElementType();
            if (serializeMode != fb_json_serialize_mode_none)
              result.stringValue.remove(0, result.stringValue.length());
            else
            {
              hp->clearS(result.data_raw);
              hp->clearS(temp_buf);
              parser_info.is_token_matches = true;
              ex = true;
            }
          }
        }
      }
      else
      {
        if (tok_descr.node_index + 1 == tok_descr.node_length)
        {
          int_updateDescriptor(fb_json_token_descriptor_update_ref, level - 1, false);
          int_updateDescriptor(fb_json_token_descriptor_update_ref, level, true);
        }
      }
    }
  }
  else
  {
    char *key = hp->newS(token->end - token->start + 10);
    if (!key)
    {
      hp->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
      return;
    }
    strncpy(key, buf + token->start, token->end - token->start);
    if (tok_descr.type != fb_json_generic_type_undefined && parser_info.current_level == level)
    {
      if (strcmp(searchKey, key) == 0)
      {
        parser_info.next_token = i + 1;
        parser_info.next_level = level;
        parser_info.parent_index = tok_descr.node_index;
        if ((int)path_list.size() != parser_info.current_level + 1)
        {
          parser_info.is_token_matches = true;
          parser_info.parsing_completed_count++;
        }
        else
        {
          if (!parser_info.to_set_ref_token)
          {
            parser_info.parsing_completed_count++;
            parser_info.ref_token_index = i + 1;
            parser_info.ref_token = i + 1;
            parser_info.to_set_ref_token = true;
            token = &tokens.get()[i + 1];
            char *dat2 = hp->newS(token->end - token->start + 10);
            if (!dat2)
            {
              hp->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
              return;
            }
            strncpy(dat2, buf + token->start, token->end - token->start);
            result.stringValue = dat2;
            hp->delS(dat2);
            result.setTokenInfo(token, i, level, true);
            int_setElementType();
            if (serializeMode != fb_json_serialize_mode_none)
              result.stringValue.remove(0, result.stringValue.length());
            else
            {
              hp->clearS(result.data_raw);
              hp->clearS(temp_buf);
              parser_info.is_token_matches = true;
              ex = true;
            }
          }
        }
      }
      else
      {
        if (tok_descr.node_index + 1 == tok_descr.node_length)
        {
          int_updateDescriptor(fb_json_token_descriptor_update_ref, level - 1, false);
          int_updateDescriptor(fb_json_token_descriptor_update_ref, level, true);
        }
      }
    }
    hp->delS(key);
  }
  if (ex)
    return;
  if (parser_info.ref_token_index == i + 1)
  {
    if (tok_descr.type == fb_json_generic_type_object)
      oskip = true;
    tok_descr.skip = true;
    parser_info.skip_level = level;
  }
  token = &tokens.get()[i];
  if (token->type == fb_json_generic_type_object || token->type == fb_json_generic_type_array)
  {
    if (serializeMode != fb_json_serialize_mode_none && (tok_descr.skip || parser_info.ref_token_index == i + 1))
    {
      if (!tok_descr.node_mark && i > 0 && resLen > 0)
      {

        if (tok_descr.node_index > 0)
          hp->appendS(result.data_raw, fb_json_str_type_cm);

        if (serializeMode == fb_json_serialize_mode_pretty && token->size >= 0)
          int_appendTab(result.data_raw, level - parser_info.skip_level, true, true);
      }
      token->type == fb_json_generic_type_object ? hp->appendS(result.data_raw, fb_json_str_type_brk1) : hp->appendS(result.data_raw, fb_json_str_type_brk3);
    }

    int_addDescriptorList(token, i, level, false, false, &tok_descr);
    level++;

    if (token->size == 0)
    {
      while (int_updateTokenIndex(fb_json_token_update_mode_parsing, i, level, searchKey, searchIndex, "", serializeMode, false, temp_buf))
      {
        delay(0);
      }
    }
  }
  else
  {
    char *tmp = hp->newS(token->end - token->start + 10);
    if (!tmp)
    {
      hp->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
      return;
    }
    if (buf[token->start - 1] != '"')
      strncpy(tmp, buf + token->start, token->end - token->start);
    else
      strncpy(tmp, buf + token->start - 1, token->end - token->start + 2);
    if (token->size > 0)
    {
      if (serializeMode != fb_json_serialize_mode_none && tok_descr.skip && !oskip)
      {

        if (tok_descr.node_index > 0)
          hp->appendS(result.data_raw, fb_json_str_type_cm);

        if (serializeMode == fb_json_serialize_mode_pretty)
          int_appendTab(result.data_raw, level - parser_info.skip_level, token->size > 0, true);

        result.data_raw += tmp;
        serializeMode == fb_json_serialize_mode_pretty ? hp->appendS(result.data_raw, fb_json_str_type_pr) : hp->appendS(result.data_raw, fb_json_str_type_pr2);
      }
      if (parser_info.create_item_list)
      {
        token_item_info_t tokenInfo;
        tokenInfo.index = i;
        tokenInfo.type = 0;
        token_item_info_list.push_back(tokenInfo);
      }
      char *tmp2 = hp->newS(token->end - token->start + 10);
      if (!tmp2)
      {
        hp->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
        return;
      }
      strncpy(tmp2, buf + token->start, token->end - token->start);
      token = &tokens.get()[i + 1];
      hp->delS(tmp2);

      if (token->type != fb_json_generic_type_object && token->type != fb_json_generic_type_array)
      {

        char *tmp2 = hp->newS(token->end - token->start + 10);
        if (!tmp2)
        {
          hp->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
          return;
        }
        strncpy(tmp2, buf + token->start, token->end - token->start);
        if (serializeMode != fb_json_serialize_mode_none && tok_descr.skip)
        {
          if (buf[token->start - 1] != '"')
            strncpy(tmp2, buf + token->start, token->end - token->start);
          else
            strncpy(tmp2, buf + token->start - 1, token->end - token->start + 2);
          result.data_raw += tmp2;
        }
        hp->delS(tmp2);
        i++;

        while (int_updateTokenIndex(fb_json_token_update_mode_parsing, i, level, searchKey, searchIndex, "", serializeMode, false, temp_buf))
        {
          delay(0);
        }
      }
      else
      {
        if (parser_info.ref_token == i + 1)
          int_updateDescriptor(fb_json_token_descriptor_update_skip, level, true);

        int_updateDescriptor(fb_json_token_descriptor_update_mark, level, true);
      }
    }
    else
    {
      if (serializeMode != fb_json_serialize_mode_none && tok_descr.skip)
      {

        if (tok_descr.node_index > 0 && resLen > 0)
          hp->appendS(result.data_raw, fb_json_str_type_cm);

        if (serializeMode == fb_json_serialize_mode_pretty)
          int_appendTab(result.data_raw, level - parser_info.skip_level, tok_descr.node_length > 0 && resLen > 0, resLen > 0);

        result.data_raw += tmp;
      }

      while (int_updateTokenIndex(fb_json_token_update_mode_parsing, i, level, searchKey, searchIndex, "", serializeMode, false, temp_buf))
      {
        delay(0);
      }

      if (parser_info.create_item_list)
      {
        token_item_info_t tokenInfo;
        tokenInfo.index = i;
        tokenInfo.type = 1;
        token_item_info_list.push_back(tokenInfo);
      }
    }
    hp->delS(tmp);

    if (parser_info.ref_token == -1 && parser_info.skip_level == level)
      int_updateDescriptor(fb_json_token_descriptor_update_skip, level, false);
  }
  parser_info.next_token = i + 1;
  parser_info.ref_token = -1;

  hp->shrinkS(result.data_raw);
}

void FirebaseJson::int_addDescriptorList(fb_json_token_t *token, int16_t index, int16_t level, bool mark, bool ref, fb_json_token_descriptor_t *descr)
{
  fb_json_token_descriptor_t tok;
  tok.index = index;
  tok.node_length = token->size;
  tok.type = token->type;
  tok.node_index = 0;
  tok.level = level;
  tok.node_mark = mark;
  tok.ref = ref;
  parser_info.ref_token != -1 ? tok.skip = true : tok.skip = descr->skip;
  fb_json_token_descriptor_list.push_back(tok);
}

void FirebaseJson::int_compileToken(uint16_t &i, const char *buf, int16_t &level, const char *searchKey, int16_t searchIndex, fb_json_serialize_mode serializeMode, const char *replace, int16_t refTokenIndex, bool isRemove)
{
  hp->clearLastError();
  if (parser_info.is_token_matches)
    return;
  fb_json_token_descriptor_t tok_descr;
  int_getTokenIndex(level, tok_descr);
  fb_json_token_t *token = &tokens.get()[i];
  bool insertFlag = false;
  bool ex = false;
  delay(0);
  if (searchIndex > -1)
  {
    fb_json_token_descriptor_t tk2;
    int level2 = level - 1;
    int_getTokenIndex(level2, tk2);
    if (tok_descr.type == fb_json_generic_type_array && parser_info.current_level == level && tk2.node_index == parser_info.parent_index)
    {
      if (tok_descr.node_index == searchIndex)
      {
        parser_info.next_token = i;
        parser_info.next_level = level;
        parser_info.parent_index = tok_descr.node_index;
        if ((int)path_list.size() != parser_info.current_level + 1)
        {
          parser_info.is_token_matches = true;
          parser_info.parsing_completed_count++;
          parser_info.ref_token_index = i + 1;
        }
        else
        {
          if (!parser_info.to_set_ref_token)
          {
            parser_info.parsing_completed_count++;
            parser_info.ref_token_index = i + 1;
            parser_info.ref_token = i + 1;
            parser_info.to_set_ref_token = true;
            single_child_parent_t p = int_findSingleChildParent(level);
            if (p.success)
            {
              parser_info.removed_token_index = p.index + 1;
              parser_info.to_remove_first_token = p.first_token;
              parser_info.to_remove_last_token = p.last_token;
            }
            else
            {
              parser_info.removed_token_index = i + 1;
              parser_info.to_remove_first_token = tok_descr.node_index == 0;
              parser_info.to_remove_last_token = tok_descr.node_index + 1 == tok_descr.node_length;
            }
          }
        }
      }
      else
      {
        if (tok_descr.node_index + 1 == tok_descr.node_length)
        {
          int_updateDescriptor(fb_json_token_descriptor_update_ref, level - 1, false);
          int_updateDescriptor(fb_json_token_descriptor_update_ref, level, true);
        }
      }
    }
  }
  else
  {
    char *key = hp->newS(token->end - token->start + 10);
    if (!key)
    {
      hp->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
      return;
    }
    strncpy(key, buf + token->start, token->end - token->start);
    if (tok_descr.type != fb_json_generic_type_undefined && parser_info.current_level == level)
    {
      if (strcmp(searchKey, key) == 0)
      {
        parser_info.next_token = i + 1;
        parser_info.next_level = level;
        parser_info.parent_index = tok_descr.node_index;
        if ((int)path_list.size() != parser_info.current_level + 1)
        {
          parser_info.is_token_matches = true;
          parser_info.parsing_completed_count++;
          parser_info.ref_token_index = i + 1;
        }
        else
        {
          if (!parser_info.to_set_ref_token)
          {
            parser_info.parsing_completed_count++;
            parser_info.ref_token_index = i + 1;
            parser_info.ref_token = i + 1;
            parser_info.to_set_ref_token = true;
            single_child_parent_t p = int_findSingleChildParent(level);
            if (p.success)
            {
              parser_info.removed_token_index = p.index + 1;
              parser_info.to_remove_first_token = p.first_token;
              parser_info.to_remove_last_token = p.last_token;
            }
            else
            {
              parser_info.removed_token_index = i + 1;
              parser_info.to_remove_first_token = tok_descr.node_index == 0;
              parser_info.to_remove_last_token = tok_descr.node_index + 1 == tok_descr.node_length;
            }
          }
        }
      }
      else
      {
        if (tok_descr.node_index + 1 == tok_descr.node_length)
        {
          int_updateDescriptor(fb_json_token_descriptor_update_ref, level - 1, false);
          int_updateDescriptor(fb_json_token_descriptor_update_ref, level, true);
        }
      }
    }
    else
    {
      if (parser_info.tokens_count == 1 && token->size == 0 && !isRemove)
      {
        int_insertChilds(replace, serializeMode);
        parser_info.next_token = i + 1;
        parser_info.next_level = 0;
        parser_info.parsing_completed_count = path_list.size();
        parser_info.is_token_matches = true;
        ex = true;
      }
    }
    hp->delS(key);
  }
  if (ex)
    return;

  token = &tokens.get()[i];
  if (token->type == fb_json_generic_type_object || token->type == fb_json_generic_type_array)
  {
    if (serializeMode != fb_json_serialize_mode_none && !tok_descr.skip)
    {
      if (!tok_descr.node_mark && i > 0)
      {

        if (tok_descr.node_index > 0)
          hp->appendS(result.data_raw, fb_json_str_type_cm);

        if (serializeMode == fb_json_serialize_mode_pretty && token->size >= 0)
          int_appendTab(result.data_raw, level + 1, true, true);
      }

      if (parser_info.ref_token == -1)
        token->type == fb_json_generic_type_object ? hp->appendS(result.data_raw, fb_json_str_type_brk1) : hp->appendS(result.data_raw, fb_json_str_type_brk3);
      else if (parser_info.ref_token != -1 && searchIndex > -1)
        result.data_raw += replace;
    }

    int_addDescriptorList(token, i, level, false, false, &tok_descr);
    level++;

    if (token->size == 0)
    {
      while (int_updateTokenIndex(fb_json_token_update_mode_compile, i, level, searchKey, searchIndex, replace, serializeMode, isRemove, temp_buf))
      {
        delay(0);
      }
    }
  }
  else
  {
    if (parser_info.ref_token_index == refTokenIndex && refTokenIndex > -1)
    {
      parser_info.ref_token = refTokenIndex;
      parser_info.ref_token_index = -1;
      insertFlag = true;
    }
    char *tmp = hp->newS(token->end - token->start + 10);
    if (!tmp)
    {
      hp->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
      return;
    }
    if (buf[token->start - 1] != '"')
      strncpy(tmp, buf + token->start, token->end - token->start);
    else
      strncpy(tmp, buf + token->start - 1, token->end - token->start + 2);
    if (token->size > 0)
    {
      if (serializeMode != fb_json_serialize_mode_none && !tok_descr.skip)
      {

        if (tok_descr.node_index > 0)
          hp->appendS(result.data_raw, fb_json_str_type_cm);

        if (serializeMode == fb_json_serialize_mode_pretty)
          int_appendTab(result.data_raw, level + 1, token->size > 0, true);

        result.data_raw += tmp;
        serializeMode == fb_json_serialize_mode_pretty ? hp->appendS(result.data_raw, fb_json_str_type_pr) : hp->appendS(result.data_raw, fb_json_str_type_pr2);
      }
      char *tmp2 = hp->newS(token->end - token->start + 10);
      if (!tmp2)
      {
        hp->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
        return;
      }
      strncpy(tmp2, buf + token->start, token->end - token->start);
      token = &tokens.get()[i + 1];
      hp->delS(tmp2);

      if (token->type != fb_json_generic_type_object && token->type != fb_json_generic_type_array)
      {
        char *tmp2 = hp->newS(token->end - token->start + 10);
        if (!tmp2)
        {
          hp->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
          return;
        }
        strncpy(tmp2, buf + token->start, token->end - token->start);

        if (serializeMode != fb_json_serialize_mode_none && !tok_descr.skip)
        {
          if (buf[token->start - 1] != '"')
            strncpy(tmp2, buf + token->start, token->end - token->start);
          else
            strncpy(tmp2, buf + token->start - 1, token->end - token->start + 2);
          if (parser_info.ref_token == i + 1)
          {
            if (!insertFlag)
              result.data_raw += replace;
            else
              int_insertChilds(replace, serializeMode);
          }
          else
            result.data_raw += tmp2;
        }
        hp->delS(tmp2);
        i++;

        while (int_updateTokenIndex(fb_json_token_update_mode_compile, i, level, searchKey, searchIndex, replace, serializeMode, isRemove, temp_buf))
        {
          delay(0);
        }
      }
      else
      {
        if (parser_info.ref_token == i + 1)
        {
          int_updateDescriptor(fb_json_token_descriptor_update_skip, level, true);
          parser_info.skip_level = level;
          if (!insertFlag)
            result.data_raw += replace;
          else
            int_insertChilds(replace, serializeMode);
          if (serializeMode != fb_json_serialize_mode_none && (level > 0 || tok_descr.node_index == tok_descr.node_length - 1))
          {
            if (serializeMode == fb_json_serialize_mode_pretty)
              int_appendTab(result.data_raw, level, true, true);
            hp->appendS(result.data_raw, fb_json_str_type_brk2);
          }
        }

        int_updateDescriptor(fb_json_token_descriptor_update_mark, level, true);
      }
    }
    else
    {
      if (serializeMode != fb_json_serialize_mode_none && !tok_descr.skip)
      {

        if (tok_descr.node_index > 0)
          hp->appendS(result.data_raw, fb_json_str_type_cm);

        if (serializeMode == fb_json_serialize_mode_pretty)
          int_appendTab(result.data_raw, level + 1, tok_descr.node_length > 0, true);

        if (parser_info.ref_token == i + 1 && !parser_info.to_insert_array)
        {
          if (!insertFlag)
            result.data_raw += replace;
          else
            int_insertChilds(replace, serializeMode);
          parser_info.to_insert_array = true;
        }
        else
          result.data_raw += tmp;
      }

      while (int_updateTokenIndex(fb_json_token_update_mode_compile, i, level, searchKey, searchIndex, replace, serializeMode, isRemove, temp_buf))
      {
        delay(0);
      }
    }
    hp->delS(tmp);

    if (parser_info.ref_token == -1 && parser_info.skip_level == level)
      int_updateDescriptor(fb_json_token_descriptor_update_skip, level, false);
  }
  parser_info.next_token = i + 1;
  parser_info.ref_token = -1;
  hp->shrinkS(result.data_raw);
}

void FirebaseJson::int_removeToken(uint16_t &i, const char *buf, int16_t &level, const char *searchKey, int16_t searchIndex, fb_json_serialize_mode serializeMode, const char *replace, int16_t refTokenIndex, bool isRemove)
{
  hp->clearLastError();
  bool ncm = false;
  fb_json_token_descriptor_t tok_descr;
  int_getTokenIndex(level, tok_descr);
  fb_json_token_t *token = &tokens.get()[i];
  delay(0);
  if (refTokenIndex == i && refTokenIndex > -1)
    ncm = parser_info.to_remove_first_token;
  if (refTokenIndex != i || (refTokenIndex == i && parser_info.to_remove_last_token))
    result.data_raw += temp_buf;
  hp->clearS(temp_buf);
  bool flag = tok_descr.node_index > 0 && !ncm && result.data_raw.c_str()[result.data_raw.length() - 1] != '{' && result.data_raw.c_str()[result.data_raw.length() - 1] != '[';
  if (refTokenIndex == i + 1 && refTokenIndex > -1)
  {
    parser_info.ref_token = refTokenIndex;
    parser_info.ref_token_index = -1;
    tok_descr.skip = true;
  }
  token = &tokens.get()[i];
  if (token->type == fb_json_generic_type_object || token->type == fb_json_generic_type_array)
  {
    if (serializeMode != fb_json_serialize_mode_none && !tok_descr.skip)
    {
      if (!tok_descr.node_mark && i > 0)
      {
        if (flag)
          hp->appendS(temp_buf, fb_json_str_type_cm);

        if (serializeMode == fb_json_serialize_mode_pretty && token->size >= 0)
          int_appendTab(temp_buf, level + 1, true, true);
      }
      if (parser_info.ref_token == -1)
        token->type == fb_json_generic_type_object ? hp->appendS(temp_buf, fb_json_str_type_brk1) : hp->appendS(temp_buf, fb_json_str_type_brk3);
      else if (parser_info.ref_token != -1 && searchIndex > -1)
        temp_buf += replace;
    }

    int_addDescriptorList(token, i, level, false, false, &tok_descr);
    level++;

    if (token->size == 0)
    {
      while (int_updateTokenIndex(fb_json_token_update_mode_remove, i, level, searchKey, searchIndex, replace, serializeMode, false, temp_buf))
      {
        delay(0);
      }
    }
  }
  else
  {
    char *tmp = hp->newS(token->end - token->start + 10);
    if (!tmp)
    {
      hp->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
      return;
    }
    if (buf[token->start - 1] != '"')
      strncpy(tmp, buf + token->start, token->end - token->start);
    else
      strncpy(tmp, buf + token->start - 1, token->end - token->start + 2);
    if (token->size > 0)
    {
      if (serializeMode != fb_json_serialize_mode_none && !tok_descr.skip)
      {

        if (flag)
          hp->appendS(temp_buf, fb_json_str_type_cm);

        if (serializeMode == fb_json_serialize_mode_pretty)
          int_appendTab(temp_buf, level + 1, token->size > 0, true);

        temp_buf += tmp;
        serializeMode == fb_json_serialize_mode_pretty ? hp->appendS(temp_buf, fb_json_str_type_pr) : hp->appendS(temp_buf, fb_json_str_type_pr2);
      }

      char *tmp2 = hp->newS(token->end - token->start + 10);
      if (!tmp2)
      {
        hp->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
        return;
      }
      strncpy(tmp2, buf + token->start, token->end - token->start);
      token = &tokens.get()[i + 1];
      hp->delS(tmp2);
      if (token->type != fb_json_generic_type_object && token->type != fb_json_generic_type_array)
      {
        char *tmp2 = hp->newS(token->end - token->start + 10);
        if (!tmp2)
        {
          hp->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
          return;
        }
        strncpy(tmp2, buf + token->start, token->end - token->start);
        if (serializeMode != fb_json_serialize_mode_none && !tok_descr.skip)
        {
          if (buf[token->start - 1] != '"')
            strncpy(tmp2, buf + token->start, token->end - token->start);
          else
            strncpy(tmp2, buf + token->start - 1, token->end - token->start + 2);
          temp_buf += tmp2;
        }
        hp->delS(tmp2);
        i++;
        while (int_updateTokenIndex(fb_json_token_update_mode_remove, i, level, searchKey, searchIndex, replace, serializeMode, false, temp_buf))
        {
          delay(0);
        }
      }
      else
      {
        if (parser_info.ref_token == i + 1)
        {
          int_updateDescriptor(fb_json_token_descriptor_update_skip, level, true);
          parser_info.skip_level = level;
          temp_buf += replace;
          if (serializeMode != fb_json_serialize_mode_none && (level > 0 || tok_descr.node_index == tok_descr.node_length - 1))
          {
            if (serializeMode == fb_json_serialize_mode_pretty)
              int_appendTab(temp_buf, level, true, true);
            hp->appendS(temp_buf, fb_json_str_type_brk2);
          }
        }
        int_updateDescriptor(fb_json_token_descriptor_update_mark, level, true);
      }
    }
    else
    {
      if (serializeMode != fb_json_serialize_mode_none && !tok_descr.skip)
      {

        if (flag)
          hp->appendS(temp_buf, fb_json_str_type_cm);

        if (serializeMode == fb_json_serialize_mode_pretty)
          int_appendTab(temp_buf, level + 1, tok_descr.node_length > 0, true);

        temp_buf += tmp;
      }
      while (int_updateTokenIndex(fb_json_token_update_mode_remove, i, level, searchKey, searchIndex, replace, serializeMode, false, temp_buf))
      {
        delay(0);
      }
    }
    hp->delS(tmp);

    if (parser_info.ref_token == -1 && parser_info.skip_level == level)
      int_updateDescriptor(fb_json_token_descriptor_update_skip, level, false);
  }
  parser_info.next_token = i + 1;
  parser_info.ref_token = -1;
  last_token.node_length = tok_descr.node_length;
  last_token.node_index = tok_descr.node_index;
  last_token.type = tok_descr.type;
  last_token.level = tok_descr.level;
  last_token.index = tok_descr.index;
  last_token.skip = tok_descr.skip;
  hp->shrinkS(result.data_raw);
}

FirebaseJson::single_child_parent_t FirebaseJson::int_findSingleChildParent(int level)
{
  single_child_parent_t res;
  res.index = -1;
  res.first_token = false;
  res.last_token = false;
  res.success = false;
  for (int i = level; i >= 0; i--)
  {
    bool match = false;
    for (size_t j = 0; j < fb_json_token_descriptor_list.size(); j++)
    {
      if (fb_json_token_descriptor_list[j].level == i - 1 && fb_json_token_descriptor_list[i].node_length == 1)
      {
        match = true;
        res.index = fb_json_token_descriptor_list[i].index;
        res.first_token = fb_json_token_descriptor_list[j].node_index == 0;
        res.last_token = fb_json_token_descriptor_list[j].node_index + 1 == fb_json_token_descriptor_list[j].node_length;
        res.success = true;
      }
    }
    if (!match)
      break;
  }
  return res;
}

void FirebaseJson::int_get(const char *key, int level, int index)
{
  hp->clearLastError();
  parser_info.is_token_matches = false;
  if (parser_info.parsing_success)
  {
    if (result.success)
    {
      hp->clearS(result.data_raw);
      parser_info.current_level = level;
      if (parser_info.next_token < 0)
        parser_info.next_token = 0;
      for (uint16_t i = parser_info.next_token; i < parser_info.tokens_count; i++)
      {
        int_parseToken(i, raw_buf.c_str(), parser_info.next_level, (char *)key, index, fb_json_serialize_mode_none);
        if (parser_info.is_token_matches)
          break;
      }
    }
    if (!parser_info.is_token_matches)
    {
      parser_info.parsing_success = false;
      result.success = false;
      int_resetParseResult();
    }
  }
}

void FirebaseJson::int_splitTokens(const std::string &str, std::vector<path_tk_t> &tk, char delim)
{
  std::size_t current, previous = 0;
  current = str.find(delim);
  std::string s;
  while (current != std::string::npos)
  {
    s = str.substr(previous, current - previous);
    int_trim(s);
    if (s.length() > 0)
    {
      path_tk_t tk_t;
      tk_t.path = s;
      tk.push_back(tk_t);
    }

    previous = current + 1;
    current = str.find(delim, previous);
    delay(0);
  }
  s = str.substr(previous, current - previous);
  int_trim(s);
  if (s.length() > 0)
  {
    path_tk_t tk_t;
    tk_t.path = s;
    tk.push_back(tk_t);
  }
  hp->clearS(s);
}

void FirebaseJson::int_ltrim(std::string &str, const std::string &chars)
{
  str.erase(0, str.find_first_not_of(chars));
}

void FirebaseJson::int_rtrim(std::string &str, const std::string &chars)
{
  str.erase(str.find_last_not_of(chars) + 1);
}

void FirebaseJson::int_trim(std::string &str, const std::string &chars)
{
  int_ltrim(str, chars);
  int_rtrim(str, chars);
}

void FirebaseJson::int_parse(const char *path, fb_json_serialize_mode serializeMode)
{
  hp->clearLastError();
  int_clearPathList();
  std::string json_path;
  hp->buildPath(json_path, path, top_level_token_type == fb_json_generic_type_array);

  int_splitTokens(json_path.c_str(), path_list, '/');
  int_fb_json_parseToken();

  hp->clearS(json_path);
  if (!result.success)
    return;
  result.success = false;
  int len = path_list.size();

  int_resetParsserInfo();

  parser_info.removed_token_index = -1;
  parser_info.to_remove_first_token = false;
  parser_info.to_remove_last_token = false;
  int_clearTokenList();
  int_clearTokenDescriptorList();

  if (len == 0)
  {
    int_parse("", 0, -2, serializeMode);
    result.success = true;
  }
  else
  {
    for (int i = 0; i < len; i++)
    {
      if (int_isString(i))
        int_parse(path_list[i].path.c_str(), i, -1, serializeMode);
      else if (int_isArray(i))
        int_parse("", i, int_getArrIndex(i), serializeMode);
      else
        int_parse(path_list[i].path.c_str(), i, -1, serializeMode);
    }
    result.success = parser_info.parsing_completed_count == len;
  }

  int_clearTokenList();
  int_clearPathList();
  hp->clearS(temp_buf);
  tokens.reset();
  tokens = nullptr;
}

void FirebaseJson::int_clearPathList()
{
  size_t len = path_list.size();
  for (size_t i = 0; i < len; i++)
    hp->clearS(path_list[i].path);
  for (int i = len - 1; i >= 0; i--)
    path_list.erase(path_list.begin() + i);
  path_list.clear();
  std::vector<path_tk_t>().swap(path_list);
}

void FirebaseJson::int_clearTokenList()
{
  size_t len = token_item_info_list.size();
  for (int i = len - 1; i >= 0; i--)
    token_item_info_list.erase(token_item_info_list.begin() + i);
  token_item_info_list.clear();
  std::vector<token_item_info_t>().swap(token_item_info_list);
}

void FirebaseJson::int_clearTokenDescriptorList()
{
  size_t len = fb_json_token_descriptor_list.size();
  for (int i = len - 1; i >= 0; i--)
    fb_json_token_descriptor_list.erase(fb_json_token_descriptor_list.begin() + i);
  fb_json_token_descriptor_list.clear();
  std::vector<fb_json_token_descriptor_t>().swap(fb_json_token_descriptor_list);
}

void FirebaseJson::int_parse(const char *key, int16_t level, int16_t index, fb_json_serialize_mode serializeMode)
{
  hp->clearLastError();
  parser_info.is_token_matches = false;
  if (parser_info.parsing_success)
  {
    parser_info.current_level = level;
    if (parser_info.next_token < 0)
      parser_info.next_token = 0;

    for (uint16_t i = parser_info.next_token; i < parser_info.tokens_count; i++)
    {

      int oDepth = parser_info.next_level;

      int_parseToken(i, raw_buf.c_str(), parser_info.next_level, (char *)key, index, serializeMode);

      if (index > -1 && oDepth == parser_info.next_level && parser_info.is_token_matches)
      {
        parser_info.is_token_matches = false;
        break;
      }

      if (oDepth > parser_info.next_level && index == -1)
      {
        if (parser_info.next_level > -1 && parser_info.next_level < (int)path_list.size())
        {
          if (path_list[parser_info.next_level].matched)
          {
            parser_info.is_token_matches = false;
            break;
          }
        }
      }

      if (parser_info.is_token_matches)
      {
        path_list[level].matched = true;
        break;
      }
    }

    if (!parser_info.is_token_matches)
    {
      parser_info.parsing_success = false;
      result.success = false;
    }
  }
}

void FirebaseJson::int_compile(const char *key, int16_t level, int16_t index, const char *replace, fb_json_serialize_mode serializeMode, int16_t refTokenIndex, bool isRemove)
{
  hp->clearLastError();
  parser_info.is_token_matches = false;
  if (parser_info.parsing_success)
  {
    parser_info.current_level = level;
    if (parser_info.next_token < 0)
      parser_info.next_token = 0;
    for (uint16_t i = parser_info.next_token; i < parser_info.tokens_count; i++)
    {
      int_compileToken(i, raw_buf.c_str(), parser_info.next_level, key, index, serializeMode, replace, refTokenIndex, isRemove);
      if (parser_info.is_token_matches)
        break;
    }
    if (!parser_info.is_token_matches)
    {
      parser_info.parsing_success = false;
      result.success = false;
    }
  }
}

void FirebaseJson::int_remove(const char *key, int16_t level, int16_t index, const char *replace, int16_t refTokenIndex, bool isRemove)
{
  hp->clearLastError();
  if (parser_info.parsing_success)
  {
    parser_info.current_level = level;
    if (parser_info.next_token < 0)
      parser_info.next_token = 0;
    for (uint16_t i = parser_info.next_token; i < parser_info.tokens_count; i++)
      int_removeToken(i, raw_buf.c_str(), parser_info.next_level, (char *)key, index, fb_json_serialize_mode_plain, (char *)replace, refTokenIndex, isRemove);
  }
}

bool FirebaseJson::int_isArray(int index)
{
  if (index < (int)path_list.size())
    return path_list[index].path.c_str()[0] == '[' && path_list[index].path.c_str()[path_list[index].path.length() - 1] == ']';
  else
    return false;
}
bool FirebaseJson::int_isString(int index)
{
  if (index < (int)path_list.size())
    return path_list[index].path.c_str()[0] == '"' && path_list[index].path.c_str()[path_list[index].path.length() - 1] == '"';
  else
    return false;
}

int FirebaseJson::int_getArrIndex(int index)
{
  int res = -1;
  if (index < (int)path_list.size())
  {
    res = atoi(path_list[index].path.substr(1, path_list[index].path.length() - 2).c_str());
    if (res < 0)
      res = 0;
  }
  return res;
}

void FirebaseJson::set(const String &path)
{
  hp->clearLastError();
  int_setNull(path.c_str());
}

void FirebaseJson::set(const String &path, const String &value)
{
  hp->clearLastError();
  int_setString(path.c_str(), value.c_str());
}

void FirebaseJson::set(const String &path, const char *value)
{
  hp->clearLastError();
  int_setString(path.c_str(), value);
}

void FirebaseJson::set(const String &path, int value)
{
  hp->clearLastError();
  int_setInt(path.c_str(), value);
}

void FirebaseJson::set(const String &path, unsigned short value)
{
  hp->clearLastError();
  int_setInt(path.c_str(), value);
}

void FirebaseJson::set(const String &path, float value)
{
  hp->clearLastError();
  int_setFloat(path.c_str(), value);
}

void FirebaseJson::set(const String &path, double value)
{
  hp->clearLastError();
  int_setDouble(path.c_str(), value);
}

void FirebaseJson::set(const String &path, bool value)
{
  hp->clearLastError();
  int_setBool(path.c_str(), value);
}

void FirebaseJson::set(const String &path, FirebaseJson &json)
{
  hp->clearLastError();
  int_setJson(path.c_str(), &json);
}

void FirebaseJson::set(const String &path, FirebaseJsonArray &arr)
{
  hp->clearLastError();
  arr.last_error = &last_error;
  int_setArray(path.c_str(), &arr);
}

template <typename T>
bool FirebaseJson::set(const String &path, T value)
{
  if (std::is_same<T, int>::value)
    return int_setInt(path, value);
  else if (std::is_same<T, float>::value)
    return int_setFloat(path, value);
  else if (std::is_same<T, double>::value)
    return int_setDouble(path, value);
  else if (std::is_same<T, bool>::value)
    return int_setBool(path, value);
  else if (std::is_same<T, const char *>::value)
    return int_setString(path, value);
  else if (std::is_same<T, FirebaseJson &>::value)
    return int_setJson(path, &value);
  else if (std::is_same<T, FirebaseJsonArray &>::value)
    return int_setArray(path, &value);
}

void FirebaseJson::int_setString(const std::string &path, const std::string &value)
{
  hp->clearLastError();
  char *tmp = hp->newS(value.length() + 32);
  if (!tmp)
  {
    hp->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
    return;
  }
  char *qt = hp->getStr(fb_json_str_type_qt);
  strcpy(tmp, qt);
  strcat(tmp, value.c_str());
  strcat(tmp, qt);
  hp->delS(qt);
  int_set(path.c_str(), tmp);
  hp->delS(tmp);
  hp->clearS(result.data_raw);
}

void FirebaseJson::int_setInt(const std::string &path, int value)
{
  char *tmp = hp->intStr(value);
  int_set(path.c_str(), tmp);
  hp->delS(tmp);
  hp->clearS(result.data_raw);
}

void FirebaseJson::int_setFloat(const std::string &path, float value)
{
  char *tmp = hp->floatStr(value);
  hp->trimDouble(tmp);
  int_set(path.c_str(), tmp);
  hp->delS(tmp);
  hp->clearS(result.data_raw);
}

void FirebaseJson::int_setDouble(const std::string &path, double value)
{
  char *tmp = hp->doubleStr(value);
  hp->trimDouble(tmp);
  int_set(path.c_str(), tmp);
  hp->delS(tmp);
  hp->clearS(result.data_raw);
}

void FirebaseJson::int_setBool(const std::string &path, bool value)
{
  char *tr = hp->getStr(fb_json_str_type_tr);
  char *fls = hp->getStr(fb_json_str_type_fls);
  if (value)
    int_set(path.c_str(), tr);
  else
    int_set(path.c_str(), fls);
  hp->delS(tr);
  hp->delS(fls);
  hp->clearS(result.data_raw);
}

void FirebaseJson::int_setNull(const std::string &path)
{
  char *nll = hp->getStr(fb_json_str_type_nll);
  int_set(path.c_str(), nll);
  hp->delS(nll);
  hp->clearS(result.data_raw);
}

void FirebaseJson::int_setJson(const std::string &path, FirebaseJson *json)
{
  int_set(path.c_str(), json->raw());
}

void FirebaseJson::int_setArray(const std::string &path, FirebaseJsonArray *arr)
{
  arr->last_error = &last_error;
  int_set(path.c_str(), arr->raw());
}

void FirebaseJson::int_set(const char *path, const char *data)
{
  hp->clearLastError();
  int_clearPathList();
  std::string json_path;
  hp->buildPath(json_path, path, top_level_token_type == fb_json_generic_type_array);

  int_splitTokens(json_path.c_str(), path_list, '/');
  int_fb_json_parseToken();
  hp->clearS(json_path);

  if (!result.success)
    return;

  result.success = false;
  int len = path_list.size();

  int_resetParsserInfo();

  parser_info.removed_token_index = -1;
  parser_info.to_remove_first_token = false;
  parser_info.to_remove_last_token = false;
  int_clearTokenList();
  int_clearTokenDescriptorList();
  for (int i = 0; i < len; i++)
  {
    if (int_isString(i))
      int_compile(path_list[i].path.c_str(), i, -1, data, fb_json_serialize_mode_plain);
    else if (int_isArray(i))
      int_compile("", i, int_getArrIndex(i), data, fb_json_serialize_mode_plain);
    else
      int_compile(path_list[i].path.c_str(), i, -1, data, fb_json_serialize_mode_plain);
  }
  int_clearTokenList();
  int_clearTokenDescriptorList();

  if (parser_info.parsing_completed_count != len)
  {
    hp->clearS(result.data_raw);
    hp->clearS(temp_buf);
    int refTokenIndex = parser_info.ref_token_index;

    int_resetParsserInfo();

    parser_info.is_token_matches = false;
    parser_info.parsing_success = true;
    for (int i = 0; i < len; i++)
    {
      if (int_isString(i))
        int_compile(path_list[i].path.c_str(), i, -1, data, fb_json_serialize_mode_plain, refTokenIndex);
      else if (int_isArray(i))
        int_compile("", i, int_getArrIndex(i), data, fb_json_serialize_mode_plain, refTokenIndex);
      else
        int_compile(path_list[i].path.c_str(), i, -1, data, fb_json_serialize_mode_plain, refTokenIndex);
    }
    int_clearTokenList();
    int_clearTokenDescriptorList();
  }

  result.success = true;

  hp->storeS(raw_buf, result.data_raw.c_str(), false);
  hp->clearS(result.data_raw);
  hp->clearS(temp_buf);

  int_clearPathList();

  tokens.reset();
  tokens = nullptr;
}

bool FirebaseJson::remove(const String &path)
{
  hp->clearLastError();
  int_clearPathList();

  std::string json_path;
  hp->buildPath(json_path, path.c_str(), top_level_token_type == fb_json_generic_type_array);

  int_splitTokens(json_path.c_str(), path_list, '/');
  int_fb_json_parseToken();
  hp->clearS(json_path);
  if (!result.success)
    return false;

  result.success = false;
  int len = path_list.size();

  int_resetParsserInfo();

  parser_info.removed_token_index = -1;
  parser_info.to_remove_first_token = false;
  parser_info.to_remove_last_token = false;
  int_clearTokenList();
  int_clearTokenDescriptorList();
  for (int i = 0; i < len; i++)
  {
    if (int_isString(i))
      int_compile(path_list[i].path.c_str(), i, -1, "", fb_json_serialize_mode_none, -1, true);
    else if (int_isArray(i))
      int_compile("", i, int_getArrIndex(i), "", fb_json_serialize_mode_none, -1, true);
    else
      int_compile(path_list[i].path.c_str(), i, -1, "", fb_json_serialize_mode_none, -1, true);
  }
  int_clearTokenList();
  int_clearTokenDescriptorList();
  hp->clearS(result.data_raw);
  int refTokenIndex = parser_info.removed_token_index;
  if (parser_info.parsing_completed_count == len)
  {

    int_resetParsserInfo();

    parser_info.is_token_matches = false;
    parser_info.parsing_success = true;
    result.success = true;
    last_token.skip = false;
    last_token.node_length = 0;
    last_token.node_index = 0;

    if (int_isString(len - 1))
      int_remove(path_list[len - 1].path.c_str(), -1, -1, "", refTokenIndex, true);
    else
      int_remove("", -1, int_getArrIndex(len - 1), "", refTokenIndex, true);
    result.data_raw += temp_buf;
    int_clearTokenList();
    int_clearTokenDescriptorList();
  }

  raw_buf = result.data_raw;

  //fix for the remaining parent when all childs removed
  if (raw_buf.length() > 0)
  {
    char *temp = hp->strP(fb_json_str_32);
    size_t p1 = raw_buf.find(temp);
    hp->delS(temp);

    if (p1 == std::string::npos)
    {
      temp = hp->strP(fb_json_str_33);
      p1 = raw_buf.find(temp);
      hp->delS(temp);
    }

    if (p1 != std::string::npos)
    {
      int p3 = p1;
      if (p3 > 0)
        p3--;
      temp = hp->strP(fb_json_str_2);
      size_t p2 = raw_buf.rfind(temp, p3);
      hp->delS(temp);
      if (p2 != std::string::npos)
      {
        if (p2 > 0)
        {
          if (raw_buf[p2 - 1] == ',')
            p2--;
        }
        p1 += 2;
        raw_buf.replace(p2, p1 - p2, "");
      }
    }
  }

  hp->shrinkS(raw_buf);

  int_clearPathList();

  hp->clearS(result.data_raw);
  hp->clearS(temp_buf);

  tokens.reset();
  tokens = nullptr;
  return result.success;
}

fb_json_last_error_t FirebaseJson::getLastError()
{
  return last_error;
}

void FirebaseJson::int_resetParsserInfo()
{
  parser_info.next_level = -1;
  parser_info.next_token = 0;
  parser_info.skip_level = -1;
  parser_info.parent_index = -1;
  parser_info.to_set_ref_token = false;
  parser_info.parsing_completed_count = 0;
  parser_info.to_replace_array = false;
  parser_info.ref_token_index = -1;
}

void FirebaseJson::int_resetParseResult()
{
  result.type_num = 0;
  result.type.remove(0, result.type.length());
  result.typeNum = 0;
  result.stringValue.remove(0, result.stringValue.length());
  hp->clearS(result.data_raw);
  result.intValue = 0;
  result.floatValue = 0;
  result.doubleValue = 0;
  result.boolValue = false;
}

void FirebaseJson::int_setElementType()
{
  hp->clearLastError();
  bool typeSet = false;
  char *buf = hp->newS(32);
  char *tmp = hp->newS(32);
  if (!buf || !tmp)
  {
    hp->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
    return;
  }
  char *tmp2 = nullptr;
  if (result.type_num == fb_json_generic_type_primitive)
  {
    tmp2 = hp->newS(result.stringValue.length() + 1);
    if (!tmp2)
    {
      hp->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
      return;
    }
    strcpy(tmp2, result.stringValue.c_str());
  }

  char *nll = hp->getStr(fb_json_str_type_nll);
  char *tr = hp->getStr(fb_json_str_type_tr);
  char *fls = hp->getStr(fb_json_str_type_fls);
  char *str = hp->getStr(fb_json_str_type_string);
  char *dbl = hp->getStr(fb_json_str_type_dbl);
  char *bl = hp->getStr(fb_json_str_type_bl);
  char *obj = hp->getStr(fb_json_str_type_obj);
  char *arry = hp->getStr(fb_json_str_type_arry);
  char *undef = hp->getStr(fb_json_str_type_undef);
  char *dot = hp->getStr(fb_json_str_type_dot);
  double d = 0;
  switch (result.type_num)
  {
  case fb_json_generic_type_undefined:
    strcpy(buf, undef);
    result.typeNum = JSON_UNDEFINED;
    break;
  case fb_json_generic_type_object:
    strcpy(buf, obj);
    result.typeNum = JSON_OBJECT;
    break;
  case fb_json_generic_type_array:
    strcpy(buf, arry);
    result.typeNum = JSON_ARRAY;
    break;
  case fb_json_generic_type_string:
    strcpy(buf, str);
    result.typeNum = JSON_STRING;
    break;
  case fb_json_generic_type_primitive:
    if (!typeSet && strcmp(tmp2, tr) == 0)
    {
      typeSet = true;
      strcpy(buf, bl);
      result.typeNum = JSON_BOOL;
      result.boolValue = true;
      result.floatValue = 1.0f;
      result.doubleValue = 1.0;
      result.intValue = 1;
    }
    else
    {
      if (!typeSet && strcmp(tmp2, fls) == 0)
      {
        typeSet = true;
        strcpy(buf, bl);
        result.typeNum = JSON_BOOL;
        result.boolValue = false;
        result.floatValue = 0.0f;
        result.doubleValue = 0.0;
        result.intValue = 0;
      }
    }

    if (!typeSet && strcmp(tmp2, nll) == 0)
    {
      typeSet = true;
      strcpy(buf, nll);
      result.typeNum = JSON_NULL;
    }

    if (!typeSet)
    {
      typeSet = true;
      strcpy(tmp, dot);
      d = atof(tmp2);
      if (d > 0x7fffffff)
      {
        strcpy(buf, dbl);
        result.floatValue = (float)d;
        result.doubleValue = d;
        result.intValue = atoi(tmp2);
        result.boolValue = atof(tmp2) > 0 ? true : false;
        result.typeNum = JSON_DOUBLE;
      }
      else
      {
        if (hp->strpos(tmp2, tmp, 0) > -1)
        {
          strcpy(buf, dbl);
          result.floatValue = (float)d;
          result.doubleValue = d;
          result.intValue = atoi(tmp2);
          result.boolValue = atof(tmp2) > 0 ? true : false;
          result.typeNum = JSON_FLOAT;
        }
        else
        {
          result.intValue = atoi(tmp2);
          result.floatValue = atof(tmp2);
          result.doubleValue = atof(tmp2);
          result.boolValue = atof(tmp2) > 0 ? true : false;
          char *in = hp->getStr(fb_json_str_type_int);
          strcpy(buf, in);
          hp->delS(in);
          result.typeNum = JSON_INT;
        }
      }
    }
    break;
  default:
    break;
  }
  hp->delS(nll);
  hp->delS(tr);
  hp->delS(fls);
  hp->delS(str);
  hp->delS(dbl);
  hp->delS(bl);
  hp->delS(obj);
  hp->delS(arry);
  hp->delS(undef);
  hp->delS(dot);
  result.type = buf;
  hp->delS(buf);
  hp->delS(tmp);
  if (tmp2)
    hp->delS(tmp2);
}

/**
 * Allocates a fresh unused token from the token pool.
 */
fb_json_token_t *FirebaseJson::int_fb_json_alloc_token(fb_json_parser *parser, fb_json_token_t *tokens, size_t num_tokens)
{
  fb_json_token_t *tok;
  if (parser->next_token >= num_tokens)
  {
    return NULL;
  }
  tok = &tokens[parser->next_token++];
  tok->start = tok->end = -1;
  tok->size = 0;
#ifdef FB_JSON_PARENT_LINKS
  tok->parent = -1;
#endif
  return tok;
}

/**
 * Fills token type and boundaries.
 */
void FirebaseJson::int_fb_json_fill_token(fb_json_token_t *token, fb_json_generic_type_t type, int start, int end)
{
  token->type = type;
  token->start = start;
  token->end = end;
  token->size = 0;
}

/**
 * Fills next available token with JSON primitive.
 */
int FirebaseJson::int_fb_json_parse_primitive(fb_json_parser *parser, const char *js, size_t len, fb_json_token_t *tokens, size_t num_tokens)
{
  fb_json_token_t *token;
  int start;

  start = parser->pos;

  for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++)
  {
    switch (js[parser->pos])
    {
#ifndef FB_JSON_STRICT
    /* In strict mode primitive must be followed by "," or "}" or "]" */
    case ':':
#endif
    case '\t':
    case '\r':
    case '\n':
    case ' ':
    case ',':
    case ']':
    case '}':
      goto found;
    }
    if (js[parser->pos] < 32 || js[parser->pos] >= 127)
    {
      parser->pos = start;
      return fb_json_err_invalid;
    }
  }
#ifdef FB_JSON_STRICT
  /* In strict mode primitive must be followed by a comma/object/array */
  parser->pos = start;
  return fb_json_err_part;
#endif

found:
  if (tokens == NULL)
  {
    parser->pos--;
    return 0;
  }
  token = int_fb_json_alloc_token(parser, tokens, num_tokens);
  if (token == NULL)
  {
    parser->pos = start;
    return fb_json_err_nomem;
  }
  int_fb_json_fill_token(token, fb_json_generic_type_primitive, start, parser->pos);
#ifdef FB_JSON_PARENT_LINKS
  token->parent = parser->super_token;
#endif
  parser->pos--;
  return 0;
}

/**
 * Fills next token with JSON string.
 */
int FirebaseJson::int_fb_json_parse_string(fb_json_parser *parser, const char *js, size_t len, fb_json_token_t *tokens, size_t num_tokens)
{
  fb_json_token_t *token;

  int start = parser->pos;

  parser->pos++;

  /* Skip starting quote */
  for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++)
  {
    char c = js[parser->pos];

    /* Quote: end of string */
    if (c == '\"')
    {
      if (tokens == NULL)
      {
        return 0;
      }
      token = int_fb_json_alloc_token(parser, tokens, num_tokens);
      if (token == NULL)
      {
        parser->pos = start;
        return fb_json_err_nomem;
      }
      int_fb_json_fill_token(token, fb_json_generic_type_string, start + 1, parser->pos);
#ifdef FB_JSON_PARENT_LINKS
      token->parent = parser->super_token;
#endif
      return 0;
    }

    /* Backslash: Quoted symbol expected */
    if (c == '\\' && parser->pos + 1 < len)
    {
      int i;
      parser->pos++;
      switch (js[parser->pos])
      {
      /* Allowed escaped symbols */
      case '\"':
      case '/':
      case '\\':
      case 'b':
      case 'f':
      case 'r':
      case 'n':
      case 't':
        break;
      /* Allows escaped symbol \uXXXX */
      case 'u':
        parser->pos++;
        for (i = 0; i < 4 && parser->pos < len && js[parser->pos] != '\0'; i++)
        {
          /* If it isn't a hex character we have an error */
          if (!((js[parser->pos] >= 48 && js[parser->pos] <= 57) || /* 0-9 */
                (js[parser->pos] >= 65 && js[parser->pos] <= 70) || /* A-F */
                (js[parser->pos] >= 97 && js[parser->pos] <= 102)))
          { /* a-f */
            parser->pos = start;
            return fb_json_err_invalid;
          }
          parser->pos++;
        }
        parser->pos--;
        break;
      /* Unexpected symbol */
      default:
        parser->pos = start;
        return fb_json_err_invalid;
      }
    }
  }
  parser->pos = start;
  return fb_json_err_part;
}

/**
 * Parse JSON string and fill tokens.
 */
int FirebaseJson::int_fb_json_parse(fb_json_parser *parser, const char *js, size_t len, fb_json_token_t *tokens, unsigned int num_tokens)
{
  int r;
  int i;
  fb_json_token_t *token;
  int count = parser->next_token;

  for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++)
  {
    char c;
    fb_json_generic_type_t type;

    c = js[parser->pos];
    switch (c)
    {
    case '{':
    case '[':
      count++;
      if (tokens == NULL)
      {
        break;
      }
      token = int_fb_json_alloc_token(parser, tokens, num_tokens);
      if (token == NULL)
      {
        return fb_json_err_nomem;
      }
      if (parser->super_token != -1)
      {
        fb_json_token_t *t = &tokens[parser->super_token];
#ifdef FB_JSON_STRICT
        /* In strict mode an object or array can't become a key */
        if (t->type == fb_json_generic_type_object)
        {
          return fb_json_err_invalid;
        }
#endif
        t->size++;
#ifdef FB_JSON_PARENT_LINKS
        token->parent = parser->super_token;
#endif
      }
      token->type = (c == '{' ? fb_json_generic_type_object : fb_json_generic_type_array);
      token->start = parser->pos;
      parser->super_token = parser->next_token - 1;
      if (parser->pos > 0)
        if (js[parser->pos - 1] == '{' && js[parser->pos] == '[')
          return fb_json_err_invalid;
      break;
    case '}':
    case ']':
      if (tokens == NULL)
      {
        break;
      }
      type = (c == '}' ? fb_json_generic_type_object : fb_json_generic_type_array);
#ifdef FB_JSON_PARENT_LINKS
      if (parser->next_token < 1)
      {
        return fb_json_err_invalid;
      }
      token = &tokens[parser->next_token - 1];
      for (;;)
      {
        if (token->start != -1 && token->end == -1)
        {
          if (token->type != type)
          {
            return fb_json_err_invalid;
          }
          token->end = parser->pos + 1;
          parser->super_token = token->parent;
          break;
        }
        if (token->parent == -1)
        {
          if (token->type != type || parser->super_token == -1)
          {
            return fb_json_err_invalid;
          }
          break;
        }
        token = &tokens[token->parent];
      }
#else
      for (i = parser->next_token - 1; i >= 0; i--)
      {
        token = &tokens[i];
        if (token->start != -1 && token->end == -1)
        {
          if (token->type != type)
          {
            return fb_json_err_invalid;
          }
          parser->super_token = -1;
          token->end = parser->pos + 1;
          break;
        }
      }
      /* Error if unmatched closing bracket */
      if (i == -1)
      {
        return fb_json_err_invalid;
      }
      for (; i >= 0; i--)
      {
        token = &tokens[i];
        if (token->start != -1 && token->end == -1)
        {
          parser->super_token = i;
          break;
        }
      }
#endif
      break;
    case '\"':
      r = int_fb_json_parse_string(parser, js, len, tokens, num_tokens);
      if (r < 0)
      {
        return r;
      }
      count++;
      if (parser->super_token != -1 && tokens != NULL)
      {
        tokens[parser->super_token].size++;
      }
      break;
    case '\t':
    case '\r':
    case '\n':
    case ' ':
      break;
    case ':':
      parser->super_token = parser->next_token - 1;
      break;
    case ',':
      if (tokens != NULL && parser->super_token != -1 &&
          tokens[parser->super_token].type != fb_json_generic_type_array &&
          tokens[parser->super_token].type != fb_json_generic_type_object)
      {
#ifdef FB_JSON_PARENT_LINKS
        parser->super_token = tokens[parser->super_token].parent;
#else
        for (i = parser->next_token - 1; i >= 0; i--)
        {
          if (tokens[i].type == fb_json_generic_type_array || tokens[i].type == fb_json_generic_type_object)
          {
            if (tokens[i].start != -1 && tokens[i].end == -1)
            {
              parser->super_token = i;
              break;
            }
          }
        }
#endif
      }
      break;
#ifdef FB_JSON_STRICT
    /* In strict mode primitives are: numbers and booleans */
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case 't':
    case 'f':
    case 'n':
      /* And they must not be keys of the object */
      if (tokens != NULL && parser->super_token != -1)
      {
        const fb_json_token_t *t = &tokens[parser->super_token];
        if (t->type == fb_json_generic_type_object ||
            (t->type == fb_json_generic_type_string && t->size != 0))
        {
          return fb_json_err_invalid;
        }
      }
#else
    /* In non-strict mode every unquoted value is a primitive */
    default:
#endif
      r = int_fb_json_parse_primitive(parser, js, len, tokens, num_tokens);
      if (r < 0)
      {
        return r;
      }
      count++;
      if (parser->super_token != -1 && tokens != NULL)
      {
        tokens[parser->super_token].size++;
      }
      break;

#ifdef FB_JSON_STRICT
    /* Unexpected char in strict mode */
    default:
      if (tokens != NULL)
        return fb_json_err_invalid;
#endif
    }
  }

  if (tokens != NULL)
  {
    for (i = parser->next_token - 1; i >= 0; i--)
    {
      /* Unmatched opened object or array */
      if (tokens[i].start != -1 && tokens[i].end == -1)
      {
        return fb_json_err_part;
      }
    }
  }

  return count;
}

/**
 * Creates a new parser based over a given  buffer with an array of tokens
 * available.
 */
void FirebaseJson::int_fb_json_init(fb_json_parser *parser)
{
  parser->pos = 0;
  parser->next_token = 0;
  parser->super_token = -1;
}

FirebaseJsonArray::FirebaseJsonArray()
{
  hp->clearLastError();
}

FirebaseJsonArray::FirebaseJsonArray(fb_json_last_error_t *lastErr)
{
  last_error = lastErr;
};

FirebaseJsonArray::~FirebaseJsonArray()
{
  delete hp;
};

FirebaseJsonArray &FirebaseJsonArray::add()
{
  int_addNull();
  return *this;
}

FirebaseJsonArray &FirebaseJsonArray::add(const String &value)
{
  int_addString(value.c_str());
  return *this;
}
FirebaseJsonArray &FirebaseJsonArray::add(const char *value)
{
  int_addString(value);
  return *this;
}
FirebaseJsonArray &FirebaseJsonArray::add(int value)
{
  int_addInt(value);
  return *this;
}

FirebaseJsonArray &FirebaseJsonArray::add(unsigned short value)
{
  int_addInt(value);
  return *this;
}

FirebaseJsonArray &FirebaseJsonArray::add(float value)
{
  int_addFloat(value);
  return *this;
}

FirebaseJsonArray &FirebaseJsonArray::add(double value)
{
  int_addDouble(value);
  return *this;
}

FirebaseJsonArray &FirebaseJsonArray::add(bool value)
{
  int_addBool(value);
  return *this;
}

FirebaseJsonArray &FirebaseJsonArray::add(FirebaseJson &json)
{
  int_addJson(&json);
  return *this;
}

FirebaseJsonArray &FirebaseJsonArray::add(FirebaseJsonArray &arr)
{
  int_addArray(&arr);
  return *this;
}

template <typename T>
FirebaseJsonArray &FirebaseJsonArray::add(T value)
{
  if (std::is_same<T, int>::value)
    int_addInt(value);
  else if (std::is_same<T, float>::value)
    int_addFloat(value);
  else if (std::is_same<T, double>::value)
    int_addDouble(value);
  else if (std::is_same<T, bool>::value)
    int_addBool(value);
  else if (std::is_same<T, const char *>::value)
    int_addString(value);
  else if (std::is_same<T, FirebaseJson &>::value)
    int_addJson(&value);
  else if (std::is_same<T, FirebaseJsonArray &>::value)
    int_addArray(&value);
  return *this;
}

void FirebaseJsonArray::int_addString(const std::string &value)
{
  arr_size++;
  js.int_addArrayStr(value.c_str(), value.length(), true);
}

void FirebaseJsonArray::int_addInt(int value)
{
  arr_size++;
  char *buf = hp->intStr(value);
  char *pd = hp->getStr(fb_json_str_type_pd);
  sprintf(buf, pd, value);
  hp->delS(pd);
  js.int_addArrayStr(buf, 60, false);
  hp->delS(buf);
}

void FirebaseJsonArray::int_addFloat(float value)
{
  arr_size++;
  char *buf = hp->floatStr(value);
  hp->trimDouble(buf);
  js.int_addArrayStr(buf, 60, false);
  hp->delS(buf);
}

void FirebaseJsonArray::int_addDouble(double value)
{
  arr_size++;
  char *buf = hp->doubleStr(value);
  hp->trimDouble(buf);
  js.int_addArrayStr(buf, 60, false);
  hp->delS(buf);
}

void FirebaseJsonArray::int_addBool(bool value)
{
  arr_size++;
  char *tr = hp->getStr(fb_json_str_type_tr);
  char *fls = hp->getStr(fb_json_str_type_fls);
  if (value)
    js.int_addArrayStr(tr, 6, false);
  else
    js.int_addArrayStr(fls, 7, false);
  hp->delS(tr);
  hp->delS(fls);
}

void FirebaseJsonArray::int_addNull()
{
  arr_size++;
  char *nll = hp->getStr(fb_json_str_type_nll);
  js.int_addArrayStr(nll, 6, false);
  hp->delS(nll);
}

void FirebaseJsonArray::int_addJson(FirebaseJson *json)
{
  arr_size++;
  js.int_addArrayStr(json->raw(), strlen(json->raw()), false);
}

void FirebaseJsonArray::int_addArray(FirebaseJsonArray *arr)
{
  arr_size++;
  js.int_addArrayStr(arr->raw(), strlen(arr->raw()), false);
}

FirebaseJsonArray &FirebaseJsonArray::setJsonArrayData(const String &data)
{
  int start_pos = data.indexOf('[');
  int end_pos = data.indexOf(']');

  if (start_pos != -1 && end_pos != -1 && start_pos != end_pos)
  {
    char *r = hp->strP(fb_json_str_21);
    hp->clearS(js.raw_buf);
    js.raw_buf = r;
    js.raw_buf += data.c_str();
    hp->appendS(js.raw_buf, fb_json_str_type_brk2);
    hp->delS(r);
    r = hp->strP(fb_json_str_26);
    FirebaseJsonData data;
    js.get(data, r);
    hp->delS(r);
    js.raw_buf = data.data_raw;
    data.stringValue.remove(0, data.stringValue.length());
    hp->shrinkS(data.data_raw);
    hp->shrinkS(js.raw_buf);
  }
  return *this;
}

bool FirebaseJsonArray::get(FirebaseJsonData &jsonData, const String &path)
{
  return int_get(jsonData, path.c_str());
}

bool FirebaseJsonArray::get(FirebaseJsonData &jsonData, int index)
{
  char *tmp = hp->intStr(index);
  std::string path = "";
  hp->appendS(path, fb_json_str_type_brk3);
  path += tmp;
  hp->appendS(path, fb_json_str_type_brk4);
  bool ret = int_get(jsonData, path.c_str());
  hp->clearS(path);
  hp->delS(tmp);
  return ret;
}

bool FirebaseJsonArray::int_get(FirebaseJsonData &jsonData, const char *path)
{
  char *root = hp->strP(fb_json_str_21);
  char *root_path = hp->strP(fb_json_str_26);
  char *slash = hp->strP(fb_json_str_27);

  js.raw_buf.insert(0, root);
  hp->appendS(js.raw_buf, fb_json_str_type_brk2);

  std::string json_path = root_path;
  json_path += slash;
  json_path += path;
  js.int_clearPathList();
  js.int_splitTokens(json_path.c_str(), js.path_list, '/');
  if (!js.int_isArray(1))
  {
    js.result.success = false;
    goto ex_;
  }
  if (js.int_getArrIndex(1) < 0)
  {
    js.result.success = false;
    goto ex_;
  }
  js.int_parse(json_path.c_str(), FirebaseJson::fb_json_serialize_mode_none);
  if (js.result.success)
  {
    if (js.result.type_num == fb_json_generic_type_string && js.result.stringValue.c_str()[0] == '"' && js.result.stringValue.c_str()[js.result.stringValue.length() - 1] == '"')
      js.result.stringValue = js.result.stringValue.substring(1, js.result.stringValue.length() - 1).c_str();
    jsonData = js.result;
  }
ex_:

  js.raw_buf.resize(js.raw_buf.size() - 1);
  js.raw_buf.erase(0, strlen(root));

  hp->shrinkS(js.raw_buf);
  hp->clearS(jsonData.data_raw);
  hp->clearS(js.result.data_raw);

  hp->delS(root);
  hp->delS(root_path);
  hp->delS(slash);
  hp->clearS(json_path);

  js.int_clearPathList();
  js.tokens.reset();
  js.tokens = nullptr;
  return js.result.success;
}

size_t FirebaseJsonArray::size()
{
  return arr_size;
}

void FirebaseJsonArray::toString(String &buf, bool prettify)
{
  prettify ? buf = int_raw(FirebaseJson::fb_json_serialize_mode_pretty) : buf = int_raw(FirebaseJson::fb_json_serialize_mode_plain);
}

const char *FirebaseJsonArray::raw()
{
  return int_raw(FirebaseJson::fb_json_serialize_mode_plain);
}

const char *FirebaseJsonArray::int_raw(FirebaseJson::fb_json_serialize_mode mode)
{
  if (mode == FirebaseJson::fb_json_serialize_mode_pretty)
  {
    char *root = hp->strP(fb_json_str_21);
    char *root_path = hp->strP(fb_json_str_26);

    hp->clearS(js.result.data_raw);
    hp->clearS(js.temp_buf);

    js.raw_buf.insert(0, root);
    hp->appendS(js.raw_buf, fb_json_str_type_brk2);

    js.int_parse(root_path, FirebaseJson::fb_json_serialize_mode_pretty);
    hp->clearS(js.temp_buf);

    js.int_clearPathList();
    js.tokens.reset();
    js.tokens = nullptr;

    hp->storeS(js.raw_buf, js.result.data_raw.c_str(), false);
    hp->delS(root);
    hp->delS(root_path);
  }

  return js.raw_buf.c_str();
}

FirebaseJsonArray &FirebaseJsonArray::clear()
{
  js.clear();
  js.result.success = false;
  js.result.stringValue.remove(0, js.result.stringValue.length());
  js.result.boolValue = false;
  js.result.doubleValue = 0;
  js.result.intValue = 0;
  js.result.floatValue = 0;
  js.result.arr_size = 0;
  arr_size = 0;
  return *this;
}
void FirebaseJsonArray::int_setByIndex(int index, const char *value, bool isStr)
{
  char *tmp = hp->newS(50);
  std::string path = "";
  hp->appendS(path, fb_json_str_type_brk3);
  sprintf(tmp, "%d", index);
  path += tmp;
  hp->appendS(path, fb_json_str_type_brk4);
  int_set(path.c_str(), value, isStr);
  hp->clearS(path);
  hp->delS(tmp);
}

void FirebaseJsonArray::int_set(const char *path, const char *value, bool isStr)
{
  char *root = hp->strP(fb_json_str_21);
  char *root_path = hp->strP(fb_json_str_26);
  char *slash = hp->strP(fb_json_str_27);
  js.result.success = false;

  js.raw_buf.insert(0, root);
  hp->appendS(js.raw_buf, fb_json_str_type_brk2);

  char *tmp2 = hp->newS(strlen(value) + 10);
  char *qt = hp->getStr(fb_json_str_type_qt);
  if (isStr)
    strcpy_P(tmp2, qt);
  strcat(tmp2, value);
  if (isStr)
    strcat_P(tmp2, qt);
  hp->delS(qt);
  std::string json_path = root_path;
  json_path += slash;
  json_path += path;
  js.int_clearPathList();
  js.int_splitTokens(json_path, js.path_list, '/');
  if (!js.int_isArray(1))
  {
    hp->delS(tmp2);
    goto ex_2;
  }

  if (js.int_getArrIndex(1) < 0)
  {
    hp->delS(tmp2);
    goto ex_2;
  }

  js.int_set(json_path.c_str(), tmp2);
  hp->delS(tmp2);
  hp->clearS(json_path);
  if (js.result.success)
  {
    hp->clearS(js.result.data_raw);
    hp->clearS(js.temp_buf);
    js.int_parse(root_path, FirebaseJson::fb_json_serialize_mode_plain);
    if (js.result.success)
    {
      arr_size = js.result.arr_size;
      hp->storeS(js.raw_buf, js.result.data_raw.c_str(), false);
    }
  }
  else
  {
    js.raw_buf.resize(js.raw_buf.size() - 1);
    js.raw_buf.erase(0, strlen(root));
  }

ex_2:

  hp->shrinkS(js.raw_buf);

  hp->delS(root);
  hp->delS(root_path);
  hp->delS(slash);
  hp->clearS(js.result.data_raw);
  hp->clearS(js.temp_buf);
  js.int_clearPathList();
  js.tokens.reset();
  js.tokens = nullptr;
}

void FirebaseJsonArray::set(int index)
{
  return int_setNull(index);
}

void FirebaseJsonArray::set(const String &path)
{
  int_setNull(path);
}

void FirebaseJsonArray::set(int index, const String &value)
{
  int_setString(index, value.c_str());
}

void FirebaseJsonArray::set(const String &path, const String &value)
{
  int_setString(path, value.c_str());
}

void FirebaseJsonArray::set(int index, const char *value)
{
  int_setString(index, value);
}

void FirebaseJsonArray::set(const String &path, const char *value)
{
  int_setString(path, value);
}

void FirebaseJsonArray::set(int index, int value)
{
  int_setInt(index, value);
}

void FirebaseJsonArray::set(int index, unsigned short value)
{
  int_setInt(index, value);
}

void FirebaseJsonArray::set(const String &path, int value)
{
  int_setInt(path, value);
}

void FirebaseJsonArray::set(const String &path, unsigned short value)
{
  int_setInt(path, value);
}

void FirebaseJsonArray::set(int index, float value)
{
  int_setFloat(index, value);
}

void FirebaseJsonArray::set(const String &path, float value)
{
  int_setFloat(path, value);
}

void FirebaseJsonArray::set(int index, double value)
{
  int_setDouble(index, value);
}

void FirebaseJsonArray::set(const String &path, double value)
{
  int_setDouble(path, value);
}

void FirebaseJsonArray::set(int index, bool value)
{
  int_setBool(index, value);
}

void FirebaseJsonArray::set(const String &path, bool value)
{
  int_setBool(path, value);
}

void FirebaseJsonArray::set(int index, FirebaseJson &json)
{
  int_setJson(index, &json);
}

void FirebaseJsonArray::set(const String &path, FirebaseJson &json)
{
  int_setJson(path, &json);
}

void FirebaseJsonArray::set(int index, FirebaseJsonArray &arr)
{
  arr.last_error = last_error;
  int_setArray(index, &arr);
}

void FirebaseJsonArray::set(const String &path, FirebaseJsonArray &arr)
{
  arr.last_error = last_error;
  int_setArray(path, &arr);
}

template <typename T>
void FirebaseJsonArray::set(int index, T value)
{
  if (std::is_same<T, int>::value)
    int_setInt(index, value);
  else if (std::is_same<T, float>::value)
    int_setFloat(index, value);
  else if (std::is_same<T, double>::value)
    int_setDouble(index, value);
  else if (std::is_same<T, bool>::value)
    int_setBool(index, value);
  else if (std::is_same<T, const char *>::value)
    int_setString(index, value);
  else if (std::is_same<T, FirebaseJson &>::value)
    int_setJson(index, &value);
  else if (std::is_same<T, FirebaseJsonArray &>::value)
    int_setArray(index, &value);
}

template <typename T>
void FirebaseJsonArray::set(const String &path, T value)
{
  if (std::is_same<T, int>::value)
    int_setInt(path, value);
  else if (std::is_same<T, float>::value)
    int_setFloat(path, value);
  else if (std::is_same<T, double>::value)
    int_setDouble(path, value);
  else if (std::is_same<T, bool>::value)
    int_setBool(path, value);
  else if (std::is_same<T, const char *>::value)
    int_setString(path, value);
  else if (std::is_same<T, FirebaseJson &>::value)
    int_setJson(path, &value);
  else if (std::is_same<T, FirebaseJsonArray &>::value)
    int_setArray(path, &value);
}

void FirebaseJsonArray::int_setString(int index, const std::string &value)
{
  int_setByIndex(index, value.c_str(), true);
}

void FirebaseJsonArray::int_setString(const String &path, const std::string &value)
{
  int_set(path.c_str(), value.c_str(), true);
}

void FirebaseJsonArray::int_setInt(int index, int value)
{
  char *tmp = hp->intStr(value);
  int_setByIndex(index, tmp, false);
  hp->delS(tmp);
}

void FirebaseJsonArray::int_setInt(const String &path, int value)
{
  char *tmp = hp->intStr(value);
  int_set(path.c_str(), tmp, false);
  hp->delS(tmp);
}

void FirebaseJsonArray::int_setFloat(int index, float value)
{
  char *tmp = hp->floatStr(value);
  hp->trimDouble(tmp);
  int_setByIndex(index, tmp, false);
  hp->delS(tmp);
}

void FirebaseJsonArray::int_setFloat(const String &path, float value)
{
  char *tmp = hp->floatStr(value);
  hp->trimDouble(tmp);
  int_set(path.c_str(), tmp, false);
  hp->delS(tmp);
}

void FirebaseJsonArray::int_setDouble(int index, double value)
{
  char *tmp = hp->doubleStr(value);
  hp->trimDouble(tmp);
  int_setByIndex(index, tmp, false);
  hp->delS(tmp);
}

void FirebaseJsonArray::int_setDouble(const String &path, double value)
{
  char *tmp = hp->doubleStr(value);
  hp->trimDouble(tmp);
  int_set(path.c_str(), tmp, false);
  hp->delS(tmp);
}

void FirebaseJsonArray::int_setBool(int index, bool value)
{
  char *tr = hp->getStr(fb_json_str_type_tr);
  char *fls = hp->getStr(fb_json_str_type_fls);
  if (value)
    int_setByIndex(index, tr, false);
  else
    int_setByIndex(index, fls, false);
  hp->delS(tr);
  hp->delS(fls);
}

void FirebaseJsonArray::int_setBool(const String &path, bool value)
{
  char *tr = hp->getStr(fb_json_str_type_tr);
  char *fls = hp->getStr(fb_json_str_type_fls);
  if (value)
    int_set(path.c_str(), tr, false);
  else
    int_set(path.c_str(), fls, false);
  hp->delS(tr);
  hp->delS(fls);
}

void FirebaseJsonArray::int_setNull(int index)
{
  char *nll = hp->getStr(fb_json_str_type_nll);
  int_setByIndex(index, nll, false);
  hp->delS(nll);
}

void FirebaseJsonArray::int_setNull(const String &path)
{
  char *nll = hp->getStr(fb_json_str_type_nll);
  int_set(path.c_str(), nll, false);
  hp->delS(nll);
}

void FirebaseJsonArray::int_setJson(int index, FirebaseJson *json)
{
  int_setByIndex(index, json->raw(), false);
}

void FirebaseJsonArray::int_setJson(const String &path, FirebaseJson *json)
{
  int_set(path.c_str(), json->raw(), false);
}

void FirebaseJsonArray::int_setArray(int index, FirebaseJsonArray *arr)
{
  arr->last_error = last_error;
  int_setByIndex(index, arr->raw(), false);
}

void FirebaseJsonArray::int_setArray(const String &path, FirebaseJsonArray *arr)
{
  arr->last_error = last_error;
  int_set(path.c_str(), arr->raw(), false);
}

bool FirebaseJsonArray::remove(int index)
{
  char *tmp = hp->intStr(index);
  std::string path;
  hp->appendS(path, fb_json_str_type_brk3);
  path += tmp;
  hp->appendS(path, fb_json_str_type_brk4);
  bool ret = int_remove(path.c_str());
  hp->clearS(path);
  hp->delS(tmp);
  return ret;
}

bool FirebaseJsonArray::remove(const String &path)
{
  return int_remove(path.c_str());
}

bool FirebaseJsonArray::int_remove(const char *path)
{
  char *root = hp->strP(fb_json_str_21);
  char *root_path = hp->strP(fb_json_str_26);
  char *slash = hp->strP(fb_json_str_27);

  js.raw_buf.insert(0, root);
  hp->appendS(js.raw_buf, fb_json_str_type_brk2);

  std::string json_path = root_path;
  json_path += slash;
  json_path += path;
  js.result.success = js.remove(json_path.c_str());
  hp->clearS(json_path);
  bool success = js.result.success;
  if (js.result.success)
  {
    hp->clearS(js.result.data_raw);
    hp->clearS(js.temp_buf);
    js.int_parse(root_path, FirebaseJson::fb_json_serialize_mode_plain);
    if (js.result.success)
    {
      arr_size = js.result.arr_size;
      hp->storeS(js.raw_buf, js.result.data_raw.c_str(), false);
    }
  }
  else
  {
    js.raw_buf.resize(js.raw_buf.size() - 1);
    js.raw_buf.erase(0, strlen(root));
  }

  if (js.raw_buf.length() == 0)
  {
    js.result.success = success;
    arr_size = 0;
  }

  hp->delS(root);
  hp->delS(root_path);
  hp->delS(slash);
  return js.result.success;
}

FirebaseJsonData::FirebaseJsonData()
{
}

FirebaseJsonData::~FirebaseJsonData()
{
  clear();
}

bool FirebaseJsonData::getArray(FirebaseJsonArray &jsonArray)
{
  if (typeNum != FirebaseJson::JSON_ARRAY || !success)
    return false;
  return getArray(data_raw.c_str(), jsonArray);
}

bool FirebaseJsonData::getArray(const char *source, FirebaseJsonArray &jsonArray)
{
  char *root = strP(fb_json_str_21);
  char *brk2 = strP(fb_json_str_9);
  clearS(jsonArray.js.raw_buf);
  jsonArray.js.raw_buf += root;
  jsonArray.js.raw_buf += source;
  jsonArray.js.raw_buf += brk2;
  delS(root);
  delS(brk2);

  char *root_path = strP(fb_json_str_26);
  clearS(jsonArray.js.result.data_raw);
  clearS(jsonArray.js.temp_buf);

  jsonArray.js.int_parse(root_path, FirebaseJson::fb_json_serialize_mode_plain);
  jsonArray.js.raw_buf = jsonArray.js.result.data_raw;
  jsonArray.arr_size = jsonArray.js.result.arr_size;
  delS(root_path);

  shrinkS(jsonArray.js.raw_buf);
  shrinkS(jsonArray.js.result.data_raw);

  return jsonArray.js.result.success;
}

bool FirebaseJsonData::getJSON(FirebaseJson &json)
{
  if (typeNum != FirebaseJson::JSON_OBJECT || !success)
    return false;
  return getJSON(stringValue.c_str(), json);
}

bool FirebaseJsonData::getJSON(const char *source, FirebaseJson &json)
{
  json.setJsonData(source);
  json.int_fb_json_parseToken();
  shrinkS(json.result.data_raw);
  shrinkS(json.raw_buf);
  return json.result.success;
}

void FirebaseJsonData::setTokenInfo(fb_json_token_t *token, int16_t index, int16_t level, bool success)
{
  this->type_num = token->type;
  this->k_start = token->start;
  this->end = token->end;
  this->token_index = index;
  this->level = level;
  this->arr_size = token->size;
  this->success = success;
}

void FirebaseJsonData::shrinkS(std::string &s)
{
  //#if defined(ESP32)
  //    s.shrink_to_fit();
  //#elif defined(ESP8266)
  std::string t = s;
  clearS(s);
  s = t;
  clearS(t);
  //#endif
}

void FirebaseJsonData::clear()
{
  clearS(data_raw);
  stringValue.remove(0, stringValue.length());
  intValue = 0;
  floatValue = 0;
  doubleValue = 0;
  boolValue = 0;
  type.remove(0, type.length());
  typeNum = 0;
  success = false;
}

void FirebaseJsonData::clearS(std::string &s)
{
  s.clear();
  std::string().swap(s);
}

char *FirebaseJsonData::strP(PGM_P p)
{
  char *t = new char[strlen_P(p) + 1];
  memset(t, 0, strlen_P(p) + 1);
  strcpy_P(t, p);
  return t;
}

void FirebaseJsonData::delS(char *v)
{
  if (v)
    delete[] v;
  v = nullptr;
}

#endif