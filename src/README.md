# Firebase Arduino Client Library for ESP8266 and ESP32


Google's Firebase Arduino Client Library for ESP8266 and ESP32 v 2.0.15


The default filessystem used in the library is flash and SD.


The file systems for flash and sd memory can be changed in FirebaseFS.h.



## Global functions

The global functions are the functions that called directly from the Firebase object e.g. Firebase.[Function Name]


#### Initialize Firebase with the config and Firebase's authentication credentials.

param **`config`** The pointer to FirebaseConfig data.

param **`auth`** The pointer to FirebaseAuth data.

 note: For FirebaseConfig and FirebaseAuth data usage, see the examples.

```cpp
void begin(FirebaseConfig *config, FirebaseAuth *auth);
```







#### Provide the details of token generation.

param **`return`** token_info_t The token_info_t structured data that indicates the status.

param **`note`** Use type property to get the type enum value.

token_type_undefined or 0,

token_type_legacy_token or 1,

token_type_id_token or 2,

token_type_custom_token or 3,

token_type_oauth2_access_token or 4

Use status property to get the status enum value.

token_status_uninitialized or 0,

token_status_on_signing or 1,

token_status_on_request or 2,

token_status_on_refresh or 3,

token_status_ready or 4

In case of token generation and refreshment errors,
use error.code property to get the error code number.

Use error.message property to get the error message string.

```cpp
struct token_info_t authTokenInfo();
```



#### Sign up for a new user.

param **`config`** The pointer to FirebaseConfig data.

param **`auth`** The pointer to FirebaseAuth data.

param **`email`** The user Email.

param **`password`** The user password.

return **`Boolean`** value, indicates the success of the operation. 

note: By calling Firebase.begin with config and auth after sign up will be signed in.

This required Email/Password provider to be enabled,

From Firebase console, select Authentication, select Sign-in method tab, under 
the Sign-in providers list, enable Email/Password provider.

If the assigned email and passowrd are empty, the anonymous user will be created if Anonymous provider is enabled.

To enable Anonymous provider, from Firebase console, select Authentication, 
select Sign-in method tab, under the Sign-in providers list, enable Anonymous provider.

```cpp
bool signUp(FirebaseConfig *config, FirebaseAuth *auth, const char *email, const char *password);
```





#### Send a user a verification Email.

param **`config`** The pointer to FirebaseConfig data.

param **`idToken`** The id token of user that was already signed in with Email and password (optional).

return **`Boolean`** value, indicates the success of the operation. 

note: The id token can be obtained from config.signer.tokens.id_token after begin with config and auth data

If the idToken is not assigned, the internal config.signer.tokens.id_token will be used. 

See the Templates of Email address verification in the Firebase console, Authentication.

```cpp
bool sendEmailVerification(FirebaseConfig *config, const char *idToken = "");
```





#### Send a user a password reset link to Email.

param **`config`** The pointer to FirebaseConfig data.

param **`email`** The user Email to send the password resset link.

return **`Boolean`** value, indicates the success of the operation. 

```cpp
bool sendResetPassword(FirebaseConfig *config, const char *email);
```





#### Reconnect WiFi if lost connection.

param **`reconnect`** The boolean to set/unset WiFi AP reconnection.

```cpp
void reconnectWiFi(bool reconnect);
```






#### Set the decimal places for float value to be stored in database.

param **`digits`** The decimal places. 

```cpp
void setFloatDigits(uint8_t digits);
```
  




#### Set the decimal places for double value to be stored in database.

param **`digits`** The decimal places. 

```cpp
void setDoubleDigits(uint8_t digits);
```






#### SD card config with GPIO pins.

param **`ss`** SPI Chip/Slave Select pin.

param **`sck`** SPI Clock pin.

param **`miso`** SPI MISO pin.

param **`mosi`** SPI MOSI pin.

return **`Boolean`** type status indicates the success of the operation.

```cpp
bool sdBegin( int8_t ss = -1, int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1);
```





## Realtime database functions

These functions can be called directly from RTDB object in the Firebase object e.g. Firebase.RTDB.[Function Name]



#### Stop Firebase and release all resources.

param **`fbdo`** The pointer to Firebase Data Object.

```cpp
void end(FirebaseData *fbdo);
```







#### Enable multiple HTTP requests at a time (for ESP32 only).

param **`enable`** The boolean value to enable/disable.

param **`note`** The multiple HTTP requessts at a time is disable by default to prevent the large memory used in multiple requests.

```cpp
void allowMultipleRequests(bool enable);
```







#### Set the timeouts of get function.

param **`fbdo`** The pointer to Firebase Data Object.

param **`millisec`** The milliseconds to limit the request (0 - 900,000 ms or 15 min).

```cpp
void setReadTimeout(FirebaseData *fbdo, int millisec);
```






#### Set the size limit of payload data that will write to the database for each request.

param **`fbdo`** The pointer to Firebase Data Object.

param **`size`** The size identified string e.g. tiny, small, medium, large and unlimited.

Size string and its write timeout in seconds e.g. tiny (1s), small (10s), medium (30s) and large (60s).

```cpp
void setwriteSizeLimit(FirebaseData *fbdo, const String &size);
```





#### Read the database rules.

param **`fbdo`** The pointer to Firebase Data Object.

return - **`Boolean`** value, indicates the success of the operation.

```cpp
bool getRules(FirebaseData *fbdo);
```





#### Write the database rules.

param **`fbdo`** The pointer to Firebase Data Object.

param **`rules`** Database rules in jSON String format.

return - **`Boolean`** value, indicates the success of the operation.

```cpp
bool setRules(FirebaseData *fbdo, const String &rules);
```






#### Determine the existent of the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

return - **`Boolean`** value, true if the defined node was found.

```cpp
bool pathExisted(FirebaseData *fbdo, const String &path);
```





#### Determine the unique identifier (ETag) of current data at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

return **`String`** of unique identifier.

```cpp
String getETag(FirebaseData *fbdo, const String &path);
```





#### Get the shallowed data at defined node path.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** node being read the data.

return **`Boolean`** value, indicates the success of the operation.

```cpp
bool getShallowData(FirebaseData *fbdo, const String &path); 
```






#### Enable the library to use only classic HTTP GET and POST methods.

param **`fbdo`** The pointer to Firebase Data Object.

param **`enable`** Boolean value, true to enable, false to disable.

This option used to escape the Firewall restriction (if the device is connected through Firewall) that allows only HTTP GET and POST
    
HTTP PATCH request was sent as PATCH which not affected by this option.

```cpp
void enableClassicRequest(FirebaseData *fbdo, bool enable);
```






#### Set the virtual child node ".priority" to the defined node. 
    
param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

param **`priority`** The priority value.
    
return - **`Boolean`** value, indicates the success of the operation.



This allows us to set priority to any node other than a priority that set through setJSON, 
pushJSON, updateNode, and updateNodeSilent functions.

```cpp
bool setPriority(FirebaseData *fbdo, const char *path, float priority);
```







#### Read the virtual child node ".priority" value at the defined node.
    
param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.
    
return - **`Boolean`** value, indicates the success of the operation.

 ```cpp
bool getPriority(FirebaseData *fbdo, const char *path);
```







####  Append (post)  new integer value to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

param **`intValue`** The appended value.

return **`Boolean`** value, indicates the success of the operation.

The key or name of new created node will be stored in Firebase Data object, 
call [FirebaseData object].pushName() to get the key.

```cpp
bool pushInt(FirebaseData *fbdo, const char *path, int intValue);

bool push(FirebaseData *fbdo, const char *path, int intValue);
```



#### Append (post) new integer value and the virtual child ".priority" to the defined node.

```cpp
bool pushInt(FirebaseData *fbdo, const char *path, int intValue, float priority);

bool push(FirebaseData *fbdo, const char *path, int intValue, float priority);
```






#### Append new float value to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which float value will be appended.

param **`floatValue`** The appended value.

return **`Boolean`** value, indicates the success of the operation.

The key or name of new created node will be stored in Firebase Data object, 
call [FirebaseData object].pushName() to get the key.

```cpp
bool pushFloat(FirebaseData *fbdo, const char *path, float floatValue);

bool push(FirebaseData *fbdo, const char *path, float floatValue);
```





#### Append (post) new float value and the virtual child ".priority" to the defined node.

```cpp
bool pushFloat(FirebaseData *fbdo, const char *path, float floatValue, float priority);

bool push(FirebaseData *fbdo, const char *path, float floatValue, float priority);
```





#### Append (post) new double value (8 bytes) to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which float value will be appended.

param **`doubleValue`** The appended value.

return **`Boolean`** value, indicates the success of the operation.

The key or name of new created node will be stored in Firebase Data object, 
call [FirebaseData object].pushName() to get the key.

```cpp
bool pushDouble(FirebaseData *fbdo, const char *path, double doubleValue);

bool push(FirebaseData *fbdo, const char *path, double doubleValue);
```





#### Append (post) new double value (8 bytes) and the virtual child ".priority" to the defined node.

```cpp
bool pushDouble(FirebaseData *fbdo, const char *path, double doubleValue, float priority);

bool push(FirebaseData *fbdo, const char *path, double doubleValue, float priority);
```






#### Append (post) new Boolean value to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which Boolean value will be appended.

param **`boolValue`** The appended value.

return **`Boolean`** value, indicates the success of the operation.

The key or name of new created node will be stored in Firebase Data object, 
call [FirebaseData object].pushName() to get the key.

```cpp
bool pushBool(FirebaseData *fbdo, const char *path, bool boolValue);

bool push(FirebaseData *fbdo, const char *path, bool boolValue);
```




#### Append (post) the new Boolean value and the virtual child ".priority" to the defined node.

```cpp
bool pushBool(FirebaseData *fbdo, const char *path, bool boolValue, float priority);

bool push(FirebaseData *fbdo, const char *path, bool boolValue, float priority);
```





#### Append (post) a new string (text) to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which string will be appended.

param **`stringValue`** The appended value.

return **`Boolean`** value, indicates the success of the operation.

The key or name of new created node will be stored in Firebase Data object, 
call [FirebaseData object].pushName() to get the key.

```cpp
bool pushString(FirebaseData *fbdo, const char *path, const String &stringValue);

bool push(FirebaseData *fbdo, const char *path, const char *stringValue);

bool push(FirebaseData *fbdo, const char *path, const String &stringValue);
```





#### Append (post) new string and the virtual child ".priority" to the defined node.

```cpp
bool pushString(FirebaseData *fbdo, const char *path, const String &stringValue, float priority);

bool push(FirebaseData *fbdo, const char *path, const char *stringValue, float priority);

bool push(FirebaseData *fbdo, const char *path, const String &stringValue, float priority);
```







#### Append (post) new child (s) to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which child (s) will be appended.

param **`json`** The pointer to the FirebaseJson object which contains the child (s) nodes.

return **`Boolean`** value, indicates the success of the operation.

The key or name of new created node will be stored in Firebase Data object, 
call [FirebaseData object].pushName() to get the key.

```cpp
bool pushJSON(FirebaseData *fbdo, const char *path, FirebaseJson *json);

bool push(FirebaseData *fbdo, const char *path, FirebaseJson *json);
```






#### Append (post) new child (s) and the virtual child ".priority" to the defined node.

```cpp
bool pushJSON(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority);

bool push(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority);
```








#### Append (post) array to the defined node. 

The old content in defined node will be replaced.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which the array will be appended.

param **`arr`** The pointer to the FirebaseJsonArray object.

return **`Boolean`** value, indicates the success of the operation.

The key or name of new created node will be stored in Firebase Data object, 
call [FirebaseData object].pushName() to get the key.

```cpp
bool pushArray(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr);

bool push(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr);
```






#### Append (post) array and virtual child ".priority" at the defined node.

```cpp
bool pushArray(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, float priority);

bool push(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, float priority);
```





#### Append (post) new blob (binary data) to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which binary data will be appended.

param **`blob`** Byte array of data.

param **`size`** Size of the byte array.

return **`Boolean`** value, indicates the success of the operation.

The key or name of new created node will be stored in Firebase Data object, 
call [FirebaseData object].pushName() to get the key.

```cpp
bool pushBlob(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size);

bool push(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size);
```






#### Append (post) new blob (binary data) and the virtual child ".priority" to the defined node.

```cpp
bool pushBlob(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, float priority);

bool push(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, float priority);
```






#### Append (post) new binary data from file stores on storage memory to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd.

The file systems can be changed in FirebaseFS.h.

param **`path`** The path to the node in which binary data will be appended.

param **`fileName`** The file path includes its name.

return **`Boolean`** value, indicates the success of the operation.

The key or name of new created node will be stored in Firebase Data object, 
call [FirebaseData object].pushName() to get the key.

```cpp
bool pushFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName);

bool push(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName);
```

To use LittleFS file system for flash memory instead of FLASH (only for ES8266 at this time), add the following macro in **FirebaseFS.h**

```cpp
#define USE_LITTLEFS
```







#### Append (post) new binary data from file and the virtual child ".priority" to the defined node.

```cpp
bool pushFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, float priority);

bool push(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, float priority);
```

To use LittleFS file system for flash memory instead of FLASH (only for ES8266 at this time), add the following macro in **FirebaseFS.h**

```cpp
#define USE_LITTLEFS
```





#### Append (post) the new Firebase server's timestamp to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which timestamp will be appended.

return - **`Boolean`** value, indicates the success of the operation.
    
