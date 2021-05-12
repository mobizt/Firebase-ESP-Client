/*
 * FirebaseJson, version 2.3.15
 * 
 * The Easiest Arduino library to parse, create and edit JSON object using a relative path.
 * 
 * May 12, 2021
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
#define JSMN_STRICT

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
  _init();
}

FirebaseJson::FirebaseJson(std::string &data)
{
  _init();
  _setJsonData(data);
}

FirebaseJson::~FirebaseJson()
{
  clear();
  _topLevelTkType = JSMN_OBJECT;
  _parser.reset();
  _parser = nullptr;
  _finalize();
  delete helper;
}

void FirebaseJson::_init()
{
  _finalize();
  _qt = helper->strP(fb_json_str_2);
  _tab = helper->strP(fb_json_str_22);
  _brk1 = helper->strP(fb_json_str_8);
  _brk2 = helper->strP(fb_json_str_9);
  _brk3 = helper->strP(fb_json_str_10);
  _brk4 = helper->strP(fb_json_str_11);
  _cm = helper->strP(fb_json_str_1);
  _pr2 = helper->strP(fb_json_str_3);
  _nl = helper->strP(fb_json_str_24);
  _nll = helper->strP(fb_json_str_18);
  _pr = helper->strP(fb_json_str_25);
  _pd = helper->strP(fb_json_str_4);
  _pf = helper->strP(fb_json_str_5);
  _fls = helper->strP(fb_json_str_6);
  _tr = helper->strP(fb_json_str_7);
  _string = helper->strP(fb_json_str_12);
  _int = helper->strP(fb_json_str_13);
  _dbl = helper->strP(fb_json_str_14);
  _bl = helper->strP(fb_json_str_15);
  _obj = helper->strP(fb_json_str_16);
  _arry = helper->strP(fb_json_str_17);
  _undef = helper->strP(fb_json_str_19);
  _dot = helper->strP(fb_json_str_20);
}

void FirebaseJson::_finalize()
{
  helper->delS(_qt);
  helper->delS(_tab);
  helper->delS(_brk1);
  helper->delS(_brk2);
  helper->delS(_brk3);
  helper->delS(_brk4);
  helper->delS(_cm);
  helper->delS(_pr2);
  helper->delS(_nl);
  helper->delS(_nll);
  helper->delS(_pr);
  helper->delS(_pd);
  helper->delS(_pf);
  helper->delS(_fls);
  helper->delS(_tr);
  helper->delS(_string);
  helper->delS(_int);
  helper->delS(_dbl);
  helper->delS(_bl);
  helper->delS(_obj);
  helper->delS(_arry);
  helper->delS(_undef);
  helper->delS(_dot);
}

FirebaseJson &FirebaseJson::_setJsonData(std::string &data)
{
  return setJsonData(data.c_str());
}

FirebaseJson &FirebaseJson::setJsonData(const String &data)
{
  _topLevelTkType = JSMN_OBJECT;
  if (data.length() > 0)
  {
    int p1 = helper->strpos(data.c_str(), _brk1, 0);
    int p2 = helper->rstrpos(data.c_str(), _brk2, data.length() - 1);
    if (p1 != -1)
      p1 += 1;
    if (p1 != -1 && p2 != -1)
      _rawbuf = data.substring(p1, p2).c_str();
    else
    {
      p1 = helper->strpos(data.c_str(), _brk3, 0);
      p2 = helper->rstrpos(data.c_str(), _brk4, data.length() - 1);
      if (p1 != -1)
        p1 += 1;
      if (p1 != -1 && p2 != -1)
      {
        char *_r = helper->strP(fb_json_str_21);
        _rawbuf = _r;
        _rawbuf += data.c_str();
        helper->delS(_r);
        _topLevelTkType = JSMN_ARRAY;
      }
      else
      {
        _rawbuf = data.c_str();
        _topLevelTkType = JSMN_PRIMITIVE;
      }
    }
  }
  else
    _rawbuf.clear();

  return *this;
}

FirebaseJson &FirebaseJson::clear()
{
  std::string().swap(_rawbuf);
  std::string().swap(_jsonData._dbuf);
  std::string().swap(_tbuf);
  clearPathTk();
  _tokens.reset();
  _tokens = nullptr;
  _topLevelTkType = JSMN_OBJECT;
  return *this;
}

FirebaseJson &FirebaseJson::add(const String &key)
{
  _addNull(key.c_str());
  return *this;
}

FirebaseJson &FirebaseJson::add(const String &key, const String &value)
{
  _addString(key.c_str(), value.c_str());
  return *this;
}
FirebaseJson &FirebaseJson::add(const String &key, const char *value)
{
  _addString(key.c_str(), value);
  return *this;
}
FirebaseJson &FirebaseJson::add(const String &key, int value)
{
  _addInt(key.c_str(), value);
  return *this;
}

FirebaseJson &FirebaseJson::add(const String &key, unsigned short value)
{
  _addInt(key.c_str(), value);
  return *this;
}

FirebaseJson &FirebaseJson::add(const String &key, float value)
{
  _addFloat(key.c_str(), value);
  return *this;
}

FirebaseJson &FirebaseJson::add(const String &key, double value)
{
  _addDouble(key.c_str(), value);
  return *this;
}

FirebaseJson &FirebaseJson::add(const String &key, bool value)
{
  _addBool(key.c_str(), value);
  return *this;
}

FirebaseJson &FirebaseJson::add(const String &key, FirebaseJson &json)
{
  _addJson(key.c_str(), &json);
  return *this;
}

FirebaseJson &FirebaseJson::add(const String &key, FirebaseJsonArray &arr)
{
  arr._lastErr = &_lastErr;
  _addArray(key.c_str(), &arr);
  return *this;
}

template <typename T>
FirebaseJson &FirebaseJson::add(const String &key, T value)
{
  if (std::is_same<T, int>::value)
    _addInt(key, value);
  else if (std::is_same<T, float>::value)
    _addFloat(key, value);
  else if (std::is_same<T, double>::value)
    _addDouble(key, value);
  else if (std::is_same<T, bool>::value)
    _addBool(key, value);
  else if (std::is_same<T, const char *>::value)
    _addString(key, value);
  else if (std::is_same<T, FirebaseJson &>::value)
    _addJson(key, &value);
  else if (std::is_same<T, FirebaseJsonArray &>::value)
    _addArray(key, &value);
  return *this;
}

void FirebaseJson::_addString(const std::string &key, const std::string &value)
{
  helper->clearLastError();
  _add(key.c_str(), value.c_str(), key.length(), value.length(), true, true);
}

void FirebaseJson::_addInt(const std::string &key, int value)
{
  helper->clearLastError();
  char *buf = helper->intStr(value);
  _add(key.c_str(), buf, key.length(), 60, false, true);
  helper->delS(buf);
}

void FirebaseJson::_addFloat(const std::string &key, float value)
{
  helper->clearLastError();
  char *buf = helper->floatStr(value);
  helper->trimDouble(buf);
  _add(key.c_str(), buf, key.length(), 60, false, true);
  helper->delS(buf);
}

void FirebaseJson::_addDouble(const std::string &key, double value)
{
  helper->clearLastError();
  char *buf = helper->doubleStr(value);
  helper->trimDouble(buf);
  _add(key.c_str(), buf, key.length(), 60, false, true);
  helper->delS(buf);
}

void FirebaseJson::_addBool(const std::string &key, bool value)
{
  helper->clearLastError();
  if (value)
    _add(key.c_str(), _tr, key.length(), 6, false, true);
  else
    _add(key.c_str(), _fls, key.length(), 7, false, true);
}

void FirebaseJson::_addNull(const std::string &key)
{
  helper->clearLastError();
  _add(key.c_str(), _nll, key.length(), 6, false, true);
}

void FirebaseJson::_addJson(const std::string &key, FirebaseJson *json)
{
  helper->clearLastError();
  std::string s;
  json->_toStdString(s);
  _add(key.c_str(), s.c_str(), key.length(), s.length(), false, true);
  std::string().swap(s);
}

void FirebaseJson::_addArray(const std::string &key, FirebaseJsonArray *arr)
{
  helper->clearLastError();
  arr->_lastErr = &_lastErr;
  String arrStr;
  arr->toString(arrStr);
  _add(key.c_str(), arrStr.c_str(), key.length(), arrStr.length(), false, true);
}

void FirebaseJson::toString(String &buf, bool prettify)
{
  if (prettify)
    _parse("", PRINT_MODE_PRETTY);
  else
    _parse("", PRINT_MODE_PLAIN);
  buf = _jsonData._dbuf.c_str();
  std::string().swap(_jsonData._dbuf);
}

void FirebaseJson::int_tostr(std::string &s, bool prettify)
{
  _tostr(s, prettify);
}

void FirebaseJson::_tostr(std::string &s, bool prettify)
{
  if (prettify)
    _parse("", PRINT_MODE_PRETTY);
  else
    _parse("", PRINT_MODE_PLAIN);
  s = _jsonData._dbuf;
  std::string().swap(_jsonData._dbuf);
}

void FirebaseJson::int_toStdString(std::string &s, bool isJson)
{
  _toStdString(s, isJson);
}

void FirebaseJson::setBufferLimit(size_t limit)
{
  if (limit >= 32 || limit <= 8192)
    _parser_buff_len = limit;
}

void FirebaseJson::_toStdString(std::string &s, bool isJson)
{
  s.clear();
  size_t bufSize = 20;
  char *buf = helper->newS(bufSize);
  if (_topLevelTkType != JSMN_PRIMITIVE)
  {
    if (isJson)
      strcat(buf, _brk1);
    else
      strcat(buf, _brk3);
  }

  s += buf;
  s += _rawbuf;
  memset(buf, 0, bufSize);
  if (_topLevelTkType != JSMN_PRIMITIVE)
  {
    if (isJson)
      strcat(buf, _brk2);
    else
      strcat(buf, _brk4);
  }
  s += buf;
  helper->delS(buf);
}

FirebaseJson &FirebaseJson::_add(const char *key, const char *value, size_t klen, size_t vlen, bool isString, bool isJson)
{
  helper->clearLastError();
  size_t bufSize = klen + vlen + _parser_buff_len;
  char *buf = helper->newS(bufSize);
  if (!buf)
  {
    helper->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
    return *this;
  }

  if (_rawbuf.length() > 0)
    strcpy_P(buf, fb_json_str_1);
  if (isJson)
  {
    strcat(buf, _qt);
    strcat(buf, key);
    strcat(buf, _qt);
    strcat_P(buf, _pr2);
  }
  if (isString)
    strcat(buf, _qt);
  strcat(buf, value);
  if (isString)
    strcat(buf, _qt);
  _rawbuf += buf;
  helper->delS(buf);
  return *this;
}

FirebaseJson &FirebaseJson::_addArrayStr(const char *value, size_t len, bool isString)
{
  helper->clearLastError();
  _add("", value, 0, len, isString, false);
  return *this;
}

bool FirebaseJson::get(FirebaseJsonData &jsonData, const String &path, bool prettify)
{
  helper->clearLastError();
  clearPathTk();
  _strToTk(path.c_str(), _pathTk, '/');
  std::string().swap(_jsonData._dbuf);
  std::string().swap(_tbuf);
  if (prettify)
    _parse(path.c_str(), PRINT_MODE_PRETTY);
  else
    _parse(path.c_str(), PRINT_MODE_PLAIN);
  if (_jsonData.success)
  {
    if (_jsonData._type == JSMN_STRING && _jsonData._dbuf.c_str()[0] == '"' && _jsonData._dbuf.c_str()[_jsonData._dbuf.length() - 1] == '"')
      _jsonData.stringValue = _jsonData._dbuf.substr(1, _jsonData._dbuf.length() - 2).c_str();
    else
      _jsonData.stringValue = _jsonData._dbuf.c_str();
  }
  jsonData = _jsonData;
  std::string().swap(_jsonData._dbuf);
  std::string().swap(_tbuf);
  clearPathTk();
  _tokens.reset();
  _tokens = nullptr;
  return _jsonData.success;
}

size_t FirebaseJson::iteratorBegin(const char *data)
{
  helper->clearLastError();
  if (data)
    setJsonData(data);
  _fbjs_parse(true);
  std::string s;
  _toStdString(s);
  int bufLen = s.length() + _parser_buff_len;
  char *buf = helper->newS(bufLen);
  if (!buf)
  {
    helper->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
    return 0;
  }
  strcpy(buf, s.c_str());
  std::string().swap(s);
  int depth = -1;
  _parser_info.collectTk = true;
  _eltk.clear();
  for (uint16_t i = 0; i < _parser_info.tokenCount; i++)
    _parseToken(i, buf, depth, "", -2, PRINT_MODE_NONE);
  _el.clear();
  helper->delS(buf);
  return _eltk.size();
}

void FirebaseJson::iteratorEnd()
{
  helper->clearLastError();
  _eltk.clear();
  clearPathTk();
  _jsonData.stringValue = "";
  std::string().swap(_jsonData._dbuf);
  std::string().swap(_tbuf);
  clearPathTk();
  _tokens.reset();
  _tokens = nullptr;
}

void FirebaseJson::iteratorGet(size_t index, int &type, String &key, String &value)
{
  helper->clearLastError();
  if (_eltk.size() < index + 1)
    return;
  std::string s;
  _toStdString(s);
  int bufLen = s.length() + _parser_buff_len;
  char *buf = helper->newS(bufLen);
  if (!buf)
  {
    helper->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
    return;
  }
  strcpy(buf, s.c_str());
  std::string().swap(s);
  if (_eltk[index].type == 0)
  {
    FirebaseJson::fbjs_tok_t *h = &_tokens.get()[_eltk[index].index];
    size_t len = h->end - h->start + 3;
    char *k = helper->newS(len);
    if (!k)
    {
      helper->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
      return;
    }
    strncpy(k, buf + h->start, h->end - h->start);
    FirebaseJson::fbjs_tok_t *g = &_tokens.get()[_eltk[index].index + 1];
    size_t len2 = g->end - g->start + 3;
    char *v = helper->newS(len2);
    if (!v)
    {
      helper->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
      return;
    }
    strncpy(v, buf + g->start, g->end - g->start);
    key = k;
    value = v;
    type = JSON_OBJECT;
    helper->delS(k);
    helper->delS(v);
  }
  else if (_eltk[index].type == 1)
  {
    FirebaseJson::fbjs_tok_t *g = &_tokens.get()[_eltk[index].index];
    size_t len2 = g->end - g->start + 3;
    char *v = helper->newS(len2);
    if (!v)
    {
      helper->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
      return;
    }
    strncpy(v, buf + g->start, g->end - g->start);
    value = v;
    key = "";
    type = JSON_ARRAY;
    helper->delS(v);
  }
  helper->delS(buf);
}

void FirebaseJson::_fbjs_parse(bool collectTk)
{
  helper->clearLastError();
  std::string s;
  _toStdString(s);
  int bufLen = s.length() + _parser_buff_len;
  char *buf = helper->newS(bufLen);
  if (!buf)
  {
    helper->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
    return;
  }
  strcpy(buf, s.c_str());
  std::string().swap(s);
  _tokens.reset();
  _parser_info.collectTk = collectTk;
  _eltk.clear();
  int cnt = fbjs_parse(_parser.get(), buf, bufLen, (FirebaseJson::fbjs_tok_t *)NULL, 0);
  if (cnt < 0)
  {
    /** Invalid character inside JSON string */
    if (cnt == JSMN_ERROR_INVAL)
      helper->setLastError(-3, __FILE__, __LINE__, fb_json_str_29);

    /** The string is not a full JSON packet, more bytes expected */
    else if (cnt == JSMN_ERROR_PART)
      helper->setLastError(-4, __FILE__, __LINE__, fb_json_str_30);
  }

  int cnt2 = 0;
  int a = 0;
  int b = 0;
  for (int i = 0; i < bufLen; i++)
  {
    if (buf[i] == ',')
      a++;
    else if (buf[i] == '[' || buf[i] == '{')
      b++;
  }

  cnt2 = 10 + (2 * (a + 1)) + b;

  if (cnt < cnt2)
    cnt = cnt2;

  _tokens = std::shared_ptr<FirebaseJson::fbjs_tok_t>(new FirebaseJson::fbjs_tok_t[cnt + 10]);
  fbjs_init(_parser.get());
  _parser_info.tokenCount = fbjs_parse(_parser.get(), buf, bufLen, _tokens.get(), cnt + 10);
  if (_parser_info.tokenCount < 0)
  {
    /** Not enough tokens were provided */
    if (_parser_info.tokenCount == JSMN_ERROR_NOMEM)
      helper->setLastError(-2, __FILE__, __LINE__, fb_json_str_31);
  }

  _parser_info.paresRes = true;
  if (_parser_info.tokenCount < 0)
    _parser_info.paresRes = false;
  if (_parser_info.tokenCount < 1 || _tokens.get()[0].type != JSMN_OBJECT)
    _parser_info.paresRes = false;
  _jsonData.success = _parser_info.paresRes;
  _parser_info.nextToken = 0;
  _parser_info.nextDepth = 0;
  _parser_info.tokenMatch = false;
  _parser_info.refToken = -1;
  _resetParseResult();
  _setElementType();
  helper->delS(buf);
}

