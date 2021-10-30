
/*
 * FirebaseJson, version 2.5.3
 * 
 * The Easiest Arduino library to parse, create and edit JSON object using a relative path.
 * 
 * October 25, 2021
 * 
 * Features
 * - Using path to access node element in search style e.g. json.get(result,"a/b/c") 
 * - Able to search with key/path and value in JSON object and array
 * - Serializing to writable objects e.g. String, C/C++ string, Client (WiFi and Ethernet), File and Hardware Serial.
 * - Deserializing from const char, char array, string literal and stream e.g. Client (WiFi and Ethernet), File and 
 *   Hardware Serial.
 * - Use managed class, FirebaseJsonData to keep the deserialized result, which can be casted to any primitive data types.
 * 
 * 
 * The MIT License (MIT)
 * Copyright (c) 2021 K. Suwatchai (Mobizt)
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
    cJSON_InitHooks(&cJSON_hooks);
}

FirebaseJsonBase::~FirebaseJsonBase()
{
    mClear();
}

FirebaseJsonBase &FirebaseJsonBase::mClear()
{
    mIteratorEnd();
    if (root != NULL)
        cJSON_Delete(root);
    root = NULL;
    clearS(buf);
    errorPos = -1;
    return *this;
}
void FirebaseJsonBase::mCopy(FirebaseJsonBase &other)
{
    mClear();
    this->root = cJSON_Duplicate(other.root, true);
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
    root = parse(raw);
    return root != NULL;
}

cJSON *FirebaseJsonBase::parse(const char *raw)
{
    const char *s = NULL;
    cJSON *e = cJSON_ParseWithOpts(raw, &s, 1);
    errorPos = (s - raw != (int)strlen(raw)) ? s - raw : -1;
    return e;
}

void FirebaseJsonBase::prepareRoot()
{
    if (root == NULL)
    {
        if (root_type == Root_Type_JSONArray)
            root = cJSON_CreateArray();
        else
            root = cJSON_CreateObject();
    }
}

void FirebaseJsonBase::searchElements(std::vector<MBSTRING> &keys, cJSON *parent, struct search_result_t &r)
{
    cJSON *e = parent;
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

cJSON *FirebaseJsonBase::getElement(cJSON *parent, const char *key, struct search_result_t &r)
{
    cJSON *e = NULL;
    bool isArrKey = isArrayKey(key);
    int index = isArrKey ? getArrIndex(key) : -1;
    if ((isArray(parent) && !isArrKey) || (isObject(parent) && isArrKey))
        r.status = key_status_mistype;
    else if (isArray(parent) && isArrKey)
    {
        e = cJSON_GetArrayItem(parent, index);
        if (e == NULL)
            r.status = key_status_out_of_range;
    }
    else if (isObject(parent) && !isArrKey)
    {
        e = cJSON_GetObjectItemCaseSensitive(parent, key);
        if (e == NULL)
            r.status = key_status_not_existed;
    }

    if (e == NULL)
        return parent;

    r.status = key_status_existed;
    return e;
}

void FirebaseJsonBase::mAdd(std::vector<MBSTRING> keys, cJSON **parent, int beginIndex, cJSON *value)
{
    cJSON *m_parent = *parent;

    for (size_t i = beginIndex; i < keys.size(); i++)
    {
        bool isArrKey = isArrayKey(keys[i].c_str());
        int index = isArrKey ? getArrIndex(keys[i].c_str()) : -1;
        cJSON *e = (i < keys.size() - 1) ? (isArrayKey(keys[i + 1].c_str()) ? cJSON_CreateArray() : cJSON_CreateObject()) : value;

        if (isArray(m_parent))
        {
            if (isArrKey)
                m_parent = addArray(m_parent, e, index + 1);
            else
                cJSON_AddItemToArray(m_parent, e);
        }
        else
        {
            if (isArrKey)
            {
                if ((int)i == beginIndex)
                {
                    m_parent = cJSON_CreateArray();
                    cJSON_Delete(*parent);
                    *parent = m_parent;
                }
                m_parent = addArray(m_parent, e, index + 1);
            }
            else
            {
                cJSON_AddItemToObject(m_parent, keys[i].c_str(), e);
                m_parent = e;
            }
        }
    }
}

void FirebaseJsonBase::makeList(const char *str, std::vector<MBSTRING> &keys, char delim)
{
    clearList(keys);

    int current = 0, previous = 0;
    current = strpos(str, delim, 0);
    MBSTRING s;
    while (current != -1)
    {
        clearS(s);
        substr(s, str, previous, current - previous);
        trim(s);
        if (s.length() > 0)
            keys.push_back(s);

        previous = current + 1;
        current = strpos(str, delim, previous);
    }

    clearS(s);

    if (previous > 0 && current == -1)
        substr(s, str, previous, strlen(str) - previous);
    else
        s = str;

    trim(s);
    if (s.length() > 0)
        keys.push_back(s);
    clearS(s);
}

void FirebaseJsonBase::clearList(std::vector<MBSTRING> &keys)
{
    size_t len = keys.size();
    for (size_t i = 0; i < len; i++)
        clearS(keys[i]);
    for (int i = len - 1; i >= 0; i--)
        keys.erase(keys.begin() + i);
    keys.clear();
    std::vector<MBSTRING>().swap(keys);
}

bool FirebaseJsonBase::isArray(cJSON *e)
{
    return cJSON_IsArray(e);
}

bool FirebaseJsonBase::isObject(cJSON *e)
{
    return cJSON_IsObject(e);
}
cJSON *FirebaseJsonBase::addArray(cJSON *parent, cJSON *e, size_t size)
{
    for (size_t i = 0; i < size - 1; i++)
        cJSON_AddItemToArray(parent, cJSON_CreateNull());
    cJSON_AddItemToArray(parent, e);
    return e;
}

void FirebaseJsonBase::appendArray(std::vector<MBSTRING> &keys, struct search_result_t &r, cJSON *parent, cJSON *value)
{
    cJSON *item = NULL;

    int index = getArrIndex(keys[r.stopIndex].c_str());

    if (r.foundIndex > -1)
    {
        if (isArray(parent))
            parent = cJSON_GetArrayItem(parent, getArrIndex(keys[r.foundIndex].c_str()));
        else
            parent = cJSON_GetObjectItemCaseSensitive(parent, keys[r.foundIndex].c_str());
    }

    if (isArray(parent))
    {
        int arrSize = cJSON_GetArraySize(parent);

        if (r.stopIndex < (int)keys.size() - 1)
        {
            item = isArrayKey(keys[r.stopIndex + 1].c_str()) ? cJSON_CreateArray() : cJSON_CreateObject();
            mAdd(keys, &item, r.stopIndex + 1, value);
        }
        else
            item = value;

        for (int i = arrSize; i < index; i++)
            cJSON_AddItemToArray(parent, cJSON_CreateNull());

        cJSON_AddItemToArray(parent, item);
    }
    else
        cJSON_Delete(value);
}

void FirebaseJsonBase::replaceItem(std::vector<MBSTRING> &keys, struct search_result_t &r, cJSON *parent, cJSON *value)
{
    if (r.foundIndex == -1)
    {
        if (r.status == key_status_not_existed)
            mAdd(keys, &parent, 0, value);
        else if (r.status == key_status_mistype)
        {
            cJSON *m_parent = cJSON_CreateObject();
            mAdd(keys, &m_parent, 0, value);
            *parent = *m_parent;
        }
        else
            cJSON_Delete(value);
    }
    else
    {
        if (r.status == key_status_not_existed && !isArrayKey(keys[r.stopIndex].c_str()))
        {
            cJSON *curItem = isArray(parent) ? cJSON_GetArrayItem(parent, getArrIndex(keys[r.foundIndex].c_str())) : cJSON_GetObjectItem(parent, keys[r.foundIndex].c_str());
            if (isObject(curItem))
            {
                mAdd(keys, &curItem, r.foundIndex + 1, value);
                return;
            }
        }

        cJSON *item = NULL;

        if ((r.status == key_status_mistype ? r.stopIndex : r.foundIndex) < (int)keys.size() - 1)
        {
            item = isArrayKey(keys[r.stopIndex].c_str()) ? cJSON_CreateArray() : cJSON_CreateObject();
            mAdd(keys, &item, r.stopIndex, value);
        }
        else
            item = value;

        replace(keys, r, parent, item);
    }
}

void FirebaseJsonBase::replace(std::vector<MBSTRING> &keys, struct search_result_t &r, cJSON *parent, cJSON *item)
{
    if (isArray(parent))
        cJSON_ReplaceItemInArray(parent, getArrIndex(keys[r.foundIndex].c_str()), item);
    else
        cJSON_ReplaceItemInObject(parent, keys[r.foundIndex].c_str(), item);
}

size_t FirebaseJsonBase::mIteratorBegin(cJSON *parent)
{
    mIteratorEnd();
    char *p = cJSON_PrintUnformatted(parent);
    if (p == NULL)
        return 0;

    buf = p;
    cJSON_free(p);
    iterator_data.buf_size = buf.length();
    int index = -1;
    mIterate(parent, index, NULL);
    return iterator_data.result.size();
}

size_t FirebaseJsonBase::mIteratorBegin(cJSON *parent, std::vector<MBSTRING> *keys, struct fb_js_search_criteria_t *criteria)
{
    mIteratorEnd();

    if (keys == NULL || criteria == NULL)
        return 0;

    iterator_data.searchEnable = true;
    iterator_data.searchKeys = keys;
    int index = -1;
    mIterate(parent, index, criteria);

    if (criteria->searchAll)
        iterator_data.searchFinished = iterator_data.matchesCount > 0;
    return iterator_data.result.size();
}

void FirebaseJsonBase::mIteratorEnd(bool clearBuf)
{
    if (clearBuf)
        clearS(buf);
    clearS(iterator_data.path);
    iterator_data.buf_size = 0;
    iterator_data.buf_offset = 0;
    iterator_data.result.clear();
    clearList(iterator_data.pathList);
    iterator_data.depth = -1;
    iterator_data._depth = 0;
    iterator_data.searchEnable = false;
    iterator_data.searchFinished = false;
    iterator_data.searchKeyDepth = -1;
    iterator_data.matchesCount = 0;
    iterator_data.searchKeys = NULL;
    if (iterator_data.parentArr != NULL)
        cJSON_Delete(iterator_data.parentArr);
    iterator_data.parentArr = NULL;
}

void FirebaseJsonBase::mIterate(cJSON *parent, int &arrIndex, struct fb_js_search_criteria_t *criteria)
{
    if (!parent)
        return;

    if (iterator_data.searchEnable && !criteria->searchAll && iterator_data.searchFinished)
    {
        iterator_data.parent = parent;
        return;
    }

    bool isAr = isArray(parent);

    if (isAr)
        arrIndex = 0;

    cJSON *e = parent->child;
    if (e)
    {
        iterator_data.depth++;
        while (e)
        {
            if (iterator_data.searchEnable && !criteria->searchAll && iterator_data.searchFinished)
                return;

            if (isAr && iterator_data.searchEnable)
                collectResult(e, NULL, arrIndex, criteria);

            if (isArray(e) || isObject(e))
                mCollectIterator(e, e->string ? JSON_OBJECT : JSON_ARRAY, arrIndex, criteria);

            if (isArray(e))
            {
                cJSON *item = e->child;
                int _arrIndex = 0;

                if (e->child)
                {
                    iterator_data.depth++;
                    while (item)
                    {
                        if (iterator_data.searchEnable && !criteria->searchAll && iterator_data.searchFinished)
                            return;

                        if (iterator_data.searchEnable)
                            collectResult(item, NULL, _arrIndex, criteria);

                        if (isArray(item) || isObject(item))
                            mIterate(item, _arrIndex, criteria);
                        else
                            mCollectIterator(item, item->string ? JSON_OBJECT : JSON_ARRAY, _arrIndex, criteria);
                        item = item->next;
                        _arrIndex++;
                    }

                    if (iterator_data.searchEnable && !criteria->searchAll && iterator_data.searchFinished)
                        return;

                    removeDepthPath();
                }
            }
            else if (isObject(e))
                mIterate(e, arrIndex, criteria);
            else
                mCollectIterator(e, e->string ? JSON_OBJECT : JSON_ARRAY, arrIndex, criteria);

            e = e->next;

            if (isAr)
                arrIndex++;
        }

        if (iterator_data.searchEnable && !criteria->searchAll && iterator_data.searchFinished)
            return;

        removeDepthPath();
    }
}

void FirebaseJsonBase::collectResult(cJSON *e, const char *key, int arrIndex, struct fb_js_search_criteria_t *criteria)
{
    if (!iterator_data.searchEnable)
        return;

    bool searchComplete = false;
    bool keyMatches = false;
    int matchesCount = iterator_data.matchesCount;
    cJSON *ref = e;

    if (key != NULL)
    {
        if ((iterator_data._depth == iterator_data.depth || iterator_data.depth == 0) && iterator_data.pathList.size() > 0)
            iterator_data.pathList[iterator_data.pathList.size() - 1] = key;
        else
            iterator_data.pathList.push_back(key);
    }
    else if (arrIndex > -1)
    {
        MBSTRING ar = (const char *)FLASH_MCR("[");
        ar += NUM2S(arrIndex).get();
        ar += (const char *)FLASH_MCR("]");

        if (arrIndex > 0 && iterator_data.pathList.size() > 0)
            iterator_data.pathList[iterator_data.pathList.size() - 1] = ar.c_str();
        else
            iterator_data.pathList.push_back(ar.c_str());
        clearS(ar);
    }

    if (criteria->path.length() > 0)
        keyMatches = checkKeys(criteria);

    if (criteria->value.length() == 0)
    {
        if (!criteria->searchAll)
            searchComplete = keyMatches;
        else if (keyMatches)
            iterator_data.matchesCount++;
    }
    else
    {
        if (iterator_data.depth >= criteria->depth)
        {
            if (criteria->endDepth > -1 && iterator_data.depth > criteria->endDepth + 1)
                return;

            char *p = cJSON_PrintUnformatted(e);
            if (p)
            {
                if (strcmp(criteria->value.c_str(), p) == 0)
                {
                    if (criteria->path.length() > 0 && keyMatches && iterator_data.depth == iterator_data.searchKeyDepth)
                    {
                        if (!criteria->searchAll)
                            searchComplete = true;
                        else
                            iterator_data.matchesCount++;
                    }
                }
                cJSON_free(p);
            }
        }
    }

    if (!criteria->searchAll)
    {
        if (searchComplete)
            iterator_data.parent = e;
        iterator_data.searchFinished = searchComplete;
    }
    else
    {
        if (matchesCount != iterator_data.matchesCount)
        {
            if (ref != NULL)
            {
                cJSON *itm = cJSON_Duplicate(ref, true);
                if (itm != NULL)
                {
                    if (iterator_data.parentArr == NULL)
                        iterator_data.parentArr = cJSON_CreateArray();
                    cJSON_AddItemToArray(iterator_data.parentArr, itm);
                }
            }

            MBSTRING path;
            mGetPath(path, iterator_data.pathList);

            if (iterator_data.path.length() > 0)
                iterator_data.path += (const char *)FLASH_MCR(",\"");
            else
                iterator_data.path += (const char *)FLASH_MCR("[\"");

            iterator_data.path += path;
            iterator_data.path += (const char *)FLASH_MCR("\"");

            clearS(path);
        }
    }
}

void FirebaseJsonBase::removeDepthPath()
{
    iterator_data.depth--;
    if (iterator_data.searchEnable)
    {
        if (iterator_data._depth != iterator_data.depth && iterator_data.pathList.size() > 0)
            iterator_data.pathList.pop_back();
    }
}

void FirebaseJsonBase::mCollectIterator(cJSON *e, int type, int &arrIndex, struct fb_js_search_criteria_t *criteria)
{
    if (!iterator_data.searchEnable)
    {
        struct iterator_result_t result;

        if (e->string)
        {
            size_t pos = buf.find((const char *)e->string, iterator_data.buf_offset);
            if (pos != MBSTRING::npos)
            {
                result.ofs1 = pos;
                result.len1 = strlen(e->string);
                iterator_data.buf_offset = (e->type != cJSON_Object && e->type != cJSON_Array) ? pos + result.len1 : pos;
            }
        }

        char *p = cJSON_PrintUnformatted(e);
        if (p)
        {
            int i = iterator_data.buf_offset;
            size_t pos = buf.find(p, i);
            if (pos != MBSTRING::npos)
            {
                result.ofs2 = pos - result.ofs1 - result.len1;
                result.len2 = strlen(p);
                cJSON_free(p);
                iterator_data.buf_offset = (e->type != cJSON_Object && e->type != cJSON_Array) ? pos + result.len2 : pos;
            }
        }
        result.type = type;
        result.depth = iterator_data.depth;
        iterator_data.result.push_back(result);
    }
    else
    {
        if (type == JSON_OBJECT)
            collectResult(e, e->string, -1, criteria);

        iterator_data._depth = iterator_data.depth;
    }
}

bool FirebaseJsonBase::checkKeys(struct fb_js_search_criteria_t *criteria)
{

    if (iterator_data.searchKeyDepth != -1 && (int)iterator_data.pathList.size() - 1 != iterator_data.searchKeyDepth)
        return false;

    bool matches = false;
    int index = -1;
    int k = 0;
    MBSTRING sPath, cPath;

    for (int i = 0; i < (int)iterator_data.searchKeys->size(); i++)
    {
        matches = false;
        k = i;
        if (iterator_data.searchKeys->at(k)[0] == '*')
        {
            while (iterator_data.searchKeys->at(k)[0] == '*' && k < (int)iterator_data.searchKeys->size())
            {
                k++;
                if (k > (int)iterator_data.searchKeys->size() - 1)
                    break;
            }
        }

        if (k < (int)iterator_data.searchKeys->size())
        {
            if (sPath.length() > 0)
                sPath += (const char *)FLASH_MCR("/");
            sPath += iterator_data.searchKeys->at(k);
        }

        if (k + criteria->depth < (int)iterator_data.pathList.size() && k < (int)iterator_data.searchKeys->size())
        {
            for (int j = k + criteria->depth; j < (int)iterator_data.pathList.size(); j++)
            {
                if (j == criteria->depth && iterator_data.searchKeys->at(k)[0] != '*' && iterator_data.searchKeys->at(k) != iterator_data.pathList[j])
                    break;

                if (iterator_data.searchKeys->at(k) == iterator_data.pathList[j] && j > index)
                {
                    if (criteria->endDepth != -1 && j > criteria->endDepth + 1)
                        break;

                    if (k > 0)
                    {
                        if (iterator_data.searchKeys->at(k - 1)[0] == '*' && j - index == 1)
                        {
                            index = j;
                            continue;
                        }
                    }

                    if (cPath.length() > 0)
                        cPath += (const char *)FLASH_MCR("/");
                    cPath += iterator_data.searchKeys->at(k).c_str();
                    index = j;
                }
            }
        }

        if (sPath == cPath)
            matches = true;

        if (!matches)
            break;

        i = k;
    }

    if (matches)
        iterator_data.searchKeyDepth = iterator_data.depth;

    clearS(sPath);
    clearS(cPath);

    return matches;
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
        char *out = mode == fb_json_serialize_mode_pretty ? cJSON_Print(root) : cJSON_PrintUnformatted(root);
        if (out)
        {
            storeS(buf, out, false);
            cJSON_free(out);
        }
    }
}

bool FirebaseJsonBase::mReadClient(Client *client)
{
    //blocking read
    clearS(buf);
    if (readClient(client, buf))
    {
        if (root != NULL)
            cJSON_Delete(root);
        root = parse(buf.c_str());
        clearS(buf);
        return root != NULL;
    }
    return false;
}

bool FirebaseJsonBase::mReadStream(Stream *s, int timeoutMS)
{
    //non-blocking read
    if (readStream(s, serData, buf, true, timeoutMS))
    {
        if (root != NULL)
            cJSON_Delete(root);
        root = parse(buf.c_str());
        clearS(buf);
        return root != NULL;
    }
    return false;
}

const char *FirebaseJsonBase::mRaw()
{
    toBuf(fb_json_serialize_mode_plain);
    return buf.c_str();
}

bool FirebaseJsonBase::mRemove(const char *path)
{
    bool ret = false;
    prepareRoot();
    std::vector<MBSTRING> keys = std::vector<MBSTRING>();
    makeList(path, keys, '/');

    if (keys.size() > 0)
    {
        if (isArrayKey(keys[0].c_str()) && root_type == Root_Type_JSON)
        {
            clearList(keys);
            return false;
        }
    }

    cJSON *parent = root;

    struct search_result_t r;
    searchElements(keys, parent, r);
    parent = r.parent;

    if (r.status == key_status_existed)
    {
        ret = true;
        if (isArray(parent))
            cJSON_DeleteItemFromArray(parent, getArrIndex(keys[r.stopIndex].c_str()));
        else
        {
            cJSON_DeleteItemFromObjectCaseSensitive(parent, keys[r.stopIndex].c_str());
            if (parent->child == NULL && r.stopIndex > 0)
            {
                MBSTRING path;
                mGetPath(path, keys, 0, r.stopIndex - 1);
                mRemove(path.c_str());
            }
        }
    }

    clearList(keys);
    return ret;
}

void FirebaseJsonBase::mGetPath(MBSTRING &path, std::vector<MBSTRING> paths, int begin, int end)
{
    if (end < 0 || end >= (int)paths.size())
        end = paths.size() - 1;
    if (begin < 0 || begin > end)
        begin = 0;

    for (int i = begin; i <= end; i++)
    {
        if (i > 0)
            path += (const char *)FLASH_MCR("/");
        path += paths[i].c_str();
    }
}

size_t FirebaseJsonBase::mGetSerializedBufferLength(bool prettify)
{
    return cJSON_SerializedBufferLength(root, prettify);
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

bool FirebaseJsonBase::mGet(cJSON *parent, FirebaseJsonData *result, const char *path, bool prettify)
{
    bool ret = false;
    prepareRoot();
    std::vector<MBSTRING> keys = std::vector<MBSTRING>();
    makeList(path, keys, '/');

    if (keys.size() > 0)
    {
        if (isArrayKey(keys[0].c_str()) && root_type == Root_Type_JSON)
        {
            clearList(keys);
            return false;
        }
    }

    cJSON *_parent = parent;
    struct search_result_t r;
    searchElements(keys, parent, r);
    _parent = r.parent;

    if (r.status == key_status_existed)
    {
        cJSON *data = NULL;
        if (isArray(_parent))
            data = cJSON_GetArrayItem(_parent, getArrIndex(keys[r.stopIndex].c_str()));
        else
            data = cJSON_GetObjectItemCaseSensitive(_parent, keys[r.stopIndex].c_str());

        if (data != NULL)
        {
            if (result != NULL)
            {
                result->clear();
                char *p = prettify ? cJSON_Print(data) : cJSON_PrintUnformatted(data);
                result->stringValue = p;
                cJSON_free(p);
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
        value[0] == '-' ? data->iVal.int64 = strtoll(value, &pEnd, 10) : data->iVal.uint64 = strtoull(value, &pEnd, 10);
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
    if (result->type_num == cJSON_Invalid)
    {
        strcpy(buf, (const char *)FLASH_MCR("undefined"));
        result->typeNum = JSON_UNDEFINED;
    }
    else if (result->type_num == cJSON_Object)
    {
        strcpy(buf, (const char *)FLASH_MCR("object"));
        result->typeNum = JSON_OBJECT;
    }
    else if (result->type_num == cJSON_Array)
    {
        strcpy(buf, (const char *)FLASH_MCR("array"));
        result->typeNum = JSON_ARRAY;
    }
    else if (result->type_num == cJSON_String)
    {
        if (result->stringValue.c_str()[0] == '"')
            result->stringValue.remove(0, 1);
        if (result->stringValue.c_str()[result->stringValue.length() - 1] == '"')
            result->stringValue.remove(result->stringValue.length() - 1, 1);

        strcpy(buf, (const char *)FLASH_MCR("string"));
        result->typeNum = JSON_STRING;
    }
    else if (result->type_num == cJSON_NULL)
    {
        strcpy(buf, (const char *)FLASH_MCR("null"));
        result->typeNum = JSON_NULL;
    }
    else if (result->type_num == cJSON_False || result->type_num == cJSON_True)
    {
        strcpy(buf, (const char *)FLASH_MCR("boolean"));
        bool t = strcmp(result->stringValue.c_str(), (const char *)FLASH_MCR("true")) == 0;
        result->typeNum = JSON_BOOL;

        result->iVal = {t};
        result->fVal.setd(t);
        result->boolValue = t;
        result->intValue = t;
        result->floatValue = t;
        result->doubleValue = t;
    }
    else if (result->type_num == cJSON_Number || result->type_num == cJSON_Raw)
    {
        mSetResInt(result, result->stringValue.c_str());
        mSetResFloat(result, result->stringValue.c_str());

        if (strpos(result->stringValue.c_str(), (const char *)FLASH_MCR("."), 0) > -1)
        {
            double d = atof(result->stringValue.c_str());
            if (d > 0x7fffffff)
            {
                strcpy(buf, (const char *)FLASH_MCR("double"));
                result->typeNum = JSON_DOUBLE;
            }
            else
            {
                strcpy(buf, (const char *)FLASH_MCR("float"));
                result->typeNum = JSON_FLOAT;
            }
        }
        else
        {
            strcpy(buf, (const char *)FLASH_MCR("int"));
            result->typeNum = JSON_INT;
        }
    }

    result->type = buf;
    delP(&buf);
}

void FirebaseJsonBase::mSet(const char *path, cJSON *value)
{
    prepareRoot();
    std::vector<MBSTRING> keys = std::vector<MBSTRING>();
    makeList(path, keys, '/');

    if (keys.size() > 0)
    {
        if ((isArrayKey(keys[0].c_str()) && root_type == Root_Type_JSON) || (!isArrayKey(keys[0].c_str()) && root_type == Root_Type_JSONArray))
        {
            cJSON_Delete(value);
            clearList(keys);
            return;
        }
    }

    cJSON *parent = root;
    struct search_result_t r;
    searchElements(keys, parent, r);
    parent = r.parent;

    if (value == NULL)
        value = cJSON_CreateNull();

    if (r.status == key_status_mistype || r.status == key_status_not_existed)
        replaceItem(keys, r, parent, value);
    else if (r.status == key_status_out_of_range)
        appendArray(keys, r, parent, value);
    else if (r.status == key_status_existed)
        replace(keys, r, parent, value);
    else
        cJSON_Delete(value);

    clearList(keys);
}

size_t FirebaseJsonBase::mSearch(cJSON *parent, struct fb_js_search_criteria_t *criteria)
{
    size_t ret = 0;
    std::vector<MBSTRING> keys = std::vector<MBSTRING>();
    if (criteria->path.length() > 0)
        makeList(criteria->path.c_str(), keys, '/');

    mIteratorBegin(parent, &keys, criteria);
    if (iterator_data.searchFinished)
        ret = criteria->searchAll ? iterator_data.matchesCount : 1;

    mIteratorEnd();
    clearList(keys);

    return ret;
}

size_t FirebaseJsonBase::mSearch(cJSON *parent, const char *path, bool searchAll)
{
    struct fb_js_search_criteria_t criteria;
    criteria.searchAll = searchAll;
    criteria.path = path;
    return mSearch(parent, &criteria);
}

const char *FirebaseJsonBase::mGetElementFullPath(cJSON *parent, const char *path, bool searchAll)
{
    struct fb_js_search_criteria_t criteria;
    criteria.searchAll = searchAll;
    criteria.path = path;
    mSearch(parent, NULL, &criteria);
    return buf.c_str();
}

size_t FirebaseJsonBase::mSearch(cJSON *parent, FirebaseJsonData *result, struct fb_js_search_criteria_t *criteria, bool prettify)
{
    size_t ret = 0;

    std::vector<MBSTRING> keys = std::vector<MBSTRING>();
    if (criteria->path.length() > 0)
        makeList(criteria->path.c_str(), keys, '/');

    mIteratorBegin(parent, &keys, criteria);

    if (iterator_data.searchFinished)
    {
        ret = criteria->searchAll ? iterator_data.matchesCount : 1;
        if (criteria->searchAll)
        {
            if (result == NULL)
            {
                clearS(buf);
                iterator_data.path += (const char *)FLASH_MCR("]");
                buf = iterator_data.path.c_str();
            }
            else
            {
                if (iterator_data.parentArr != NULL)
                {
                    char *p = prettify ? cJSON_Print(iterator_data.parentArr) : cJSON_PrintUnformatted(iterator_data.parentArr);
                    result->stringValue = p;
                    cJSON_free(p);
                    result->type_num = iterator_data.parentArr->type;
                    iterator_data.path += (const char *)FLASH_MCR("]");
                    result->searchPath = iterator_data.path.c_str();
                    result->success = true;
                    mSetElementType(result);
                }
            }
        }
        else
        {
            if (result == NULL)
            {
                clearS(buf);
                mGetPath(buf, iterator_data.pathList);
            }
            else
            {
                MBSTRING path;
                mGetPath(path, iterator_data.pathList);
                result->searchPath = path.c_str();
                if (iterator_data.parent != NULL)
                {
                    char *p = prettify ? cJSON_Print(iterator_data.parent) : cJSON_PrintUnformatted(iterator_data.parent);
                    result->stringValue = p;
                    cJSON_free(p);
                    result->type_num = iterator_data.parent->type;
                    result->success = true;
                    mSetElementType(result);
                }
            }
        }
    }

    mIteratorEnd(result != NULL);
    clearList(keys);

    return ret;
}

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

FirebaseJson &FirebaseJson::nAdd(const char *key, cJSON *value)
{
    prepareRoot();
    std::vector<MBSTRING> keys = std::vector<MBSTRING>();
    //makeList(key, keys, '/');
    keys.push_back(key);

    if (value == NULL)
        value = cJSON_CreateNull();

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

FirebaseJsonArray &FirebaseJsonArray::nAdd(cJSON *value)
{
    prepareRoot();

    if (value == NULL)
        value = cJSON_CreateNull();

    cJSON_AddItemToArray(root, value);

    return *this;
}

bool FirebaseJsonArray::mGetIdx(FirebaseJsonData *result, int index, bool prettify)
{
    bool ret = false;
    prepareRoot();

    result->clear();

    cJSON *data = NULL;
    if (isArray(root))
        data = cJSON_GetArrayItem(root, index);

    if (data != NULL)
    {
        char *p = prettify ? cJSON_Print(data) : cJSON_PrintUnformatted(data);
        result->stringValue = p;
        cJSON_free(p);
        result->type_num = data->type;
        result->success = true;
        mSetElementType(result);
        ret = true;
    }
    return ret;
}

bool FirebaseJsonArray::mSetIdx(int index, cJSON *value)
{
    int size = cJSON_GetArraySize(root);
    if (index < size)
        return cJSON_ReplaceItemInArray(root, index, value);
    else
    {
        while (size < index)
        {
            cJSON_AddItemToArray(root, cJSON_CreateNull());
            size++;
        }
        cJSON_AddItemToArray(root, value);
    }
    return true;
}

bool FirebaseJsonArray::mRemoveIdx(int index)
{
    int size = cJSON_GetArraySize(root);
    if (index < size)
    {
        cJSON_DeleteItemFromArray(root, index);
        return size != cJSON_GetArraySize(root);
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
    cJSON *e = cJSON_Duplicate(value.root, true);
    nAdd(e);
    return *this;
}

FirebaseJsonArray &FirebaseJsonArray::add(FirebaseJsonArray &value)
{
    cJSON *e = cJSON_Duplicate(value.root, true);
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
        cJSON_Delete(jsonArray.root);

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
        cJSON_Delete(json.root);

    json.root = json.parse(source);

    return json.root != NULL;
}

void FirebaseJsonData::clear()
{
    stringValue.remove(0, stringValue.length());
    searchPath.remove(0, searchPath.length());
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