The key or name of new created node will be stored in Firebase Data object, 
call [FirebaseData object].pushName() to get the key.

```cpp
bool pushTimestamp(FirebaseData *fbdo, const char *path);
```






#### Set (put) the integer value at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node which integer value will be set.

param **`intValue`** Integer value to set.

return **`Boolean`** value, indicates the success of the operation.

Call [FirebaseData object].dataType to get the type of data that successfully stored in the database. 

Call [FirebaseData object].intData to get the integer value that stored on the defined node.

```cpp
bool setInt(FirebaseData *fbdo, const char *path, int intValue);

bool set(FirebaseData *fbdo, const char *path, int intValue);
```




#### Set (put) the integer value and virtual child ".priority" at the defined node.

```cpp
bool setInt(FirebaseData *fbdo, const char *path, int intValue, float priority);

bool set(FirebaseData *fbdo, const char *path, int intValue, float priority);
```



#### Set (put) the integer value at the defined node if defined node's ETag matched the defined ETag value.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which integer value will be set.

param **`intValue`** Integer value to set.

param **`ETag`** Known unique identifier string (ETag) of the defined node.

return - **`Boolean`** value, indicates the success of the operation.


Call [FirebaseData object].dataType to get the type of data that successfully stored in the database. 
Call [FirebaseData object].intData to get the integer value that stored on the defined node.

If ETag at the defined node does not match the provided ETag parameter,
the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched). 

If the operation failed due to ETag is not match, call [FirebaseData object].ETag() to get the current ETag value. 
Also call [FirebaseData object].intData to get the current integer value.
    
```cpp
bool setInt(FirebaseData *fbdo, const char *path, int intValue, const char *ETag);

bool set(FirebaseData *fbdo, const char *path, int intValue, const char *ETag);
```






#### Set (put) integer value and the virtual child ".priority" if defined ETag matches at the defined node 

```cpp
bool setInt(FirebaseData *fbdo, const char *path, int intValue, float priority, const char *ETag);

bool set(FirebaseData *fbdo, const char *path, int intValue, float priority, const char *ETag);
```






#### Set (put) float value at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which float value will be set.

param **`floatValue`** Float value to set.

return **`Boolean`** value, indicates the success of the operation.

Call [FirebaseData object].dataType to get the type of data that successfully stored in the database. 

Call [FirebaseData object].floatData to get the float value that stored on the defined node.

```cpp
bool setFloat(FirebaseData *fbdo, const char *path, float floatValue);

bool set(FirebaseData *fbdo, const char *path, float floatValue);
```




#### Set (put) float value and virtual child ".priority" at the defined node.

```cpp
bool setFloat(FirebaseData *fbdo, const char *path, float floatValue, float priority);

bool set(FirebaseData *fbdo, const char *path, float floatValue, float priority);
```





#### Set (put) float value at the defined node if defined node's ETag matched the ETag value.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which float data will be set.

param **`floatValue`** Float value to set.

param **`ETag`** Known unique identifier string (ETag) of defined node.

return **`Boolean`** value, indicates the success of the operation.

Call [FirebaseData object].dataType to get the type of data that successfully stored in the database. 

Call [FirebaseData object].floatData to get the float value that stored on the defined node.

If ETag at the defined node does not match the provided ETag parameter,
the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched). 

If the operation failed due to ETag is not match, call [FirebaseData object].ETag() to get the current ETag value. 

Also call [FirebaseData object].floatData to get the current float value.

```cpp
bool setFloat(FirebaseData *fbdo, const char *path, float floatValue, const char *ETag);

bool set(FirebaseData *fbdo, const char *path, float floatValue, const char *ETag);
```




#### Set (put) float value and the virtual child ".priority" if defined ETag matches at the defined node. 

```cpp
bool setFloat(FirebaseData *fbdo, const char *path, float floatValue, float priority, const char *ETag);

bool set(FirebaseData *fbdo, const char *path, float floatValue, float priority, const char *ETag);
```





#### Set (put) double value at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which float data will be set.

param **`doubleValue`** Double value to set.

return **`Boolean`** value, indicates the success of the operation.

Call [FirebaseData object].dataType to get the type of data that successfully stored in the database. 

Call [FirebaseData object].doubleData to get the double value that stored on the defined node.

Due to bugs in Serial.print in Arduino, to print large double value with zero decimal place, 
use printf("%.9lf\n", firebaseData.doubleData()); for print the returned double value up to 9 decimal places.


```cpp
bool setDouble(FirebaseData *fbdo, const char *path, double doubleValue);

bool set(FirebaseData *fbdo, const char *path, double doubleValue);
```





#### Set (put) double value and virtual child ".priority" at the defined node.

```cpp
bool setDouble(FirebaseData *fbdo, const char *path, double doubleValue, float priority);

bool set(FirebaseData *fbdo, const char *path, double doubleValue, float priority);
```





#### Set (put) double value at the defined node if defined node's ETag matched the ETag value.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which float data will be set.

param **`doubleValue`** Double value to set.

param **`ETag`** Known unique identifier string (ETag) of defined node.

return **`Boolean`** value, indicates the success of the operation.

Call [FirebaseData object].dataType to get the type of data that successfully stored in the database. 

Call [FirebaseData object].doubleData to get the double value that stored on the defined node.

If ETag at the defined node does not match the provided ETag parameter,
the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched). 

If the operation failed due to ETag is not match, call [FirebaseData object].ETag() to get the current ETag value. 

Also call [FirebaseData object].doubeData to get the current double value.

```cpp
bool setDouble(FirebaseData *fbdo, const char *path, double doubleValue, const char *ETag);

bool set(FirebaseData *fbdo, const char *path, double doubleValue, const char *ETag);
```






#### Set (put) double value and the virtual child ".priority" if defined ETag matches at the defined node. 

```cpp
bool setDouble(FirebaseData *fbdo, const char *path, double doubleValue, float priority, const char *ETag);

bool set(FirebaseData *fbdo, const char *path, double doubleValue, float priority, const char *ETag);
```





#### Set (put) boolean value at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which Boolean data will be set.

param **`boolValue`** Boolean value to set.

return **`Boolean`** value, indicates the success of the operation.

Call [FirebaseData object].dataType to get the type of data that successfully stored in the database. 

Call [FirebaseData object].boolData to get the integer value that stored on the defined node.

```cpp
bool setBool(FirebaseData *fbdo, const char *path, bool boolValue);

bool set(FirebaseData *fbdo, const char *path, bool boolValue);
```





#### Set (put) boolean value and virtual child ".priority" at the defined node.

```cpp
bool setBool(FirebaseData *fbdo, const char *path, bool boolValue, float priority);

bool set(FirebaseData *fbdo, const char *path, bool boolValue, float priority);
```





#### Set (put) boolean value at the defined node if defined node's ETag matched the ETag value.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which Boolean data will be set.

param **`boolValue`** Boolean value to set.

param **`ETag`** Known unique identifier string (ETag) of defined node.

return **`Boolean`** value, indicates the success of the operation.

Call [FirebaseData object].dataType to get the type of data that successfully stored in the database.

Call [FirebaseData object].boolData to get the boolean value that stored on the defined node.

If ETag at the defined node does not match the provided ETag parameter,
the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched). 

If the operation failed due to ETag is not match, call [FirebaseData object].ETag() to get the current ETag value. 

Also call [FirebaseData object].boolData to get the current boolean value.

```cpp
bool setBool(FirebaseData *fbdo, const char *path, bool boolValue, const char *ETag);

bool set(FirebaseData *fbdo, const char *path, bool boolValue, const char *ETag);
```





#### Set (put) boolean value and the virtual child ".priority" if defined ETag matches at the defined node. 

```cpp
bool setBool(FirebaseData *fbdo, const char *path, bool boolValue, float priority, const char *ETag);

bool set(FirebaseData *fbdo, const char *path, bool boolValue, float priority, const char *ETag);
```






#### Set (put) string at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which string data will be set.

param **`stringValue`** String or text to set.

return **`Boolean`** value, indicates the success of the operation.

Call [FirebaseData object].dataType to get the type of data that successfully stored in the database. 

Call [FirebaseData object].stringData to get the string value that stored on the defined node.

```cpp
bool setString(FirebaseData *fbdo, const char *path, const String &stringValue);

bool set(FirebaseData *fbdo, const char *path, const char *stringValue);

bool set(FirebaseData *fbdo, const char *path, const String &stringValue);
```






#### Set (put) string value and virtual child ".priority" at the defined node.

```cpp
bool setString(FirebaseData *fbdo, const char *path, const String &stringValue, float priority);

bool set(FirebaseData *fbdo, const char *path, const char *stringValue, float priority);

bool set(FirebaseData *fbdo, const char *path, const String &stringValue, float priority);
```






#### Set (put) string at the defined node if defined node's ETag matched the ETag value.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which string data will be set.

param **`stringValue`** String or text to set.

param **`ETag`** Known unique identifier string (ETag) of defined node.

return **`Boolean`** value, indicates the success of the operation.

Call [FirebaseData object].dataType to get the type of data that successfully stored in the database. 

Call [FirebaseData object].stringData to get the string value that stored on the defined node.

If ETag at the defined node does not match the provided ETag parameter,
the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched).

If the operation failed due to ETag is not match, call [FirebaseData object].ETag() to get the current ETag value.

Also, call [FirebaseData object].stringData to get the current string value.

```cpp
bool setString(FirebaseData *fbdo, const char *path, const String &stringValue, const char *ETag);

bool set(FirebaseData *fbdo, const char *path, const char *stringValue, const char *ETag);

bool set(FirebaseData *fbdo, const char *path, const String &stringValue, const char *ETag);
```





#### Set string data and the virtual child ".priority" if defined ETag matches at the defined node. 

```cpp
bool setString(FirebaseData *fbdo, const char *path, const String &stringValue, float priority, const char *ETag);

bool set(FirebaseData *fbdo, const char *path, const char *stringValue, float priority, const char *ETag);

bool set(FirebaseData *fbdo, const char *path, const String &stringValue, float priority, const char *ETag);
```







#### Set (put) the child (s) nodes to the defined node. 

The old content in defined node will be replaced.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which child (s) nodes will be replaced or set.

param **`json`** The pointer to FirebaseJson object.

return **`Boolean`** value, indicates the success of the operation.

Call [FirebaseData object].dataType to get the type of data that successfully stored in the database. 

Call [FirebaseData object].jsonData and [FirebaseData object].jsonDataPtr to get the JSON data that stored on the defined node.

```cpp
bool setJSON(FirebaseData *fbdo, const char *path, FirebaseJson *json);

bool set(FirebaseData *fbdo, const char *path, FirebaseJson *json);
```







#### Set (put) the child (s) nodes and virtual child ".priority" at the defined node.

```cpp
bool setJSON(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority);

  bool set(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority);
```







#### Set (put) the child (s) nodes to the defined node, if defined node's ETag matched the ETag value. 

The old content in defined node will be replaced.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which child(s) nodes will be replaced or set.

param **`json`** The pointer to FirebaseJson object.

param **`ETag`** KKnown unique identifier string (ETag) of defined node.

return **`Boolean`** value, indicates the success of the operation.

Call [FirebaseData object].dataType to get the type of data that successfully stored in the database.

Call [FirebaseData object].jsonData and [FirebaseData object].jsonDataPtr to get the JSON data that stored on the defined node.

If ETag at the defined node does not match the provided ETag parameter,
the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched).

If the operation failed due to ETag is not match, call [FirebaseData object].ETag() to get the current ETag value. 

Also call [FirebaseData object].jsonData and [FirebaseData object].jsonDataPtr to get the JSON data.

```cpp
bool setJSON(FirebaseData *fbdo, const char *path, FirebaseJson *json, const char *ETag);

bool set(FirebaseData *fbdo, const char *path, FirebaseJson *json, const char *ETag);
```






#### Set (put) the child (s) nodes and the virtual child ".priority" if defined ETag matches at the defined node.

```cpp
bool setJSON(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority, const char *ETag);

bool set(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority, const char *ETag);
```




 
 
####  Set (put) the array to the defined node.

The old content in defined node will be replaced.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which array will be replaced or set.

param **`arr`** The pointer to FirebaseJsonArray object.

return - **`Boolean`** value, indicates the success of the operation.

Call [FirebaseData object].dataType to determine what type of data that successfully stores in the database. 

Call [FirebaseData object].jsonArray and [FirebaseData object].jsonArrayPtr will return object and pointer to 
FirebaseJsonArray object which contains the array.

```cpp
bool setArray(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr);

bool set(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr);
```







#### Set (put) array and virtual child ".priority" at the defined node.

```cpp
bool setArray(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, float priority);

bool set(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, float priority);
```






#### Set (put) the array to the defined node if defined node's ETag matched the ETag value. 

The old content in defined node will be replaced.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which array will be replaced or set.

param **`arr`** The pointer to FirebaseJsonArray object.

param **`ETag`** Known unique identifier string (ETag) of defined node.

return - **`Boolean`** value, indicates the success of the operation.
    
Call [FirebaseData object].dataType to determine what type of data successfully stores in the database. 

Call [FirebaseData object].jsonArray and [FirebaseData object].jsonArrayPtr will return object and 
pointer to FirebaseJsonArray object that contains the array; 

If ETag at the defined node does not match the provided ETag parameter,
the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched).

If the operation failed due to ETag is not match, call [FirebaseData object].ETag() to get the current ETag value.