void FirebaseJson::_setMark(int depth, bool mark)
{
  for (size_t i = 0; i < _el.size(); i++)
  {
    if (_el[i].depth == depth - 1)
    {
      _el[i].omark = mark;
      break;
    }
  }
}

void FirebaseJson::_setSkip(int depth, bool skip)
{
  for (size_t i = 0; i < _el.size(); i++)
  {
    if (_el[i].depth == depth - 1)
    {
      _el[i].skip = skip;
      break;
    }
  }
}

void FirebaseJson::_setRef(int depth, bool ref)
{
  for (size_t i = 0; i < _el.size(); i++)
  {
    if (ref)
    {
      if (_el[i].depth == depth - 1)
      {
        _el[i].ref = ref;
        break;
      }
    }
    else
      _el[i].ref = false;
  }
}

void FirebaseJson::_getTkIndex(int depth, tk_index_t &tk)
{
  tk.oindex = 0;
  tk.olen = 0;
  tk.omark = false;
  tk.type = JSMN_UNDEFINED;
  tk.depth = -1;
  tk.skip = false;
  tk.ref = false;
  tk.index = -1;
  for (size_t i = 0; i < _el.size(); i++)
  {
    if (_el[i].depth == depth - 1)
    {
      tk.index = _el[i].index;
      tk.omark = _el[i].omark;
      tk.ref = _el[i].ref;
      tk.type = _el[i].type;
      tk.depth = _el[i].depth;
      tk.oindex = _el[i].oindex;
      tk.olen = _el[i].olen;
      tk.skip = _el[i].skip;
      break;
    }
  }
}

bool FirebaseJson::_updateTkIndex(uint16_t index, int &depth, const char *searchKey, int searchIndex, const char *replace, PRINT_MODE printMode, bool advanceCount)
{
  int len = -1;
  bool skip = false;
  bool ref = false;
  for (size_t i = 0; i < _el.size(); i++)
  {
    if (_el[i].depth == depth - 1)
    {
      if (_el[i].type == JSMN_OBJECT || _el[i].type == JSMN_ARRAY)
      {
        _el[i].oindex++;
        if (_el[i].oindex >= _el[i].olen)
        {
          depth = _el[i].depth;
          len = _el[i].olen;
          skip = _el[i].skip;

          if (!_parser_info.TkRefOk && _el[i].type == JSMN_OBJECT)
            ref = _el[i].ref;
          else if (!_parser_info.TkRefOk && _el[i].type == JSMN_ARRAY && searchIndex > -1)
            ref = _el[i].ref;

          _el.erase(_el.begin() + i);

          if (printMode != PRINT_MODE_NONE && !skip)
          {
            if (len > 0 && !_parser_info.arrReplaced)
            {
              if (ref)
                _jsonData._dbuf += _cm;
              if (_el[i].type == JSMN_OBJECT)
              {
                if (printMode == PRINT_MODE_PRETTY)
                  _jsonData._dbuf += _nl;
                if (printMode == PRINT_MODE_PRETTY && !ref)
                {
                  for (int j = 0; j < depth + 1; j++)
                    _jsonData._dbuf += _tab;
                }
              }
            }
            if (ref)
            {
              if (!advanceCount)
                _parser_info.parseCompleted++;

              if (!_parser_info.arrReplaced)
              {
                if (_el[i].type == JSMN_OBJECT)
                {
                  if (printMode == PRINT_MODE_PRETTY)
                  {
                    for (int j = 0; j < depth + 2; j++)
                      _jsonData._dbuf += _tab;
                  }
                  _jsonData._dbuf += _qt;
                  _jsonData._dbuf += searchKey;
                  _jsonData._dbuf += _qt;
                  if (printMode == PRINT_MODE_PRETTY)
                    _jsonData._dbuf += _pr;
                  else
                    _jsonData._dbuf += _pr2;
                  if (_parser_info.parseCompleted == (int)_pathTk.size())
                    _jsonData._dbuf += replace;
                  else
                    _insertChilds(replace, printMode);
                  _parser_info.arrReplaced = true;
                  if (printMode == PRINT_MODE_PRETTY)
                  {
                    _jsonData._dbuf += _nl;
                    for (int j = 0; j < depth + 1; j++)
                      _jsonData._dbuf += _tab;
                  }
                }
                else
                {
                  for (int k = _el[i].oindex - 1; k < searchIndex; k++)
                  {
                    if (printMode == PRINT_MODE_PRETTY)
                    {
                      _jsonData._dbuf += _nl;
                      for (int j = 0; j < depth + 2; j++)
                        _jsonData._dbuf += _tab;
                    }
                    if (k == searchIndex - 1)
                    {
                      if (_parser_info.parseCompleted == (int)_pathTk.size())
                        _jsonData._dbuf += replace;
                      else
                        _insertChilds(replace, printMode);
                      _parser_info.arrReplaced = true;
                    }
                    else
                    {
                      _jsonData._dbuf += _nll;
                      _jsonData._dbuf += _cm;
                    }
                  }
                }
              }
              _setRef(depth, false);
              if (!advanceCount)
                _parser_info.parseCompleted = _pathTk.size();
            }

            if (_el[i].type == JSMN_OBJECT)
              _jsonData._dbuf += _brk2;
            else
            {
              if (len > 0)
              {
                if (printMode == PRINT_MODE_PRETTY)
                {
                  _jsonData._dbuf += _nl;
                  for (int j = 0; j < depth + 1; j++)
                    _jsonData._dbuf += _tab;
                }
              }
              _jsonData._dbuf += _brk4;
            }
          }
          return true;
        }
      }
      break;
    }
  }
  return false;
}

bool FirebaseJson::_updateTkIndex2(std::string &str, uint16_t index, int &depth, const char *searchKey, int searchIndex, const char *replace, PRINT_MODE printMode)
{
  int len = -1;
  bool skip = false;
  bool ref = false;
  for (size_t i = 0; i < _el.size(); i++)
  {
    if (_el[i].depth == depth - 1)
    {
      if (_el[i].type == JSMN_OBJECT || _el[i].type == JSMN_ARRAY)
      {
        _el[i].oindex++;
        if (_el[i].oindex >= _el[i].olen)
        {
          depth = _el[i].depth;
          len = _el[i].olen;
          skip = _el[i].skip;
          if (!_parser_info.TkRefOk && _el[i].type == JSMN_OBJECT)
            ref = _el[i].ref;
          else if (!_parser_info.TkRefOk && _el[i].type == JSMN_ARRAY && searchIndex > -1)
            ref = _el[i].ref;

          _el.erase(_el.begin() + i);

          if (printMode != PRINT_MODE_NONE && !skip)
          {
            if (len > 0)
            {
              if (printMode == PRINT_MODE_PRETTY)
                str += _nl;
              if (_el[i].type == JSMN_OBJECT)
              {
                if (printMode == PRINT_MODE_PRETTY && !ref)
                {
                  for (int j = 0; j < depth + 1; j++)
                    str += _tab;
                }
              }
              else
              {
                if (printMode == PRINT_MODE_PRETTY)
                {
                  for (int j = 0; j < depth + 1; j++)
                    str += _tab;
                }
              }
            }
            if (ref)
              _setRef(depth, false);
            if (_el[i].type == JSMN_OBJECT)
              str += _brk2;
            else
              str += _brk4;
          }
          return true;
        }
      }
      break;
    }
  }
  return false;
}

bool FirebaseJson::_updateTkIndex3(uint16_t index, int &depth, const char *searchKey, int searchIndex, PRINT_MODE printMode)
{
  int len = -1;
  bool skip = false;
  bool ref = false;
  for (size_t i = 0; i < _el.size(); i++)
  {
    if (_el[i].depth == depth - 1)
    {
      if (_el[i].type == JSMN_OBJECT || _el[i].type == JSMN_ARRAY)
      {
        _el[i].oindex++;
        if (_el[i].oindex >= _el[i].olen)
        {
          depth = _el[i].depth;
          len = _el[i].olen;
          skip = _el[i].skip;
          if (!_parser_info.TkRefOk && _el[i].type == JSMN_OBJECT)
            ref = _el[i].ref;
          else if (!_parser_info.TkRefOk && _el[i].type == JSMN_ARRAY && searchIndex > -1)
            ref = _el[i].ref;

          _el.erase(_el.begin() + i);

          if (depth < _parser_info.skipDepth)
            return false;
          if (printMode != PRINT_MODE_NONE && skip)
          {
            if (len > 0)
            {
              if (printMode == PRINT_MODE_PRETTY)
                _jsonData._dbuf += _nl;
              if (_el[i].type == JSMN_OBJECT)
              {
                if (printMode == PRINT_MODE_PRETTY && !ref)
                {
                  for (int j = 0; j < depth + 1 - (_parser_info.skipDepth + 1); j++)
                    _jsonData._dbuf += _tab;
                }
              }
              else
              {
                if (printMode == PRINT_MODE_PRETTY)
                {
                  for (int j = 0; j < depth + 1 - (_parser_info.skipDepth + 1); j++)
                    _jsonData._dbuf += _tab;
                }
              }
            }
            if (ref)
              _setRef(depth, false);

            if (_el[i].type == JSMN_OBJECT)
              _jsonData._dbuf += _brk2;
            else
              _jsonData._dbuf += _brk4;
          }
          return true;
        }
      }
      break;
    }
  }
  return false;
}

void FirebaseJson::_insertChilds(const char *data, PRINT_MODE printMode)
{
  std::string str = "";
  for (int i = _pathTk.size() - 1; i > _parser_info.parseCompleted - 1; i--)
  {
    if (_isArrTk(i))
    {
      std::string _str;
      _addArrNodes(_str, str, i, data, printMode);
      str = _str;
      std::string().swap(_str);
    }
    else
    {
      std::string _str;
      _addObjNodes(_str, str, i, data, printMode);
      str = _str;
      std::string().swap(_str);
    }
  }
  if ((int)_pathTk.size() == _parser_info.parseCompleted)
    str = data;
  _jsonData._dbuf += str;
  std::string().swap(str);
}

