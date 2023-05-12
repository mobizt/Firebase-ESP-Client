/*
 * FirebaseJson, version 3.0.6
 *
 * The Easiest Arduino library to parse, create and edit JSON object using a relative path.
 *
 * Created March 5, 2023
 *
 * Features
 * - Using path to access node element in search style e.g. json.get(result,"a/b/c")
 * - Serializing to writable objects e.g. String, C/C++ string, Clients (WiFi, Ethernet, and GSM), File and Hardware Serial.
 * - Deserializing from const char, char array, string literal and stream e.g. Clients (WiFi, Ethernet, and GSM), File and
 *   Hardware Serial.
 * - Use managed class, FirebaseJsonData to keep the deserialized result, which can be casted to any primitive data types.
 *
 *
 * The MIT License (MIT)
 * Copyright (c) 2023 K. Suwatchai (Mobizt)
 * Copyright (c) 2009-2017 Dave Gamble and cJSON contributors
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

#include "FirebaseJson.h"

FirebaseJsonBase::FirebaseJsonBase()
{
    MB_JSON_InitHooks(&MB_JSON_hooks);
}

FirebaseJsonBase::~FirebaseJsonBase()
{
    mClear();
}

FirebaseJsonBase &FirebaseJsonBase::mClear()
{
    mIteratorEnd();
    if (root != NULL)
        MB_JSON_Delete(root);
    root = NULL;
    buf.clear();
    errorPos = -1;
    return *this;
}
void FirebaseJsonBase::mCopy(FirebaseJsonBase &other)
{
    mClear();
    this->root = MB_JSON_Duplicate(other.root, true);
    this->doubleDigits = other.doubleDigits;
    this->floatDigits = other.floatDigits;
    this->httpCode = other.httpCode;
    this->serData = other.serData;
    this->root_type = other.root_type;
    this->iterator_data = other.iterator_data;
    this->buf = other.buf;
}

bool FirebaseJsonBase::setRaw(const char *raw)
{
    mClear();

    if (raw)
    {
        size_t i = 0;
        while (i < strlen(raw) && raw[i] == ' ')
        {
            i++;
        }

        if (raw[i] == '{' || raw[i] == '[')
        {
            this->root_type = (raw[i] == '{') ? Root_Type_JSON : Root_Type_JSONArray;
            root = parse(raw);
        }
        else
        {
            this->root_type = Root_Type_Raw;
            root = MB_JSON_CreateRaw(raw);
        }
    }

    return root != NULL;
}

MB_JSON *FirebaseJsonBase::parse(const char *raw)
{
    const char *s = NULL;
    MB_JSON *e = MB_JSON_ParseWithOpts(raw, &s, 1);
    errorPos = (s - raw != (int)strlen(raw)) ? s - raw : -1;
    return e;
}

void FirebaseJsonBase::prepareRoot()
{
    if (root == NULL)
    {
        if (root_type == Root_Type_JSONArray)
            root = MB_JSON_CreateArray();
        else
            root = MB_JSON_CreateObject();
    }
}

void FirebaseJsonBase::searchElements(MB_VECTOR<MB_String> &keys, MB_JSON *parent, struct search_result_t &r)
{
    MB_JSON *e = parent;
    for (size_t i = 0; i < keys.size(); i++)
    {
        r.status = key_status_not_existed;
        e = getElement(parent, keys[i].c_str(), r);
        r.stopIndex = i;
        if (r.status != key_status_existed)
        {
            if (i == 0)
                r.parent = parent;
            break;
        }
        r.parent = parent;
        r.foundIndex = i;
        parent = e;
    }
}

MB_JSON *FirebaseJsonBase::getElement(MB_JSON *parent, const char *key, struct search_result_t &r)
{
    MB_JSON *e = NULL;
    bool isArrKey = isArrayKey(key);
    int index = isArrKey ? getArrIndex(key) : -1;
    if ((isArray(parent) && !isArrKey) || (isObject(parent) && isArrKey))
        r.status = key_status_mistype;
    else if (isArray(parent) && isArrKey)
    {
        e = MB_JSON_GetArrayItem(parent, index);
        if (e == NULL)
            r.status = key_status_out_of_range;
    }
    else if (isObject(parent) && !isArrKey)
    {
        e = MB_JSON_GetObjectItemCaseSensitive(parent, key);
        if (e == NULL)
            r.status = key_status_not_existed;
    }

    if (e == NULL)
        return parent;

    r.status = key_status_existed;
    return e;
}

void FirebaseJsonBase::mAdd(MB_VECTOR<MB_String> keys, MB_JSON **parent, int beginIndex, MB_JSON *value)
{
    MB_JSON *m_parent = *parent;

    for (size_t i = beginIndex; i < keys.size(); i++)
    {
        bool isArrKey = isArrayKey(keys[i].c_str());
        int index = isArrKey ? getArrIndex(keys[i].c_str()) : -1;
        MB_JSON *e = (i < keys.size() - 1) ? (isArrayKey(keys[i + 1].c_str()) ? MB_JSON_CreateArray() : MB_JSON_CreateObject()) : value;

        if (isArray(m_parent))
        {
            if (isArrKey)
                m_parent = addArray(m_parent, e, index + 1);
            else
                MB_JSON_AddItemToArray(m_parent, e);
        }
        else
        {
            if (isArrKey)
            {
                if ((int)i == beginIndex)
                {
                    m_parent = MB_JSON_CreateArray();
                    MB_JSON_Delete(*parent);
                    *parent = m_parent;
                }
                m_parent = addArray(m_parent, e, index + 1);
            }
            else
            {
                MB_JSON_AddItemToObject(m_parent, keys[i].c_str(), e);
                m_parent = e;
            }
        }
    }
}

void FirebaseJsonBase::makeList(const MB_String &str, MB_VECTOR<MB_String> &keys, char delim)
{
    clearList(keys);
    size_t current, previous = 0;
    current = str.find(delim, previous);
    MB_String s;
    while (current != MB_String::npos)
    {
        pushLish(str.substr(previous, current - previous), keys);
        previous = current + 1;
        current = str.find(delim, previous);
    }
    pushLish(str.substr(previous, current - previous), keys);
}

void FirebaseJsonBase::pushLish(const MB_String &str, MB_VECTOR<MB_String> &keys)
{
    MB_String s = str;
    s.trim();
    if (s.length() > 0)
        keys.push_back(s);
}

void FirebaseJsonBase::clearList(MB_VECTOR<MB_String> &keys)
{
    size_t len = keys.size();
    for (size_t i = 0; i < len; i++)
        keys[i].clear();
    for (int i = len - 1; i >= 0; i--)
        keys.erase(keys.begin() + i);
    keys.clear();
#if defined(MB_USE_STD_VECTOR)
    MB_VECTOR<MB_String>().swap(keys);
#endif
}

bool FirebaseJsonBase::isArray(MB_JSON *e)
{
    return MB_JSON_IsArray(e);
}

bool FirebaseJsonBase::isObject(MB_JSON *e)
{
    return MB_JSON_IsObject(e);
}
MB_JSON *FirebaseJsonBase::addArray(MB_JSON *parent, MB_JSON *e, size_t size)
{
    for (size_t i = 0; i < size - 1; i++)
        MB_JSON_AddItemToArray(parent, MB_JSON_CreateNull());
    MB_JSON_AddItemToArray(parent, e);
    return e;
}

void FirebaseJsonBase::appendArray(MB_VECTOR<MB_String> &keys, struct search_result_t &r, MB_JSON *parent, MB_JSON *value)
{
    MB_JSON *item = NULL;

    int index = getArrIndex(keys[r.stopIndex].c_str());

    if (r.foundIndex > -1)
    {
        if (isArray(parent))
            parent = MB_JSON_GetArrayItem(parent, getArrIndex(keys[r.foundIndex].c_str()));
        else
            parent = MB_JSON_GetObjectItemCaseSensitive(parent, keys[r.foundIndex].c_str());
    }

    if (isArray(parent))
    {
        int arrSize = MB_JSON_GetArraySize(parent);

        if (r.stopIndex < (int)keys.size() - 1)
        {
            item = isArrayKey(keys[r.stopIndex + 1].c_str()) ? MB_JSON_CreateArray() : MB_JSON_CreateObject();
            mAdd(keys, &item, r.stopIndex + 1, value);
        }
        else
            item = value;

        for (int i = arrSize; i < index; i++)
            MB_JSON_AddItemToArray(parent, MB_JSON_CreateNull());

        MB_JSON_AddItemToArray(parent, item);
    }
    else
        MB_JSON_Delete(value);
}

void FirebaseJsonBase::replaceItem(MB_VECTOR<MB_String> &keys, struct search_result_t &r, MB_JSON *parent, MB_JSON *value)
{
    if (r.foundIndex == -1)
    {
        if (r.status == key_status_not_existed)
            mAdd(keys, &parent, 0, value);
        else if (r.status == key_status_mistype)
        {
            MB_JSON *m_parent = MB_JSON_CreateObject();
            mAdd(keys, &m_parent, 0, value);
            *parent = *m_parent;
        }
        else
            MB_JSON_Delete(value);
    }
    else
    {
        if (r.status == key_status_not_existed && !isArrayKey(keys[r.stopIndex].c_str()))
        {
            MB_JSON *curItem = isArray(parent) ? MB_JSON_GetArrayItem(parent, getArrIndex(keys[r.foundIndex].c_str())) : MB_JSON_GetObjectItem(parent, keys[r.foundIndex].c_str());
            if (isObject(curItem))
            {
                mAdd(keys, &curItem, r.foundIndex + 1, value);
                return;
            }
        }

        MB_JSON *item = NULL;

        if ((r.status == key_status_mistype ? r.stopIndex : r.foundIndex) < (int)keys.size() - 1)
        {
            item = isArrayKey(keys[r.stopIndex].c_str()) ? MB_JSON_CreateArray() : MB_JSON_CreateObject();
            mAdd(keys, &item, r.stopIndex, value);
        }
        else
            item = value;

        replace(keys, r, parent, item);
    }
}

void FirebaseJsonBase::replace(MB_VECTOR<MB_String> &keys, struct search_result_t &r, MB_JSON *parent, MB_JSON *item)
{
    if (isArray(parent))
        MB_JSON_ReplaceItemInArray(parent, getArrIndex(keys[r.foundIndex].c_str()), item);
    else
        MB_JSON_ReplaceItemInObject(parent, keys[r.foundIndex].c_str(), item);
}

size_t FirebaseJsonBase::mIteratorBegin(MB_JSON *parent)
{
    mIteratorEnd();
    char *p = MB_JSON_PrintUnformatted(parent);
    if (p == NULL)
        return 0;

    buf = p;
    MB_JSON_free(p);
    iterator_data.buf_size = buf.length();
    int index = -1;
    mIterate(parent, index);
    return iterator_data.result.size();
}

size_t FirebaseJsonBase::mIteratorBegin(MB_JSON *parent, MB_VECTOR<MB_String> *keys)
{
    mIteratorEnd();

    if (keys == NULL)
        return 0;

    int index = -1;
    mIterate(parent, index);

    return iterator_data.result.size();
}

void FirebaseJsonBase::mIteratorEnd(bool clearBuf)
{
    if (clearBuf)
        buf.clear();
    iterator_data.path.clear();
    iterator_data.buf_size = 0;
    iterator_data.buf_offset = 0;
    iterator_data.result.clear();
    iterator_data.depth = -1;
    iterator_data._depth = 0;
    if (iterator_data.parentArr != NULL)
        MB_JSON_Delete(iterator_data.parentArr);
    iterator_data.parentArr = NULL;
}

void FirebaseJsonBase::mIterate(MB_JSON *parent, int &arrIndex)
{
    if (!parent)
        return;

    bool isAr = isArray(parent);

    if (isAr)
        arrIndex = 0;

    MB_JSON *e = parent->child;
    if (e)
    {
        iterator_data.depth++;
        while (e)
        {

            if (isArray(e) || isObject(e))
                mCollectIterator(e, e->string ? JSON_OBJECT : JSON_ARRAY, arrIndex);

            if (isArray(e))
            {
                MB_JSON *item = e->child;
                int _arrIndex = 0;

                if (e->child)
                {
                    iterator_data.depth++;
                    while (item)
                    {

                        if (isArray(item) || isObject(item))
                            mIterate(item, _arrIndex);
                        else
                            mCollectIterator(item, item->string ? JSON_OBJECT : JSON_ARRAY, _arrIndex);
                        item = item->next;
                        _arrIndex++;
                    }
                }
            }
            else if (isObject(e))
                mIterate(e, arrIndex);
            else
                mCollectIterator(e, e->string ? JSON_OBJECT : JSON_ARRAY, arrIndex);

            e = e->next;

            if (isAr)
                arrIndex++;
        }
    }
}

void FirebaseJsonBase::mCollectIterator(MB_JSON *e, int type, int &arrIndex)
{
    struct iterator_result_t result;

    if (e->string)
    {
        size_t pos = buf.find((const char *)e->string, iterator_data.buf_offset);
        if (pos != MB_String::npos)
        {
            result.ofs1 = pos;
            result.len1 = strlen(e->string);
            iterator_data.buf_offset = (e->type != MB_JSON_Object && e->type != MB_JSON_Array) ? pos + result.len1 : pos;
        }
    }

    char *p = MB_JSON_PrintUnformatted(e);
    if (p)
    {
        int i = iterator_data.buf_offset;
        size_t pos = buf.find(p, i);
        if (pos != MB_String::npos)
        {
            result.ofs2 = pos - result.ofs1 - result.len1;
            result.len2 = strlen(p);
            MB_JSON_free(p);
            iterator_data.buf_offset = (e->type != MB_JSON_Object && e->type != MB_JSON_Array) ? pos + result.len2 : pos;
        }
    }
    result.type = type;
    result.depth = iterator_data.depth;
    iterator_data.result.push_back(result);
}

int FirebaseJsonBase::mIteratorGet(size_t index, int &type, String &key, String &value)
{
    key.remove(0, key.length());
    value.remove(0, value.length());
    int depth = -1;

    if (buf.length() == iterator_data.buf_size)
    {
        if (index > iterator_data.result.size() - 1)
            return depth;

        if (iterator_data.result[index].len1 > 0)
        {
            char *m_key = (char *)newP(iterator_data.result[index].len1 + 1);
            if (m_key)
            {
                memset(m_key, 0, iterator_data.result[index].len1 + 1);
                strncpy(m_key, &buf[iterator_data.result[index].ofs1], iterator_data.result[index].len1);
                key = m_key;
                delP(&m_key);
            }
        }

        char *m_val = (char *)newP(iterator_data.result[index].len2 + 1);
        if (m_val)
        {
            memset(m_val, 0, iterator_data.result[index].len2 + 1);
            int ofs = iterator_data.result[index].ofs1 + iterator_data.result[index].len1 + iterator_data.result[index].ofs2;
            int len = iterator_data.result[index].len2;

            if (iterator_data.result[index].type == JSON_STRING)
            {
                if (buf[ofs] == '"')
                    ofs++;
                if (buf[ofs + len - 1] == '"')
                    len--;
            }

            strncpy(m_val, &buf[ofs], len);
            value = m_val;
            delP(&m_val);
        }
        type = iterator_data.result[index].type;
        depth = iterator_data.result[index].depth;
    }
    return depth;
}

struct FirebaseJsonBase::fb_js_iterator_value_t FirebaseJsonBase::mValueAt(size_t index)
{
    struct fb_js_iterator_value_t value;
    int depth = mIteratorGet(index, value.type, value.key, value.value);
    value.depth = depth;
    return value;
}

void FirebaseJsonBase::toBuf(fb_json_serialize_mode mode)
{
    if (root != NULL)
    {
        char *out = mode == fb_json_serialize_mode_pretty ? MB_JSON_Print(root) : MB_JSON_PrintUnformatted(root);
        if (out)
        {
            buf = out;
            MB_JSON_free(out);
        }
    }
}

bool FirebaseJsonBase::mReadClient(Client *client)
{
    // blocking read
    buf.clear();
    if (readClient(client, buf))
    {
        if (root != NULL)
            MB_JSON_Delete(root);
        root = parse(buf.c_str());
        buf.clear();
        return root != NULL;
    }
    return false;
}

bool FirebaseJsonBase::mReadStream(Stream *s, int timeoutMS)
{
    // non-blocking read
    if (readStream(s, serData, buf, true, timeoutMS))
    {
        if (root != NULL)
            MB_JSON_Delete(root);
        root = parse(buf.c_str());
        buf.clear();
        return root != NULL;
    }
    return false;
}

#if defined(ESP32_SD_FAT_INCLUDED)
bool FirebaseJsonBase::mReadSdFat(SD_FAT_FILE &file, int timeoutMS)
{
    // non-blocking read
    if (readSdFatFile(file, serData, buf, true, timeoutMS))
    {
        if (root != NULL)
            MB_JSON_Delete(root);
        root = parse(buf.c_str());
        buf.clear();
        return root != NULL;
    }
    return false;
}
#endif

const char *FirebaseJsonBase::mRaw()
{
    toBuf(fb_json_serialize_mode_plain);
    return buf.c_str();
}

bool FirebaseJsonBase::mRemove(const char *path)
{
    bool ret = false;
    prepareRoot();
    MB_VECTOR<MB_String> keys = MB_VECTOR<MB_String>();
    makeList(path, keys, '/');

    if (keys.size() > 0)
    {
        if (isArrayKey(keys[0].c_str()) && root_type == Root_Type_JSON)
        {
            clearList(keys);
            return false;
        }
    }

    MB_JSON *parent = root;

    struct search_result_t r;
    searchElements(keys, parent, r);
    parent = r.parent;

    if (r.status == key_status_existed)
    {
        ret = true;
        if (isArray(parent))
            MB_JSON_DeleteItemFromArray(parent, getArrIndex(keys[r.stopIndex].c_str()));
        else
        {
            MB_JSON_DeleteItemFromObjectCaseSensitive(parent, keys[r.stopIndex].c_str());
            if (parent->child == NULL && r.stopIndex > 0)
            {
                MB_String path;
                mGetPath(path, keys, 0, r.stopIndex - 1);
                mRemove(path.c_str());
            }
        }
    }

    clearList(keys);
    return ret;
}

void FirebaseJsonBase::mGetPath(MB_String &path, MB_VECTOR<MB_String> paths, int begin, int end)
{
    if (end < 0 || end >= (int)paths.size())
        end = paths.size() - 1;
    if (begin < 0 || begin > end)
        begin = 0;

    for (int i = begin; i <= end; i++)
    {
        if (i > 0)
            path += (const char *)MBSTRING_FLASH_MCR("/");
        path += paths[i].c_str();
    }
}

size_t FirebaseJsonBase::mGetSerializedBufferLength(bool prettify)
{
    if (!root)
        return 0;
    return MB_JSON_SerializedBufferLength(root, prettify);
}

void FirebaseJsonBase::mSetFloatDigits(uint8_t digits)
{
    floatDigits = digits;
}

void FirebaseJsonBase::mSetDoubleDigits(uint8_t digits)
{
    doubleDigits = digits;
}

int FirebaseJsonBase::mResponseCode()
{
    return httpCode;
}

bool FirebaseJsonBase::mGet(MB_JSON *parent, FirebaseJsonData *result, const char *path, bool prettify)
{
    bool ret = false;
    prepareRoot();
    MB_VECTOR<MB_String> keys = MB_VECTOR<MB_String>();
    makeList(path, keys, '/');

    if (keys.size() > 0)
    {
        if (isArrayKey(keys[0].c_str()) && root_type == Root_Type_JSON)
        {
            clearList(keys);
            return false;
        }
    }

    MB_JSON *_parent = parent;
    struct search_result_t r;
    searchElements(keys, parent, r);
    _parent = r.parent;

    if (r.status == key_status_existed)
    {
        MB_JSON *data = NULL;
        if (isArray(_parent))
            data = MB_JSON_GetArrayItem(_parent, getArrIndex(keys[r.stopIndex].c_str()));
        else
            data = MB_JSON_GetObjectItemCaseSensitive(_parent, keys[r.stopIndex].c_str());

        if (data != NULL)
        {
            if (result != NULL)
            {
                result->clear();
                char *p = prettify ? MB_JSON_Print(data) : MB_JSON_PrintUnformatted(data);
                result->stringValue = p;
                MB_JSON_free(p);
                result->type_num = data->type;
                result->success = true;
                mSetElementType(result);
            }
            ret = true;
        }
    }

    clearList(keys);
    return ret;
}

void FirebaseJsonBase::mSetResInt(FirebaseJsonData *data, const char *value)
{
    if (strlen(value) > 0)
    {
        char *pEnd;
#if !defined(__AVR__)
        value[0] == '-' ? data->iVal.int64 = strtoll(value, &pEnd, 10) : data->iVal.uint64 = strtoull(value, &pEnd, 10);
#else
        value[0] == '-' ? data->iVal.int64 = strtol(value, &pEnd, 10) : data->iVal.uint64 = strtoull_alt(value);
#endif
    }
    else
        data->iVal = {0};

    data->intValue = data->iVal.int32;
    data->boolValue = data->iVal.int32 > 0;
}

void FirebaseJsonBase::mSetResFloat(FirebaseJsonData *data, const char *value)
{
    if (strlen(value) > 0)
    {
        char *pEnd;
        data->fVal.setd(strtod(value, &pEnd));
    }
    else
        data->fVal.setd(0);

    data->doubleValue = data->fVal.d;
    data->floatValue = data->fVal.f;
}

void FirebaseJsonBase::mSetElementType(FirebaseJsonData *result)
{
    char *buf = (char *)newP(32);
    if (result->type_num == MB_JSON_Invalid)
    {
        strcpy(buf, (const char *)MBSTRING_FLASH_MCR("undefined"));
        result->typeNum = JSON_UNDEFINED;
    }
    else if (result->type_num == MB_JSON_Object)
    {
        strcpy(buf, (const char *)MBSTRING_FLASH_MCR("object"));
        result->typeNum = JSON_OBJECT;
    }
    else if (result->type_num == MB_JSON_Array)
    {
        strcpy(buf, (const char *)MBSTRING_FLASH_MCR("array"));
        result->typeNum = JSON_ARRAY;
    }
    else if (result->type_num == MB_JSON_String)
    {
        if (result->stringValue.c_str()[0] == '"')
            result->stringValue.remove(0, 1);
        if (result->stringValue.c_str()[result->stringValue.length() - 1] == '"')
            result->stringValue.remove(result->stringValue.length() - 1, 1);

        strcpy(buf, (const char *)MBSTRING_FLASH_MCR("string"));
        result->typeNum = JSON_STRING;

        // try casting the string to numbers
        if (result->stringValue.length() <= 32)
        {
            mSetResInt(result, result->stringValue.c_str());
            mSetResFloat(result, result->stringValue.c_str());
        }
    }
    else if (result->type_num == MB_JSON_NULL)
    {
        strcpy(buf, (const char *)MBSTRING_FLASH_MCR("null"));
        result->typeNum = JSON_NULL;
    }
    else if (result->type_num == MB_JSON_False || result->type_num == MB_JSON_True)
    {
        strcpy(buf, (const char *)MBSTRING_FLASH_MCR("boolean"));
        bool t = strcmp(result->stringValue.c_str(), (const char *)MBSTRING_FLASH_MCR("true")) == 0;
        result->typeNum = JSON_BOOL;

        result->iVal = {t};
        result->fVal.setd(t);
        result->boolValue = t;
        result->intValue = t;
        result->floatValue = t;
        result->doubleValue = t;
    }
    else if (result->type_num == MB_JSON_Number || result->type_num == MB_JSON_Raw)
    {
        mSetResInt(result, result->stringValue.c_str());
        mSetResFloat(result, result->stringValue.c_str());

        if (strpos(result->stringValue.c_str(), (const char *)MBSTRING_FLASH_MCR("."), 0) > -1)
        {
            double d = atof(result->stringValue.c_str());
            if (d > 0x7fffffff)
            {
                strcpy(buf, (const char *)MBSTRING_FLASH_MCR("double"));
                result->typeNum = JSON_DOUBLE;
            }
            else
            {
                strcpy(buf, (const char *)MBSTRING_FLASH_MCR("float"));
                result->typeNum = JSON_FLOAT;
            }
        }
        else
        {
            strcpy(buf, (const char *)MBSTRING_FLASH_MCR("int"));
            result->typeNum = JSON_INT;
        }
    }

    result->type = buf;
    delP(&buf);
}

void FirebaseJsonBase::mSet(const char *path, MB_JSON *value)
{
    prepareRoot();
    MB_VECTOR<MB_String> keys = MB_VECTOR<MB_String>();
    makeList(path, keys, '/');

    if (keys.size() > 0)
    {
        if ((isArrayKey(keys[0].c_str()) && root_type == Root_Type_JSON) || (!isArrayKey(keys[0].c_str()) && root_type == Root_Type_JSONArray))
        {
            MB_JSON_Delete(value);
            clearList(keys);
            return;
        }
    }

    MB_JSON *parent = root;
    struct search_result_t r;
    searchElements(keys, parent, r);
    parent = r.parent;

    if (value == NULL)
        value = MB_JSON_CreateNull();

    if (r.status == key_status_mistype || r.status == key_status_not_existed)
        replaceItem(keys, r, parent, value);
    else if (r.status == key_status_out_of_range)
        appendArray(keys, r, parent, value);
    else if (r.status == key_status_existed)
        replace(keys, r, parent, value);
    else
        MB_JSON_Delete(value);

    clearList(keys);
}

#if defined(__AVR__)
unsigned long long FirebaseJsonBase::strtoull_alt(const char *s)
{
    unsigned long long sum = 0;
    while (*s)
    {
        sum = sum * 10 + (*s++ - '0');
    }
    return sum;
}
#endif

FirebaseJson &FirebaseJson::operator=(FirebaseJson other)
{
    if (isObject(other.root))
        mCopy(other);
    return *this;
}

FirebaseJson::FirebaseJson(FirebaseJson &other)
{
    if (isObject(other.root))
        mCopy(other);
}

FirebaseJson::~FirebaseJson()
{
    clear();
}

FirebaseJson &FirebaseJson::nAdd(const char *key, MB_JSON *value)
{
    prepareRoot();
    MB_VECTOR<MB_String> keys = MB_VECTOR<MB_String>();
    // makeList(key, keys, '/');
    MB_String ky = key;
    keys.push_back(ky);

    if (value == NULL)
        value = MB_JSON_CreateNull();

    if (keys.size() > 0)
    {
        if (!isArrayKey(keys[0].c_str()) || root_type == Root_Type_JSONArray)
            mAdd(keys, &root, 0, value);
    }

    clearList(keys);

    return *this;
}

FirebaseJson &FirebaseJson::clear()
{
    mClear();
    return *this;
}

FirebaseJsonArray::~FirebaseJsonArray()
{
    mClear();
};

FirebaseJsonArray &FirebaseJsonArray::operator=(FirebaseJsonArray other)
{
    if (isArray(other.root))
        mCopy(other);
    return *this;
}

FirebaseJsonArray::FirebaseJsonArray(FirebaseJsonArray &other)
{
    if (isArray(other.root))
        mCopy(other);
}

FirebaseJsonArray &FirebaseJsonArray::nAdd(MB_JSON *value)
{
    if (root_type != Root_Type_JSONArray)
        mClear();

    root_type = Root_Type_JSONArray;

    prepareRoot();

    if (value == NULL)
        value = MB_JSON_CreateNull();

    MB_JSON_AddItemToArray(root, value);

    return *this;
}

bool FirebaseJsonArray::mGetIdx(FirebaseJsonData *result, int index, bool prettify)
{
    bool ret = false;
    prepareRoot();

    result->clear();

    MB_JSON *data = NULL;
    if (isArray(root))
        data = MB_JSON_GetArrayItem(root, index);

    if (data != NULL)
    {
        char *p = prettify ? MB_JSON_Print(data) : MB_JSON_PrintUnformatted(data);
        result->stringValue = p;
        MB_JSON_free(p);
        result->type_num = data->type;
        result->success = true;
        mSetElementType(result);
        ret = true;
    }
    return ret;
}

bool FirebaseJsonArray::mSetIdx(int index, MB_JSON *value)
{
    if (root_type != Root_Type_JSONArray)
        mClear();

    root_type = Root_Type_JSONArray;

    prepareRoot();

    int size = MB_JSON_GetArraySize(root);
    if (index < size)
        return MB_JSON_ReplaceItemInArray(root, index, value);
    else
    {
        while (size < index)
        {
            MB_JSON_AddItemToArray(root, MB_JSON_CreateNull());
            size++;
        }
        MB_JSON_AddItemToArray(root, value);
    }
    return true;
}

bool FirebaseJsonArray::mRemoveIdx(int index)
{
    int size = MB_JSON_GetArraySize(root);
    if (index < size)
    {
        MB_JSON_DeleteItemFromArray(root, index);
        return size != MB_JSON_GetArraySize(root);
    }
    return false;
}

FirebaseJsonArray &FirebaseJsonArray::clear()
{
    mClear();
    return *this;
}

FirebaseJsonArray &FirebaseJsonArray::add(FirebaseJson &value)
{
    MB_JSON *e = MB_JSON_Duplicate(value.root, true);
    nAdd(e);
    return *this;
}

FirebaseJsonArray &FirebaseJsonArray::add(FirebaseJsonArray &value)
{
    MB_JSON *e = MB_JSON_Duplicate(value.root, true);
    nAdd(e);
    return *this;
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
    if (typeNum != FirebaseJson::JSON_ARRAY || !success || stringValue.length() == 0)
        return false;
    return getArray(stringValue.c_str(), jsonArray);
}

bool FirebaseJsonData::mGetArray(const char *source, FirebaseJsonArray &jsonArray)
{

    if (jsonArray.root != NULL)
        MB_JSON_Delete(jsonArray.root);

    jsonArray.root = jsonArray.parse(source);

    return jsonArray.root != NULL;
}

bool FirebaseJsonData::getJSON(FirebaseJson &json)
{
    if (typeNum != FirebaseJson::JSON_OBJECT || !success || stringValue.length() == 0)
        return false;
    return getJSON(stringValue.c_str(), json);
}

bool FirebaseJsonData::mGetJSON(const char *source, FirebaseJson &json)
{
    if (json.root != NULL)
        MB_JSON_Delete(json.root);

    json.root = json.parse(source);

    return json.root != NULL;
}

size_t FirebaseJsonData::getReservedLen(size_t len)
{
    int blen = len + 1;

    int newlen = (blen / 4) * 4;

    if (newlen < blen)
        newlen += 4;

    return (size_t)newlen;
}

void FirebaseJsonData::delP(void *ptr)
{
    void **p = (void **)ptr;
    if (*p)
    {
        free(*p);
        *p = 0;
    }
}

void *FirebaseJsonData::newP(size_t len)
{
    void *p;
    size_t newLen = getReservedLen(len);
#if defined(BOARD_HAS_PSRAM) && defined(MB_STRING_USE_PSRAM)
    if (ESP.getPsramSize() > 0)
        p = (void *)ps_malloc(newLen);
    else
        p = (void *)malloc(newLen);
    if (!p)
        return NULL;

#else

#if defined(ESP8266_USE_EXTERNAL_HEAP)
    ESP.setExternalHeap();
#endif

    p = (void *)malloc(newLen);
    bool nn = p ? true : false;

#if defined(ESP8266_USE_EXTERNAL_HEAP)
    ESP.resetHeap();
#endif

    if (!nn)
        return NULL;

#endif
    memset(p, 0, newLen);
    return p;
}

void FirebaseJsonData::clear()
{
    stringValue.remove(0, stringValue.length());
    iVal = {0};
    fVal.setd(0);
    intValue = 0;
    floatValue = 0;
    doubleValue = 0;
    boolValue = false;
    type.remove(0, type.length());
    typeNum = 0;
    success = false;
}

#endif