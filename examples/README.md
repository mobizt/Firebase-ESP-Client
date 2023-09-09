# Migrate from Firebase-ESP32 or Firebase-ESP8266 library to Firebase-ESP-Client library


This library includes the Firebase and Google Cloud Storage fnctions, Cloud Firestore, Cloud Functions and the Firebase Cloud Messageing upgrades.

The major changes from Firebase-ESP32 or Firebase-ESP8266 library are.

* All Firebase Realtime database functions moved from Firebase class to Firebase.RTDB member class.

* All parameters in RTDB functions are the pointer to the variables unless the String type value.

* The names of StreamData class changes to FirebaseStream, and MultiPathStreamData changes to MultiPathStream.

* The Firebase Cloud Messaging functions moved from FirebaseData class to Firebase.FCM.

* The Firebase Cloud Messaging functions are not compatible with the Firebase-ESP32 or Firebase-ESP8266 library.

* The Firebase Cloud Messaging added supports for full message constructors for legacy HTTP and HTTPv1 APIs.

* New Firebase Storage functions added to the class Firebase.Storage.

* The storage type strutured data are now removed and use fb_esp_mem_storage_type instead.


## Realtime database functions migration

### Initializing

The begin function is now accept only the pointer to the FirebaseAuth and FirebaseConfig data

#### Before

```cpp
 Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
```

#### After

```cpp
 Firebase.begin(&config, &auth);
```
 

### Normal HTTP mode functions

The functions e.g. get, set, push, update and delete, are now accept the pointer.

#### Before

```cpp
 Firebase.setInt(fbdo,...
```

#### After

```cpp
 Firebase.setInt(&fbdo,...
```


### HTTP Stream

The stream functions are now accept the pointer. The callback functions are now support the new class name.

#### Before

```cpp
 Firebase.beginStream(fbdo,...
 
 Firebase.setStreamCallback(fbdo,...

 void streamCallback(StreamData data);

 void streamCallback(MultiPathStreamData data);
```

#### After

```cpp
 Firebase.RTDB.beginStream(&fbdo,...

 Firebase.RTDB.setStreamCallback(&fbdo,...

 void streamCallback(FirebasseStream data);

 void streamCallback(MultiPathStream data);
```

### Function name changes

#### Before

```cpp
 Firebase.pathExist(fbdo,...
```

#### After

```cpp
  Firebase.RTDB.pathExisted(&fbdo,...
```


### Storage type struct to enum

#### Before

```cpp
 StorageType::SD
 StorageType::SPIFFS
```

#### After

```cpp
 mem_storage_type_sd
 mem_storage_type_flash
```

### Other objects params

The FirebaseData, FirebaseJson, FirebaseJsonArray, QueryFilter objects will pass as the pointer to
these objects in all function of the library.




## Firebase Cloud Messaging migration

No compatibilities between this library and Firebase-ESP32 or Firebase-ESP8266 library.


See the examples for the details of usages of this new library.


 

## License

The MIT License (MIT)

Copyright (c) 2019, 2020, 2021, 2022, 2023 K. Suwatchai (Mobizt)


Permission is hereby granted, free of charge, to any person returning a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