void FirebaseJson::_addArrNodes(std::string &str, std::string &str2, int index, const char *data, PRINT_MODE printMode)
{

  int i = _getArrIndex(index);
  str += _brk3;
  if (printMode == PRINT_MODE_PRETTY)
    str += _nl;
  for (int k = 0; k <= i; k++)
  {
    if (printMode == PRINT_MODE_PRETTY)
    {
      for (int j = 0; j < index + 1; j++)
        str += _tab;
    }
    if (k == i)
    {
      if (index == (int)_pathTk.size() - 1)
        str += data;
      else
        str += str2;
    }
    else
    {
      str += _nll;
      str += _cm;
    }

    if (printMode == PRINT_MODE_PRETTY)
      str += _nl;
  }

  if (printMode == PRINT_MODE_PRETTY)
  {
    for (int j = 0; j < index; j++)
      str += _tab;
  }
  str += _brk4;
}

void FirebaseJson::_addObjNodes(std::string &str, std::string &str2, int index, const char *data, PRINT_MODE printMode)
{
  str += _brk1;
  if (printMode == PRINT_MODE_PRETTY)
  {
    str += _nl;
    for (int j = 0; j < index + 1; j++)
      str += _tab;
  }
  str += _qt;
  str += _pathTk[index].tk.c_str();
  str += _qt;
  if (printMode == PRINT_MODE_PRETTY)
    str += _pr;
  else
    str += _pr2;
  if (index == (int)_pathTk.size() - 1)
    str += data;
  else
    str += str2;
  if (printMode == PRINT_MODE_PRETTY)
  {
    str += _nl;
    for (int j = 0; j < index; j++)
      str += _tab;
  }
  str += _brk2;
}

void FirebaseJson::_parseToken(uint16_t &i, char *buf, int &depth, const char *searchKey, int searchIndex, PRINT_MODE printMode)
{
  helper->clearLastError();
  tk_index_t tk;
  _getTkIndex(depth, tk);
  FirebaseJson::fbjs_tok_t *h = &_tokens.get()[i];
  bool oskip = false;
  bool ex = false;
  size_t resLen = _jsonData._dbuf.length();
  if (searchIndex == -2)
    tk.skip = true;
  delay(0);
  if (searchIndex > -1)
  {
    tk_index_t tk2;
    int depth2 = depth - 1;
    _getTkIndex(depth2, tk2);
    if (tk.type == JSMN_ARRAY && _parser_info.parseDepth == depth && tk2.oindex == _parser_info.parentIndex)
    {
      if (tk.oindex == searchIndex)
      {
        _parser_info.nextToken = i;
        _parser_info.nextDepth = depth;
        _parser_info.parentIndex = tk.oindex;

        if ((int)_pathTk.size() != _parser_info.parseDepth + 1)
        {
          _parser_info.tokenMatch = true;
          _parser_info.parseCompleted++;
        }
        else
        {
          if (!_parser_info.TkRefOk)
          {
            _parser_info.parseCompleted++;
            _parser_info.refTkIndex = i + 1;
            _parser_info.refToken = i + 1;
            _parser_info.TkRefOk = true;
            char *dat1 = helper->newS(h->end - h->start + 10);
            if (!dat1)
            {
              helper->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
              return;
            }
            strncpy(dat1, buf + h->start, h->end - h->start);
            _jsonData.stringValue = dat1;
            helper->delS(dat1);
            _jsonData._type = h->type;
            _jsonData._k_start = h->start;
            _jsonData._start = h->start;
            _jsonData._end = h->end;
            _jsonData._tokenIndex = i;
            _jsonData._depth = depth;
            _jsonData._len = h->size;
            _jsonData.success = true;
            _setElementType();
            if (printMode != PRINT_MODE_NONE)
              _jsonData.stringValue = "";
            else
            {
              std::string().swap(_jsonData._dbuf);
              std::string().swap(_tbuf);
              _parser_info.tokenMatch = true;
              ex = true;
            }
          }
        }
      }
      else
      {
        if (tk.oindex + 1 == tk.olen)
        {
          _setRef(depth - 1, false);
          _setRef(depth, true);
        }
      }
    }
  }
  else
  {
    char *key = helper->newS(h->end - h->start + 10);
    if (!key)
    {
      helper->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
      return;
    }
    strncpy(key, buf + h->start, h->end - h->start);
    if (tk.type != JSMN_UNDEFINED && _parser_info.parseDepth == depth)
    {
      if (strcmp(searchKey, key) == 0)
      {
        _parser_info.nextToken = i + 1;
        _parser_info.nextDepth = depth;
        _parser_info.parentIndex = tk.oindex;
        if ((int)_pathTk.size() != _parser_info.parseDepth + 1)
        {
          _parser_info.tokenMatch = true;
          _parser_info.parseCompleted++;
        }
        else
        {
          if (!_parser_info.TkRefOk)
          {
            _parser_info.parseCompleted++;
            _parser_info.refTkIndex = i + 1;
            _parser_info.refToken = i + 1;
            _parser_info.TkRefOk = true;
            h = &_tokens.get()[i + 1];
            char *dat2 = helper->newS(h->end - h->start + 10);
            if (!dat2)
            {
              helper->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
              return;
            }
            strncpy(dat2, buf + h->start, h->end - h->start);
            _jsonData.stringValue = dat2;
            helper->delS(dat2);
            _jsonData._type = h->type;
            _jsonData._k_start = h->start;
            _jsonData._start = h->start;
            _jsonData._end = h->end;
            _jsonData._tokenIndex = i;
            _jsonData._depth = depth;
            _jsonData._len = h->size;
            _jsonData.success = true;
            _setElementType();
            if (printMode != PRINT_MODE_NONE)
              _jsonData.stringValue = "";
            else
            {
              std::string().swap(_jsonData._dbuf);
              std::string().swap(_tbuf);
              _parser_info.tokenMatch = true;
              ex = true;
            }
          }
        }
      }
      else
      {
        if (tk.oindex + 1 == tk.olen)
        {
          _setRef(depth - 1, false);
          _setRef(depth, true);
        }
      }
    }
    helper->delS(key);
  }
  if (ex)
    return;
  if (_parser_info.refTkIndex == i + 1)
  {
    if (tk.type == JSMN_OBJECT)
      oskip = true;
    tk.skip = true;
    _parser_info.skipDepth = depth;
  }
  h = &_tokens.get()[i];
  if (h->type == JSMN_OBJECT || h->type == JSMN_ARRAY)
  {
    if (printMode != PRINT_MODE_NONE && (tk.skip || _parser_info.refTkIndex == i + 1))
    {
      if (!tk.omark && i > 0 && resLen > 0)
      {
        if (tk.oindex > 0)
          _jsonData._dbuf += _cm;
        if (printMode == PRINT_MODE_PRETTY && h->size >= 0)
          _jsonData._dbuf += _nl;
        if (printMode == PRINT_MODE_PRETTY && h->size >= 0)
        {
          for (int j = 0; j < depth - (_parser_info.skipDepth + 1); j++)
            _jsonData._dbuf += _tab;
          _jsonData._dbuf += _tab;
        }
      }
      if (h->type == JSMN_OBJECT)
        _jsonData._dbuf += _brk1;
      else
        _jsonData._dbuf += _brk3;
    }
    el_t e;
    e.index = i;
    e.olen = h->size;
    e.type = h->type;
    e.oindex = 0;
    e.depth = depth;
    e.omark = false;
    e.ref = false;
    if (_parser_info.refToken != -1)
      e.skip = true;
    else
      e.skip = tk.skip;
    _el.push_back(e);
    depth++;
    if (h->size == 0)
    {
      while (_updateTkIndex3(i, depth, searchKey, searchIndex, printMode))
      {
        delay(0);
      }
    }
  }
  else
  {
    char *tmp = helper->newS(h->end - h->start + 10);
    if (!tmp)
    {
      helper->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
      return;
    }
    if (buf[h->start - 1] != '"')
      strncpy(tmp, buf + h->start, h->end - h->start);
    else
      strncpy(tmp, buf + h->start - 1, h->end - h->start + 2);
    if (h->size > 0)
    {
      if (printMode != PRINT_MODE_NONE && tk.skip && !oskip)
      {
        if (tk.oindex > 0)
          _jsonData._dbuf += _cm;
        if (printMode == PRINT_MODE_PRETTY)
          _jsonData._dbuf += _nl;
        if (printMode == PRINT_MODE_PRETTY && h->size > 0)
        {
          for (int j = 0; j < depth - (_parser_info.skipDepth + 1); j++)
            _jsonData._dbuf += _tab;
          _jsonData._dbuf += _tab;
        }
        _jsonData._dbuf += tmp;
        if (printMode == PRINT_MODE_PRETTY)
          _jsonData._dbuf += _pr;
        else
          _jsonData._dbuf += _pr2;
      }
      if (_parser_info.collectTk)
      {
        eltk_t el;
        el.index = i;
        el.type = 0;
        _eltk.push_back(el);
      }
      char *tmp2 = helper->newS(h->end - h->start + 10);
      if (!tmp2)
      {
        helper->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
        return;
      }
      strncpy(tmp2, buf + h->start, h->end - h->start);
      h = &_tokens.get()[i + 1];
      helper->delS(tmp2);

      if (h->type != JSMN_OBJECT && h->type != JSMN_ARRAY)
      {

        char *tmp2 = helper->newS(h->end - h->start + 10);
        if (!tmp2)
        {
          helper->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
          return;
        }
        strncpy(tmp2, buf + h->start, h->end - h->start);
        if (printMode != PRINT_MODE_NONE && tk.skip)
        {
          if (buf[h->start - 1] != '"')
            strncpy(tmp2, buf + h->start, h->end - h->start);
          else
            strncpy(tmp2, buf + h->start - 1, h->end - h->start + 2);
          _jsonData._dbuf += tmp2;
        }
        helper->delS(tmp2);
        i++;
        while (_updateTkIndex3(i, depth, searchKey, searchIndex, printMode))
        {
          delay(0);
        }
      }
      else
      {
        if (_parser_info.refToken == i + 1)
        {
          _setSkip(depth, true);
        }
        _setMark(depth, true);
      }
    }
    else
    {
      if (printMode != PRINT_MODE_NONE && tk.skip)
      {
        if (tk.oindex > 0 && resLen > 0)
        {
          _jsonData._dbuf += _cm;
        }
        if (printMode == PRINT_MODE_PRETTY && resLen > 0)
          _jsonData._dbuf += _nl;

        if (printMode == PRINT_MODE_PRETTY && tk.olen > 0 && resLen > 0)
        {
          for (int j = 0; j < depth - (_parser_info.skipDepth + 1); j++)
            _jsonData._dbuf += _tab;
          _jsonData._dbuf += _tab;
        }
        _jsonData._dbuf += tmp;
      }
      while (_updateTkIndex3(i, depth, searchKey, searchIndex, printMode))
      {
        delay(0);
      }
      if (_parser_info.collectTk)
      {
        eltk_t el;
        el.index = i;
        el.type = 1;
        _eltk.push_back(el);
      }
    }
    helper->delS(tmp);

    if (_parser_info.refToken == -1 && _parser_info.skipDepth == depth)
      _setSkip(depth, false);
  }
  _parser_info.nextToken = i + 1;
  _parser_info.refToken = -1;
}