Also call [FirebaseData object].jsonArray and [FirebaseData object].jsonArrayPtr to get the array.

```cpp
bool setArray(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, const char *ETag);

bool set(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, const char *ETag);
```





#### Set (put) array and the virtual child ".priority" if defined ETag matches at the defined node. 

```cpp
bool setArray(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, float priority, const char *ETag);

bool set(FirebaseData *fbdo, const char *path, FirebaseJsonArray *arr, float priority, const char *ETag);
```





#### Set (put) the blob (binary data) at the defined node. 

The old content in defined node will be replaced.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to node in which binary data will be set.

param **`blob`** Byte array of data.

param **`size`** Size of the byte array.

return **`Boolean`** value, indicates the success of the operation.

No payload returned from the server.

```cpp
bool setBlob(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size);

bool set(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size);
```





#### Set (put) the blob data and virtual child ".priority" at the defined node.

```cpp
bool setBlob(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, float priority);

bool set(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, float priority);
```






#### Set blob (binary data) at the defined node if defined node's ETag matched the ETag value.

The old content in defined node will be replaced.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** he path to the node in which binary data will be set.

param **`blob`** Byte array of data.

param **`size`** Size of the byte array.

param **`ETag`** Known unique identifier string (ETag) of defined node.

return **`Boolean`** value, indicates the success of the operation.

No payload returned from the server.

If ETag at the defined node does not match the provided ETag parameter,
the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched).

```cpp
bool setBlob(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, const char *ETag);

bool set(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, const char *ETag);
```





#### Set (put) the binary data and the virtual child ".priority" if defined ETag matches at the defined node. 

```cpp
bool setBlob(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, float priority, const char *ETag);

bool set(FirebaseData *fbdo, const char *path, uint8_t *blob, size_t size, float priority, const char *ETag);
```





#### Set (put) the binary data from file to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd.

The file systems can be changed in FirebaseFS.h.

param **`path`** The path to the node in which binary data will be set.

param **`fileName`** The file path includes its name.

return **`Boolean`** value, indicates the success of the operation.

No payload returned from the server.

```cpp
bool setFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName);

bool set(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName);
```

To use LittleFS file system for flash memory instead of FLASH (only for ESP8266 at this time), add the following macro in **FirebaseFS.h**

```cpp
#define USE_LITTLEFS
```




#### Set (put) the binary data from file and virtual child ".priority" at the defined node.

```cpp
bool setFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, float priority);

bool set(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, float priority);
```






#### Set (put) the binary data from file to the defined node if defined node's ETag matched the ETag value.

param **`fbdo`** The pointer to Firebase Data Object.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd.

The file systems can be changed in FirebaseFS.h.

param **`path`** The path to the node in which binary data from the file will be set.

param **`fileName`** The file path includes its name.

param **`ETag`** Known unique identifier string (ETag) of defined node.

return **`Boolean`** value, indicates the success of the operation.

No payload returned from the server. 

If ETag at the defined node does not match the provided ETag parameter,
the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched).

```cpp
bool setFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, const char *ETag);

bool set(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, const char *ETag);
```

To use LittleFS file system for flash memory instead of FLASH only for ESP8266 at this time), add the following macro in **FirebaseFS.h**

```cpp
#define USE_LITTLEFS
```





#### Set (put) the binary data from the file and the virtual child ".priority" if defined ETag matches at the defined node. 

```cpp
bool setFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, float priority, const char *ETag);

bool set(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *path, const char *fileName, float priority, const char *ETag);


```






#### Set (put) the Firebase server's timestamp to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which timestamp will be set.

return - **`Boolean`** value, indicates the success of the operation.
    
Call [FirebaseData object].intData will return the integer value of timestamp in seconds 
or [FirebaseData object].doubleData to get millisecond timestamp. 

Due to bugs in Serial.print in Arduino, to print large double value with zero decimal place, 
use printf("%.0lf\n", firebaseData.doubleData());.

```cpp
bool setTimestamp(FirebaseData *fbdo, const char *path);
```







#### Update (patch) the child (s) nodes to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which child (s) nodes will be updated.

param **`json`** The pointer to FirebaseJson object used for the update.

rCall [FirebaseData object].jsonData and [FirebaseData object].jsonDataPtr 
to get the JSON data that already updated on the defined node.

```cpp
bool updateNode(FirebaseData *fbdo, const char *path, FirebaseJson *json);
```







#### Update (patch) the child (s) nodess and virtual child ".priority" to the defined node.

```cpp
bool updateNode(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority);
```







#### Update (patch) the child (s) nodes to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which child (s) nodes will be updated.

param **`json`** The pointer to FirebaseJson object used for the update.

return **`Boolean`** value, indicates the success of the operation.

Owing to the objective of this function to reduce network data usage, 
no payload will be returned from the server.

```cpp
bool updateNodeSilent(FirebaseData *fbdo, const char *path, FirebaseJson *json);
```





#### Update (patch) the child (s) nodes and virtual child ".priority" to the defined node.

```cpp
bool updateNodeSilent(FirebaseData *fbdo, const char *path, FirebaseJson *json, float priority);
```







#### Read any type of value at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

return - **`Boolean`** value, indicates the success of the operation.

Call [FirebaseData object].dataType to determine what type of data successfully stores in the database. 

Call [FirebaseData object].intData, [FirebaseData object].floatData, [FirebaseData object].doubleData, 
[FirebaseData object].boolData, [FirebaseData object].stringData, [FirebaseData object].jsonObject,
[FirebaseData object].jsonObjectPtr (pointer), [FirebaseData object].jsonArray,
[FirebaseData object].jsonArrayPtr (pointer) and [FirebaseData object].blobData corresponded to 
its type that get from [FirebaseData object].dataType.

```cpp
bool get(FirebaseData *fbdo, const char *path);
```




#### Read (get) the integer value at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

return **`Boolean`** value, indicates the success of the operation.

Call [FirebaseData object].dataType to determine what type of data successfully stores in the database. 
    
Call [FirebaseData object].intData will return the integer value of
payload returned from server.

If the type of payload returned from server is not integer, float and double, 
the function [FirebaseData object].intData will return zero (0).

```cpp
bool getInt(FirebaseData *fbdo, const char *path);
```





#### Read (get) the integer value at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

param **`target`** The pointer to int type variable to store the value.

return **`Boolean`** value, indicates the success of the operation.

If the type of payload returned from the server is not an integer, float and double, 
the target variable's value will be zero (0).

```cpp
bool getInt(FirebaseData *fbdo, const char *path, int *target);
```






#### Read (get) the float value at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

return **`Boolean`** value, indicates the success of the operation.

Call [FirebaseData object].dataType to determine what type of data successfully stores in the database.

Call [FirebaseData object].floatData to get float value. 

If the payload returned from server is not integer, float and double, 
the function [FirebaseData object].floatData will return zero (0).

```cpp
bool getFloat(FirebaseData *fbdo, const char *path);
```






#### Read (get) the float value at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

param **`target`** The pointer to float type variable to store the value.

return **`Boolean`** value, indicates the success of the operation.

If the type of payload returned from the server is not an integer, float and double, 
the target variable's value will be zero (0).

```cpp
bool getFloat(FirebaseData *fbdo, const char *path, float *target);
```







#### Read (get) the double value at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

return **`Boolean`** value, indicates the success of the operation.

Call [FirebaseData object].dataType to determine what type of data successfully stores in the database. 

Call [FirebaseData object].doubleData to get double value.

If the payload returned from server is not integer, float and double, 
the function [FirebaseData object].doubleData will return zero (0).

Due to bugs in Serial.print in Arduino, to print large double value with zero decimal place, 
use printf("%.9lf\n", firebaseData.doubleData()); for print value up to 9 decimal places.

```cpp
bool getDouble(FirebaseData *fbdo, const char *path);
```







#### Read (get) the double value at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

param **`target`** The pointer to double type variable to store the value.

return **`Boolean`** value, indicates the success of the operation.

If the type of payload returned from the server is not an integer, float and double, 
the target variable's value will be zero (0).

```cpp
bool getDouble(FirebaseData *fbdo, const char *path, double *target);
```






#### Read the Boolean value at the defined node

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

return **`Boolean`** value, indicates the success of the operation.

all [FirebaseData object].dataType to determine what type of data successfully stores in the database. 

Call [FirebaseData object].boolData to get boolean value.

If the type of payload returned from the server is not Boolean, 
the function [FirebaseData object].boolData will return false.

```cpp
bool getBool(FirebaseData *fbdo, const char *path);
```







#### Read (get) the Boolean value at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

param **`target`** The pointer to boolean type variable to store the value.

return **`Boolean`** value, indicates the success of the operation.

If the type of payload returned from the server is not Boolean, 
the target variable's value will be false.

```cpp
bool getBool(FirebaseData *fbdo, const char *path, bool *target);
```






#### Read (get) the string at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

return **`Boolean`** value, indicates the success of the operation.

Call [FirebaseData object].dataType to determine what type of data that successfully
stores in the database.

Call [FirebaseData object].stringData to get string value.

If the type of payload returned from the server is not a string,
the function [FirebaseData object].stringData will return empty string.

```cpp
bool getString(FirebaseData *fbdo, const char *path);
```






#### Read (get) the string at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

param **`target`** The pointer to String object variable to store the value.

return **`Boolean`** value, indicates the success of the operation.

If the type of payload returned from the server is not a string,
the target String object's value will be empty.

```cpp
bool getString(FirebaseData *fbdo, const char *path, String *target);
```





#### Read (get) the child (s) nodes at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

return **`Boolean`** value, indicates the success of the operation.

Call [FirebaseData object].dataType to determine what type of data that successfully stores in the database.

Call [FirebaseData object].jsonData and [FirebaseData object].jsonDataPtr 
to get the JSON data at the defined node.

If the type of payload returned from server is not json,
the function [FirebaseData object].jsonObject will contain empty object.

```cpp
bool getJSON(FirebaseData *fbdo, const char *path);
```






#### Read (get) the JSON string at the defined node. 

The returned the pointer to FirebaseJson that contains 
JSON payload represents the child nodes and their value.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

param **`target`** The pointer to FirebaseJson object variable to store the value.

return **`Boolean`** value, indicates the success of the operation.

If the type of payload returned from the server is not JSON,
the target FirebaseJson object will contain an empty object.

```cpp
bool getJSON(FirebaseData *fbdo, const char *path, FirebaseJson *target);
```







#### Read (get) the JSON string at the defined node. 

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

param **`query`** QueryFilter class to set query parameters to filter data.

return **`Boolean`** value, indicates the success of the operation.

The Available query parameters for filtering the data are the following.

**`QueryFilter.orderBy`** Required parameter to specify which data used for data filtering included child key, key, and value.

Use "$key" for filtering data by keys of all nodes at the defined node.

Use "$value" for filtering data by value of all nodes at the defined node.

Use "$priority" for filtering data by "virtual child" named .priority of all nodes.

Use any child key to filter by that key.


**`QueryFilter.limitToFirst`**  The total children (number) to filter from the first child.

**`QueryFilter.limitToLast`**   The total last children (number) to filter. 

**`QueryFilter.startAt`**       Starting value of range (number or string) of query upon orderBy param.

**`QueryFilter.endAt`**         Ending value of range (number or string) of query upon orderBy param.

**`QueryFilter.equalTo`**       Value (number or string) matches the orderBy param


Call [FirebaseData object].dataType to determine what type of data successfully stores in the database.

Call [FirebaseData object].jsonData and [FirebaseData object].jsonDataPtr 
to get the JSON data at the defined node.

If the type of payload returned from server is not JSON,
the function [FirebaseData object].jsonObject will contain empty object.

```cpp
bool getJSON(FirebaseData *fbdo, const char *path, QueryFilter *query);
```







#### Read (get) the JSON string at the defined node.

The returned payload JSON string represents the child nodes and their value.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

param **`target`** The pointer to FirebaseJson object variable to store the value.

return **`Boolean`** value, indicates the success of the operation.

If the type of payload returned from the server is not JSON,
the target FirebaseJson object will contain an empty object.

```cpp
bool getJSON(FirebaseData *fbdo, const char *path, QueryFilter *query, FirebaseJson *target);
```









#### Read (get) the array at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

return - **`Boolean`** value, indicates the success of the operation.
    
Call [FirebaseData object].dataType to determine what type of data that successfully
stores in the database.

Call [FirebaseData object].jsonArray and [FirebaseData object].jsonArrayPtr will return object and 
pointer to FirebaseJsonArray object that contains the array; 

If the type of payload returned from the server is not an array,
the array element in [FirebaseData object].jsonArray will be empty.

```cpp
bool getArray(FirebaseData *fbdo, const char *path);
```




#### Read (get) the array at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

param **`target`** The pointer to FirebaseJsonArray object variable to store the value.

return - **`Boolean`** value, indicates the success of the operation.

If the type of payload returned from the server is not an array, 
the target FirebaseJsonArray object will contain an empty array.

```cpp
bool getArray(FirebaseData *fbdo, const char *path, FirebaseJsonArray *target);
```




#### Read (get) the array data at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

param **`query`** QueryFilter class to set query parameters to filter data.

return - **`Boolean`** value, indicates the success of the operation.

The Available query parameters for filtering the data are the following.

QueryFilter.orderBy -       Required parameter to specify which data used for data filtering included child key, key, and value.
                            Use "$key" for filtering data by keys of all nodes at the defined node.
                            Use "$value" for filtering data by value of all nodes at the defined node.
                            Use "$priority" for filtering data by "virtual child" named .priority of all nodes.
                            Use any child key to filter by that key.


