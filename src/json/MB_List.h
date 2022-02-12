/*
 * Just a simple dynamic array implementation, MB_List v1.0.3
 * 
 * Created February 4, 2022
 * 
 * The MIT License (MIT)
 * Copyright (c) 2022 K. Suwatchai (Mobizt)
 * 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
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

#ifndef MB_LIST_H
#define MB_LIST_H

#if !defined(__AVR__)
#define MB_VECTOR std::vector
#define MB_USE_STD_VECTOR
#else
#define MB_VECTOR MB_List
#endif

#if defined(__AVR__)
#define MB_LIST_NULL NULL
#else
#define MB_LIST_NULL nullptr
#endif

template <class eType>
class MB_List
{
public:
    MB_List()
    {
        clear();
    }

    ~MB_List()
    {
        clear();
    }

    void swap(MB_List &el)
    {
        MB_List _el;
       _el.e = this->e;
       _el.eSize = this->eSize;
       clear();
       this->e = el.e;
       this->eSize = el.eSize;
       el.clear();
       el.e = _el.e;
       el.eSize = _el.eSize;
    }

    void push_back(eType &e)
    {
        add(&e, eSize, 1);
    }

    void insert(int position, eType &e)
    {
        add(&e, position, 1);
    }

    void insert(int position, int size, eType &e)
    {
        add(&e, position, size);
    }

    void pop_back()
    {
        remove(eSize - 1, 1);
    }

    void erase(int beginIndex, int endIndex)
    {

        if (beginIndex > endIndex || beginIndex < 0)
            return;

        remove(beginIndex, endIndex - beginIndex + 1);
    }

    void erase(int beginIndex)
    {
        remove(beginIndex, 1);
    }

    int begin()
    {
        return 0;
    }

    int end()
    {
        if (eSize > 0)
            return eSize - 1;
        return 0;
    }

    void clear()
    {
        if (e)
            delete[] e;
        e = MB_LIST_NULL;
        eSize = 0;
    }

    size_t size()
    {
        return eSize;
    }

    eType &operator[](int index)
    {
        if (index < eSize && index >= 0)
            return e[index];
        return e[0];
    }

private:
    eType *e = NULL;
    int eSize = 0;

    void add(eType *e, int index, int size)
    {

        if (index > eSize)
            return;

        if (eSize == 0)
        {
            this->e = new eType[1];
            if (this->e)
            {
                eSize = 1;
                this->e[0] = *e;
            }
        }
        else
        {
            eType *tmp = new eType[eSize];

            if (tmp)
            {

                for (int i = 0; i < eSize; i++)
                    tmp[i] = this->e[i];

                delete[] this->e;

                this->e = new eType[eSize + size];
                if (this->e)
                {
                    for (int i = 0; i < index; i++)
                        this->e[i] = tmp[i];

                    for (int i = index; i < index + size; i++)
                        this->e[i] = *e;

                    for (int i = index; i < eSize; i++)
                        this->e[i + size] = tmp[i];

                    eSize += size;
                }

                delete[] tmp;
            }
        }
    }

    void remove(int index, int size)
    {

        if (eSize == 0 || index < 0)
            return;

        if (index + size > eSize)
            size = eSize - index;

        eType *tmp = new eType[eSize];

        if (tmp)
        {

            for (int i = 0; i < eSize; i++)
                tmp[i] = this->e[i];

            delete[] this->e;

            this->e = new eType[eSize - size];
            if (this->e)
            {
                for (int i = 0; i < index; i++)
                    this->e[i] = tmp[i];

                for (int i = index; i < eSize - size; i++)
                    this->e[i] = tmp[i + size];

                eSize -= size;
            }

            delete[] tmp;
        }
    }
};

#endif