void FirebaseJson::_compileToken(uint16_t &i, char *buf, int &depth, const char *searchKey, int searchIndex, PRINT_MODE printMode, const char *replace, int refTokenIndex, bool removeTk)
{
  helper->clearLastError();
  if (_parser_info.tokenMatch)
    return;
  tk_index_t tk;
  _getTkIndex(depth, tk);
  FirebaseJson::fbjs_tok_t *h = &_tokens.get()[i];
  bool insertFlag = false;
  bool ex = false;
  delay(0);
  if (searchIndex > -1)
  {
    tk_index_t tk2;
    int depth2 = depth - 1;
    _getTkIndex(depth2, tk2);
    if (tk.type == JSMN_ARRAY && _parser_info.parseDepth == depth && tk2.oindex == _parser_info.parentIndex)
    {
      if (tk.oindex == searchIndex)
      {
        _parser_info.nextToken = i;
        _parser_info.nextDepth = depth;
        _parser_info.parentIndex = tk.oindex;
        if ((int)_pathTk.size() != _parser_info.parseDepth + 1)
        {
          _parser_info.tokenMatch = true;
          _parser_info.parseCompleted++;
          _parser_info.refTkIndex = i + 1;
        }
        else
        {
          if (!_parser_info.TkRefOk)
          {
            _parser_info.parseCompleted++;
            _parser_info.refTkIndex = i + 1;
            _parser_info.refToken = i + 1;
            _parser_info.TkRefOk = true;
            single_child_parent_t p = _findSCParent(depth);
            if (p.success)
            {
              _parser_info.remTkIndex = p.index + 1;
              _parser_info.remFirstTk = p.firstTk;
              _parser_info.remLastTk = p.lastTk;
            }
            else
            {
              _parser_info.remTkIndex = i + 1;
              _parser_info.remFirstTk = tk.oindex == 0;
              _parser_info.remLastTk = tk.oindex + 1 == tk.olen;
            }
          }
        }
      }
      else
      {
        if (tk.oindex + 1 == tk.olen)
        {
          _setRef(depth - 1, false);
          _setRef(depth, true);
        }
      }
    }
  }
  else
  {
    char *key = helper->newS(h->end - h->start + 10);
    if (!key)
    {
      helper->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
      return;
    }
    strncpy(key, buf + h->start, h->end - h->start);
    if (tk.type != JSMN_UNDEFINED && _parser_info.parseDepth == depth)
    {
      if (strcmp(searchKey, key) == 0)
      {
        _parser_info.nextToken = i + 1;
        _parser_info.nextDepth = depth;
        _parser_info.parentIndex = tk.oindex;
        if ((int)_pathTk.size() != _parser_info.parseDepth + 1)
        {
          _parser_info.tokenMatch = true;
          _parser_info.parseCompleted++;
          _parser_info.refTkIndex = i + 1;
        }
        else
        {
          if (!_parser_info.TkRefOk)
          {
            _parser_info.parseCompleted++;
            _parser_info.refTkIndex = i + 1;
            _parser_info.refToken = i + 1;
            _parser_info.TkRefOk = true;
            single_child_parent_t p = _findSCParent(depth);
            if (p.success)
            {
              _parser_info.remTkIndex = p.index + 1;
              _parser_info.remFirstTk = p.firstTk;
              _parser_info.remLastTk = p.lastTk;
            }
            else
            {
              _parser_info.remTkIndex = i + 1;
              _parser_info.remFirstTk = tk.oindex == 0;
              _parser_info.remLastTk = tk.oindex + 1 == tk.olen;
            }
          }
        }
      }
      else
      {
        if (tk.oindex + 1 == tk.olen)
        {
          _setRef(depth - 1, false);
          _setRef(depth, true);
        }
      }
    }
    else
    {
      if (_parser_info.tokenCount == 1 && h->size == 0 && !removeTk)
      {
        _insertChilds(replace, printMode);
        _parser_info.nextToken = i + 1;
        _parser_info.nextDepth = 0;
        _parser_info.parseCompleted = _pathTk.size();
        _parser_info.tokenMatch = true;
        ex = true;
      }
    }
    helper->delS(key);
  }
  if (ex)
    return;

  h = &_tokens.get()[i];
  if (h->type == JSMN_OBJECT || h->type == JSMN_ARRAY)
  {
    if (printMode != PRINT_MODE_NONE && !tk.skip)
    {
      if (!tk.omark && i > 0)
      {
        if (tk.oindex > 0)
          _jsonData._dbuf += _cm;
        if (printMode == PRINT_MODE_PRETTY && h->size >= 0)
          _jsonData._dbuf += _nl;
        if (printMode == PRINT_MODE_PRETTY && h->size >= 0)
        {
          for (int j = 0; j < depth; j++)
            _jsonData._dbuf += _tab;
          _jsonData._dbuf += _tab;
        }
      }
      if (_parser_info.refToken == -1)
      {
        if (h->type == JSMN_OBJECT)
          _jsonData._dbuf += _brk1;
        else
          _jsonData._dbuf += _brk3;
      }
      else if (_parser_info.refToken != -1 && searchIndex > -1)
        _jsonData._dbuf += replace;
    }
    el_t e;
    e.index = i;
    e.olen = h->size;
    e.type = h->type;
    e.oindex = 0;
    e.depth = depth;
    e.omark = false;
    e.ref = false;
    if (_parser_info.refToken != -1)
      e.skip = true;
    else
      e.skip = tk.skip;
    _el.push_back(e);
    depth++;
    if (h->size == 0)
    {
      while (_updateTkIndex(i, depth, searchKey, searchIndex, replace, printMode, removeTk))
      {
        delay(0);
      }
    }
  }
  else
  {
    if (_parser_info.refTkIndex == refTokenIndex && refTokenIndex > -1)
    {
      _parser_info.refToken = refTokenIndex;
      _parser_info.refTkIndex = -1;
      insertFlag = true;
    }
    char *tmp = helper->newS(h->end - h->start + 10);
    if (!tmp)
    {
      helper->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
      return;
    }
    if (buf[h->start - 1] != '"')
      strncpy(tmp, buf + h->start, h->end - h->start);
    else
      strncpy(tmp, buf + h->start - 1, h->end - h->start + 2);
    if (h->size > 0)
    {
      if (printMode != PRINT_MODE_NONE && !tk.skip)
      {
        if (tk.oindex > 0)
          _jsonData._dbuf += _cm;
        if (printMode == PRINT_MODE_PRETTY)
          _jsonData._dbuf += _nl;
        if (printMode == PRINT_MODE_PRETTY && h->size > 0)
        {
          for (int j = 0; j < depth; j++)
            _jsonData._dbuf += _tab;
          _jsonData._dbuf += _tab;
        }
        _jsonData._dbuf += tmp;
        if (printMode == PRINT_MODE_PRETTY)
          _jsonData._dbuf += _pr;
        else
          _jsonData._dbuf += _pr2;
      }
      char *tmp2 = helper->newS(h->end - h->start + 10);
      if (!tmp2)
      {
        helper->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
        return;
      }
      strncpy(tmp2, buf + h->start, h->end - h->start);
      h = &_tokens.get()[i + 1];
      helper->delS(tmp2);

      if (h->type != JSMN_OBJECT && h->type != JSMN_ARRAY)
      {
        char *tmp2 = helper->newS(h->end - h->start + 10);
        if (!tmp2)
        {
          helper->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
          return;
        }
        strncpy(tmp2, buf + h->start, h->end - h->start);

        if (printMode != PRINT_MODE_NONE && !tk.skip)
        {
          if (buf[h->start - 1] != '"')
            strncpy(tmp2, buf + h->start, h->end - h->start);
          else
            strncpy(tmp2, buf + h->start - 1, h->end - h->start + 2);
          if (_parser_info.refToken == i + 1)
          {
            if (!insertFlag)
              _jsonData._dbuf += replace;
            else
              _insertChilds(replace, printMode);
          }
          else
            _jsonData._dbuf += tmp2;
        }
        helper->delS(tmp2);
        i++;
        while (_updateTkIndex(i, depth, searchKey, searchIndex, replace, printMode, removeTk))
        {
          delay(0);
        }
      }
      else
      {
        if (_parser_info.refToken == i + 1)
        {
          _setSkip(depth, true);
          _parser_info.skipDepth = depth;
          if (!insertFlag)
            _jsonData._dbuf += replace;
          else
            _insertChilds(replace, printMode);
          if (printMode != PRINT_MODE_NONE && (depth > 0 || tk.oindex == tk.olen - 1))
          {
            if (printMode == PRINT_MODE_PRETTY)
              _jsonData._dbuf += _nl;
            if (printMode == PRINT_MODE_PRETTY)
            {
              for (int j = 0; j < depth; j++)
                _jsonData._dbuf += _tab;
            }
            _jsonData._dbuf += _brk2;
          }
        }
        _setMark(depth, true);
      }
    }
    else
    {
      if (printMode != PRINT_MODE_NONE && !tk.skip)
      {
        if (tk.oindex > 0)
          _jsonData._dbuf += _cm;
        if (printMode == PRINT_MODE_PRETTY)
          _jsonData._dbuf += _nl;
        if (printMode == PRINT_MODE_PRETTY && tk.olen > 0)
        {
          for (int j = 0; j < depth; j++)
            _jsonData._dbuf += _tab;
          _jsonData._dbuf += _tab;
        }

        if (_parser_info.refToken == i + 1 && !_parser_info.arrInserted)
        {
          if (!insertFlag)
            _jsonData._dbuf += replace;
          else
            _insertChilds(replace, printMode);
          _parser_info.arrInserted = true;
        }
        else
          _jsonData._dbuf += tmp;
      }
      while (_updateTkIndex(i, depth, searchKey, searchIndex, replace, printMode, removeTk))
      {
        delay(0);
      }
    }
    helper->delS(tmp);

    if (_parser_info.refToken == -1 && _parser_info.skipDepth == depth)
      _setSkip(depth, false);
  }
  _parser_info.nextToken = i + 1;
  _parser_info.refToken = -1;
}

void FirebaseJson::_removeToken(uint16_t &i, char *buf, int &depth, const char *searchKey, int searchIndex, PRINT_MODE printMode, const char *replace, int refTokenIndex, bool removeTk)
{
  helper->clearLastError();
  bool ncm = false;
  tk_index_t tk;
  _getTkIndex(depth, tk);
  FirebaseJson::fbjs_tok_t *h = &_tokens.get()[i];
  delay(0);
  if (refTokenIndex == i && refTokenIndex > -1)
    ncm = _parser_info.remFirstTk;
  if (refTokenIndex != i || (refTokenIndex == i && _parser_info.remLastTk))
    _jsonData._dbuf += _tbuf;
  _tbuf.clear();
  bool flag = tk.oindex > 0 && !ncm && _jsonData._dbuf.c_str()[_jsonData._dbuf.length() - 1] != '{' && _jsonData._dbuf.c_str()[_jsonData._dbuf.length() - 1] != '[';
  if (refTokenIndex == i + 1 && refTokenIndex > -1)
  {
    _parser_info.refToken = refTokenIndex;
    _parser_info.refTkIndex = -1;
    tk.skip = true;
  }
  h = &_tokens.get()[i];
  if (h->type == JSMN_OBJECT || h->type == JSMN_ARRAY)
  {
    if (printMode != PRINT_MODE_NONE && !tk.skip)
    {
      if (!tk.omark && i > 0)
      {
        if (flag)
          _tbuf += _cm;
        if (printMode == PRINT_MODE_PRETTY && h->size >= 0)
          _tbuf += _nl;
        if (printMode == PRINT_MODE_PRETTY && h->size >= 0)
        {
          for (int j = 0; j < depth; j++)
            _tbuf += _tab;
          _tbuf += _tab;
        }
      }
      if (_parser_info.refToken == -1)
      {
        if (h->type == JSMN_OBJECT)
          _tbuf += _brk1;
        else
          _tbuf += _brk3;
      }
      else if (_parser_info.refToken != -1 && searchIndex > -1)
        _tbuf += replace;
    }
    el_t e;
    e.index = i;
    e.olen = h->size;
    e.type = h->type;
    e.oindex = 0;
    e.depth = depth;
    e.omark = false;
    e.ref = false;
    if (_parser_info.refToken != -1)
      e.skip = true;
    else
      e.skip = tk.skip;
    _el.push_back(e);
    depth++;
    if (h->size == 0)
    {
      while (_updateTkIndex2(_tbuf, i, depth, searchKey, searchIndex, replace, printMode))
      {
        delay(0);
      }
    }
  }
  else
  {
    char *tmp = helper->newS(h->end - h->start + 10);
    if (!tmp)
    {
      helper->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
      return;
    }
    if (buf[h->start - 1] != '"')
      strncpy(tmp, buf + h->start, h->end - h->start);
    else
      strncpy(tmp, buf + h->start - 1, h->end - h->start + 2);
    if (h->size > 0)
    {
      if (printMode != PRINT_MODE_NONE && !tk.skip)
      {
        if (flag)
          _tbuf += _cm;
        if (printMode == PRINT_MODE_PRETTY)
          _tbuf += _nl;
        if (printMode == PRINT_MODE_PRETTY && h->size > 0)
        {
          for (int j = 0; j < depth; j++)
            _tbuf += _tab;
          _tbuf += _tab;
        }
        _tbuf += tmp;
        if (printMode == PRINT_MODE_PRETTY)
          _tbuf += _pr;
        else
          _tbuf += _pr2;
      }

      char *tmp2 = helper->newS(h->end - h->start + 10);
      if (!tmp2)
      {
        helper->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
        return;
      }
      strncpy(tmp2, buf + h->start, h->end - h->start);
      h = &_tokens.get()[i + 1];
      helper->delS(tmp2);
      if (h->type != JSMN_OBJECT && h->type != JSMN_ARRAY)
      {
        char *tmp2 = helper->newS(h->end - h->start + 10);
        if (!tmp2)
        {
          helper->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
          return;
        }
        strncpy(tmp2, buf + h->start, h->end - h->start);
        if (printMode != PRINT_MODE_NONE && !tk.skip)
        {
          if (buf[h->start - 1] != '"')
            strncpy(tmp2, buf + h->start, h->end - h->start);
          else
            strncpy(tmp2, buf + h->start - 1, h->end - h->start + 2);
          _tbuf += tmp2;
        }
        helper->delS(tmp2);
        i++;
        while (_updateTkIndex2(_tbuf, i, depth, searchKey, searchIndex, replace, printMode))
        {
          delay(0);
        }
      }
      else
      {
        if (_parser_info.refToken == i + 1)
        {
          _setSkip(depth, true);
          _parser_info.skipDepth = depth;
          _tbuf += replace;
          if (printMode != PRINT_MODE_NONE && (depth > 0 || tk.oindex == tk.olen - 1))
          {
            if (printMode == PRINT_MODE_PRETTY)
              _tbuf += _nl;
            if (printMode == PRINT_MODE_PRETTY)
            {
              for (int j = 0; j < depth; j++)
                _tbuf += _tab;
            }
            _tbuf += _brk2;
          }
        }
        _setMark(depth, true);
      }
    }
    else
    {
      if (printMode != PRINT_MODE_NONE && !tk.skip)
      {
        if (flag)
          _tbuf += _cm;
        if (printMode == PRINT_MODE_PRETTY)
          _tbuf += _nl;
        if (printMode == PRINT_MODE_PRETTY && tk.olen > 0)
        {
          for (int j = 0; j < depth; j++)
            _tbuf += _tab;
          _tbuf += _tab;
        }
        _tbuf += tmp;
      }
      while (_updateTkIndex2(_tbuf, i, depth, searchKey, searchIndex, replace, printMode))
      {
        delay(0);
      }
    }
    helper->delS(tmp);

    if (_parser_info.refToken == -1 && _parser_info.skipDepth == depth)
      _setSkip(depth, false);
  }
  _parser_info.nextToken = i + 1;
  _parser_info.refToken = -1;
  _lastTk.olen = tk.olen;
  _lastTk.oindex = tk.oindex;
  _lastTk.type = tk.type;
  _lastTk.depth = tk.depth;
  _lastTk.index = tk.index;
  _lastTk.skip = tk.skip;
}