QueryFilter.limitToFirst -  The total children (number) to filter from the first child.

QueryFilter.limitToLast -   The total last children (number) to filter.

QueryFilter.startAt -       Starting value of range (number or string) of query upon orderBy param.

QueryFilter.endAt -         Ending value of range (number or string) of query upon orderBy param.

QueryFilter.equalTo -       Value (number or string) matches the orderBy param


Call [FirebaseData object].dataType to determine what type of data that successfully 
stores in the database.

Call [FirebaseData object].jsonArray will return the pointer to FirebaseJsonArray object contains array of 
payload returned from server.

If the type of payload returned from the server is not an array,
the function [FirebaseData object].jsonArray will contain empty array.

```cpp
bool getArray(FirebaseData *fbdo, const char *path, QueryFilter *query);
```






#### Read (get) the array data at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

param **`target`** The pointer to FirebaseJsonArray object variable to store the value.

return - **`Boolean`** value, indicates the success of the operation.

If the type of payload returned from the server is not an array,
the target FirebaseJsonArray object will contain an empty array.

```cpp
bool getArray(FirebaseData *fbdo, const char *path, QueryFilter *query, FirebaseJsonArray *target);
```




#### Read (get) the blob (binary data) at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

return **`Boolean`** value, indicates the success of the operation.

Call [FirebaseData object].dataType to determine what type of data successfully stores in the database.

Call [FirebaseData object].blobData to get the uint8_t vector.

If the type of payload returned from the server is not a blob,
the function [FirebaseData object].blobData will return empty array.

```cpp
bool getBlob(FirebaseData *fbdo, const char *path);
```






#### Read (get) the blob (binary data) at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node

param **`target`** The pointer to uint8_t vector variable to store the value.

return **`Boolean`** value, indicates the success of the operation.

If the type of payload returned from the server is not a blob, 
the target variable value will be an empty array.

```cpp
bool getBlob(FirebaseData *fbdo, const char *path, std::vector<uint8_t> *target);
```







#### Download file data at the defined node and save to storage memory.

The downloaded data will be decoded to binary and save to SD card/Flash memory, 
then please make sure that data at the defined node is the file type.

param **`fbdo`** The pointer to Firebase Data Object.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd.

The file systems can be changed in FirebaseFS.h.

param **`nodePath`** The path to the node that file data will be downloaded.

param **`fileName`**  The file path includes its name.

return **`Boolean`** value, indicates the success of the operation.

```cpp
bool getFile(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *nodePath, const char *fileName);
```

To use LittleFS file system for flash memory instead of FLASH (only for ESP8266 at this time), add the following macro in **FirebaseFS.h**

```cpp
#define USE_LITTLEFS
```




#### Delete all child nodes at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node to be deleted.

return **`Boolean`** value, indicates the success of the operation.*

```cpp
bool deleteNode(FirebaseData *fbdo, const char *path);
```





#### Delete all child nodes at the defined node if defined node's ETag matched the ETag value.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** nThe path to the node to be deleted.

param **`ETag`** Known unique identifier string (ETag) of defined node.

return **`Boolean`** value, indicates the success of the operation.*

If ETag at the defined node does not match the provided ETag parameter,
the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched).

```cpp
bool deleteNode(FirebaseData *fbdo, const char *path, const char *ETag);
```







#### Subscribe to the value changes on the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node to subscribe.

return **`Boolean`** value, indicates the success of the operation.*

```cpp
bool beginStream(FirebaseData *fbdo, const char *path);
```






#### Subscribe to the value changes on the children of the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`parentPath`** The path to the parent node to subscribe.

param **`childPath`** The string array of the path to child nodes.

param **`size`** The size of string array of the path to the child nodes.

return **`Boolean`** value, indicates the success of the operation.*

```cpp
bool beginMultiPathStream(FirebaseData *fbdo, const char *parentPath, const String *childPath, size_t size);
```







#### Read the stream event data at the defined node. 

Once beginStream was called e.g. in setup(), the readStream function 
should call inside the continuous loop block.

param **`fbdo`** The pointer to Firebase Data Object.

return **`Boolean`** value, indicates the success of the operation.

Using the shared Firebase Data object for stream read/monitoring associated 
with normal Firebase call e.g. read, set, push, update and delete will break or interrupt
the current stream connection.
    
The stream will be resumed or reconnected automatically when calling the function readStream.

```cpp
bool readStream(FirebaseData *fbdo);
```





#### End the stream connection at a defined node. 

It can be restart again by calling the function beginStream.

param **`fbdo`** The pointer to Firebase Data Object.

return **`Boolean`** value, indicates the success of the operation.
 
```cpp
bool endStream(FirebaseData *fbdo);
```





#### Set the stream callback functions.

param **`fbdo`** The pointer to Firebase Data Object.

param **`dataAvailablecallback`** The Callback function that accepts FirebaseStream parameter.

param **`timeoutCallback`** The Callback function will be called when the stream connection was timed out (optional).

ESP32 only parameter
param **`streamTaskStackSize`** The stream task (RTOS task) reserved stack memory in byte (optional) (8192 is default).


The dataAvailableCallback will be called When data in the defined path changed or the stream path changed or stream connection 
was resumed from getXXX, setXXX, pushXXX, updateNode, deleteNode.

The payload returned from the server will be one of these integer, float, string, JSON and blob types.

Call [FirebaseStream object].dataType to determine what type of data successfully stores in the database. 

Call [FirebaseStream object].xxxData will return the appropriate data type of 
the payload returned from the server.

```cpp

void setStreamCallback(FirebaseData *fbdo, FirebaseData::StreamEventCallback dataAvailableCallback, FirebaseData::StreamTimeoutCallback timeoutCallback, size_t streamTaskStackSize = 8192);

void setStreamCallback(FirebaseData *fbdo, FirebaseData::StreamEventCallback dataAvailableCallback, FirebaseData::StreamTimeoutCallback timeoutCallback);

```







#### Set the multiple paths stream callback functions. 

setMultiPathStreamCallback should be called before Firebase.beginMultiPathStream.

param **`fbdo`** The pointer to Firebase Data Object.

param **`multiPathDataCallback`** The Callback function that accepts MultiPathStream parameter.

param **`timeoutCallback`** The Callback function will be called when the stream connection was timed out (optional).

ESP32 only parameter
param **`streamTaskStackSize`** The stream task (RTOS task) reserved stack memory in byte (optional) (8192 is default).


The multiPathDataCallback will be called When children value of the defined node changed or the stream path changed or stream connection was resumed from normal Firebase calls.

The payload returned from the server will be one of these types e.g. boolean, integer, float, string, JSON, array, blob and file.

Call [MultiPathStream object].get to get the child node value, type and its data path. 

The properties [MultiPathStream object].value, [MultiPathStream object].dataPath, and [MultiPathStream object].type will return the value, path of data, and type of data respectively.

These properties will store the result from calling the function [MultiPathStream object].get.

```cpp
void setMultiPathStreamCallback(FirebaseData *fbdo, FirebaseData::MultiPathStreamEventCallback multiPathDataCallback, FirebaseData::StreamTimeoutCallback timeoutCallback = NULL, size_t streamTaskStackSize = 8192);

void setMultiPathStreamCallback(FirebaseData *fbdo, FirebaseData::MultiPathStreamEventCallback multiPathDataCallback, FirebaseData::StreamTimeoutCallback timeoutCallback = NULL);

```









#### Remove stream callback functions.

param **`fbdo`** The pointer to Firebase Data Object.

```cpp
void removeStreamCallback(FirebaseData *fbdo);
```








#### Remove multiple paths stream callback functions.

param **`fbdo`** The pointer to Firebase Data Object.

```cpp
void removeMultiPathStreamCallback(FirebaseData *fbdo);
```








#### Backup (download) the database at the defined node to the storage memory.

param **`fbdo`** The pointer to Firebase Data Object.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd.

The file systems can be changed in FirebaseFS.h.

param **`nodePath`** The path to the node to be backuped.

param **`fileName`**  File name to save.

Only 8.3 DOS format (max. 8 bytes file name and 3 bytes file extension) can be saved to SD card/Flash memory.

return **`Boolean`** value, indicates the success of the operation.


```cpp
bool backup(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *nodePath, const char *fileName);
```

To use LittleFS file system for flash memory instead of FLASH (only for ESP8266 at this time), add the following macro in **FirebaseFS.h**

```cpp
#define USE_LITTLEFS
```




#### Restore the database at a defined path using backup file saved on SD card/Flash memory.

param **`fbdo`** The pointer to Firebase Data Object.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd.

The file systems can be changed in FirebaseFS.h.

param **`nodePath`** The path to the node to be restored the data.

param **`fileName`** File name to read.

return **`Boolean`** value, indicates the success of the operation.

```cpp
bool restore(FirebaseData *fbdo, fb_esp_mem_storage_type storageType, const char *nodePath, const char *fileName);
```

To use LittleFS file system for flash memory instead of FLASH (only for ESP8266 at this time), add the following macro in **FirebaseFS.h**

```cpp
#define USE_LITTLEFS
```








#### Set maximum Firebase read/store retry operation (0 - 255) in case of network problems and buffer overflow.

param **`fbdo`** The pointer to Firebase Data Object.

param **`num`** The maximum retry.

```cpp
void setMaxRetry(FirebaseData *fbdo, uint8_t num);
```





#### Set the maximum Firebase Error Queues in the collection (0 255). 

Firebase read/store operation causes by network problems and buffer overflow will be added to Firebase Error Queues collection.

param **`fbdo`** The pointer to Firebase Data Object.

param **`num`** The maximum Firebase Error Queues.

```cpp
void setMaxErrorQueue(FirebaseData *fbdo, uint8_t num);
```






#### Save Firebase Error Queues as file in flash memory (save only database store queues). 

The Firebase read (get) operation will not save.

param **`fbdo`** The pointer to Firebase Data Object.

param **`filename`** Filename to be saved.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd.

The file systems can be changed in FirebaseFS.h.
    
```cpp
bool saveErrorQueue(FirebaseData *fbdo, const char *filename, fb_esp_mem_storage_type storageType);
```
   






#### Delete file in storage memory.

param **`filename`** File name to delete.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd.

The file systems can be changed in FirebaseFS.h.
    
```cpp
bool deleteStorageFile(const char *filename, fb_esp_mem_storage_type storageType);
```

To use LittleFS file system for flash memory instead of FLASH (only for ESP8266 at this time), add the following macro in **FirebaseFS.h**

```cpp
#define USE_LITTLEFS
```







#### Restore the Firebase Error Queues from the queue file (flash memory).

param **`fbdo`** The pointer to Firebase Data Object.

param **`filename`** Filename to be read and restore queues.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd.

The file systems can be changed in FirebaseFS.h.
    
```cpp
bool restoreErrorQueue(FirebaseData *fbdo, const char *filename, fb_esp_mem_storage_type storageType);
```






#### Determine the number of Firebase Error Queues stored in a defined file (flash memory).

param **`fbdo`** The pointer to Firebase Data Object.

param **`filename`** Filename to be read and count for queues.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd.

The file systems can be changed in FirebaseFS.h.

return **`Number`** (0-255) of queues store in defined queue file.

```cpp
uint8_t errorQueueCount(FirebaseData *fbdo, const char *filename, fb_esp_mem_storage_type storageType);
```






#### Determine number of queues in Firebase Data object's Error Queues collection.

param **`fbdo`** The pointer to Firebase Data Object.

return **`Number`** (0-255) of queues in Firebase Data object queue collection.

```cpp
uint8_t errorQueueCount(FirebaseData *fbdo);
```








#### Determine whether the Firebase Error Queues collection was full or not.

param **`fbdo`** The pointer to Firebase Data Object.

return **`Boolean`** value, indicates the full of queue.

```cpp
bool isErrorQueueFull(FirebaseData *fbdo);
```








#### Process all failed Firebase operation queue items when network is available.

param **`fbdo`** The pointer to Firebase Data Object.

param **`callback`** Callback function that accepts QueueInfo parameter.
  
```cpp
void processErrorQueue(FirebaseData *fbdo, QueueInfoCallback callback = NULL);
```






#### Return Firebase Error Queue ID of last Firebase Error. 

Return 0 if there is no Firebase Error from last operation.

param **`fbdo`** The pointer to Firebase Data Object.
    
return **`Number`** of Queue ID.

```cpp
uint32_t getErrorQueueID(FirebaseData *fbdo);
```







#### Determine whether the Firebase Error Queue currently exists in the Error Queue collection or not.

param **`fbdo`** The pointer to Firebase Data Object.

param **`errorQueueID`** The Firebase Error Queue ID get from getErrorQueueID.
    
return - **`Boolean`** typestatus indicates the queue existence.

```cpp
bool isErrorQueueExisted(FirebaseData *fbdo, uint32_t errorQueueID);
```







#### Start the Firebase Error Queues Auto Run Process.

param **`fbdo`** The pointer to Firebase Data Object.

param **`callback`** The Callback function that accepts QueueInfo Object as a parameter, optional.

ESP32 only parameter
param **`queueTaskStackSize`** The stream task (RTOS task) reserved stack memory in byte (optional) (8192 is default).

The following functions are available from QueueInfo Object accepted by the callback.

**queueInfo.totalQueues()**, get the total Error Queues in Error Queue Collection.

