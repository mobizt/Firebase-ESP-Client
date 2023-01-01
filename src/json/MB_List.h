/*
 * Just a simple dynamic array implementation, MB_List v1.0.4
 *
 * Created November 1, 2022
 *
 * The MIT License (MIT)
 * Copyright (c) 2023 K. Suwatchai (Mobizt)
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

#if !defined(__AVR__) // #if __cplusplus >= 201103
#define MB_VECTOR std::vector
#define MB_USE_STD_VECTOR
#else
#define MB_VECTOR MB_List
#endif

template <class T>
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

    void push_back(T &data)
    {
        add(&data, current, 1);
    }

    void insert(int position, T &data)
    {
        add(&data, position, 1);
    }

    void insert(int position, int size, T &data)
    {
        add(&data, position, size);
    }

    void pop_back()
    {
        current--;
        if (current < 0)
            current = 0;
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
        if (current > 0)
            return current - 1;
        return 0;
    }

    void clear()
    {
        if (arr)
            delete[] arr;
        arr = NULL;
        current = 0;
    }

    size_t size()
    {
        return current;
    }

    T &operator[](int index)
    {
        if (index < current && index >= 0)
            return arr[index];
        return arr[0];
    }

    void swap(MB_List &item)
    {
        MB_List temp;
        temp.arr = this->arr;
        temp.current = this->current;
        temp.capacity = this->capacity;
        clear();
        this->arr = item.arr;
        this->current = item.current;
        this->capacity = item.capacity;
        item.clear();
        item.arr = temp.arr;
        item.current = temp.current;
        item.capacity = temp.capacity;
    }

private:
    T *arr = NULL;
    int current = 0;
    int capacity = 0;

    void add(T *data, int index, int size)
    {

        if (index > current)
            return;

        if (current == 0)
        {
            arr = new T[1];
            if (arr)
            {
                current = 1;
                arr[0] = *data;
                capacity = 1;
            }
        }
        else
        {
            if (current + size >= capacity)
            {
                T *temp = new T[current];

                if (temp)
                {
                    for (int i = 0; i < current; i++)
                        temp[i] = arr[i];

                    delete[] arr;
                    arr = NULL;

                    capacity = current + size;
                    capacity *= 2;

                    arr = new T[capacity];
                    if (arr)
                    {
                        for (int i = 0; i < index; i++)
                            arr[i] = temp[i];

                        for (int i = index; i < index + size; i++)
                            arr[i] = *data;

                        for (int i = index; i < current; i++)
                            arr[i + size] = temp[i];

                        current += size;
                    }

                    delete[] temp;
                    temp = NULL;
                }
            }
            else
            {
                for (int i = index; i < index + size; i++)
                    arr[i] = *data;
                current += size;
            }
        }
    }

    void remove(int index, int size)
    {

        if (current == 0 || index < 0)
            return;

        if (index + size > current)
            size = current - index;

        T *temp = new T[current];

        if (temp)
        {

            for (int i = 0; i < current; i++)
                temp[i] = arr[i];

            delete[] arr;
            arr = NULL;

            capacity = current - size;
            capacity *= 2;

            arr = new T[capacity];
            if (arr)
            {
                for (int i = 0; i < index; i++)
                    arr[i] = temp[i];

                for (int i = index; i < current - size; i++)
                    arr[i] = temp[i + size];

                current -= size;
            }

            delete[] temp;
            temp = NULL;
        }
    }
};

#endif