FirebaseJson::single_child_parent_t FirebaseJson::_findSCParent(int depth)
{
  single_child_parent_t res;
  res.index = -1;
  res.firstTk = false;
  res.lastTk = false;
  res.success = false;
  for (int i = depth; i >= 0; i--)
  {
    bool match = false;
    for (size_t j = 0; j < _el.size(); j++)
    {
      if (_el[j].depth == i - 1 && _el[i].olen == 1)
      {
        match = true;
        res.index = _el[i].index;
        res.firstTk = _el[j].oindex == 0;
        res.lastTk = _el[j].oindex + 1 == _el[j].olen;
        res.success = true;
      }
    }
    if (!match)
      break;
  }
  return res;
}

void FirebaseJson::_get(const char *key, int depth, int index)
{
  helper->clearLastError();
  _parser_info.tokenMatch = false;
  if (_parser_info.paresRes)
  {
    std::string s;
    _toStdString(s);
    int bufLen = s.length() + _parser_buff_len;
    char *buf = helper->newS(bufLen);
    if (!buf)
    {
      helper->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
      return;
    }
    strcpy(buf, s.c_str());
    std::string().swap(s);

    if (_jsonData.success)
    {
      _jsonData._dbuf.clear();
      _parser_info.parseDepth = depth;
      if (_parser_info.nextToken < 0)
        _parser_info.nextToken = 0;
      for (uint16_t i = _parser_info.nextToken; i < _parser_info.tokenCount; i++)
      {
        _parseToken(i, buf, _parser_info.nextDepth, (char *)key, index, PRINT_MODE_NONE);
        if (_parser_info.tokenMatch)
          break;
      }
    }
    helper->delS(buf);
    if (!_parser_info.tokenMatch)
    {
      _parser_info.paresRes = false;
      _jsonData.success = false;
      _resetParseResult();
    }
  }
}

void FirebaseJson::_strToTk(const std::string &str, std::vector<path_tk_t> &tk, char delim)
{
  std::size_t current, previous = 0;
  current = str.find(delim);
  std::string s;
  while (current != std::string::npos)
  {
    s = str.substr(previous, current - previous);
    _trim(s);
    if (s.length() > 0)
    {
      path_tk_t tk_t;
      tk_t.tk = s;
      tk.push_back(tk_t);
    }

    previous = current + 1;
    current = str.find(delim, previous);
    delay(0);
  }
  s = str.substr(previous, current - previous);
  _trim(s);
  if (s.length() > 0)
  {
    path_tk_t tk_t;
    tk_t.tk = s;
    tk.push_back(tk_t);
  }
  std::string().swap(s);
}

void FirebaseJson::_ltrim(std::string &str, const std::string &chars)
{
  str.erase(0, str.find_first_not_of(chars));
}

void FirebaseJson::_rtrim(std::string &str, const std::string &chars)
{
  str.erase(str.find_last_not_of(chars) + 1);
}

void FirebaseJson::_trim(std::string &str, const std::string &chars)
{
  _ltrim(str, chars);
  _rtrim(str, chars);
}
void FirebaseJson::int_parse(const char *path, PRINT_MODE printMode)
{
  _parse(path, printMode);
}

void FirebaseJson::_parse(const char *path, PRINT_MODE printMode)
{
  helper->clearLastError();
  clearPathTk();
  std::string _path;

  if (_topLevelTkType == JSMN_ARRAY)
  {
    char *_root = helper->strP(fb_json_str_26);
    char *_slash = helper->strP(fb_json_str_27);
    _path = _root;
    _path += _slash;
    _path += path;
    helper->delS(_root);
    helper->delS(_slash);
  }
  else
    _path = path;

  _strToTk(_path.c_str(), _pathTk, '/');
  _fbjs_parse();
  std::string().swap(_path);
  if (!_jsonData.success)
    return;
  _jsonData.success = false;
  int len = _pathTk.size();

  _resetParsserInfo();

  _parser_info.remTkIndex = -1;
  _parser_info.remFirstTk = false;
  _parser_info.remLastTk = false;
  _el.clear();
  _eltk.clear();
  if (len == 0)
  {
    _parse("", 0, -2, printMode);
    _jsonData.success = true;
  }
  else
  {
    for (int i = 0; i < len; i++)
    {
      if (_isStrTk(i))
        _parse(_pathTk[i].tk.c_str(), i, -1, printMode);
      else if (_isArrTk(i))
        _parse("", i, _getArrIndex(i), printMode);
      else
        _parse(_pathTk[i].tk.c_str(), i, -1, printMode);
    }
    _jsonData.success = _parser_info.parseCompleted == len;
  }
  _el.clear();
  _eltk.clear();
  clearPathTk();
  std::string().swap(_tbuf);
  _tokens.reset();
  _tokens = nullptr;
}
void FirebaseJson::int_clearPathTk()
{
  clearPathTk();
}

void FirebaseJson::clearPathTk()
{
  size_t len = _pathTk.size();
  for (size_t i = 0; i < len; i++)
    std::string().swap(_pathTk[i].tk);
  for (size_t i = 0; i < len; i++)
    _pathTk.erase(_pathTk.end());
  _pathTk.clear();
  std::vector<path_tk_t>().swap(_pathTk);
}

void FirebaseJson::int_clearTokens()
{
  _tokens.reset();
  _tokens = nullptr;
}

size_t FirebaseJson::int_get_jsondata_len()
{
  return _jsonData._len;
}

void FirebaseJson::_parse(const char *key, int depth, int index, PRINT_MODE printMode)
{
  helper->clearLastError();
  _parser_info.tokenMatch = false;
  if (_parser_info.paresRes)
  {
    std::string s;
    _toStdString(s);
    int bufLen = s.length() + _parser_buff_len;
    char *buf = helper->newS(bufLen);
    if (!buf)
    {
      helper->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
      return;
    }
    strcpy(buf, s.c_str());
    std::string().swap(s);
    _parser_info.parseDepth = depth;
    if (_parser_info.nextToken < 0)
      _parser_info.nextToken = 0;

    for (uint16_t i = _parser_info.nextToken; i < _parser_info.tokenCount; i++)
    {

      int oDepth = _parser_info.nextDepth;

      _parseToken(i, buf, _parser_info.nextDepth, (char *)key, index, printMode);

      if (index > -1 && oDepth == _parser_info.nextDepth && _parser_info.tokenMatch)
      {
        _parser_info.tokenMatch = false;
        break;
      }

      if (oDepth > _parser_info.nextDepth && index == -1)
      {
        if (_parser_info.nextDepth > -1 && _parser_info.nextDepth < (int)_pathTk.size())
        {
          if (_pathTk[_parser_info.nextDepth].matched)
          {
            _parser_info.tokenMatch = false;
            break;
          }
        }
      }

      if (_parser_info.tokenMatch)
      {
        _pathTk[depth].matched = true;
        break;
      }
    }

    helper->delS(buf);
    if (!_parser_info.tokenMatch)
    {
      _parser_info.paresRes = false;
      _jsonData.success = false;
    }
  }
}

void FirebaseJson::_compile(const char *key, int depth, int index, const char *replace, PRINT_MODE printMode, int refTokenIndex, bool removeTk)
{
  helper->clearLastError();
  _parser_info.tokenMatch = false;
  if (_parser_info.paresRes)
  {
    std::string s;
    _toStdString(s);
    int bufLen = s.length() + _parser_buff_len;
    char *buf = helper->newS(bufLen);
    if (!buf)
    {
      helper->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
      return;
    }
    strcpy(buf, s.c_str());
    std::string().swap(s);
    _parser_info.parseDepth = depth;
    if (_parser_info.nextToken < 0)
      _parser_info.nextToken = 0;
    for (uint16_t i = _parser_info.nextToken; i < _parser_info.tokenCount; i++)
    {
      _compileToken(i, buf, _parser_info.nextDepth, key, index, printMode, replace, refTokenIndex, removeTk);
      if (_parser_info.tokenMatch)
        break;
    }
    helper->delS(buf);
    if (!_parser_info.tokenMatch)
    {
      _parser_info.paresRes = false;
      _jsonData.success = false;
    }
  }
}

void FirebaseJson::_remove(const char *key, int depth, int index, const char *replace, int refTokenIndex, bool removeTk)
{
  helper->clearLastError();
  if (_parser_info.paresRes)
  {
    std::string s;
    _toStdString(s);
    int bufLen = s.length() + _parser_buff_len;
    char *buf = helper->newS(bufLen);
    if (!buf)
    {
      helper->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
      return;
    }
    strcpy(buf, s.c_str());
    std::string().swap(s);
    _parser_info.parseDepth = depth;
    if (_parser_info.nextToken < 0)
      _parser_info.nextToken = 0;
    for (uint16_t i = _parser_info.nextToken; i < _parser_info.tokenCount; i++)
    {
      _removeToken(i, buf, _parser_info.nextDepth, (char *)key, index, PRINT_MODE_PLAIN, (char *)replace, refTokenIndex, removeTk);
    }
    helper->delS(buf);
  }
}

bool FirebaseJson::_isArrTk(int index)
{
  if (index < (int)_pathTk.size())
    return _pathTk[index].tk.c_str()[0] == '[' && _pathTk[index].tk.c_str()[_pathTk[index].tk.length() - 1] == ']';
  else
    return false;
}
bool FirebaseJson::_isStrTk(int index)
{
  if (index < (int)_pathTk.size())
    return _pathTk[index].tk.c_str()[0] == '"' && _pathTk[index].tk.c_str()[_pathTk[index].tk.length() - 1] == '"';
  else
    return false;
}

int FirebaseJson::_getArrIndex(int index)
{
  int res = -1;
  if (index < (int)_pathTk.size())
  {
    res = atoi(_pathTk[index].tk.substr(1, _pathTk[index].tk.length() - 2).c_str());
    if (res < 0)
      res = 0;
  }
  return res;
}

void FirebaseJson::set(const String &path)
{
  helper->clearLastError();
  _setNull(path.c_str());
}

void FirebaseJson::set(const String &path, const String &value)
{
  helper->clearLastError();
  _setString(path.c_str(), value.c_str());
}

void FirebaseJson::set(const String &path, const char *value)
{
  helper->clearLastError();
  _setString(path.c_str(), value);
}

void FirebaseJson::set(const String &path, int value)
{
  helper->clearLastError();
  _setInt(path.c_str(), value);
}

void FirebaseJson::set(const String &path, unsigned short value)
{
  helper->clearLastError();
  _setInt(path.c_str(), value);
}

void FirebaseJson::set(const String &path, float value)
{
  helper->clearLastError();
  _setFloat(path.c_str(), value);
}

void FirebaseJson::set(const String &path, double value)
{
  helper->clearLastError();
  _setDouble(path.c_str(), value);
}

void FirebaseJson::set(const String &path, bool value)
{
  helper->clearLastError();
  _setBool(path.c_str(), value);
}

void FirebaseJson::set(const String &path, FirebaseJson &json)
{
  helper->clearLastError();
  _setJson(path.c_str(), &json);
}

void FirebaseJson::set(const String &path, FirebaseJsonArray &arr)
{
  helper->clearLastError();
  arr._lastErr = &_lastErr;
  _setArray(path.c_str(), &arr);
}

template <typename T>
bool FirebaseJson::set(const String &path, T value)
{
  if (std::is_same<T, int>::value)
    return _setInt(path, value);
  else if (std::is_same<T, float>::value)
    return _setFloat(path, value);
  else if (std::is_same<T, double>::value)
    return _setDouble(path, value);
  else if (std::is_same<T, bool>::value)
    return _setBool(path, value);
  else if (std::is_same<T, const char *>::value)
    return _setString(path, value);
  else if (std::is_same<T, FirebaseJson &>::value)
    return _setJson(path, &value);
  else if (std::is_same<T, FirebaseJsonArray &>::value)
    return _setArray(path, &value);
}

void FirebaseJson::_setString(const std::string &path, const std::string &value)
{
  helper->clearLastError();
  char *tmp = helper->newS(value.length() + _parser_buff_len);
  if (!tmp)
  {
    helper->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
    return;
  }
  strcpy(tmp, _qt);
  strcat(tmp, value.c_str());
  strcat(tmp, _qt);
  _set(path.c_str(), tmp);
  helper->delS(tmp);
  std::string().swap(_jsonData._dbuf);
}

void FirebaseJson::_setInt(const std::string &path, int value)
{
  char *tmp = helper->intStr(value);
  _set(path.c_str(), tmp);
  helper->delS(tmp);
  std::string().swap(_jsonData._dbuf);
}

void FirebaseJson::_setFloat(const std::string &path, float value)
{
  char *tmp = helper->floatStr(value);
  helper->trimDouble(tmp);
  _set(path.c_str(), tmp);
  helper->delS(tmp);
  std::string().swap(_jsonData._dbuf);
}

void FirebaseJson::_setDouble(const std::string &path, double value)
{
  char *tmp = helper->doubleStr(value);
  helper->trimDouble(tmp);
  _set(path.c_str(), tmp);
  helper->delS(tmp);
  std::string().swap(_jsonData._dbuf);
}

void FirebaseJson::_setBool(const std::string &path, bool value)
{
  if (value)
    _set(path.c_str(), _tr);
  else
    _set(path.c_str(), _fls);
  std::string().swap(_jsonData._dbuf);
}

void FirebaseJson::_setNull(const std::string &path)
{
  _set(path.c_str(), _nll);
  std::string().swap(_jsonData._dbuf);
}