**queueInfo.currentQueueID()**, get current Error Queue ID that being process.

**queueInfo.isQueueFull()**, determine whether Error Queue Collection is full or not.

**queueInfo.dataType()**, get a string of the Firebase call data type that being process of current Error Queue.

**queueInfo.method()**, get a string of the Firebase call method that being process of current Error Queue.

**queueInfo.path()**, get a string of the Firebase call path that being process of current Error Queue.

```cpp
void beginAutoRunErrorQueue(FirebaseData *fbdo, FirebaseData::QueueInfoCallback callback = NULL, size_t queueTaskStackSize = 8192);

void beginAutoRunErrorQueue(FirebaseData *fbdo, FirebaseData::QueueInfoCallback callback = NULL);
```





#### Stop the Firebase Error Queues Auto Run Process.

param **`fbdo`** The pointer to Firebase Data Object.

```cpp
void endAutoRunErrorQueue(FirebaseData *fbdo);
```






#### Clear all Firbase Error Queues in Error Queue collection.

param **`fbdo`** The pointer to Firebase Data Object.

```cpp
void clearErrorQueue(FirebaseData *fbdo);
```





## Firebase Cloud Firestore Functions


These functions can be called directly from Firestore object in the Firebase object e.g. Firebase.Firestore.[Function Name]




#### Export the documents in the database to the Firebase Storage data bucket.

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`databaseId`** The Firebase Cloud Firestore database id which is (default) or empty "".

param **`bucketID`** The Firebase storage bucket ID in the project.

param **`storagePath`** The path in the Firebase Storage data bucket to store the exported database.

param **`collectionIds`** Which collection ids to export. Unspecified means all collections. 

Use comma (,) to separate between the collection ids.

return **`Boolean`** value, indicates the success of the operation.

Use FirebaseData.payload() to get the returned payload.

This function requires OAuth2.0 authentication.

```cpp
bool exportDocuments(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *bucketID, const char *storagePath, const char *collectionIds = "");
```






#### Import the exported documents stored in the Firebase Storage data bucket.

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`databaseId`** The Firebase Cloud Firestore database id which is (default) or empty "".

param **`bucketID`** The Firebase storage bucket ID in the project.

param **`storagePath`** The path in the Firebase Storage data bucket that stores the exported database.

param **`collectionIds`** Which collection ids to import. Unspecified means all collections included in the import. 

Use comma (,) to separate between the collection ids.

return **`Boolean`** value, indicates the success of the operation.

Use FirebaseData.payload() to get the returned payload.

This function requires OAuth2.0 authentication.

```cpp
bool importDocuments(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *bucketID, const char *storagePath, const char *collectionIds = "");
```







#### Create a document at the defined document path.

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`databaseId`** The Firebase Cloud Firestore database id which is (default) or empty "".

param **`documentPath`** The relative path of document to create in the collection.

param **`content`** A Firestore document. 

See https://firebase.google.com/docs/firestore/reference/rest/v1/projects.databases.documents#Document

param **`mask`** The fields to return. If not set, returns all fields. 

Use comma (,) to separate between the field names.

return **`Boolean`** value, indicates the success of the operation.

Use FirebaseData.payload() to get the returned payload.

This function requires Email/password, Custom token or OAuth2.0 authentication.

```cpp
bool createDocument(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *documentPath, const char *content, const char *mask = "");
```







#### Create a document in the defined collection id.

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`databaseId`** The Firebase Cloud Firestore database id which is (default) or empty "".

param **`collectionId`** The relative path of document collection id to create the document.

param **`documentId`** The document id of document to be created.

param **`content`** A Firestore document. 

See https://firebase.google.com/docs/firestore/reference/rest/v1/projects.databases.documents#Document

param **`mask`** The fields to return. If not set, returns all fields. 

Use comma (,) to separate between the field names.

return **`Boolean`** value, indicates the success of the operation.

Use FirebaseData.payload() to get the returned payload.

This function requires Email/password, Custom token or OAuth2.0 authentication.

```cpp
bool createDocument(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *collectionId, const char *documentId, const char *content, const char *mask = "");
```







#### Patch or update a document at the defined path.

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`databaseId`** The Firebase Cloud Firestore database id which is (default) or empty "".

param **`documentPath`** The relative path of document to patch with the input document.

param **`content`** A Firestore document. 

See https://firebase.google.com/docs/firestore/reference/rest/v1/projects.databases.documents#Document

param **`updateMask`** The fields to update. If the document exists on the server and has fields not referenced in the mask, they are left unchanged.

Fields referenced in the mask, but not present in the input document (content), are deleted from the document on the server. 

Use comma (,) to separate between the field names. 

param **`mask`** The fields to return. If not set, returns all fields. 

If the document has a field that is not present in this mask, that field will not be returned in the response. 

Use comma (,) to separate between the field names.

param **`exists`** When set to true, the target document must exist. When set to false, the target document must not exist.

param **`updateTime`** When set, the target document must exist and have been last updated at that time. 

A timestamp in RFC3339 UTC "Zulu" format, with nanosecond resolution and up to nine fractional digits. 

Examples: "2014-10-02T15:01:23Z" and "2014-10-02T15:01:23.045123456Z". 

return **`Boolean`** value, indicates the success of the operation.

Use FirebaseData.payload() to get the returned payload.

This function requires Email/password, Custom token or OAuth2.0 authentication.

```cpp
bool patchDocument(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *documentPath, const char *content, const char *updateMask, const char *mask = "", const char *exists = "", const char *updateTime = "");
```







#### Get a document at the defined path.

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`databaseId`** The Firebase Cloud Firestore database id which is (default) or empty "".

param **`documentPath`** The relative path of document to get.

param **`mask`** The fields to return. If not set, returns all fields. 

If the document has a field that is not present in this mask, that field will not be returned in the response. 

Use comma (,) to separate between the field names.

param **`transaction`** Reads the document in a transaction. A base64-encoded string.

param **`readTime`** Reads the version of the document at the given time. This may not be older than 270 seconds.

A timestamp in RFC3339 UTC "Zulu" format, with nanosecond resolution and up to nine fractional digits. 

Examples: "2014-10-02T15:01:23Z" and "2014-10-02T15:01:23.045123456Z".

return **`Boolean`** value, indicates the success of the operation.

Use FirebaseData.payload() to get the returned payload.

This function requires Email/password, Custom token or OAuth2.0 authentication.

```cpp
bool getDocument(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *documentPath, const char *mask = "", const char *transaction = "", const char *readTime = "");
```







#### Get a document at the defined path.

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`databaseId`** The Firebase Cloud Firestore database id which is (default) or empty "".

param **`documentPath`** The relative path of document to get.

param **`structuredQuery`** The pointer to FirebaseJson object that contains the Firestore query. For the description of structuredQuery, see https://cloud.google.com/firestore/docs/reference/rest/v1/StructuredQuery

param **`consistencyMode`** Optional. The consistency mode for this transaction 
e.g. fb_esp_firestore_consistency_mode_transaction,
fb_esp_firestore_consistency_mode_newTransaction
and fb_esp_firestore_consistency_mode_readTime

param **`consistency`** Optional. The value based on consistency mode e.g. transaction string, TransactionOptions (JSON) and date time string.

For more description, see https://cloud.google.com/firestore/docs/reference/rest/v1/projects.databases.documents/runQuery#body.request_body.FIELDS

return **`Boolean`** value, indicates the success of the operation.

Use FirebaseData.payload() to get the returned payload.

This function requires Email/password, Custom token or OAuth2.0 authentication.

```cpp
bool runQuery(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *documentPath, const char *structuredQuery, fb_esp_firestore_consistency_mode consistencyMode, const char *consistency);
```







#### Delete a document at the defined path.

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`databaseId`** The Firebase Cloud Firestore database id which is (default) or empty "".

param **`documentPath`** The relative path of document to delete.

param **`exists`** When set to true, the target document must exist. When set to false, the target document must not exist.

param **`updateTime`** When set, the target document must exist and have been last updated at that time.

A timestamp in RFC3339 UTC "Zulu" format, with nanosecond resolution and up to nine fractional digits.

Examples: "2014-10-02T15:01:23Z" and "2014-10-02T15:01:23.045123456Z".

return **`Boolean`** value, indicates the success of the operation.

This function requires Email/password, Custom token or OAuth2.0 authentication.

```cpp
bool deleteDocument(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *documentPath, const char *exists = "", const char *updateTime = "");
```






#### List the documents in the defined documents collection.

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`databaseId`** The Firebase Cloud Firestore database id which is (default) or empty "".

param **`collectionId`** The relative path of document colection.

param **`pageSize`** The maximum number of documents to return.

param **`pageToken`** The nextPageToken value returned from a previous List request, if any.

param **`orderBy`** The order to sort results by. For example: priority desc, name.

param **`mask`** The fields to return. If not set, returns all fields.

If a document has a field that is not present in this mask, that field will not be returned in the response.

param **`showMissing`** If the list should show missing documents. 

A missing document is a document that does not exist but has sub-documents.

return **`Boolean`** value, indicates the success of the operation.

Use FirebaseData.payload() to get the returned payload.

This function requires Email/password, Custom token or OAuth2.0 authentication (when showMissing is true).

```cpp
bool listDocuments(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *collectionId, int pageSize, const char *pageToken, const char *orderBy, const char *mask, bool showMissing);
```
   





#### List the document collection ids in the defined document path.

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`databaseId`** The Firebase Cloud Firestore database id which is (default) or empty "".

param **`documentPath`** The relative path of document to get its collections' id.

param **`pageSize`** The maximum number of results to return.

param **`pageToken`** The nextPageToken value returned from a previous List request, if any.

return **`Boolean`** value, indicates the success of the operation.

Use FirebaseData.payload() to get the returned payload.

This function requires Email/password, Custom token or OAuth2.0 authentication (when showMissing is true).

```cpp
bool listCollectionIds(FirebaseData *fbdo, const char *projectId, const char *databaseId, const char *documentPath, int pageSize, const char *pageToken);
```








## Firebase Cloud Messaging Functions


These functions can be called directly from FCM object in the Firebase object e.g. Firebase.FCM.[Function Name]




#### Clear all Firbase Error Queues in Error Queue collection.

param **`fbdo`** The pointer to Firebase Data Object.


```cpp
void clearErrorQueue(FirebaseData *fbdo);
```





#### Set the server key.

param **`serverKey`** Server key found on Console: Project settings > Cloud Messaging

@note This server key required for sending message via legacy HTTP API.

The API key created in the Google Cloud console, cannot be used for authorizing FCM requests. 

```cpp
void setServerKey(const char *serverKey);
```






#### Send Firebase Cloud Messaging to the devices with JSON payload using the FCM legacy API.

param **`fbdo`** The pointer to Firebase Data Object.

param **`msg`** The pointer to the message to send which is the FCM_Legacy_JSON_Message type data.

return **`Boolean`** value, indicates the success of the operation. 

The FCM_Legacy_JSON_Message properties are

targets - The targets of messages e.g. to, registration_ids, condition.

options - The options of message contained the sub-properties e.g, collapse_key, priority, content_available, 
mutable_content,time_to_live, restricted_package_name, and dry_run.

The sub-properties value of the options should be assigned in string.

payloads - The two payloads i.e. notification and data.

The payloads.notification properties are available e.g.

| Name | Type | Platform |
| ----- | ----- | ----- |
| title | string | all |
| body | string | all |
| icon | string | Andoid, web |
| click_action | string | all |
| sound | string | iOS, Android |
| badge | number | iOS |
| subtitle | string | iOS |
| body_loc_key | string | iOS, Android |
| body_loc_args | JSON array of string | iOS, Android |
| title_loc_key | string  iOS, Android |
| title_loc_args | JSON array of string | iOS, Android |
| android_channel_id | string | Android |
| tag | string | Android |
| color | string | Android |

The payloads.data is the JSON object. 

Read more details about legacy HTTP API here https://firebase.google.com/docs/cloud-messaging/http-server-ref

```cpp
bool send(FirebaseData *fbdo, FCM_Legacy_HTTP_Message *msg);
```






#### Send Firebase Cloud Messaging to the devices using the FCM HTTP v1 API.

param **`fbdo`** The pointer to Firebase Data Object.

param **`msg`** The pointer to the message to send which is the FCM_HTTPv1_JSON_Message type data.

return **`Boolean`** value, indicates the success of the operation. 

Read more details about HTTP v1 API here https://firebase.google.com/docs/reference/fcm/rest/v1/projects.messages
```cpp
bool send(FirebaseData *fbdo, FCM_HTTPv1_JSON_Message *msg);
```





#### Subscribe the devices to the topic.

param **`fbdo`** The pointer to Firebase Data Object.

param **`topic`** The topic to subscribe.

param **`IID`** The instance ID tokens or registration tokens array.

param **`numToken`** The size of instance ID tokens array.

return **`Boolean`** value, indicates the success of the operation. 

```cpp
bool subscibeTopic(FirebaseData *fbdo, const char *topic, const char *IID[], size_t numToken);
```






#### Unsubscribe the devices from the topic.

param **`fbdo`** The pointer to Firebase Data Object.

param **`topic`** The topic to romove the subscription.

param **`IID`** The instance ID tokens or registration tokens array.

param **`numToken`** The size of instance ID tokens array.

return **`Boolean`** value, indicates the success of the operation. 

```cpp
bool unsubscibeTopic(FirebaseData *fbdo, const char *topic, const char *IID[], size_t numToken);
```