void FirebaseJson::_setJson(const std::string &path, FirebaseJson *json)
{
  std::string s;
  json->_toStdString(s);
  _set(path.c_str(), s.c_str());
  std::string().swap(s);
}

void FirebaseJson::_setArray(const std::string &path, FirebaseJsonArray *arr)
{
  arr->_lastErr = &_lastErr;
  std::string s;
  arr->_toStdString(s);
  _set(path.c_str(), s.c_str());
  std::string().swap(s);
}

void FirebaseJson::_set(const char *path, const char *data)
{
  helper->clearLastError();
  clearPathTk();
  std::string _path;

  if (_topLevelTkType == JSMN_ARRAY)
  {
    char *_root = helper->strP(fb_json_str_26);
    char *_slash = helper->strP(fb_json_str_27);
    _path = _root;
    _path += _slash;
    _path += path;
    helper->delS(_root);
    helper->delS(_slash);
  }
  else
    _path = path;

  _strToTk(_path.c_str(), _pathTk, '/');
  _fbjs_parse();
  std::string().swap(_path);
  if (!_jsonData.success)
    return;
  _jsonData.success = false;
  int len = _pathTk.size();

  _resetParsserInfo();

  _parser_info.remTkIndex = -1;
  _parser_info.remFirstTk = false;
  _parser_info.remLastTk = false;
  _el.clear();
  _eltk.clear();
  for (int i = 0; i < len; i++)
  {
    if (_isStrTk(i))
      _compile(_pathTk[i].tk.c_str(), i, -1, data, PRINT_MODE_PLAIN);
    else if (_isArrTk(i))
      _compile("", i, _getArrIndex(i), data, PRINT_MODE_PLAIN);
    else
      _compile(_pathTk[i].tk.c_str(), i, -1, data, PRINT_MODE_PLAIN);
  }
  _el.clear();
  _eltk.clear();
  if (_parser_info.parseCompleted != len)
  {
    std::string().swap(_jsonData._dbuf);
    std::string().swap(_tbuf);
    int refTokenIndex = _parser_info.refTkIndex;

    _resetParsserInfo();

    _parser_info.tokenMatch = false;
    _parser_info.paresRes = true;
    for (int i = 0; i < len; i++)
    {
      if (_isStrTk(i))
        _compile(_pathTk[i].tk.c_str(), i, -1, data, PRINT_MODE_PLAIN, refTokenIndex);
      else if (_isArrTk(i))
        _compile("", i, _getArrIndex(i), data, PRINT_MODE_PLAIN, refTokenIndex);
      else
        _compile(_pathTk[i].tk.c_str(), i, -1, data, PRINT_MODE_PLAIN, refTokenIndex);
    }
    _el.clear();
    _eltk.clear();
  }
  if (_jsonData._dbuf.length() >= 2)
  {
    _jsonData.success = true;
    _rawbuf = _jsonData._dbuf.substr(1, _jsonData._dbuf.length() - 2);
  }
  else
    _rawbuf.clear();
  clearPathTk();
  std::string().swap(_jsonData._dbuf);
  std::string().swap(_tbuf);
  _tokens.reset();
  _tokens = nullptr;
}

bool FirebaseJson::remove(const String &path)
{
  helper->clearLastError();
  clearPathTk();
  std::string _path;

  if (_topLevelTkType == JSMN_ARRAY)
  {
    char *_root = helper->strP(fb_json_str_26);
    char *_slash = helper->strP(fb_json_str_27);
    _path = _root;
    _path += _slash;
    _path += path.c_str();
    helper->delS(_root);
    helper->delS(_slash);
  }
  else
    _path = path.c_str();

  _strToTk(_path.c_str(), _pathTk, '/');
  _fbjs_parse();
  std::string().swap(_path);
  if (!_jsonData.success)
    return false;

  _jsonData.success = false;
  int len = _pathTk.size();

  _resetParsserInfo();

  _parser_info.remTkIndex = -1;
  _parser_info.remFirstTk = false;
  _parser_info.remLastTk = false;
  _el.clear();
  _eltk.clear();
  for (int i = 0; i < len; i++)
  {
    if (_isStrTk(i))
      _compile(_pathTk[i].tk.c_str(), i, -1, "", PRINT_MODE_NONE, -1, true);
    else if (_isArrTk(i))
      _compile("", i, _getArrIndex(i), "", PRINT_MODE_NONE, -1, true);
    else
      _compile(_pathTk[i].tk.c_str(), i, -1, "", PRINT_MODE_NONE, -1, true);
  }
  _el.clear();
  _eltk.clear();
  std::string().swap(_jsonData._dbuf);
  int refTokenIndex = _parser_info.remTkIndex;
  if (_parser_info.parseCompleted == len)
  {

    _resetParsserInfo();

    _parser_info.tokenMatch = false;
    _parser_info.paresRes = true;
    _jsonData.success = true;
    _lastTk.skip = false;
    _lastTk.olen = 0;
    _lastTk.oindex = 0;
    if (_isStrTk(len - 1))
      _remove(_pathTk[len - 1].tk.c_str(), -1, -1, "", refTokenIndex, true);
    else
      _remove("", -1, _getArrIndex(len - 1), "", refTokenIndex, true);
    _jsonData._dbuf += _tbuf;
    _el.clear();
    _eltk.clear();
  }
  if (_jsonData._dbuf.length() >= 2)
    _rawbuf = _jsonData._dbuf.substr(1, _jsonData._dbuf.length() - 2);
  else
    _rawbuf.clear();

  //fix for the remaining parent when all childs removed
  if (_rawbuf.length() > 0)
  {
    char *temp = helper->strP(fb_json_str_32);
    size_t p1 = _rawbuf.find(temp);
    helper->delS(temp);

    if (p1 == std::string::npos)
    {
      temp = helper->strP(fb_json_str_33);
      p1 = _rawbuf.find(temp);
      helper->delS(temp);
    }

    if (p1 != std::string::npos)
    {
      int p3 = p1;
      if (p3 > 0)
        p3--;
      temp = helper->strP(fb_json_str_2);
      size_t p2 = _rawbuf.rfind(temp, p3);
      helper->delS(temp);
      if (p2 != std::string::npos)
      {
        if (p2 > 0)
        {
          if (_rawbuf[p2 - 1] == ',')
            p2--;
        }
        p1 += 2;
        _rawbuf.replace(p2, p1 - p2, "");
      }
    }
  }

  clearPathTk();
  std::string().swap(_jsonData._dbuf);
  std::string().swap(_tbuf);
  _tokens.reset();
  _tokens = nullptr;
  return _jsonData.success;
}

fb_json_last_error_t FirebaseJson::getLastError()
{
  return _lastErr;
}

void FirebaseJson::_resetParsserInfo()
{
  _parser_info.nextDepth = -1;
  _parser_info.nextToken = 0;
  _parser_info.skipDepth = -1;
  _parser_info.parentIndex = -1;
  _parser_info.TkRefOk = false;
  _parser_info.parseCompleted = 0;
  _parser_info.arrReplaced = false;
  _parser_info.refTkIndex = -1;
}

void FirebaseJson::_resetParseResult()
{
  _jsonData._type = 0;
  _jsonData.type = "";
  _jsonData.typeNum = 0;
  _jsonData.stringValue = "";
  _jsonData._dbuf = "";
  _jsonData.intValue = 0;
  _jsonData.floatValue = 0;
  _jsonData.doubleValue = 0;
  _jsonData.boolValue = false;
}

void FirebaseJson::_setElementType()
{
  helper->clearLastError();
  bool typeSet = false;
  char *buf = helper->newS(_parser_buff_len);
  char *tmp = helper->newS(_parser_buff_len);
  if (!buf || !tmp)
  {
    helper->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
    return;
  }
  char *tmp2 = nullptr;
  if (_jsonData._type == JSMN_PRIMITIVE)
  {
    tmp2 = helper->newS(_jsonData.stringValue.length() + 1);
    if (!tmp2)
    {
      helper->setLastError(-1, __FILE__, __LINE__, fb_json_str_28);
      return;
    }
    strcpy(tmp2, _jsonData.stringValue.c_str());
  }
  switch (_jsonData._type)
  {
  case JSMN_UNDEFINED:
    strcpy(buf, _undef);
    _jsonData.typeNum = JSON_UNDEFINED;
    break;
  case JSMN_OBJECT:
    strcpy(buf, _obj);
    _jsonData.typeNum = JSON_OBJECT;
    break;
  case JSMN_ARRAY:
    strcpy(buf, _arry);
    _jsonData.typeNum = JSON_ARRAY;
    break;
  case JSMN_STRING:
    strcpy(buf, _string);
    _jsonData.typeNum = JSON_STRING;
    break;
  case JSMN_PRIMITIVE:
    if (!typeSet && strcmp(tmp2, _tr) == 0)
    {
      typeSet = true;
      strcpy(buf, _bl);
      _jsonData.typeNum = JSON_BOOL;
      _jsonData.boolValue = true;
      _jsonData.floatValue = 1.0f;
      _jsonData.doubleValue = 1.0;
      _jsonData.intValue = 1;
    }
    else
    {
      if (!typeSet && strcmp(tmp2, _fls) == 0)
      {
        typeSet = true;
        strcpy(buf, _bl);
        _jsonData.typeNum = JSON_BOOL;
        _jsonData.boolValue = false;
        _jsonData.floatValue = 0.0f;
        _jsonData.doubleValue = 0.0;
        _jsonData.intValue = 0;
      }
    }

    if (!typeSet && strcmp(tmp2, _nll) == 0)
    {
      typeSet = true;
      strcpy(buf, _nll);
      _jsonData.typeNum = JSON_NULL;
    }
    if (!typeSet)
    {
      typeSet = true;
      strcpy(tmp, _dot);
      double d = atof(tmp2);
      if (d > 0x7fffffff)
      {
        strcpy(buf, _dbl);
        _jsonData.floatValue = (float)d;
        _jsonData.doubleValue = d;
        _jsonData.intValue = atoi(tmp2);
        _jsonData.boolValue = atof(tmp2) > 0 ? true : false;
        _jsonData.typeNum = JSON_DOUBLE;
      }
      else
      {
        if (helper->strpos(tmp2, tmp, 0) > -1)
        {
          strcpy(buf, _dbl);
          _jsonData.floatValue = (float)d;
          _jsonData.doubleValue = d;
          _jsonData.intValue = atoi(tmp2);
          _jsonData.boolValue = atof(tmp2) > 0 ? true : false;
          _jsonData.typeNum = JSON_FLOAT;
        }
        else
        {
          _jsonData.intValue = atoi(tmp2);
          _jsonData.floatValue = atof(tmp2);
          _jsonData.doubleValue = atof(tmp2);
          _jsonData.boolValue = atof(tmp2) > 0 ? true : false;
          strcpy(buf, _int);
          _jsonData.typeNum = JSON_INT;
        }
      }
    }
    break;
  default:
    break;
  }
  _jsonData.type = buf;
  helper->delS(buf);
  helper->delS(tmp);
  if (tmp2)
    helper->delS(tmp2);
}

/**
 * Allocates a fresh unused token from the token pool.
 */
FirebaseJson::fbjs_tok_t *FirebaseJson::fbjs_alloc_token(fbjs_parser *parser, FirebaseJson::fbjs_tok_t *tokens, size_t num_tokens)
{
  FirebaseJson::fbjs_tok_t *tok;
  if (parser->toknext >= num_tokens)
  {
    return NULL;
  }
  tok = &tokens[parser->toknext++];
  tok->start = tok->end = -1;
  tok->size = 0;
#ifdef JSMN_PARENT_LINKS
  tok->parent = -1;
#endif
  return tok;
}

/**
 * Fills token type and boundaries.
 */
void FirebaseJson::fbjs_fill_token(fbjs_tok_t *token, fbjs_type_t type, int start, int end)
{
  token->type = type;
  token->start = start;
  token->end = end;
  token->size = 0;
}

/**
 * Fills next available token with JSON primitive.
 */
int FirebaseJson::fbjs_parse_primitive(fbjs_parser *parser, const char *js, size_t len, fbjs_tok_t *tokens, size_t num_tokens)
{
  fbjs_tok_t *token;
  int start;

  start = parser->pos;

  for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++)
  {
    switch (js[parser->pos])
    {
#ifndef JSMN_STRICT
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
      return JSMN_ERROR_INVAL;
    }
  }
#ifdef JSMN_STRICT
  /* In strict mode primitive must be followed by a comma/object/array */
  parser->pos = start;
  return JSMN_ERROR_PART;
#endif

found:
  if (tokens == NULL)
  {
    parser->pos--;
    return 0;
  }
  token = fbjs_alloc_token(parser, tokens, num_tokens);
  if (token == NULL)
  {
    parser->pos = start;
    return JSMN_ERROR_NOMEM;
  }
  fbjs_fill_token(token, JSMN_PRIMITIVE, start, parser->pos);
#ifdef JSMN_PARENT_LINKS
  token->parent = parser->toksuper;
#endif
  parser->pos--;
  return 0;
}

/**
 * Fills next token with JSON string.
 */
int FirebaseJson::fbjs_parse_string(fbjs_parser *parser, const char *js, size_t len, fbjs_tok_t *tokens, size_t num_tokens)
{
  fbjs_tok_t *token;

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
      token = fbjs_alloc_token(parser, tokens, num_tokens);
      if (token == NULL)
      {
        parser->pos = start;
        return JSMN_ERROR_NOMEM;
      }
      fbjs_fill_token(token, JSMN_STRING, start + 1, parser->pos);
#ifdef JSMN_PARENT_LINKS
      token->parent = parser->toksuper;
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
            return JSMN_ERROR_INVAL;
          }
          parser->pos++;
        }
        parser->pos--;
        break;
      /* Unexpected symbol */
      default:
        parser->pos = start;
        return JSMN_ERROR_INVAL;
      }
    }
  }
  parser->pos = start;
  return JSMN_ERROR_PART;
}

/**
 * Parse JSON string and fill tokens.
 */
int FirebaseJson::fbjs_parse(fbjs_parser *parser, const char *js, size_t len, fbjs_tok_t *tokens, unsigned int num_tokens)
{
  int r;
  int i;
  fbjs_tok_t *token;
  int count = parser->toknext;
  
  for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++)
  {
    char c;
    fbjs_type_t type;

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
      token = fbjs_alloc_token(parser, tokens, num_tokens);
      if (token == NULL)
      {
        return JSMN_ERROR_NOMEM;
      }
      if (parser->toksuper != -1)
      {
        fbjs_tok_t *t = &tokens[parser->toksuper];
#ifdef JSMN_STRICT
        /* In strict mode an object or array can't become a key */
        if (t->type == JSMN_OBJECT)
        {
          return JSMN_ERROR_INVAL;
        }
#endif
        t->size++;
#ifdef JSMN_PARENT_LINKS
        token->parent = parser->toksuper;
#endif
      }
      token->type = (c == '{' ? JSMN_OBJECT : JSMN_ARRAY);
      token->start = parser->pos;
      parser->toksuper = parser->toknext - 1;
      if (parser->pos > 0)
        if (js[parser->pos - 1] == '{' && js[parser->pos] == '[')
          return JSMN_ERROR_INVAL;
      break;
    case '}':
    case ']':
      if (tokens == NULL)
      {
        break;
      }
      type = (c == '}' ? JSMN_OBJECT : JSMN_ARRAY);
#ifdef JSMN_PARENT_LINKS
      if (parser->toknext < 1)
      {
        return JSMN_ERROR_INVAL;
      }
      token = &tokens[parser->toknext - 1];
      for (;;)
      {
        if (token->start != -1 && token->end == -1)
        {
          if (token->type != type)
          {
            return JSMN_ERROR_INVAL;
          }
          token->end = parser->pos + 1;
          parser->toksuper = token->parent;
          break;
        }
        if (token->parent == -1)
        {
          if (token->type != type || parser->toksuper == -1)
          {
            return JSMN_ERROR_INVAL;
          }
          break;
        }
        token = &tokens[token->parent];
      }
#else
      for (i = parser->toknext - 1; i >= 0; i--)
      {
        token = &tokens[i];
        if (token->start != -1 && token->end == -1)
        {
          if (token->type != type)
          {
            return JSMN_ERROR_INVAL;
          }
          parser->toksuper = -1;
          token->end = parser->pos + 1;
          break;
        }
      }
      /* Error if unmatched closing bracket */
      if (i == -1)
      {
        return JSMN_ERROR_INVAL;
      }
      for (; i >= 0; i--)
      {
        token = &tokens[i];
        if (token->start != -1 && token->end == -1)
        {
          parser->toksuper = i;
          break;
        }
      }
#endif
      break;
    case '\"':
      r = fbjs_parse_string(parser, js, len, tokens, num_tokens);
      if (r < 0)
      {
        return r;
      }
      count++;
      if (parser->toksuper != -1 && tokens != NULL)
      {
        tokens[parser->toksuper].size++;
      }
      break;
    case '\t':
    case '\r':
    case '\n':
    case ' ':
      break;
    case ':':
      parser->toksuper = parser->toknext - 1;
      break;
    case ',':
      if (tokens != NULL && parser->toksuper != -1 &&
          tokens[parser->toksuper].type != JSMN_ARRAY &&
          tokens[parser->toksuper].type != JSMN_OBJECT)
      {
#ifdef JSMN_PARENT_LINKS
        parser->toksuper = tokens[parser->toksuper].parent;
#else
        for (i = parser->toknext - 1; i >= 0; i--)
        {
          if (tokens[i].type == JSMN_ARRAY || tokens[i].type == JSMN_OBJECT)
          {
            if (tokens[i].start != -1 && tokens[i].end == -1)
            {
              parser->toksuper = i;
              break;
            }
          }
        }
#endif
      }
      break;
#ifdef JSMN_STRICT
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
      if (tokens != NULL && parser->toksuper != -1)
      {
        const fbjs_tok_t *t = &tokens[parser->toksuper];
        if (t->type == JSMN_OBJECT ||
            (t->type == JSMN_STRING && t->size != 0))
        {
          return JSMN_ERROR_INVAL;
        }
      }
#else
    /* In non-strict mode every unquoted value is a primitive */
    default:
#endif
      r = fbjs_parse_primitive(parser, js, len, tokens, num_tokens);
      if (r < 0)
      {
        return r;
      }
      count++;
      if (parser->toksuper != -1 && tokens != NULL)
      {
        tokens[parser->toksuper].size++;
      }
      break;

#ifdef JSMN_STRICT
    /* Unexpected char in strict mode */
    default:
      if (tokens != NULL)
        return JSMN_ERROR_INVAL;
#endif
    }
  }

  if (tokens != NULL)
  {
    for (i = parser->toknext - 1; i >= 0; i--)
    {
      /* Unmatched opened object or array */
      if (tokens[i].start != -1 && tokens[i].end == -1)
      {
        return JSMN_ERROR_PART;
      }
    }
  }

  return count;
}

/**
 * Creates a new parser based over a given  buffer with an array of tokens
 * available.
 */
void FirebaseJson::fbjs_init(fbjs_parser *parser)
{
  parser->pos = 0;
  parser->toknext = 0;
  parser->toksuper = -1;
}

FirebaseJsonArray::FirebaseJsonArray()
{
  _init();
}
FirebaseJsonArray::FirebaseJsonArray(fb_json_last_error_t *lastErr, size_t bufLimit)
{
  if (bufLimit >= 32 || bufLimit <= 8192)
    _parser_buff_len = bufLimit;
  _lastErr = lastErr;
};

FirebaseJsonArray::~FirebaseJsonArray()
{
  _finalize();
  std::string().swap(_jbuf);
  delete helper;
};

void FirebaseJsonArray::_init()
{
  _finalize();

  _pd = helper->strP(fb_json_str_4);
  _pf = helper->strP(fb_json_str_5);
  _fls = helper->strP(fb_json_str_6);
  _tr = helper->strP(fb_json_str_7);
  _brk3 = helper->strP(fb_json_str_10);
  _brk4 = helper->strP(fb_json_str_11);
  _nll = helper->strP(fb_json_str_18);
  _root = helper->strP(fb_json_str_21);
  _root2 = helper->strP(fb_json_str_26);
  _qt = helper->strP(fb_json_str_2);
  _slash = helper->strP(fb_json_str_27);
  helper->clearLastError();
}

std::string *FirebaseJsonArray::int_dbuf()
{
  return &_json._jsonData._dbuf;
}

std::string *FirebaseJsonArray::int_tbuf()
{
  return &_json._tbuf;
}

std::string *FirebaseJsonArray::int_jbuf()
{
  return &_jbuf;
}
std::string *FirebaseJsonArray::int_rawbuf()
{
  return &_json._rawbuf;
}
FirebaseJson *FirebaseJsonArray::int_json()
{
  return &_json;
}
void FirebaseJsonArray::int_set_arr_len(size_t len)
{
  _arrLen = len;
}

void FirebaseJsonArray::_finalize()
{
  helper->delS(_pd);
  helper->delS(_pf);
  helper->delS(_fls);
  helper->delS(_tr);
  helper->delS(_brk3);
  helper->delS(_brk4);
  helper->delS(_nll);
  helper->delS(_root);
  helper->delS(_root2);
  helper->delS(_qt);
  helper->delS(_slash);
}

FirebaseJsonArray &FirebaseJsonArray::add()
{
  _addNull();
  return *this;
}

FirebaseJsonArray &FirebaseJsonArray::add(const String &value)
{
  _addString(value.c_str());
  return *this;
}
FirebaseJsonArray &FirebaseJsonArray::add(const char *value)
{
  _addString(value);
  return *this;
}
FirebaseJsonArray &FirebaseJsonArray::add(int value)
{
  _addInt(value);
  return *this;
}

FirebaseJsonArray &FirebaseJsonArray::add(unsigned short value)
{
  _addInt(value);
  return *this;
}

FirebaseJsonArray &FirebaseJsonArray::add(float value)
{
  _addFloat(value);
  return *this;
}

FirebaseJsonArray &FirebaseJsonArray::add(double value)
{
  _addDouble(value);
  return *this;
}

FirebaseJsonArray &FirebaseJsonArray::add(bool value)
{
  _addBool(value);
  return *this;
}

FirebaseJsonArray &FirebaseJsonArray::add(FirebaseJson &json)
{
  _addJson(&json);
  return *this;
}

FirebaseJsonArray &FirebaseJsonArray::add(FirebaseJsonArray &arr)
{
  _addArray(&arr);
  return *this;
}

template <typename T>
FirebaseJsonArray &FirebaseJsonArray::add(T value)
{
  if (std::is_same<T, int>::value)
    _addInt(value);
  else if (std::is_same<T, float>::value)
    _addFloat(value);
  else if (std::is_same<T, double>::value)
    _addDouble(value);
  else if (std::is_same<T, bool>::value)
    _addBool(value);
  else if (std::is_same<T, const char *>::value)
    _addString(value);
  else if (std::is_same<T, FirebaseJson &>::value)
    _addJson(&value);
  else if (std::is_same<T, FirebaseJsonArray &>::value)
    _addArray(&value);
  return *this;
}

void FirebaseJsonArray::_addString(const std::string &value)
{
  _arrLen++;
  _json._addArrayStr(value.c_str(), value.length(), true);
}

void FirebaseJsonArray::_addInt(int value)
{
  _arrLen++;
  char *buf = helper->intStr(value);
  sprintf(buf, _pd, value);
  _json._addArrayStr(buf, 60, false);
  helper->delS(buf);
}

void FirebaseJsonArray::_addFloat(float value)
{
  _arrLen++;
  char *buf = helper->floatStr(value);
  helper->trimDouble(buf);
  _json._addArrayStr(buf, 60, false);
  helper->delS(buf);
}

void FirebaseJsonArray::_addDouble(double value)
{
  _arrLen++;
  char *buf = helper->doubleStr(value);
  helper->trimDouble(buf);
  _json._addArrayStr(buf, 60, false);
  helper->delS(buf);
}

void FirebaseJsonArray::_addBool(bool value)
{
  _arrLen++;
  if (value)
    _json._addArrayStr(_tr, 6, false);
  else
    _json._addArrayStr(_fls, 7, false);
}

void FirebaseJsonArray::_addNull()
{
  _arrLen++;
  _json._addArrayStr(_nll, 6, false);
}

void FirebaseJsonArray::_addJson(FirebaseJson *json)
{
  _arrLen++;
  std::string s;
  json->_toStdString(s);
  _json._addArrayStr(s.c_str(), s.length(), false);
  std::string().swap(s);
}

void FirebaseJsonArray::_addArray(FirebaseJsonArray *arr)
{
  _arrLen++;
  String arrStr;
  arr->toString(arrStr);
  _json._addArrayStr(arrStr.c_str(), arrStr.length(), false);
}

FirebaseJsonArray &FirebaseJsonArray::setJsonArrayData(const String &data)
{
  int start_pos = data.indexOf('[');
  int end_pos = data.indexOf(']');

  if (start_pos != -1 && end_pos != -1 && start_pos != end_pos)
  {
    char *r = helper->strP(fb_json_str_21);
    _json._rawbuf = r;
    _json._rawbuf += data.c_str();
    helper->delS(r);
    r = helper->strP(fb_json_str_26);
    FirebaseJsonData data(_parser_buff_len);
    _json.get(data, r);
    helper->delS(r);
    data.getArray(*this);
    data.stringValue = "";
  }
  return *this;
}

bool FirebaseJsonArray::get(FirebaseJsonData &jsonData, const String &path)
{
  return _get(jsonData, path.c_str());
}

bool FirebaseJsonArray::get(FirebaseJsonData &jsonData, int index)
{
  char *tmp = helper->intStr(index);
  std::string path = "";
  path += _brk3;
  path += tmp;
  path += _brk4;
  bool ret = _get(jsonData, path.c_str());
  std::string().swap(path);
  helper->delS(tmp);
  return ret;
}

bool FirebaseJsonArray::_get(FirebaseJsonData &jsonData, const char *path)
{
  _json._toStdString(_jbuf, false);
  _json._rawbuf = _root;
  _json._rawbuf += _jbuf;
  std::string path2 = _root2;
  path2 += _slash;
  path2 += path;
  _json.clearPathTk();
  _json._strToTk(path2.c_str(), _json._pathTk, '/');
  if (!_json._isArrTk(1))
  {
    _json._jsonData.success = false;
    goto ex_;
  }
  if (_json._getArrIndex(1) < 0)
  {
    _json._jsonData.success = false;
    goto ex_;
  }
  _json._parse(path2.c_str(), FirebaseJson::PRINT_MODE_NONE);
  if (_json._jsonData.success)
  {
    _json._rawbuf = _jbuf.substr(1, _jbuf.length() - 2).c_str();
    if (_json._jsonData._type == FirebaseJson::JSMN_STRING && _json._jsonData.stringValue.c_str()[0] == '"' && _json._jsonData.stringValue.c_str()[_json._jsonData.stringValue.length() - 1] == '"')
      _json._jsonData.stringValue = _json._jsonData.stringValue.substring(1, _json._jsonData.stringValue.length() - 1).c_str();
    jsonData = _json._jsonData;
  }
ex_:
  _json.clearPathTk();
  _json._tokens.reset();
  _json._tokens = nullptr;
  return _json._jsonData.success;
}