#### Get the app instance info.

param **`fbdo`** The pointer to Firebase Data Object.

param **`IID`** The instance ID token of device.

return **`Boolean`** value, indicates the success of the operation. 

```cpp
bool appInstanceInfo(FirebaseData *fbdo, const char *IID);
```






#### Create registration tokens for APNs tokens.

param **`fbdo`** The pointer to Firebase Data Object.

param **`application`** The Bundle id of the app.

param **`sandbox`** The Boolean to indicate sandbox environment (TRUE) or production (FALSE).

param **`APNs`** The iOS APNs tokens array.

param **`numToken`** The size of instance ID tokens array.

return **`Boolean`** value, indicates the success of the operation. 

```cpp
bool regisAPNsTokens(FirebaseData *fbdo, const char *application, bool sandbox, const char *APNs[], size_t numToken);
```






#### Get the server payload.

param **`fbdo`** The pointer to Firebase Data Object.

return **`String`** of payload returned from the server.

```cpp
String payload(FirebaseData *fbdo);
```




## Firebase Storage Functions.

These functions can be called directly from Storage object in the Firebase object e.g. Firebase.Storage.[Function Name]



#### Upload file to the Firebase Storage data bucket.

param **`fbdo`** The pointer to Firebase Data Object.

param **`bucketID`** The Firebase storage bucket ID in the project.

param **`localFileName`** The file path includes its name to upload.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd.

The file systems can be changed in FirebaseFS.h.

param **`remotetFileName`** The file path includes its name of uploaded file in data bucket.

param **`mime`** The file MIME type

return **`Boolean`** value, indicates the success of the operation. 

Use FirebaseData.downloadURL() to get the download link.

```cpp
bool upload(FirebaseData *fbdo, const char *bucketID, const char *localFileName, fb_esp_mem_storage_type storageType, const char *remotetFileName, const char *mime);
```







#### Upload byte array to the Firebase Storage data bucket.

param **`fbdo`** The pointer to Firebase Data Object.

param **`bucketID`** The Firebase storage bucket ID in the project.

param **`data`** The byte array of data.

param **`len`** The size of byte array data in bytes.

param **`remotetFileName`** The file path includes its name of uploaded file in data bucket.

param **`mime`** The file MIME type

return **`Boolean`** value, indicates the success of the operation. 

Use FirebaseData.downloadURL() to get the download link.

```cpp
bool upload(FirebaseData *fbdo, const char *bucketID, uint8_t *data, size_t len, const char *remoteFileName, const char *mime);
```




#### Download file from the Firebase Storage data bucket.

param **`fbdo`** The pointer to Firebase Data Object.

param **`bucketID`** The Firebase storage bucket ID in the project.

param **`remotetFileName`** The file path includes its name of file in the data bucket to download.

param **`localFileName`** The file path includes its name to save.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd.

The file systems can be changed in FirebaseFS.h.

return **`Boolean`** value, indicates the success of the operation. 

```cpp
bool download(FirebaseData *fbdo, const char *bucketID, const char *remoteFileName, const char *localFileName, fb_esp_mem_storage_type storageType);
```





#### Get the meta data of file in Firebase Storage data bucket

param **`fbdo`** The pointer to Firebase Data Object.

param **`bucketID`** The Firebase storage bucket ID in the project.

param **`remotetFileName`** The file path includes its name of file in the data bucket.

return **`Boolean`** value, indicates the success of the operation. 

Use the FileMetaInfo type data to get name, bucket, contentType, size, 
generation, etag, crc32, downloadTokens properties from file.

```cpp
bool getMetadata(FirebaseData *fbdo, const char *bucketID, const char *remoteFileName);
```





#### Delete file from Firebase Storage data bucket

param **`fbdo`** The pointer to Firebase Data Object.

param **`bucketID`** The Firebase storage bucket ID in the project.

param **`remotetFileName`** The file path includes its name of file in the data bucket.

return **`Boolean`** value, indicates the success of the operation. 

```cpp
bool deleteFile(FirebaseData *fbdo, const char *bucketID, const char *fileName);
```




#### List all files in the Firebase Storage data bucket.

param **`fbdo`** The pointer to Firebase Data Object.

param **`bucketID`** The Firebase storage bucket ID in the project.

return **`Boolean`** value, indicates the success of the operation. 

Use the FileList type data to get name and bucket properties for each item.

```cpp
bool listFiles(FirebaseData *fbdo, const char *bucketID);
```








## Google Cloud Storage Functions.

These functions can be called directly from GCStorage object in the Firebase object e.g. Firebase.GCStorage.[Function Name]



#### Upload file to the Google Cloud Storage data bucket.

param **`fbdo`** The pointer to Firebase Data Object.

param **`bucketID`** The Firebase or Google Cloud Storage bucket ID.

param **`localFileName`** The file path includes its name to upload.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd.

The file systems can be changed in FirebaseFS.h.

param **`uploadType`** The enum of type of upload methods e.g. gcs_upload_type_simple, gcs_upload_type_multipart, gcs_upload_type_resumable

param **`remotetFileName`** The file path includes its name of uploaded file in data bucket.

param **`mime`** The file MIME type.

param **`uploadOptions`** Optional. The UploadOptions data contains the query parameters options.

For query parameters options, see https://cloud.google.com/storage/docs/json_api/v1/objects/insert#optional-parameters

param **`requestProps`** Optional. The RequestProperties data contains the request payload properties.

For request payload properties, see https://cloud.google.com/storage/docs/json_api/v1/objects/insert#optional-properties

param **`status`** Optional. The UploadStatusInfo data to get the upload status.

param **`callback`** Optional. The callback function that accept UploadStatusInfo data.

return **`Boolean`** value, indicates the success of the operation. 

This function requires OAuth2.0 authentication.

The upload types of methods can be selectable.

The gcs_upload_type_simple upload type is used for small file upload in a single request without metadata.

gcs_upload_type_multipart upload type is for small file upload in a single reques with metadata.

gcs_upload_type_resumable upload type is for medium or large file (larger than or equal to 256 256 KiB) upload with metadata and can be resumable.

The upload with metadata supports allows the library to add the metadata internally for Firebase to request the download access token in Firebase Storage bucket.

User also can add custom metadata for the uploading file (object).

```cpp
bool upload(FirebaseData *fbdo, const char *bucketID, const char *localFileName, fb_esp_mem_storage_type storageType, fb_esp_gcs_upload_type uploadType, const char *remoteFileName, const char *mime, UploadOptions *uploadOptions = nullptr, RequestProperties *requestProps = nullptr, UploadStatusInfo *status = nullptr, ProgressCallback callback = NULL);
```






#### Downoad file from the Google Cloud Storage data bucket.

param **`fbdo`** The pointer to Firebase Data Object.

param **`bucketID`** The Firebase or Google Cloud Storage bucket ID.

param **`remotetFileName`** The file path includes its name of file in the data bucket to download.

param **`localFileName`** The file path includes its name to save.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd.

The file systems can be changed in FirebaseFS.h.

param **`option`** Optional. The pointer to StorageGetOptions data that contains the get query parameters.

For the query parameters options, see https://cloud.google.com/storage/docs/json_api/v1/objects/get#optional-parameters

return **`Boolean`** value, indicates the success of the operation. 

This function requires OAuth2.0 authentication.

```cpp
bool download(FirebaseData *fbdo, const char *bucketID, const char *remoteFileName, const char *localFileName, fb_esp_mem_storage_type storageType, StorageGetOptions *options = nullptr);
```





####  Get the meta data of file in Firebase or Google Cloud Storage data bucket.

param **`fbdo`** The pointer to Firebase Data Object.

param **`bucketID`** The Firebase or Google Cloud Storage bucket ID.

param **`remotetFileName`** The file path includes its name of file in the data bucket.

param **`options`** Optional. The pointer to StorageGetOptions data that contains the get query parameters.

For the query parameters options, see https://cloud.google.com/storage/docs/json_api/v1/objects/get#optional-parameters

return **`Boolean`** value, indicates the success of the operation. 

Use the FileMetaInfo type data to get name, bucket, contentType, size, 
generation, metageneration, etag, crc32, downloadTokens properties from file.

```cpp
bool getMetadata(FirebaseData *fbdo, const char *bucketID, const char *remoteFileName);
```





#### Delete file from Firebase or Google Cloud Storage data bucket.

param **`fbdo`** The pointer to Firebase Data Object.

param **`bucketID`** The Firebase or Google Cloud Storage bucket ID.

param **`remotetFileName`** The file path includes its name of file in the data bucket.

param **`options`** Optional. The pointer to DeleteOptions data contains the query parameters.

For query parameters options, see https://cloud.google.com/storage/docs/json_api/v1/objects/delete#optional-parameters

return **`Boolean`** value, indicates the success of the operation. 

```cpp
bool deleteFile(FirebaseData *fbdo, const char *bucketID, const char *fileName, DeleteOptions *options = nullptr);
```




#### List all files in the Firebase or Google Cloud Storage data bucket.

param **`fbdo`** The pointer to Firebase Data Object.

param **`bucketID`** The Firebase or Google Cloud Storage bucket ID.

param **`options`** Optional. The pointer to ListOptions data that contains the query parameters

Fore query parameters description, see https://cloud.google.com/storage/docs/json_api/v1/objects/list#optional-parameters

return **`Boolean`** value, indicates the success of the operation. 

Use the FileList type data to get name and bucket properties for each item.

```cpp
bool listFiles(FirebaseData *fbdo, const char *bucketID, ListOptions *options = nullptr);
```









## Cloud Functions for Firebase Functions


These functions can be called directly from Functions object in the Firebase object e.g. Firebase.Functions.[Function Name]




#### Synchronously invokes a deployed Cloud Function. 

To be used for testing purposes as very limited traffic is allowed. 

For more information on the actual limits, refer to Rate Limits. https://cloud.google.com/functions/quotas#rate_limits

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`locationId`** The project location.

param **`functionId`** The name of function.

param **`data`** The Input to be passed to the function.

return **`Boolean`** value, indicates the success of the operation. 

Use FirebaseData.payload() to get the returned payload.

This function requires OAuth2.0 authentication.

```cpp
bool callFunction(FirebaseData *fbdo, const char *projectId, const char *locationId, const char *functionId, const char *data);
```







#### Creates a new function. 

If a function with the given name already exists in the specified project, the long running operation will return ALREADY_EXISTS error.

param **`fbdo`** The pointer to Firebase Data Object.

param **`config`** The pointer to FunctionsConfig object that encapsulates the function and triggers configurationston.

param **`callback`** The callback function to get the Cloud Function creation status.

return **`Boolean`** value, indicates the success of the operation. 

Use FirebaseData.payload() to get the returned payload.

This function requires OAuth2.0 authentication.

```cpp
bool createFunction(FirebaseData *fbdo, FunctionsConfig *config, FunctionsOperationCallback callback = NULL);
```






#### Creates a new function. 

If a function with the given name already exists in the specified project, the long running operation will return ALREADY_EXISTS error.

param **`fbdo`** The pointer to Firebase Data Object.

param **`config`** The pointer to FunctionsConfig object that encapsulates the function and triggers configurationston.

param **`statusInfo`** The pointer to FunctionsOperationStatusInfo data to get the Cloud Function creation status later.

return **`Boolean`** value, indicates the success of the operation. 

Use FirebaseData.payload() to get the returned payload.

This function requires OAuth2.0 authentication.

```cpp
bool createFunction(FirebaseData *fbdo, FunctionsConfig *config, FunctionsOperationStatusInfo *statusInfo);
```




#### Updates existing function. 

param **`fbdo`** The pointer to Firebase Data Object.

param **`functionId`** The name of function.

param **`patchData`** The pointer to FunctionsConfig object that encapsulates the function and triggers configurationston.

return **`Boolean`** value, indicates the success of the operation. 

Use FirebaseData.payload() to get the returned payload.

This function requires OAuth2.0 authentication.

```cpp
bool patchFunction(FirebaseData *fbdo, const char *functionId, FunctionsConfig *patchData);
```






#### Sets the IAM access control policy on the specified function. Replaces any existing policy.

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`locationId`** The project location.

param **`functionId`** The name of function.

param **`policy`** The pointer to PolicyBuilder data concapsulates the policy configuration.

The complete policy to be applied to the resource.

param **`updateMask`** A FieldMask specifying which fields of the policy to modify. 

Only the fields in the mask will be modified. If no mask is provided, the following default mask is used:

paths: "bindings, etag"

A comma-separated list of fully qualified names of fields. Example: "user.displayName,photo"

return **`Boolean`** value, indicates the success of the operation. 

Use FirebaseData.payload() to get the returned payload.

This function requires OAuth2.0 authentication.

```cpp
bool setIamPolicy(FirebaseData *fbdo, const char *projectId, const char *locationId, const char *functionId, PolicyBuilder *policy, const char *updateMask = "");
```






#### Gets the IAM access control policy for a function. 

Returns an empty policy if the function exists and does not have a policy set.

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`locationId`** The project location.

param **`functionId`** The name of function.

param **`version`** Optional. The policy format version to be returned.

Valid values are 0, 1, and 3. Requests specifying an invalid value will be rejected.

return **`Boolean`** value, indicates the success of the operation. 

Use FirebaseData.payload() to get the returned payload.

This function requires OAuth2.0 authentication.

```cpp
bool getIamPolicy(FirebaseData *fbdo, const char *projectId, const char *locationId, const char *functionId, const char *version = "");
```






#### Returns a function with the given name from the requested project. 

Returns an empty policy if the function exists and does not have a policy set.

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`locationId`** The project location.

param **`functionId`** The name of function.

return **`Boolean`** value, indicates the success of the operation. 

Use FirebaseData.payload() to get the returned payload.

This function requires OAuth2.0 authentication.

```cpp
bool getFunction(FirebaseData *fbdo, const char *projectId, const char *locationId, const char *functionId);
```






#### Deletes a function with the given name from the specified project. 

If the given function is used by some trigger, the trigger will be updated to remove this function. 

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`locationId`** The project location.

param **`functionId`** The name of function.

return **`Boolean`** value, indicates the success of the operation. 

Use FirebaseData.payload() to get the returned payload.

This function requires OAuth2.0 authentication.

```cpp
bool deleteFunction(FirebaseData *fbdo, const char *projectId, const char *locationId, const char *functionId);
```






#### Returns a signed URL for downloading deployed function source code. 

The URL is only valid for a limited period and should be used within minutes after generation.

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`locationId`** The project location.

param **`functionId`** The name of function.

param **`versionId`** The optional version of function. If not set, default, current version is used.

return **`Boolean`** value, indicates the success of the operation. 

Use FirebaseData.payload() to get the returned payload.

This function requires OAuth2.0 authentication.

```cpp
bool generateDownloadUrl(FirebaseData *fbdo, const char *projectId, const char *locationId, const char *functionId, const char *versionId = "");
```






#### Returns a signed URL for uploading a function source code. 

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`locationId`** The project location.

return **`Boolean`** value, indicates the success of the operation. 

Use FirebaseData.payload() to get the returned payload.

This function requires OAuth2.0 authentication.

```cpp
bool generateUploadUrl(FirebaseData *fbdo, const char *projectId, const char *locationId);
```





#### Returns a list of functions that belong to the requested project.

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`locationId`** The project location.

param **`pageSize`** Maximum number of functions to return per call.

param **`pageToken`** The value returned by the last ListFunctionsResponse; indicates that this is a continuation of a prior functions.list call, and that the system should return the next page of data.

return **`Boolean`** value, indicates the success of the operation. 

Use FirebaseData.payload() to get the returned payload.

This function requires OAuth2.0 authentication.

```cpp
bool listFunctions(FirebaseData *fbdo, const char *projectId, const char *locationId, size_t pageSize, const char *pageToken = "");
```





#### Returns a function with the given name from the requested project.

param **`fbdo`** The pointer to Firebase Data Object.

param **`filter`** filter The Firebase project id (only the name without the firebaseio.com).

A filter for matching the requested operations.

The supported formats of filter are: 

To query for a specific function:

project:*,location:*,function:*

To query for all of the latest operations for a project:

project:*,latest:true

param **`pageSize`** he maximum number of records that should be returned.

Requested page size cannot exceed 100. If not set, the default page size is 100

param **`pageToken`** Token identifying which result to start with, which is returned by a previous list call.

return **`Boolean`** value, indicates the success of the operation. 

Use FirebaseData.payload() to get the returned payload.

This function requires OAuth2.0 authentication.

```cpp
bool listOperations(FirebaseData *fbdo, const char *filter, int pageSize, const char *pageToken);
```





## PolicyBuilder and FunctionsConfig classes

The description of PolicyBuilder and FunctionsConfig classes and their functions are available in the header files
[/src/functions/PolicyBuilder.h](/functions/PolicyBuilder.h) and [/src/functions/FunctionsConfig.h](/functions/FunctionsConfig.h).






## Firebase Data Object Functions



#### Set the receive and transmit buffer memory size for secured mode BearSSL WiFi client.

param **`rx`** The number of bytes for receive buffer memory for secured mode BearSSL (512 is minimum, 16384 is maximum).

param **`tx`** The number of bytes for transmit buffer memory for secured mode BearSSL (512 is minimum, 16384 is maximum). 


Set this option to false to support get large Blob and File operations.

```cpp
void void setBSSLBufferSize(uint16_t rx, uint16_t tx);
```






#### Set the http response size limit.

param **`len`** The server response buffer size limit.

```cpp
void setResponseSize(uint16_t len);
```







#### Get WiFi client instance


return **`WiFi client instance`**.

```cpp
WiFiClientSecure *getWiFiClient();
```





#### Pause/Unpause WiFiClient from all Firebase operations

param **`pause`** True for pause and False for unpause.

return **`Boolean`** value, indicates the success of the operation.

```cpp
bool pauseFirebase(bool pause);
```






#### Determine the data type of payload returned from the server

return **`The one of these data type e.g. integer, float, string, JSON and blob.`**

```cpp
String dataType();
```






#### Determine the event type of stream

return **`The one of these event type String e.g. put, patch, cancel, and auth_revoked.`**

The event type "put" indicated that data at the event path relative to the stream path was completely changed. The event path can be determined by dataPath().

The event type "patch" indicated that data at the event path relative to stream path was updated. The event path can be determined by dataPath().

The event type "cancel" indicated something wrong and cancel by the server.

The event type "auth_revoked" indicated the provided Firebase Authentication Data (Database secret) is no longer valid.

```cpp
String eventType();
```






#### Determine the unique identifier (ETag) of current data

return **`String.`** of unique identifier.

```cpp
String ETag();
```






#### Determine the current stream path

return **`The database streaming path.`**

```cpp
String streamPath();
```







#### Determine the current data path

return **`The node which belongs to server' s returned payload.`**

The node returned from this function in case of stream, also changed upon the child or parent's stream
value changes.

```cpp
String dataPath();
```







#### Determine the error reason String from the process

return **`The error description string (String object).`**

```cpp
String errorReason();
```







#### Return the integer data of server returned payload

return **`Integer value.`**

```cpp
int intData();
```







#### Return the float data of server returned payload

return **`Float value.`**

```cpp
float floatData();
```





#### Return the double data of server returned payload

return **`Double value.`**

```cpp
float doubleData();
```





#### Return the Boolean data of server returned payload

return **`Boolean value.`**

```cpp
float boolData();
```






#### Return the String data of server returned payload

return **`String (String object).`**

```cpp
String stringData();
```







#### Return the JSON String data of server returned payload

return **`String (String object).`**

```cpp
String jsonString();
```





#### Return the Firebase JSON object of server returned payload.

return **`FirebaseJson object.`**

```cpp
FirebaseJson &jsonObject();
```





#### Return the Firebase JSON object pointer of server returned payload.

return **`FirebaseJson object `**pointer.

```cpp
FirebaseJson *jsonObjectPtr();
```





#### Return the Firebase JSON Array object of server returned payload.

return **`FirebaseJsonArray object`**.

```cpp
FirebaseJsonArray &jsonArray();
```




#### Return the Firebase JSON Array object pointer of server returned payload.

return **`FirebaseJsonArray object pointer`**.

```cpp
FirebaseJsonArray *jsonArrayPtr();
```





#### the Firebase JSON Data object that keeps the get(parse) result.

return **`FirebaseJsonData object `**.

```cpp
FirebaseJsonData &jsonData();
```






#### the Firebase JSON Data object pointer that keeps the get(parse) result.

return **`FirebaseJsonData object `**pointer.

```cpp
FirebaseJsonData *jsonData();
```





#### Return the blob data (uint8_t) array of server returned payload

return **`Dynamic array`** of 8-bit unsigned integer i.e. `std::vector<uint8_t>`.

```cpp
std::vector<uint8_t> blobData();
```







#### Return the new appended node's name or key of server returned payload when calling pushXXX function

return **`String`** (String object).

```cpp
String pushName();
```







#### Determine the stream connection status

return **`Boolean`** type status indicates whether the Firebase Data object is working with the stream or not.

```cpp
bool isStream();
```






#### Determine the server connection status

return **`Boolean`** type status indicates whether the Firebase Data object is connected to the server or not.

```cpp
bool httpConnected();
```





#### Determine the timeout event of server's stream (30 sec is the default)

Nothing to do when stream connection timeout, the stream connection will be automatically resumed.

return **`Boolean`** type status indicates whether the stream was time out or not.

```cpp
bool streamTimeout();
```





#### Determine the availability of data or payload returned from the server

return **`Boolean`** type status indicates whether the server returns the new payload or not.

```cpp
bool dataAvailable();
```





#### Determine the availability of stream event-data payload returned from the server

return **`Boolean`** type status indicates whether the server returns the stream event-data 
payload or not.

```cpp
bool streamAvailable();
```





#### Determine the matching between data type that intend to get from/store to database and the server's return payload data type

return **`Boolean`** type status indicates whether the type of data being get from/store to database 
and the server's returned payload is matched or not.

```cpp
bool mismatchDataType();
```





#### Determine the HTTP status code return from the server

return **`Integer`** number of HTTP status.

```cpp
int httpCode();
```





#### Check overflow of the returned payload data buffer

return **`Boolean`** of the overflow status.


Total default HTTP response buffer size is 400 bytes which can be set through Firebase.setResponseSize.


```cpp
bool bufferOverflow();
```





#### Determine the name (full path) of backup file in SD card/flash memory

return **`String`** (String object) of a file name that stores on SD card/flash memory after backup operation.

```cpp
String getBackupFilename();
```
To use LittleFS file system for flash memory instead of FLASH, add the following macro in **FirebaseFS.h**

```cpp
#define USE_LITTLEFS
```




#### Determine the size of the backup file

return **`Number of byte`** of backup file in byte after backup operation.

```cpp
size_t getBackupFileSize();
```





#### Clear or empty data in the Firebase Data object

```cpp
void clear();
```





#### Determine the error description for file transferring (pushFile, setFile, backup and restore)

return **`Error description string* (String object).`**

```cpp
String fileTransferError();
```





#### Return the server's payload data

return **`Payload string* (String object).`**

```cpp
String payload();
```





## FirebaseJSON object Functions



#### Clear internal buffer of FirebaseJson object.
    
return **`instance of an object.`**

```cpp
FirebaseJson &clear();
```






#### Set JSON data (JSON object string) to FirebaseJson object.
    
param **`data`** The JSON object string.

return **`instance of an object.`**

```cpp
FirebaseJson &setJsonData(const String &data);
```






#### Add null to FirebaseJson object.
    
param **`key`** The new key string that null to be added.

return **`instance of an object.`**

```cpp
FirebaseJson &add(const String &key);
```






#### Add string to FirebaseJson object.
    
param **`key`** The new key string that string value to be added.

param **`value`** The String value for the new specified key.

return **`instance of an object.`**

```cpp
FirebaseJson &add(const String &key, const String &value);
```






#### Add string (chars array) to FirebaseJson object.
    
param **`key`** The new key string that string (chars array) value to be added.

param **`value`** The char array for the new specified key.

return **`instance of an object.`**

```cpp
FirebaseJson &add(const String &key, const char *value);
```






#### Add integer/unsigned short to FirebaseJson object.
    
param **`key`** The new key string that the value to be added.

param **`value`** The integer/unsigned short value for new specified key.

return **`instance of an object.`**

```cpp
FirebaseJson &add(const String &key, int value);
FirebaseJson &add(const String &key, unsigned short value);
```





#### Add float to FirebaseJson object.
    
param **`key`** The new key string that float value to be added.

param **`value`** The float value for the new specified key.

return **`instance of an object.`**

```cpp
FirebaseJson &add(const String &key, float value);
```






#### Add double to FirebaseJson object.
    
param **`key`** The new key string that double value to be added.

param **`value`** The double value for the new specified key.

return **`instance of an object.`**

```cpp
FirebaseJson &add(const String &key, double value);
```





#### Add boolean to FirebaseJson object.
    
param **`key`** The new key string that bool value to be added.

param **`value`** The boolean value for new specified key.

return **`instance of an object.`**

```cpp
FirebaseJson &add(const String &key, bool value);
```





#### Add nested FirebaseJson object into FirebaseJson object.
    
param **`key`** The new key string that FirebaseJson object to be added.

param **`json`** The FirebaseJson object for the new specified key.

return **`instance of an object.`**

```cpp
FirebaseJson &add(const String &key, FirebaseJson &json);
```






#### Add nested FirebaseJsonArray object into FirebaseJson object.
    
param **`key`** The new key string that FirebaseJsonArray object to be added.

param **`arr`** The FirebaseJsonArray for the new specified key.

return **`instance of an object.`**

```cpp
FirebaseJson &add(const String &key, FirebaseJsonArray &arr);
```






#### Get the FirebaseJson object serialized string.

param **`buf`** The returning String object.

param **`prettify`** Boolean flag for return the pretty format string i.e. with text indentation and newline. 


```cpp
void toString(String &buf, bool prettify = false);
```






#### Get the value from the specified node path in FirebaseJson object.

param **`jsonData`** The returning FirebaseJsonData that hold the returned data.

param **`path`** Relative path to the specific node in FirebaseJson object.

param **`prettify`** The bool flag for the prettifying string in FirebaseJsonData's stringValue.

return **`boolean status of the operation.`**

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

 
 ```cpp
 bool get(FirebaseJsonData &jsonData, const String &path, bool prettify = false);
 ```






#### Parse and collect all node/array elements in FirebaseJson object.  

param **`data`** The JSON data string to parse (optional for replacing the internal buffer with new data).

return **`number`** of child/array elements in FirebaseJson object.

 ```cpp
 size_t iteratorBegin(const char* data = NULL);
 ```