size_t FirebaseJsonArray::size()
{
  return _arrLen;
}

void FirebaseJsonArray::toString(String &buf, bool prettify)
{
  char *tmp = helper->newS(_parser_buff_len);
  std::string().swap(_json._jsonData._dbuf);
  std::string().swap(_json._tbuf);
  _json._toStdString(_jbuf, false);
  _json._rawbuf = _root;
  _json._rawbuf += _jbuf;
  if (prettify)
    _json._parse(_root2, FirebaseJson::PRINT_MODE_PRETTY);
  else
    _json._parse(_root2, FirebaseJson::PRINT_MODE_PLAIN);
  std::string().swap(_json._tbuf);
  std::string().swap(_jbuf);
  _json.clearPathTk();
  _json._tokens.reset();
  _json._tokens = nullptr;
  helper->delS(tmp);
  _json._rawbuf = _json._jsonData._dbuf.substr(1, _json._jsonData._dbuf.length() - 2);
  buf = _json._jsonData._dbuf.c_str();
  std::string().swap(_json._jsonData._dbuf);
}

FirebaseJsonArray &FirebaseJsonArray::clear()
{
  _json.clear();
  std::string().swap(_jbuf);
  _json._jsonData.success = false;
  _json._jsonData.stringValue = "";
  _json._jsonData.boolValue = false;
  _json._jsonData.doubleValue = 0;
  _json._jsonData.intValue = 0;
  _json._jsonData.floatValue = 0;
  _json._jsonData._len = 0;
  _arrLen = 0;
  return *this;
}
void FirebaseJsonArray::_set2(int index, const char *value, bool isStr)
{
  char *tmp = helper->newS(50);
  std::string path = _brk3;
  sprintf(tmp, "%d", index);
  path += tmp;
  path += _brk4;
  _set(path.c_str(), value, isStr);
  std::string().swap(path);
  helper->delS(tmp);
}

void FirebaseJsonArray::_set(const char *path, const char *value, bool isStr)
{
  _json._jsonData.success = false;
  _json._toStdString(_jbuf, false);
  _json._rawbuf = _root;
  _json._rawbuf += _jbuf;
  char *tmp2 = helper->newS(strlen(value) + 10);
  if (isStr)
    strcpy_P(tmp2, _qt);
  strcat(tmp2, value);
  if (isStr)
    strcat_P(tmp2, _qt);
  std::string path2 = _root2;
  path2 += _slash;
  path2 += path;
  _json.clearPathTk();
  _json._strToTk(path2, _json._pathTk, '/');
  if (!_json._isArrTk(1))
  {
    helper->delS(tmp2);
    goto ex_2;
  }

  if (_json._getArrIndex(1) < 0)
  {
    helper->delS(tmp2);
    goto ex_2;
  }

  _json._set(path2.c_str(), tmp2);
  helper->delS(tmp2);
  std::string().swap(path2);
  if (_json._jsonData.success)
  {
    std::string().swap(_json._jsonData._dbuf);
    std::string().swap(_json._tbuf);
    _json._parse(_root2, FirebaseJson::PRINT_MODE_PLAIN);
    if (_json._jsonData.success)
    {
      _arrLen = _json._jsonData._len;
      _json._rawbuf = _json._jsonData._dbuf.substr(1, _json._jsonData._dbuf.length() - 2);
    }
  }
  else
    _json._rawbuf = _jbuf.substr(1, _jbuf.length() - 2);
ex_2:
  std::string().swap(_json._jsonData._dbuf);
  std::string().swap(_json._tbuf);
  std::string().swap(_jbuf);
  _json.clearPathTk();
  _json._tokens.reset();
  _json._tokens = nullptr;
}

void FirebaseJsonArray::set(int index)
{
  return _setNull(index);
}

void FirebaseJsonArray::set(const String &path)
{
  _setNull(path);
}

void FirebaseJsonArray::set(int index, const String &value)
{
  _setString(index, value.c_str());
}

void FirebaseJsonArray::set(const String &path, const String &value)
{
  _setString(path, value.c_str());
}

void FirebaseJsonArray::set(int index, const char *value)
{
  _setString(index, value);
}

void FirebaseJsonArray::set(const String &path, const char *value)
{
  _setString(path, value);
}

void FirebaseJsonArray::set(int index, int value)
{
  _setInt(index, value);
}

void FirebaseJsonArray::set(int index, unsigned short value)
{
  _setInt(index, value);
}

void FirebaseJsonArray::set(const String &path, int value)
{
  _setInt(path, value);
}

void FirebaseJsonArray::set(const String &path, unsigned short value)
{
  _setInt(path, value);
}

void FirebaseJsonArray::set(int index, float value)
{
  _setFloat(index, value);
}

void FirebaseJsonArray::set(const String &path, float value)
{
  _setFloat(path, value);
}

void FirebaseJsonArray::set(int index, double value)
{
  _setDouble(index, value);
}

void FirebaseJsonArray::set(const String &path, double value)
{
  _setDouble(path, value);
}

void FirebaseJsonArray::set(int index, bool value)
{
  _setBool(index, value);
}

void FirebaseJsonArray::set(const String &path, bool value)
{
  _setBool(path, value);
}

void FirebaseJsonArray::set(int index, FirebaseJson &json)
{
  _setJson(index, &json);
}

void FirebaseJsonArray::set(const String &path, FirebaseJson &json)
{
  _setJson(path, &json);
}

void FirebaseJsonArray::set(int index, FirebaseJsonArray &arr)
{
  arr._lastErr = _lastErr;
  _setArray(index, &arr);
}

void FirebaseJsonArray::set(const String &path, FirebaseJsonArray &arr)
{
  arr._lastErr = _lastErr;
  _setArray(path, &arr);
}

template <typename T>
void FirebaseJsonArray::set(int index, T value)
{
  if (std::is_same<T, int>::value)
    _setInt(index, value);
  else if (std::is_same<T, float>::value)
    _setFloat(index, value);
  else if (std::is_same<T, double>::value)
    _setDouble(index, value);
  else if (std::is_same<T, bool>::value)
    _setBool(index, value);
  else if (std::is_same<T, const char *>::value)
    _setString(index, value);
  else if (std::is_same<T, FirebaseJson &>::value)
    _setJson(index, &value);
  else if (std::is_same<T, FirebaseJsonArray &>::value)
    _setArray(index, &value);
}

template <typename T>
void FirebaseJsonArray::set(const String &path, T value)
{
  if (std::is_same<T, int>::value)
    _setInt(path, value);
  else if (std::is_same<T, float>::value)
    _setFloat(path, value);
  else if (std::is_same<T, double>::value)
    _setDouble(path, value);
  else if (std::is_same<T, bool>::value)
    _setBool(path, value);
  else if (std::is_same<T, const char *>::value)
    _setString(path, value);
  else if (std::is_same<T, FirebaseJson &>::value)
    _setJson(path, &value);
  else if (std::is_same<T, FirebaseJsonArray &>::value)
    _setArray(path, &value);
}

void FirebaseJsonArray::_setString(int index, const std::string &value)
{
  _set2(index, value.c_str(), true);
}

void FirebaseJsonArray::_setString(const String &path, const std::string &value)
{
  _set(path.c_str(), value.c_str(), true);
}

void FirebaseJsonArray::_setInt(int index, int value)
{
  char *tmp = helper->intStr(value);
  _set2(index, tmp, false);
  helper->delS(tmp);
}

void FirebaseJsonArray::_setInt(const String &path, int value)
{
  char *tmp = helper->intStr(value);
  _set(path.c_str(), tmp, false);
  helper->delS(tmp);
}

void FirebaseJsonArray::_setFloat(int index, float value)
{
  char *tmp = helper->floatStr(value);
  helper->trimDouble(tmp);
  _set2(index, tmp, false);
  helper->delS(tmp);
}

void FirebaseJsonArray::_setFloat(const String &path, float value)
{
  char *tmp = helper->floatStr(value);
  helper->trimDouble(tmp);
  _set(path.c_str(), tmp, false);
  helper->delS(tmp);
}

void FirebaseJsonArray::_setDouble(int index, double value)
{
  char *tmp = helper->doubleStr(value);
  helper->trimDouble(tmp);
  _set2(index, tmp, false);
  helper->delS(tmp);
}

void FirebaseJsonArray::_setDouble(const String &path, double value)
{
  char *tmp = helper->doubleStr(value);
  helper->trimDouble(tmp);
  _set(path.c_str(), tmp, false);
  helper->delS(tmp);
}

void FirebaseJsonArray::_setBool(int index, bool value)
{
  if (value)
    _set2(index, _tr, false);
  else
    _set2(index, _fls, false);
}

void FirebaseJsonArray::_setBool(const String &path, bool value)
{
  if (value)
    _set(path.c_str(), _tr, false);
  else
    _set(path.c_str(), _fls, false);
}

void FirebaseJsonArray::_setNull(int index)
{
  _set2(index, _nll, false);
}

void FirebaseJsonArray::_setNull(const String &path)
{
  _set(path.c_str(), _nll, false);
}

void FirebaseJsonArray::_setJson(int index, FirebaseJson *json)
{
  std::string s;
  json->_toStdString(s);
  _set2(index, s.c_str(), false);
  std::string().swap(s);
}

void FirebaseJsonArray::_setJson(const String &path, FirebaseJson *json)
{
  std::string s;
  json->_toStdString(s);
  _set(path.c_str(), s.c_str(), false);
  std::string().swap(s);
}

void FirebaseJsonArray::_setArray(int index, FirebaseJsonArray *arr)
{
  arr->_lastErr = _lastErr;
  std::string s;
  arr->_toStdString(s);
  _set2(index, s.c_str(), false);
  std::string().swap(s);
}

void FirebaseJsonArray::_setArray(const String &path, FirebaseJsonArray *arr)
{
  arr->_lastErr = _lastErr;
  std::string s;
  arr->_toStdString(s);
  _set(path.c_str(), s.c_str(), false);
  std::string().swap(s);
}

bool FirebaseJsonArray::remove(int index)
{
  char *tmp = helper->intStr(index);
  std::string path = "";
  path += _brk3;
  path += tmp;
  path += _brk4;
  bool ret = _remove(path.c_str());
  std::string().swap(path);
  helper->delS(tmp);
  return ret;
}

bool FirebaseJsonArray::remove(const String &path)
{
  return _remove(path.c_str());
}

bool FirebaseJsonArray::_remove(const char *path)
{
  _json._toStdString(_jbuf, false);
  _json._rawbuf = _root;
  _json._rawbuf += _jbuf;
  std::string path2 = _root2;
  path2 += _slash;
  path2 += path;
  _json._jsonData.success = _json.remove(path2.c_str());
  std::string().swap(path2);
  bool success = _json._jsonData.success;
  if (_json._jsonData.success)
  {
    std::string().swap(_json._jsonData._dbuf);
    std::string().swap(_json._tbuf);
    _json._parse(_root2, FirebaseJson::PRINT_MODE_PLAIN);
    if (_json._jsonData.success)
    {
      _arrLen = _json._jsonData._len;
      _json._rawbuf = _json._jsonData._dbuf.substr(1, _json._jsonData._dbuf.length() - 2);
    }
  }
  else
    _json._rawbuf = _jbuf.substr(1, _jbuf.length() - 2);

  if (_json._rawbuf.length() == 0)
  {
    _json._jsonData.success = success;
    _arrLen = 0;
  }

  return _json._jsonData.success;
}

void FirebaseJsonArray::int_toStdString(std::string &s)
{
  _json._toStdString(s);
}

void FirebaseJsonArray::_toStdString(std::string &s)
{
  _json._toStdString(s, false);
}

FirebaseJsonData::FirebaseJsonData()
{
}

FirebaseJsonData::FirebaseJsonData(size_t bufLimit)
{
  if (bufLimit >= 32 || bufLimit <= 8192)
    _parser_buff_len = bufLimit;
}

FirebaseJsonData::~FirebaseJsonData()
{
  std::string().swap(_dbuf);
}

bool FirebaseJsonData::getArray(FirebaseJsonArray &jsonArray)
{
  if (typeNum != FirebaseJson::JSON_ARRAY || !success)
    return false;
  char *tmp = new char[_parser_buff_len];
  memset(tmp, 0, _parser_buff_len);
  char *nbuf = new char[2];
  memset(nbuf, 0, 2);
  strcpy_P(tmp, fb_json_str_21);
  jsonArray._json._toStdString(jsonArray._jbuf, false);
  jsonArray._json._rawbuf = tmp;
  jsonArray._json._rawbuf += stringValue.c_str();
  memset(tmp, 0, _parser_buff_len);
  strcpy_P(tmp, fb_json_str_26);
  std::string().swap(jsonArray._json._jsonData._dbuf);
  std::string().swap(jsonArray._json._tbuf);
  jsonArray._json._parse(tmp, FirebaseJson::PRINT_MODE_PLAIN);
  jsonArray._json._rawbuf = jsonArray._json._jsonData._dbuf.substr(1, jsonArray._json._jsonData._dbuf.length() - 2).c_str();
  jsonArray._arrLen = jsonArray._json._jsonData._len;
  delete[] tmp;
  delete[] nbuf;
  return jsonArray._json._jsonData.success;
}

bool FirebaseJsonData::getJSON(FirebaseJson &json)
{
  if (typeNum != FirebaseJson::JSON_OBJECT || !success)
    return false;
  json.setJsonData(stringValue);
  json._fbjs_parse();
  return json._jsonData.success;
}

#endif