#### Get child/array elements from FirebaseJson objects at specified index.
    
param **`index`** The element index to get.

param **`type`** The integer which holds the type of data i.e. FirebaseJson::OBJECT and FirebaseJson::ARR

param **`key`** The string which holds the key/name of the object, can return empty String if the data type is an array.

param **`value`** The string which holds the value for the element key or array.   

 ```cpp
 void iteratorGet(size_t index, int &type, String &key, String &value);
 ```





#### Clear all iterator buffer (should be called since iteratorBegin was called).

 ```cpp
 void iteratorEnd();
 ```




#### Set null to FirebaseJson object at the specified node path.
    
param **`path`** The relative path that null to be set.


The relative path can be mixed with array index (number placed inside square brackets) and node names e.g. /myRoot/[2]/Sensor1/myData/[3].


```cpp
void set(const String &path);
```





#### Set String value to FirebaseJson object at the specified node path.
    
param **`path`** The relative path that string value to be set.

param **`value`** The string value to set.


The relative path can be mixed with array index (number placed inside square brackets) and node names 
e.g. /myRoot/[2]/Sensor1/myData/[3].

```cpp
void set(const String &path, const String &value);
```





#### Set string (chars array) value to FirebaseJson object at the specified node path.
    
param **`path`** The relative path that string (chars array) to be set.

param **`value`** The char array to set.

The relative path can be mixed with array index (number placed inside square brackets) and node names 
e.g. /myRoot/[2]/Sensor1/myData/[3].

```cpp
void set(const String &path, const char *value);
```





#### Set integer/unsigned short value to FirebaseJson object at specified node path.
    
param **`path`** The relative path that int value to be set.

param **`value`** The integer/unsigned short value to set.

The relative path can be mixed with array index (number placed inside square brackets) and node names 
e.g. /myRoot/[2]/Sensor1/myData/[3].

```cpp
void set(const String &path, int value);
void set(const String &path, unsigned short value);
```






#### Set the float value to FirebaseJson object at the specified node path.
    
param **`path`** The relative path that float value to be set.

param **`value`** The float value to set.

The relative path can be mixed with array index (number placed inside square brackets) and node names 
e.g. /myRoot/[2]/Sensor1/myData/[3].

```cpp
void set(const String &path, float value);
```






#### Set the double value to FirebaseJson object at the specified node path.
    
param **`path`** The relative path that double value to be set.

param **`value`** The double value to set.

The relative path can be mixed with array index (number placed inside square brackets) and node names 
e.g. /myRoot/[2]/Sensor1/myData/[3].

```cpp
void set(const String &path, double value);
```







#### Set the boolean value to FirebaseJson object at the specified node path.
    
param **`path`** The relative path that bool value to be set.

param **`value`** The boolean value to set.

The relative path can be mixed with array index (number placed inside square brackets) and node names 
e.g. /myRoot/[2]/Sensor1/myData/[3].

```cpp
void set(const String &path, bool value);
```







#### Set nested FirebaseJson object to FirebaseJson object at the specified node path.
    
param **`path`** The relative path that nested FirebaseJson object to be set.

param **`json`** The FirebaseJson object to set.

The relative path can be mixed with array index (number placed inside square brackets) and node names 
e.g. /myRoot/[2]/Sensor1/myData/[3].

 ```cpp
void set(const String &path, FirebaseJson &json);
```







#### Set nested FirebaseJsonAtrray object to FirebaseJson object at specified node path.
    
param **`path`** The relative path that nested FirebaseJsonAtrray object to be set.

param **`arr`** The FirebaseJsonAtrray object to set.

The relative path can be mixed with array index (number placed inside square brackets) and node names 
e.g. /myRoot/[2]/Sensor1/myData/[3].

```cpp
void set(const String &path, FirebaseJsonArray &arr);
```






#### Remove the specified node and its content.

param **`path`** The relative path to remove its contents/children.

return **`bool`** value represents the successful operation.

```cpp
bool remove(const String &path);
```





### FirebaseJsonArray object functions


#### Add null to FirebaseJsonArray object.

return **`instance of an object.`**

```cpp
FirebaseJsonArray &add();
```






#### Add string to FirebaseJsonArray object.

param **`value`** The String value to add.

return **`instance of an object.`**

```cpp
FirebaseJsonArray &add(const String &value);
```






#### Add string (chars arrar) to FirebaseJsonArray object.

param **`value`** The chars array to add.

return **`instance of an object.`**

```cpp
FirebaseJsonArray &add(const char *value);
```





#### Add integer/unsigned short to FirebaseJsonArray object.

param **`value`** The integer/unsigned short value to add.

return **`instance of an object.`**

```cpp
FirebaseJsonArray &add(int value);
FirebaseJsonArray &add(unsigned short value);
```





#### Add float to FirebaseJsonArray object.

param **`value`** The float value to add.

return **`instance of an object.`**

```cpp
FirebaseJsonArray &add(float value);
```






#### Add double to FirebaseJsonArray object.

param **`value`** The double value to add.

return **`instance of an object.`**

```cpp
FirebaseJsonArray &add(double value);
```






#### Add boolean to FirebaseJsonArray object.

param **`value`** The boolean value to add.

return **`instance of an object.`**

```cpp
FirebaseJsonArray &add(bool value);
```






#### Add nested FirebaseJson object  to FirebaseJsonArray object.

param **`json`** The FirebaseJson object to add.

return **`instance of an object.`**

```cpp
FirebaseJsonArray &add(FirebaseJson &json);
```





#### Add nested FirebaseJsonArray object  to FirebaseJsonArray object.

param **`arr`** The FirebaseJsonArray object to add.

return **`instance of an object.`**

```cpp
FirebaseJsonArray &add(FirebaseJsonArray &arr);
```





#### Get the array value at specified index from FirebaseJsonArray object.

param **`jsonObj`** The returning FirebaseJsonData object that holds data at the specified index.

param **`index`** Index of data in FirebaseJsonArray object.    

return **`boolean`** status of the operation.

```cpp
bool get(FirebaseJsonData &jsonObj, int index);
bool get(FirebaseJsonData *jsonData, int index);
```







#### Get the array value at the specified path from FirebaseJsonArray object.

param **`jsonObj`** The returning FirebaseJsonData object that holds data at the specified path.

param **`path`** Relative path to data in FirebaseJsonArray object.    

return **`boolean status of the operation.`**

The relative path must begin with array index (number placed inside square brackets) followed by 
other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

```cpp
bool get(FirebaseJsonData &jsonData, const String &path);
```






#### Get the length of array in FirebaseJsonArray object.  

return **`length of the array.`**

```cpp
size_t size();
```






#### Get the FirebaseJsonArray object serialized string.

param **`buf`** The returning String object.

param **`prettify`** Boolean flag for return the pretty format string i.e. with text indentation and newline. 


```cpp
void toString(String &buf, bool prettify = false);
```






#### Clear all array in FirebaseJsonArray object.

return **`instance of an object.`**

```cpp
FirebaseJsonArray &clear();
```







#### Set null to FirebaseJsonArray object at specified index.
    
param **`index`** The array index to set null.

```cpp
void set(int index);
```







#### Set String to FirebaseJsonArray object at specified index.
    
param **`index`**The array index that String value to be set.

param **`value`** The String to set.


```cpp
void set(int index, const String &value);
```







#### Set string (chars array) to FirebaseJsonArray object at specified index.
    
param **`index`** The array index that string (chars array) to be set.

param **`value`** The char array to set.

```cpp
void set(int index, const char *value);
```





#### Set integer/unsigned short value to FirebaseJsonArray object at specified index.
    
param **`index`** The array index that int/unsigned short to be set.

param **`value`** The integer/unsigned short value to set.

```cpp
void set(int index, int value);
void set(int index, unsigned short value);
```




#### Set float value to FirebaseJsonArray object at specified index.
    
param **`index`** The array index that float value to be set.

param **`value`** The float value to set.

```cpp
void set(int index, float value);
```






#### Set double value to FirebaseJsonArray object at specified index.
    
param **`index`** The array index that double value to be set.

param **`value`** The double value to set.

```cpp
void set(int index, double value);
```






#### Set boolean value to FirebaseJsonArray object at specified index.
    
param **`index`** The array index that bool value to be set.

param **`value`** The boolean value to set.

```cpp
void set(int index, bool value);
```






#### Set nested FirebaseJson object to FirebaseJsonArray object at specified index.
    
param **`index`** The array index that nested FirebaseJson object to be set.

param **`value`** The FirebaseJson object to set.

```cpp
void set(int index, FirebaseJson &json);
```






#### Set nested FirebaseJsonArray object to FirebaseJsonArray object at specified index.
    
param **`index`** The array index that nested FirebaseJsonArray object to be set.

param **`value`** The FirebaseJsonArray object to set.

```cpp
void set(int index, FirebaseJsonArray &arr);
```






    
#### Set null to FirebaseJson object at the specified path.
    
param **`path`** The relative path that null to be set.

The relative path must begin with array index (number placed inside square brackets) followed by 
other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

```cpp
void set(const String &path);
```





#### Set String to FirebaseJsonArray object at specified path.
    
param **`path`** The relative path that string value to be set.

param **`value`** The String to set.

The relative path must begin with array index (number placed inside square brackets) followed by 
other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

```cpp
void set(const String &path, const String &value);
```




#### Set string (chars array) to FirebaseJsonArray object at specified path.
    
param **`path`** The relative path that string (chars array) value to be set.

param **`value`** The char array to set.

The relative path must begin with array index (number placed inside square brackets) followed by 
other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

```cpp
void set(const String &path, const char *value);
```





#### Set integer/unsigned short value to FirebaseJsonArray object at specified path.
    
param **`path`** The relative path that integer/unsigned short value to be set.

param **`value`** The integer value to set.

The relative path must begin with array index (number placed inside square brackets) followed by 
other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

```cpp
void set(const String &path, int value);
void set(const String &path, unsigned short value);
```





#### Set float value to FirebaseJsonArray object at specified path.
    
param **`path`** The relative path that float value to be set.

param **`value`** The float to set.

The relative path must begin with array index (number placed inside square brackets) followed by 
other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

 ```cpp
void set(const String &path, float value);
```







#### Set double value to FirebaseJsonArray object at specified path.
    
param **`path`** The relative path that double value to be set.

param **`value`** The double to set.

The relative path must begin with array index (number placed inside square brackets) followed by 
other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

 ```cpp
void set(const String &path, double value);
```





#### Set boolean value to FirebaseJsonArray object at specified path.
    
param **`path`** The relative path that bool value to be set.

param **`value`** The boolean value to set.

The relative path must begin with array index (number placed inside square brackets) followed by 
other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

```cpp
void set(const String &path, bool value);
```





#### Set the nested FirebaseJson object to FirebaseJsonArray object at the specified path.
    
param **`path`** The relative path that nested FirebaseJson object to be set.

param **`value`** The FirebaseJson object to set.

The relative path must begin with array index (number placed inside square brackets) followed by 
other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

```cpp
void set(const String &path, FirebaseJson &json);
```






#### Set the nested FirebaseJsonArray object to FirebaseJsonArray object at specified path.
    
param **`path`** The relative path that nested FirebaseJsonArray object to be set.

param **`value`** The FirebaseJsonArray object to set.

The relative path must begin with array index (number placed inside square brackets) followed by 
other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

```cpp
void set(const String &path, FirebaseJsonArray &arr);
```






#### Remove the array value at specified index from FirebaseJsonArray object.

param **`index`** The array index to be removed.

return **`bool`** value represents the successful operation.

```cpp
bool remove(int index);
```






#### Remove the array value at the specified path from FirebaseJsonArray object.

param **`path`** The relative path to array in FirebaseJsonArray object to be removed.

return **`bool`** value represents the successful operation.

The relative path must begin with array index (number placed inside square brackets) followed by 
other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

```cpp
bool remove(const String &path);
```





### FirebaseJsonData object functions


#### Get array data as FirebaseJsonArray object from FirebaseJsonData object.
    
param **`jsonArray`**The returning FirebaseJsonArray object.

return **`bool`** status for successful operation.

This should call after pares or get functions.

```cpp
bool getArray(FirebaseJsonArray &jsonArray);
```





#### Get array data as FirebaseJson object from FirebaseJsonData object.
    
param **`jsonArray`** The returning FirebaseJson object.

return **`bool`** status for successful operation.

This should call after pares or get functions.

```cpp
bool getJSON(FirebaseJson &json);
```



### FirebaseJsonData object properties


**`stringValue`** The String value of parses data.

**`intValue`** The int value of parses data.

**`floatValue`** The double value of parses data.

**`doubleValue`** The double value of parses data.

**`boolValue`** The bool value of parses data.

**`success`** used to determine the result of the get operation.

**`type`** The type String of parses data e.g. string, int, double, boolean, array, object, null and undefined.

**`typeNum`** The type (number) of parses data in form of the following **`jsonDataType`** value.

**FirebaseJson::UNDEFINED = 0**

**FirebaseJson::OBJECT = 1**

**FirebaseJson::ARRAY = 2**

**FirebaseJson::STRING = 3**

**FirebaseJson::INT = 4**

**FirebaseJson::FLOAT = 5**

**FirebaseJson::DOUBLE = 6**

**FirebaseJson::BOOL = 7 and**

**FirebaseJson::NULL = 8**

**`success`** The success flag of parsing data.



## License

The MIT License (MIT)

Copyright (c) 2021 K. Suwatchai (Mobizt)


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

