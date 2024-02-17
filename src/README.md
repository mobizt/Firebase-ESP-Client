# Firebase Arduino Client Library for Arduino


Google's Firebase Arduino Client Library for ESP8266 and ESP32


The default filessystem used in the library is flash and SD.


The file systems for flash and sd memory can be changed in FirebaseFS.h.



## Global functions



The global functions are the functions that called directly from the Firebase object e.g. Firebase.\<function name\>



#### Initialize Firebase with the config and Firebase's authentication credentials.

param **`config`** The pointer to FirebaseConfig data.

param **`auth`** The pointer to FirebaseAuth data.

note: For FirebaseConfig and FirebaseAuth data usage, see the examples.

```cpp
void begin(FirebaseConfig *config, FirebaseAuth *auth);
```


#### Setup the ID token for authentication.

param **`config`** The pointer to FirebaseConfig data.

param **`idToken`** The ID Token.

param **`expire`** The expired interval in seeconds (max.3600 sec).

param **`refreshToken`** The refresh token for token refreshment.

note For FirebaseConfig and FirebaseAuth data usage, see the examples.

```cpp
void setIdToken(FirebaseConfig *config, <string> idToken, size_t expire = 3600, <string> refreshToken = "");
```




#### Setup the access token for authentication.

param **`config`** The pointer to FirebaseConfig data.

param **`accessToken`** The access Token.

param **`expire`** The expired interval in seeconds (max.3600 sec).

param **`refreshToken`** The refresh token for token refreshment.

param **`clientId`** The The client identifier issued to the client during the registration process.

param **`clientSecret`** The client secret.

note For FirebaseConfig and FirebaseAuth data usage, see the examples.

```cpp
void setAccessToken(FirebaseConfig *config, <string> accessToken, size_t expire = 3600, <string> refreshToken = "", <string> clientId = "", <string> clientSecret = "");
```






#### Setup the custom token for authentication.

param **`config`** The pointer to FirebaseConfig data.

param **`customToken`** The Identity Platform custom token.

If the refresh token from Custom token verification or sign in, was assigned here instead of 
custom token (signed JWT token), the token refresh process will be performed immediately.

Any token that is not in the form header.payload.signature i.e., xxxxx.yyyyy.zzzzz will be treated as refresh token.

note For FirebaseConfig and FirebaseAuth data usage, see the examples.

```cpp
void setCustomToken(FirebaseConfig *config, <string> customToken);
```



#### Check for token expiry status.

return **`bool`** of expiry status.

```cpp
bool isTokenExpired();
```




#### Force the token to expire immediately and refresh.

param **`config`** The pointer to FirebaseConfig data.

```cpp
void refreshToken(FirebaseConfig *config);
```





#### Reset stored config and auth credentials.

param **`config`** The pointer to FirebaseConfig data.

```cpp
void reset(FirebaseConfig *config);
```





#### Provide the details of token generation.

return **`token_info_t`** The token_info_t structured data that indicates the status.

note: Use type property to get the type enum value.

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



#### Provide the ready status of token generation.

return **`Boolean`** type status indicates the token generation is completed.

This function should be called repeatedly to handle authentication tasks.

```cpp
bool ready();
```



#### Provide the grant access status for Firebase Services.

return **`Boolean`** type status indicates the device can access to the services.

This returns false if ready() returns false (token generation is not ready).

```cpp
bool authenticated();
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
bool signUp(FirebaseConfig *config, FirebaseAuth *auth, <string> email, <string> password);
```


#### Delete user from project.

param **`config`** The pointer to FirebaseConfig data.

param **`auth`** The pointer to FirebaseAuth data.

param **`idToken`** (optional) The id token of user, leave blank to delete the current sign in user.

return **`Boolean`** type status indicates the success of the operation.

```cpp
bool deleteUser(FirebaseConfig *config, FirebaseAuth *auth, const char* idToken = "");
```


#### Send a user a verification Email.

param **`config`** The pointer to FirebaseConfig data.

param **`idToken`** The id token of user that was already signed in with Email and password (optional).

return **`Boolean`** value, indicates the success of the operation. 

note: The id token can be obtained from Firebase.getToken() after begin with config and auth data.

If the idToken is not assigned, the internal id_token will be used.

See the Templates of Email address verification in the Firebase console, Authentication.

```cpp
bool sendEmailVerification(FirebaseConfig *config, <string> idToken);
```



#### Send a user a password reset link to Email.

param **`config`** The pointer to FirebaseConfig data.

param **`email`** The user Email to send the password resset link.

return **`Boolean`** value, indicates the success of the operation. 

```cpp
bool sendResetPassword(FirebaseConfig *config, <string> email);
```



#### Reconnect WiFi if lost connection.

param **`reconnect`** The boolean to set/unset WiFi AP reconnection.

```cpp
void reconnectWiFi(bool reconnect);
```



#### Get currently used auth token string.

param **`constant char*`** of currently used auth token.

```cpp
const char *getToken();
```



#### Get refresh token string.

return **`constant char*`** of refresh token.

```cpp
const char *getRefreshToken();
```



#### Get free Heap memory.

return **`int *`** of free Heap memory size.

```cpp
int getFreeHeap();
```



#### Get current timestamp.

return **`time_t *`** of current timestamp.

```cpp
time_t getCurrentTime();
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



#### Initiate SD card with SPI port configuration.

param **`ss`** The SPI Chip/Slave Select pin.

param **`sck`** The SPI Clock pin.

param **`miso`** The SPI MISO pin.

param **`mosi`** The SPI MOSI pin.

aram **`frequency`** The SPI frequency.

return **`boolean`** The boolean value indicates the success of operation.

```cpp
bool sdBegin(int8_t ss = -1, int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1, uint32_t frequency = 4000000);
```





#### Initiate SD card with SD FS configurations (ESP8266 only).

param **`ss`** SPI Chip/Slave Select pin.

param **`sdFSConfig`** The pointer to SDFSConfig object (ESP8266 only).

return **`boolean`** type status indicates the success of the operation.

```cpp
  bool sdBegin(SDFSConfig *sdFSConfig);
```





#### Initiate SD card with chip select and SPI configuration (ESP32 only).

param **`ss`** The SPI Chip/Slave Select pin.

param **`spiConfig`** The pointer to SPIClass object for SPI configuartion.

param **`frequency`** The SPI frequency.

return **`boolean`** The boolean value indicates the success of operation.

```cpp
bool sdBegin(int8_t ss, SPIClass *spiConfig = nullptr, uint32_t frequency = 4000000);
```





#### Initiate SD card with SdFat SPI and pins configurations (with SdFat included only).

param **`sdFatSPIConfig`** The pointer to SdSpiConfig object for SdFat SPI configuration.

param **`ss`** The SPI Chip/Slave Select pin.

param **`sck`** The SPI Clock pin.

param **`miso`** The SPI MISO pin.

param **`mosi`** The SPI MOSI pin.

return **`boolean`** The boolean value indicates the success of operation.

```cpp
 bool sdBegin(SdSpiConfig *sdFatSPIConfig, int8_t ss = -1, int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1);
```





#### Initiate SD card with SdFat SDIO configuration (with SdFat included only).

param **`sdFatSDIOConfig`** The pointer to SdioConfig object for SdFat SDIO configuration.

return **`boolean`** The boolean value indicates the success of operation.

```cpp
 bool sdBegin(SdioConfig *sdFatSDIOConfig);
```




#### Initialize the SD_MMC card (ESP32 only).

param **`mountpoint`** The mounting point.

param **`mode1bit`** Allow 1 bit data line (SPI mode).

param **`format_if_mount_failed`** Format SD_MMC card if mount failed.

return **`Boolean`** type status indicates the success of the operation.

```cpp
bool sdMMCBegin(const char *mountpoint = "/sdcard", bool mode1bit = false, bool format_if_mount_failed = false);
```



#### Set system time with timestamp.

param **`ts`** timestamp in seconds from midnight Jan 1, 1970.

return **`Boolean`** type status indicates the success of the operation.

This function allows the internal time setting by timestamp i.e. timestamp from external RTC. 

```cpp
bool setSystemTime(time_t ts);
```


#### Provide the http code error string

param **`httpCode`** The http code.

param **`buff`** The String buffer out.

```cpp
void errorToString(int httpCode, String &buff)
```



## Realtime database functions

These functions can be called directly from RTDB object in the Firebase object e.g. Firebase.RTDB.\<function name\>



#### Stop Firebase and release all resources.

param **`fbdo`** The pointer to Firebase Data Object.

```cpp
void end(FirebaseData *fbdo);
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
void setwriteSizeLimit(FirebaseData *fbdo, <string> size);
```



#### Read the database rules.

param **`fbdo`** The pointer to Firebase Data Object.

return **`Boolean`** value, indicates the success of the operation.

```cpp
bool getRules(FirebaseData *fbdo);
```


#### Save the database rules to file.

param **`fbdo`** The pointer to Firebase Data Object.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.

param **`filename`** Filename to save rules.

param **`callback`** Optional. The callback function that accept RTDB_DownloadStatusInfo data.

return **`Boolean`** type status indicates the success of the operation.

```cpp
 getRules(FirebaseData *fbdo, firebase_mem_storage_type storageType, <string> filename, RTDB_DownloadProgressCallback callback = NULL)
```


#### Write the database rules.

param **`fbdo`** The pointer to Firebase Data Object.

param **`rules`** Database rules in jSON String format.

return **`Boolean`** value, indicates the success of the operation.

```cpp
bool setRules(FirebaseData *fbdo, <string> rules);
```



#### Restore the database rules from file.

param **`fbdo`** The pointer to Firebase Data Object.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.

param **`filename`** Filename to read the rules from.

param **`callback`** Optional. The callback function that accept RTDB_UploadStatusInfo data.

return **`Boolean`** type status indicates the success of the operation.

```cpp
 setRules(FirebaseData *fbdo, firebase_mem_storage_type storageType, <string> filename, RTDB_UploadProgressCallback callback = NULL)
```




#### Set the .read and .write database rules.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The parent path of child's node that the .read and .write rules are being set.

param **`var`** The child node key that the .read and .write rules are being set.

param **`readVal`** The child node key .read value.

param **`writeVal`** The child node key .write value.

param **`databaseSecret`** The database secret.

return **`Boolean`** value, indicates the success of the operation.

note: The databaseSecret can be empty if the auth type is OAuth2.0 or legacy and required if auth type is Email/Password sign-in. 

```cpp
bool setReadWriteRules(FirebaseData *fbdo, <string> path, <string> var, <string> readVal, <string> writeVal, <string> databaseSecret);
```



#### Set the query index to the database rules.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The parent path of child's node that is being queried.

param **`node`** The child node key that is being queried.

param **`databaseSecret`** The database secret.

return **`Boolean`** value, indicates the success of the operation.

note: The databaseSecret can be empty if the auth type is OAuth2.0 or legacy and required if auth type is Email/Password sign-in.

```cpp
bool setQueryIndex(FirebaseData *fbdo, <string> path, <string> node, <string> databaseSecret);
```



#### Remove the query index from the database rules.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The parent path of child's node that the index is being removed.

param **`databaseSecret`** The database secret.

return **`Boolean`** value, indicates the success of the operation.

note: The databaseSecret can be empty if the auth type is OAuth2.0 or legacy and required if auth type is Email/Password sign-in.

```cpp
bool removeQueryIndex(FirebaseData *fbdo, <string> path, <string> databaseSecret);
```



#### Get the existent of the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

return **`Boolean`** value, true if the defined node was found.

```cpp
bool pathExisted(FirebaseData *fbdo, <string> path);
```



#### Get the unique identifier (ETag) of current data at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

return **`String`** of unique identifier.

```cpp
String getETag(FirebaseData *fbdo, <string> path);
```



#### Get the shallowed data at defined node path.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** node that is being read the data.

return **`Boolean`** value, indicates the success of the operation.

```cpp
bool getShallowData(FirebaseData *fbdo, <string> path); 
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
    
return **`Boolean`** value, indicates the success of the operation.

This allows us to set priority to any node other than a priority that set through setJSON, 
pushJSON, updateNode, and updateNodeSilent functions.

```cpp
bool setPriority(FirebaseData *fbdo, <string> path, float priority);

bool setPriorityAsync(FirebaseData *fbdo, <string> path, float priority);
```



#### Read the virtual child node ".priority" value at the defined node.
    
param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.
    
return **`Boolean`** value, indicates the success of the operation.

```cpp
bool getPriority(FirebaseData *fbdo, <string> path);
```



####  Append new generic value to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

param **`value`** The appended value of any type e.g. bool, int, float, double, String, FirebaseJson object and array.

param **`blob`** Byte array of data (blob).

param **`size`** Size of the byte array (blob).

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd.

The file systems can be changed in FirebaseFS.h.

param **`fileName`** The file path includes its name.

return **`Boolean`** value, indicates the success of the operation.

The key or name of new created node will be stored in Firebase Data object, 
call `<FirebaseData>.pushName()` to get the key.

```cpp
bool push(FirebaseData *fbdo, <string> path, <type> value);

bool pushAsync(FirebaseData *fbdo, <string> path, <type> value);

bool push(FirebaseData *fbdo, <string> path, uint8_t *blob, size_t size);

bool pushAsync(FirebaseData *fbdo, <string> path, uint8_t *blob, size_t size);

bool push(FirebaseData *fbdo, firebase_mem_storage_type storageType, <string> path, <string> fileName);

bool pushAsync(FirebaseData *fbdo, firebase_mem_storage_type storageType, <string> path, <string> fileName);
```



#### Append new genric value and the virtual child ".priority" to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

param **`value`** The appended value of any type e.g. bool, int, float, double, String, FirebaseJson object and array.

param **`priority`** The priority value.

return **`Boolean`** value, indicates the success of the operation.

The key or name of new created node will be stored in Firebase Data object, 
call `<FirebaseData>.pushName()` to get the key.

```cpp
bool push(FirebaseData *fbdo, <string> path, <type> value,  float priority);

bool pushAsync(FirebaseData *fbdo, <string> path, <type> value,  float priority);
```



####  Append new integer value to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

param **`value`** The appended value.

return **`Boolean`** value, indicates the success of the operation.

The key or name of new created node will be stored in Firebase Data object, 
call `<FirebaseData>.pushName()` to get the key.

```cpp
bool pushInt(FirebaseData *fbdo, <string> path, int value);

bool pushIntAsync(FirebaseData *fbdo, <string> path, int value);
```



#### Append new integer value and the virtual child ".priority" to the defined node.

```cpp
bool pushInt(FirebaseData *fbdo, <string> path, int value, float priority);

bool pushIntAsync(FirebaseData *fbdo, <string> path, int value, float priority);
```



#### Append new float value to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which float value will be appended.

param **`value`** The appended value.

return **`Boolean`** value, indicates the success of the operation.

The key or name of new created node will be stored in Firebase Data object, 
call `<FirebaseData>.pushName()` to get the key.

```cpp
bool pushFloat(FirebaseData *fbdo, <string> path, float value);

bool pushFloatAsync(FirebaseData *fbdo, <string> path, float value);
```



#### Append new float value and the virtual child ".priority" to the defined node.

```cpp
bool pushFloat(FirebaseData *fbdo, <string> path, float value, float priority);

bool pushFloatAsync(FirebaseData *fbdo, <string> path, float value, float priority);
```



#### Append new double value (8 bytes) to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which float value will be appended.

param **`value`** The appended value.

return **`Boolean`** value, indicates the success of the operation.

The key or name of new created node will be stored in Firebase Data object, 
call `<FirebaseData>.pushName()` to get the key.

```cpp
bool pushDouble(FirebaseData *fbdo, <string> path, double value);

bool pushDoubleAsync(FirebaseData *fbdo, <string> path, double value);
```



#### Append new double value (8 bytes) and the virtual child ".priority" to the defined node.

```cpp
bool pushDouble(FirebaseData *fbdo, <string> path, double value, float priority);

bool pushDoubleAsync(FirebaseData *fbdo, <string> path, double value, float priority);
```



#### Append new Boolean value to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which Boolean value will be appended.

param **`value`** The appended value.

return **`Boolean`** value, indicates the success of the operation.

The key or name of new created node will be stored in Firebase Data object, 
call `<FirebaseData>.pushName()` to get the key.

```cpp
bool pushBool(FirebaseData *fbdo, <string> path, bool value);

bool pushBoolAsync(FirebaseData *fbdo, <string> path, bool value);
```



#### Append the new Boolean value and the virtual child ".priority" to the defined node.

```cpp
bool pushBool(FirebaseData *fbdo, <string> path, bool value, float priority);

bool pushBoolAsync(FirebaseData *fbdo, <string> path, bool value, float priority);
```



#### Append a new string (text) to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which string will be appended.

param **`value`** The appended value.

return **`Boolean`** value, indicates the success of the operation.

The key or name of new created node will be stored in Firebase Data object, 
call `<FirebaseData>.pushName()` to get the key.

```cpp
bool pushString(FirebaseData *fbdo, <string> path, <string> value);

bool pushStringAsync(FirebaseData *fbdo, <string> path, <string> value);
```



#### Append new string and the virtual child ".priority" to the defined node.

```cpp
bool pushString(FirebaseData *fbdo, <string> path, <string> value, float priority);

bool pushStringAsync(FirebaseData *fbdo, <string> path, <string> value, float priority);
```



#### Append new child (s) to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which child (s) will be appended.

param **`json`** The pointer to the FirebaseJson object which contains the child (s) nodes.

return **`Boolean`** value, indicates the success of the operation.

The key or name of new created node will be stored in Firebase Data object, 
call `<FirebaseData>.pushName()` to get the key.

```cpp
bool pushJSON(FirebaseData *fbdo, <string> path, FirebaseJson *json);

bool pushJSONAsync(FirebaseData *fbdo, <string> path, FirebaseJson *json);
```



#### Append new child (s) and the virtual child ".priority" to the defined node.

```cpp
bool pushJSON(FirebaseData *fbdo, <string> path, FirebaseJson *json, float priority);

bool pushJSONAsync(FirebaseData *fbdo, <string> path, FirebaseJson *json, float priority);
```



#### Append array to the defined node. 

The old content in defined node will be replaced.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which the array will be appended.

param **`arr`** The pointer to the FirebaseJsonArray object.

return **`Boolean`** value, indicates the success of the operation.

The key or name of new created node will be stored in Firebase Data object, 
call `<FirebaseData>.pushName()` to get the key.

```cpp
bool pushArray(FirebaseData *fbdo, <string> path, FirebaseJsonArray *arr);

bool pushArrayAsync(FirebaseData *fbdo, <string> path, FirebaseJsonArray *arr);
```



#### Append array and virtual child ".priority" at the defined node.

```cpp
bool pushArray(FirebaseData *fbdo, <string> path, FirebaseJsonArray *arr, float priority);

bool pushArrayAsync(FirebaseData *fbdo, <string> path, FirebaseJsonArray *arr, float priority);
```



#### Append new blob (binary data) to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which binary data will be appended.

param **`blob`** Byte array of data.

param **`size`** Size of the byte array.

return **`Boolean`** value, indicates the success of the operation.

The key or name of new created node will be stored in Firebase Data object, 
call `<FirebaseData>.pushName()` to get the key.

```cpp
bool pushBlob(FirebaseData *fbdo, <string> path, uint8_t *blob, size_t size);

bool pushAsync(FirebaseData *fbdo, <string> path, uint8_t *blob, size_t size);
```



#### Append new binary data from file stores on storage memory to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd.

The file systems can be changed in FirebaseFS.h.

param **`path`** The path to the node in which binary data will be appended.

param **`fileName`** The file path includes its name.

param **`callback`** Optional. The callback function that accept RTDB_UploadStatusInfo data.

return **`Boolean`** value, indicates the success of the operation.

The key or name of new created node will be stored in Firebase Data object, 
call `<FirebaseData>.pushName()` to get the key.

```cpp
bool pushFile(FirebaseData *fbdo, firebase_mem_storage_type storageType, <string> path, <string> fileName, RTDB_UploadProgressCallback callback = NULL);

bool pushFileAsync(FirebaseData *fbdo, firebase_mem_storage_type storageType, <string> path, <string> fileName, RTDB_UploadProgressCallback callback = NULL);
```



#### Append the new Firebase server's timestamp to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which timestamp will be appended.

return **`Boolean`** value, indicates the success of the operation.
    
The key or name of new created node will be stored in Firebase Data object, 
call `<FirebaseData>.pushName()` to get the key.

```cpp
bool pushTimestamp(FirebaseData *fbdo, <string> path);

bool pushTimestampAsync(FirebaseData *fbdo, <string> path);
```



#### Set the generic value at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node which integer value will be set.

param **`value`** the value to set of any type e.g. bool, int, float, double, String, FirebaseJson object and array.

param **`blob`** Byte array of data (blob).

param **`size`** Size of the byte array (blob).

param **`fileName`** The file path includes its name.

return **`Boolean`** value, indicates the success of the operation.

Call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to get the type of data that successfully stored in the database. 

Call `<FirebaseData>.to<type>()` to get value (cast) that stored on the defined node e.g. `<FirebaseData>.to<String>()` to get the String value.

```cpp
bool set(FirebaseData *fbdo, <string> path, <type> value);

bool setAsync(FirebaseData *fbdo, <string> path, <type> value);

bool set(FirebaseData *fbdo, <string> path, uint8_t *blob, size_t size);

bool setAsync(FirebaseData *fbdo, <string> path, uint8_t *blob, size_t size);

bool set(FirebaseData *fbdo, firebase_mem_storage_type storageType, <string> path, <string> fileName);

bool setAsync(FirebaseData *fbdo, firebase_mem_storage_type storageType, <string> path, <string> fileName);
```



#### Set the generic value and virtual child ".priority" at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node which integer value will be set.

param **`value`** the value to set of any type e.g. bool, int, float, double, String, FirebaseJson object and array.

param **`priority`** The priority value.

return **`Boolean`** value, indicates the success of the operation.

Call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to get the type of data that successfully stored in the database. 

Call `<FirebaseData>.to<type>()` to get value (cast) that stored on the defined node e.g. `<FirebaseData>.to<String>()` to get the String value.

```cpp
bool set(FirebaseData *fbdo, <string> path, <type> value, float priority);

bool setAsync(FirebaseData *fbdo, <string> path, <type> value, float priority);
```



#### Set the generic value if defined node's ETag matched the defined ETag value.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node which integer value will be set.

param **`value`** the value to set of any type e.g. bool, int, float, double, String, FirebaseJson object and array.

param **`blob`** Byte array of data (blob).

param **`size`** Size of the byte array (blob).

param **`fileName`** The file path includes its name.

param **`ETag`** Known unique identifier string (ETag) of defined node.

return **`Boolean`** value, indicates the success of the operation.



If ETag at the defined node does not match the provided ETag parameter,
the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched). 

If the operation failed due to ETag is not match, call `<FirebaseData>.ETag()` to get the current ETag value. 

Also call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to get the type of data that successfully stored in the database. 

And `<FirebaseData>.to<type>()` to get value (cast) that stored on the defined node e.g. `<FirebaseData>.to<String>()` to get the String value.

```cpp
bool set(FirebaseData *fbdo, <string> path, <type> value, <string> ETag);

bool setAsync(FirebaseData *fbdo, <string> path, <type> value, <string> ETag);

bool set(FirebaseData *fbdo, <string> path, uint8_t *blob, size_t size, <string> ETag);

bool setAsync(FirebaseData *fbdo, <string> path, uint8_t *blob, size_t size, <string> ETag);

bool set(FirebaseData *fbdo, firebase_mem_storage_type storageType, <string> path, <string> fileName, <string> ETag);

bool setAsync(FirebaseData *fbdo, firebase_mem_storage_type storageType, <string> path, <string> fileName, <string> ETag);
```



#### Set the generic value and the virtual child ".priority" if defined ETag matches at the defined node. 

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node which integer value will be set.

param **`value`** the value to set of any type e.g. bool, int, float, double, String, FirebaseJson object and array.

param **`priority`** The priority value.

param **`ETag`** Known unique identifier string (ETag) of defined node.

return **`Boolean`** value, indicates the success of the operation.



If ETag at the defined node does not match the provided ETag parameter,
the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched). 

If the operation failed due to ETag is not match, call `<FirebaseData>.ETag()` to get the current ETag value. 

Also call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to get the type of data that successfully stored in the database. 

And `<FirebaseData>.to<type>()` to get value (cast) that stored on the defined node e.g. `<FirebaseData>.to<String>()` to get the String value.

```cpp
bool set(FirebaseData *fbdo, <string> path, <type> value, float priority, <string> ETag);

bool setAsync(FirebaseData *fbdo, <string> path, <type> value, float priority, <string> ETag);
```



#### Set the integer value at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node which integer value will be set.

param **`value`** Integer value to set.

return **`Boolean`** value, indicates the success of the operation.



Call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to get the type of data that successfully stored in the database. 

Call `<FirebaseData>.to<int>()` to get the integer value that stored on the defined node.

```cpp
bool setInt(FirebaseData *fbdo, <string> path, int value);

bool setIntAsync(FirebaseData *fbdo, <string> path, int value);
```



#### Set the integer value and virtual child ".priority" at the defined node.

```cpp
bool setInt(FirebaseData *fbdo, <string> path, int value, float priority);

bool setIntAsync(FirebaseData *fbdo, <string> path, int value, float priority);
```



#### Set the integer value at the defined node if defined node's ETag matched the defined ETag value.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which integer value will be set.

param **`value`** Integer value to set.

param **`ETag`** Known unique identifier string (ETag) of the defined node.

return **`Boolean`** value, indicates the success of the operation.



If ETag at the defined node does not match the provided ETag parameter,
the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched). 

If the operation failed due to ETag is not match, call `<FirebaseData>.ETag()` to get the current ETag value. 

Also call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to get the type of data that successfully stored in the database. 

And `<FirebaseData>.to<int>()` to get the integer value that stored on the defined node.
    
```cpp
bool setInt(FirebaseData *fbdo, <string> path, int value, <string> ETag);

bool setIntAsync(FirebaseData *fbdo, <string> path, int value, <string> ETag);
```



#### Set integer value and the virtual child ".priority" if defined ETag matches at the defined node 

```cpp
bool setInt(FirebaseData *fbdo, <string> path, int value, float priority, <string> ETag);

bool setIntAsync(FirebaseData *fbdo, <string> path, int value, float priority, <string> ETag);
```



#### Set float value at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which float value will be set.

param **`value`** Float value to set.

return **`Boolean`** value, indicates the success of the operation.



Call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to get the type of data that successfully stored in the database. 

Call `<FirebaseData>.to<float>()` to get the float value that stored on the defined node.

```cpp
bool setFloat(FirebaseData *fbdo, <string> path, float value);

bool setFloatAsync(FirebaseData *fbdo, <string> path, float value);
```



#### Set float value and virtual child ".priority" at the defined node.

```cpp
bool setFloat(FirebaseData *fbdo, <string> path, float value, float priority);

bool setFloatAsync(FirebaseData *fbdo, <string> path, float value, float priority);
```



#### Set float value at the defined node if defined node's ETag matched the ETag value.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which float data will be set.

param **`value`** Float value to set.

param **`ETag`** Known unique identifier string (ETag) of defined node.

return **`Boolean`** value, indicates the success of the operation.



If ETag at the defined node does not match the provided ETag parameter,
the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched). 

If the operation failed due to ETag is not match, call `<FirebaseData>.ETag()` to get the current ETag value. 

Also call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to get the type of data that successfully stored in the database. 

Call `<FirebaseData>.to<float>()` to get the float value that stored on the defined node.

```cpp
bool setFloat(FirebaseData *fbdo, <string> path, float value, <string> ETag);

bool setFloatAsync(FirebaseData *fbdo, <string> path, float value, <string> ETag);
```



#### Set float value and the virtual child ".priority" if defined ETag matches at the defined node. 

```cpp
bool setFloat(FirebaseData *fbdo, <string> path, float value, float priority, <string> ETag);

bool setFloatAsync(FirebaseData *fbdo, <string> path, float value, float priority, <string> ETag);
```



#### Set double value at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which float data will be set.

param **`value`** Double value to set.

return **`Boolean`** value, indicates the success of the operation.



Call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to get the type of data that successfully stored in the database. 

Call `<FirebaseData>.to<double>()` to get the double value that stored on the defined node.

Due to bugs in Serial.print in Arduino, to print large double value with zero decimal place, 
use `Serial.printf("%.9lf\n", firebaseData.to<double>());` for print the returned double value up to 9 decimal places.

```cpp
bool setDouble(FirebaseData *fbdo, <string> path, double value);

bool setDoubleAsync(FirebaseData *fbdo, <string> path, double value);
```



#### Set double value and virtual child ".priority" at the defined node.

```cpp
bool setDouble(FirebaseData *fbdo, <string> path, double value, float priority);

bool setDoubleAsync(FirebaseData *fbdo, <string> path, double value, float priority);
```



#### Set double value at the defined node if defined node's ETag matched the ETag value.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which float data will be set.

param **`value`** Double value to set.

param **`ETag`** Known unique identifier string (ETag) of defined node.

return **`Boolean`** value, indicates the success of the operation.



If ETag at the defined node does not match the provided ETag parameter,
the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched). 

If the operation failed due to ETag is not match, call `<FirebaseData>.ETag()` to get the current ETag value. 

Also call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to get the type of data that successfully stored in the database. 

And `<FirebaseData>.to<double>()` to get the double value that stored on the defined node.

```cpp
bool setDouble(FirebaseData *fbdo, <string> path, double value, <string> ETag);

bool setDoubleAsync(FirebaseData *fbdo, <string> path, double value, <string> ETag);
```



#### Set double value and the virtual child ".priority" if defined ETag matches at the defined node. 

```cpp
bool setDouble(FirebaseData *fbdo, <string> path, double value, float priority, <string> ETag);

bool setDoubleAsync(FirebaseData *fbdo, <string> path, double value, float priority, <string> ETag);
```



#### Set boolean value at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which Boolean data will be set.

param **`value`** Boolean value to set.

return **`Boolean`** value, indicates the success of the operation.



Call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to get the type of data that successfully stored in the database. 

Call `<FirebaseData>.to<bool>()` to get the boolean value that stored on the defined node.

```cpp
bool setBool(FirebaseData *fbdo, <string> path, bool value);

bool setBoolAsync(FirebaseData *fbdo, <string> path, bool value);
```



#### Set boolean value and virtual child ".priority" at the defined node.

```cpp
bool setBool(FirebaseData *fbdo, <string> path, bool value, float priority);

bool setBoolAsync(FirebaseData *fbdo, <string> path, bool value, float priority);
```



#### Set boolean value at the defined node if defined node's ETag matched the ETag value.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which Boolean data will be set.

param **`value`** Boolean value to set.

param **`ETag`** Known unique identifier string (ETag) of defined node.

return **`Boolean`** value, indicates the success of the operation.



If ETag at the defined node does not match the provided ETag parameter,
the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched). 

If the operation failed due to ETag is not match, call `<FirebaseData>.ETag()` to get the current ETag value. 

Also call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to get the type of data that successfully stored in the database. 

Call `<FirebaseData>.to<bool>()` to get the boolean value that stored on the defined node.

```cpp
bool setBool(FirebaseData *fbdo, <string> path, bool value, <string> ETag);

bool setBoolAsync(FirebaseData *fbdo, <string> path, bool value, <string> ETag);
```



#### Set boolean value and the virtual child ".priority" if defined ETag matches at the defined node. 

```cpp
bool setBool(FirebaseData *fbdo, <string> path, bool value, float priority, <string> ETag);

bool setBoolAsync(FirebaseData *fbdo, <string> path, bool value, float priority, <string> ETag);
```



#### Set string at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which string data will be set.

param **`value`** String or text to set.

return **`Boolean`** value, indicates the success of the operation.



Call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to get the type of data that successfully stored in the database. 

Call `<FirebaseData>.to<String>()` to get the String value that stored on the defined node.

```cpp
bool setString(FirebaseData *fbdo, <string> path, <string> value);

bool setStringAsync(FirebaseData *fbdo, <string> path, <string> value);
```



#### Set string value and virtual child ".priority" at the defined node.

```cpp
bool setString(FirebaseData *fbdo, <string> path, <string> value, float priority);

bool setStringAsync(FirebaseData *fbdo, <string> path, <string> value, float priority);
```



#### Set string at the defined node if defined node's ETag matched the ETag value.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which string data will be set.

param **`value`** String or text to set.

param **`ETag`** Known unique identifier string (ETag) of defined node.

return **`Boolean`** value, indicates the success of the operation.



If ETag at the defined node does not match the provided ETag parameter,
the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched).

If the operation failed due to ETag is not match, call `<FirebaseData>.ETag()` to get the current ETag value.

Also, call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to get the type of data that successfully stored in the database. 

Call `<FirebaseData>.to<String>()` to get the String value that stored on the defined node.

```cpp
bool setString(FirebaseData *fbdo, <string> path, <string> value, <string> ETag);

bool setStringAsync(FirebaseData *fbdo, <string> path, <string> value, <string> ETag);
```



#### Set string data and the virtual child ".priority" if defined ETag matches at the defined node. 

```cpp
bool setString(FirebaseData *fbdo, <string> path, <string> value, float priority, <string> ETag);

bool setStringAsync(FirebaseData *fbdo, <string> path, <string> value, float priority, <string> ETag);
```



#### Set the child (s) nodes to the defined node. 

The old content in defined node will be replaced.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which child (s) nodes will be replaced or set.

param **`json`** The pointer to FirebaseJson object.

return **`Boolean`** value, indicates the success of the operation.



Call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to get the type of data that successfully stored in the database. 

Call `<FirebaseData>.to<FirebaseJson>()` and `<FirebaseData>.to<FirebaseJson *>()` will return reference to object and pointer to FirebaseJson object from payload.

```cpp
bool setJSON(FirebaseData *fbdo, <string> path, FirebaseJson *json);

bool setJSONAsync(FirebaseData *fbdo, <string> path, FirebaseJson *json);
```



#### Set the child (s) nodes and virtual child ".priority" at the defined node.

```cpp
bool setJSON(FirebaseData *fbdo, <string> path, FirebaseJson *json, float priority);

bool setJSONAsync(FirebaseData *fbdo, <string> path, FirebaseJson *json, float priority);
```



#### Set the child (s) nodes to the defined node, if defined node's ETag matched the ETag value. 

The old content in defined node will be replaced.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which child(s) nodes will be replaced or set.

param **`json`** The pointer to FirebaseJson object.

param **`ETag`** KKnown unique identifier string (ETag) of defined node.

return **`Boolean`** value, indicates the success of the operation.



If ETag at the defined node does not match the provided ETag parameter,
the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched).

If the operation failed due to ETag is not match, call `<FirebaseData>.ETag()` to get the current ETag value. 

Also call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to get the type of data that successfully stored in the database. 

Call `<FirebaseData>.to<FirebaseJson>()` and `<FirebaseData>.to<FirebaseJson *>()` will return reference to object and pointer to FirebaseJson object from payload.

```cpp
bool setJSON(FirebaseData *fbdo, <string> path, FirebaseJson *json, <string> ETag);

bool setJSONAsync(FirebaseData *fbdo, <string> path, FirebaseJson *json, <string> ETag);
```



#### Set the child (s) nodes and the virtual child ".priority" if defined ETag matches at the defined node.

```cpp
bool setJSON(FirebaseData *fbdo, <string> path, FirebaseJson *json, float priority, <string> ETag);

bool setJSONAsync(FirebaseData *fbdo, <string> path, FirebaseJson *json, float priority, <string> ETag);
```



####  Set the array to the defined node.

The old content in defined node will be replaced.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which array will be replaced or set.

param **`arr`** The pointer to FirebaseJsonArray object.

return **`Boolean`** value, indicates the success of the operation.



Call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to get the type of data that successfully stored in the database. 

Call `<FirebaseData>.to<FirebaseJsonArray>()` and `<FirebaseData>.to<FirebaseJsonArray *>()` will return reference to object and pointer to FirebaseJsonArray object that contains the array from payload.

```cpp
bool setArray(FirebaseData *fbdo, <string> path, FirebaseJsonArray *arr);

bool setArrayAsync(FirebaseData *fbdo, <string> path, FirebaseJsonArray *arr);
```



#### Set array and virtual child ".priority" at the defined node.

```cpp
bool setArray(FirebaseData *fbdo, <string> path, FirebaseJsonArray *arr, float priority);

bool setArrayAsync(FirebaseData *fbdo, <string> path, FirebaseJsonArray *arr, float priority);
```



#### Set the array to the defined node if defined node's ETag matched the ETag value. 

The old content in defined node will be replaced.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which array will be replaced or set.

param **`arr`** The pointer to FirebaseJsonArray object.

param **`ETag`** Known unique identifier string (ETag) of defined node.

return **`Boolean`** value, indicates the success of the operation.



If ETag at the defined node does not match the provided ETag parameter,
the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched).

If the operation failed due to ETag is not match, call `<FirebaseData>.ETag()` to get the current ETag value.

Also call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to get the type of data that successfully stored in the database. 

And `<FirebaseData>.to<FirebaseJsonArray>()` to get the FirebaseJsonArray value that stored on the defined node.

```cpp
bool setArray(FirebaseData *fbdo, <string> path, FirebaseJsonArray *arr, <string> ETag);

bool setArrayAsync(FirebaseData *fbdo, <string> path, FirebaseJsonArray *arr, <string> ETag);
```



#### Set array and the virtual child ".priority" if defined ETag matches at the defined node. 

```cpp
bool setArray(FirebaseData *fbdo, <string> path, FirebaseJsonArray *arr, float priority, <string> ETag);

bool setArrayAsync(FirebaseData *fbdo, <string> path, FirebaseJsonArray *arr, float priority, <string> ETag);
```



#### Set the blob (binary data) at the defined node. 

The old content in defined node will be replaced.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to node in which binary data will be set.

param **`blob`** Byte array of data.

param **`size`** Size of the byte array.

return **`Boolean`** value, indicates the success of the operation.

No payload returned from the server.

```cpp
bool setBlob(FirebaseData *fbdo, <string> path, uint8_t *blob, size_t size);

bool setBlobAsync(FirebaseData *fbdo, <string> path, uint8_t *blob, size_t size);
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
bool setBlob(FirebaseData *fbdo, <string> path, uint8_t *blob, size_t size, <string> ETag);

bool setBlobAsync(FirebaseData *fbdo, <string> path, uint8_t *blob, size_t size, <string> ETag);
```



#### Set the binary data from file to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd.

The file systems can be changed in FirebaseFS.h.

param **`path`** The path to the node in which binary data will be set.

param **`fileName`** The file path includes its name.

param **`callback`** Optional. The callback function that accept RTDB_DownloadStatusInfo data.

return **`Boolean`** value, indicates the success of the operation.

No payload returned from the server.

```cpp
bool setFile(FirebaseData *fbdo, firebase_mem_storage_type storageType, <string> path, <string> fileName, RTDB_DownloadProgressCallback callback = NULL);

bool setFileAsync(FirebaseData *fbdo, firebase_mem_storage_type storageType, <string> path, <string> fileName, RTDB_DownloadProgressCallback callback = NULL);
```



#### Set the binary data from file to the defined node if defined node's ETag matched the ETag value.

param **`fbdo`** The pointer to Firebase Data Object.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd.

The file systems can be changed in FirebaseFS.h.

param **`path`** The path to the node in which binary data from the file will be set.

param **`fileName`** The file path includes its name.

param **`ETag`** Known unique identifier string (ETag) of defined node.

param **`callback`** Optional. The callback function that accept RTDB_DownloadStatusInfo data.

return **`Boolean`** value, indicates the success of the operation.

No payload returned from the server. 

If ETag at the defined node does not match the provided ETag parameter,
the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched).

```cpp
bool setFile(FirebaseData *fbdo, firebase_mem_storage_type storageType, <string> path, <string> fileName, <string> ETag, RTDB_DownloadProgressCallback callback = NULL);

bool setFileAsync(FirebaseData *fbdo, firebase_mem_storage_type storageType, <string> path, <string> fileName, <string> ETag, RTDB_DownloadProgressCallback callback = NULL);
```




#### Set the Firebase server's timestamp to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which timestamp will be set.

return **`Boolean`** value, indicates the success of the operation.
    
Call `<FirebaseData>.to<int>()` will return the integer value of timestamp in seconds 
or `<FirebaseData>.to<double>()` to get millisecond timestamp. 

Due to bugs in Serial.print in Arduino, to print large double value with zero decimal place, 
use `Serial.printf("%.0lf\n", firebaseData.to<double>());`.

```cpp
bool setTimestamp(FirebaseData *fbdo, <string> path);

bool setTimestampAsync(FirebaseData *fbdo, <string> path);
```



#### Update (patch) the child (s) nodes to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which child (s) nodes will be updated.

param **`json`** The pointer to FirebaseJson object used for the update.

```cpp
bool updateNode(FirebaseData *fbdo, <string> path, FirebaseJson *json);

bool updateNodeAsync(FirebaseData *fbdo, <string> path, FirebaseJson *json);
```



#### Update (patch) the child (s) nodess and virtual child ".priority" to the defined node.

```cpp
bool updateNode(FirebaseData *fbdo, <string> path, FirebaseJson *json, float priority);

bool updateNodeAsync(FirebaseData *fbdo, <string> path, FirebaseJson *json, float priority);
```



#### Update (patch) the child (s) nodes to the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node in which child (s) nodes will be updated.

param **`json`** The pointer to FirebaseJson object used for the update.

return **`Boolean`** value, indicates the success of the operation.

Owing to the objective of this function to reduce network data usage, 
no payload will be returned from the server.

```cpp
bool updateNodeSilent(FirebaseData *fbdo, <string> path, FirebaseJson *json);

bool updateNodeSilentAsync(FirebaseData *fbdo, <string> path, FirebaseJson *json);
```



#### Update (patch) the child (s) nodes and virtual child ".priority" to the defined node.

```cpp
bool updateNodeSilent(FirebaseData *fbdo, <string> path, FirebaseJson *json, float priority);

bool updateNodeSilentAsync(FirebaseData *fbdo, <string> path, FirebaseJson *json, float priority);
```



#### Read generic type of value at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

return **`Boolean`** value, indicates the success of the operation.



Call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to determine what type of data successfully stores in the database. 

Call `<FirebaseData>.to<int>()`, `<FirebaseData>.to<float>`, `<FirebaseData>.to<double>`, 
`<FirebaseData>.to<bool>`, `<FirebaseData>.to<String>`, `<FirebaseData>.to<FirebaseJson>()`,
`<FirebaseData>.to<FirebaseJson *>()`, `<FirebaseData>.to<FirebaseJsonArray>()`,
`<FirebaseData>.to<FirebaseJsonArray *>()`, `<FirebaseData>.to<std::vector<uint8_t> *>` and `<FirebaseData>.to<File>()` corresponded to its type that get from `<FirebaseData>.dataType`.

```cpp
bool get(FirebaseData *fbdo, <string> path);
```



#### Read (get) the integer value at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

return **`Boolean`** value, indicates the success of the operation.

Call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to determine what type of data successfully stores in the database. 
    
Call `<FirebaseData>.to<int>()` will return the integer value of payload returned from server.

If the type of payload returned from server is not integer, float and double, 
the function `<FirebaseData>.to<int>()` will return zero (0).

```cpp
bool getInt(FirebaseData *fbdo, <string> path);
```



#### Read (get) the integer value at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

param **`target`** The pointer to int type variable to store the value.

return **`Boolean`** value, indicates the success of the operation.

If the type of payload returned from the server is not an integer, float and double, 
the target variable's value will be zero (0).

```cpp
bool getInt(FirebaseData *fbdo, <string> path, int *target);
```



#### Read (get) the float value at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

return **`Boolean`** value, indicates the success of the operation.

Call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to determine what type of data successfully stores in the database. 
    
Call `<FirebaseData>.to<float>()` will return the float value of payload returned from server.

If the payload returned from server is not integer, float and double, 
the function `<FirebaseData>.to<float>()` will return zero (0).

```cpp
bool getFloat(FirebaseData *fbdo, <string> path);
```



#### Read (get) the float value at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

param **`target`** The pointer to float type variable to store the value.

return **`Boolean`** value, indicates the success of the operation.

If the type of payload returned from the server is not an integer, float and double, 
the target variable's value will be zero (0).

```cpp
bool getFloat(FirebaseData *fbdo, <string> path, float *target);
```



#### Read (get) the double value at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

return **`Boolean`** value, indicates the success of the operation.



Call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to determine what type of data successfully stores in the database. 
    
Call `<FirebaseData>.to<double>()` will return the double value of payload returned from server.

If the payload returned from server is not integer, float and double, 
the function `<FirebaseData>.to<double>()` will return zero (0).

Due to bugs in Serial.print in Arduino, to print large double value with zero decimal place, 
use `Serial.printf("%.9lf\n", firebaseData.to<double>());` for print value up to 9 decimal places.

```cpp
bool getDouble(FirebaseData *fbdo, <string> path);
```



#### Read (get) the double value at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

param **`target`** The pointer to double type variable to store the value.

return **`Boolean`** value, indicates the success of the operation.

If the type of payload returned from the server is not an integer, float and double, 
the target variable's value will be zero (0).

```cpp
bool getDouble(FirebaseData *fbdo, <string> path, double *target);
```



#### Read the Boolean value at the defined node

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

return **`Boolean`** value, indicates the success of the operation.



Call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to determine what type of data successfully stores in the database. 
    
Call `<FirebaseData>.to<bool>()` will return the boolean value of payload returned from server.

If the type of payload returned from the server is not Boolean, 
the function `<FirebaseData>.to<bool>()` will return false.

```cpp
bool getBool(FirebaseData *fbdo, <string> path);
```



#### Read (get) the Boolean value at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

param **`target`** The pointer to boolean type variable to store the value.

return **`Boolean`** value, indicates the success of the operation.

If the type of payload returned from the server is not Boolean, 
the target variable's value will be false.

```cpp
bool getBool(FirebaseData *fbdo, <string> path, bool *target);
```



#### Read (get) the string at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

return **`Boolean`** value, indicates the success of the operation.



Call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to determine what type of data successfully stores in the database. 
    
Call `<FirebaseData>.to<String>()` will return the String value of payload returned from server.

If the type of payload returned from the server is not a string,
the function `<FirebaseData>.to<String>()` will return empty string.

```cpp
bool getString(FirebaseData *fbdo, <string> path);
```



#### Read (get) the string at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

param **`target`** String, std::string or chars array variable to store the value.

return **`Boolean`** value, indicates the success of the operation.

If the target is chars array, the size of chars array should be greater than the size of payload string to prevent error.


```cpp
bool getString(FirebaseData *fbdo, <string> path, <string_or_chars_array> *target);
```



#### Read (get) the child (s) nodes at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

return **`Boolean`** value, indicates the success of the operation.



Call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to determine what type of data successfully stores in the database. 
    
Call `<FirebaseData>.to<FirebaseJson>()` and `<FirebaseData>.to<FirebaseJson *>()` will return reference to object and pointer to FirebaseJson object from payload. 

If the type of payload returned from server is not json,
the function `<FirebaseData>.to<FirebaseJson>()` will contain empty object.

```cpp
bool getJSON(FirebaseData *fbdo, <string> path);
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
bool getJSON(FirebaseData *fbdo, <string> path, FirebaseJson *target);
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


Call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to determine what type of data successfully stores in the database. 
    
Call `<FirebaseData>.to<FirebaseJson>()` and `<FirebaseData>.to<FirebaseJson *>()` will return reference to object and pointer to FirebaseJson object from payload.

If the type of payload returned from server is not JSON,
the function `<FirebaseData>.to<FirebaseJson>()` will contain empty object.

```cpp
bool getJSON(FirebaseData *fbdo, <string> path, QueryFilter *query);
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
bool getJSON(FirebaseData *fbdo, <string> path, QueryFilter *query, FirebaseJson *target);
```



#### Read (get) the array at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

return **`Boolean`** value, indicates the success of the operation.



Call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to determine what type of data successfully stores in the database. 
    
Call `<FirebaseData>.to<FirebaseJsonArray>()` and `<FirebaseData>.to<FirebaseJsonArray *>()` will return reference to object and pointer to FirebaseJsonArray object that contains the array from payload.

If the type of payload returned from the server is not an array,
the array element in `<FirebaseData>.to<FirebaseJsonArray>()` will be empty.

```cpp
bool getArray(FirebaseData *fbdo, <string> path);
```



#### Read (get) the array at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

param **`target`** The pointer to FirebaseJsonArray object variable to store the value.

return **`Boolean`** value, indicates the success of the operation.

If the type of payload returned from the server is not an array, 
the target FirebaseJsonArray object will contain an empty array.

```cpp
bool getArray(FirebaseData *fbdo, <string> path, FirebaseJsonArray *target);
```



#### Read (get) the array data at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

param **`query`** QueryFilter class to set query parameters to filter data.

return **`Boolean`** value, indicates the success of the operation.



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




Call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to determine what type of data successfully stores in the database. 
    
Call `<FirebaseData>.to<FirebaseJsonArray>()` and `<FirebaseData>.to<FirebaseJsonArray *>()` will return reference to object and pointer to FirebaseJsonArray object that contains the array from payload.

If the type of payload returned from the server is not an array,
the function `<FirebaseData>.to<FirebaseJsonArray>()` will contain empty array.

```cpp
bool getArray(FirebaseData *fbdo, <string> path, QueryFilter *query);
```



#### Read (get) the array data at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

param **`target`** The pointer to FirebaseJsonArray object variable to store the value.

return **`Boolean`** value, indicates the success of the operation.

If the type of payload returned from the server is not an array,
the target FirebaseJsonArray object will contain an empty array.

```cpp
bool getArray(FirebaseData *fbdo, <string> path, QueryFilter *query, FirebaseJsonArray *target);
```



#### Read (get) the blob (binary data) at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node.

return **`Boolean`** value, indicates the success of the operation.



Call `<FirebaseData>.dataType` or `<FirebaseData>.dataTypeNum` to determine what type of data successfully stores in the database.

Call `<FirebaseData>.to<std::vector<uint8_t> *>` will return the pointer to uint8_t dynamic array data of payload returned from server.

If the type of payload returned from the server is not a blob,
the function `<FirebaseData>.to<std::vector<uint8_t> *>` will return empty array.

```cpp
bool getBlob(FirebaseData *fbdo, <string> path);
```



#### Read (get) the blob (binary data) at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node

param **`target`** The pointer to uint8_t vector variable to store the value.

return **`Boolean`** value, indicates the success of the operation.

If the type of payload returned from the server is not a blob, 
the target variable value will be an empty array.

```cpp
bool getBlob(FirebaseData *fbdo, <string> path, std::vector<uint8_t> *target);
```



#### Download file data at the defined node and save to storage memory.

The downloaded data will be decoded to binary and save to SD card/Flash memory, 
then please make sure that data at the defined node is the file type.

param **`fbdo`** The pointer to Firebase Data Object.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd.

The file systems can be changed in FirebaseFS.h.

param **`nodePath`** The path to the node that file data will be downloaded.

param **`fileName`**  The file path includes its name.

param **`callback`** Optional. The callback function that accept RTDB_DownloadStatusInfo data.

return **`Boolean`** value, indicates the success of the operation.

```cpp
bool getFile(FirebaseData *fbdo, firebase_mem_storage_type storageType, <string> nodePath, <string> fileName, RTDB_DownloadProgressCallback callback = NULL);
```



#### Download a firmware file from the database.

param **`fbdo`** The pointer to Firebase Data Object.

param **`fwPath`** The firmware data path.

param **`callback`** Optional. The callback function that accept RTDB_DownloadStatusInfo data.

return **`Boolean`** value, indicates the success of the operation.

Note: In ESP8266, this function will allocate 16k+ memory for internal SSL client.

Firmware data is the bin file that stored on datanbase using pushFile or setFile function. 

```cpp
bool downloadOTA(FirebaseData *fbdo, <string> fwPath, RTDB_DownloadProgressCallback callback = NULL);
```



#### Delete all child nodes at the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node to be deleted.

return **`Boolean`** value, indicates the success of the operation.*

```cpp
bool deleteNode(FirebaseData *fbdo, <string> path);
```



#### Delete all child nodes at the defined node if defined node's ETag matched the ETag value.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** nThe path to the node to be deleted.

param **`ETag`** Known unique identifier string (ETag) of defined node.

return **`Boolean`** value, indicates the success of the operation.*

If ETag at the defined node does not match the provided ETag parameter,
the operation will be failed with the http return code 412, Precondition Failed (ETag is not matched).

```cpp
bool deleteNode(FirebaseData *fbdo, <string> path, <string> ETag);
```



#### Delete nodes that its timestamp node exceeded the data retaining period.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The parent path of children nodes that is being deleted.

param **`timestampNode`** The sub-child node that keep the timestamp. 

param **`limit`** The maximum number of children nodes to delete at once, 30 is maximum.

param **`dataRetentionPeriod`** The period in seconds of data in the past which will be retained.

return **`Boolean`** value, indicates the success of the operation.*

note: The databaseSecret can be empty if the auth type is OAuth2.0 or legacy and required if auth type is Email/Password sign-in.

```cpp
 bool deleteNodesByTimestamp(FirebaseData *fbdo, <string> path, <string> timestampNode, size_t limit, unsigned long dataRetentionPeriod);
```



#### Subscribe to the value changes on the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`path`** The path to the node to subscribe.

return **`Boolean`** value, indicates the success of the operation.*

```cpp
bool beginStream(FirebaseData *fbdo, <string> path);
```



#### Subscribe to the value changes on the children of the defined node.

param **`fbdo`** The pointer to Firebase Data Object.

param **`parentPath`** The path to the parent node to subscribe.

return **`Boolean`** value, indicates the success of the operation.*

```cpp
bool beginMultiPathStream(FirebaseData *fbdo, <string> parentPath);
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




#### Run Stream manually.

To manually triggering the stream callback function, 
this should call repeatedly in loop().

```cpp
void runStream();
```




#### Backup (download) the database at the defined node to the storage memory.

param **`fbdo`** The pointer to Firebase Data Object.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd.

The file systems can be changed in FirebaseFS.h.

param **`nodePath`** The path to the node to be backuped.

param **`fileName`**  File name to save.

param **`callback`** Optional. The callback function that accept RTDB_DownloadStatusInfo data.

return **`Boolean`** value, indicates the success of the operation.

```cpp
bool backup(FirebaseData *fbdo, firebase_mem_storage_type storageType, <string> nodePath, <string> fileName, RTDB_DownloadProgressCallback callback = NULL);
```



#### Restore the database at a defined path using backup file saved on SD card/Flash memory.

param **`fbdo`** The pointer to Firebase Data Object.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd.

The file systems can be changed in FirebaseFS.h.

param **`nodePath`** The path to the node to be restored the data.

param **`fileName`** File name to read.

param **`callback`** Optional. The callback function that accept RTDB_UploadStatusInfo data.

return **`Boolean`** value, indicates the success of the operation.

```cpp
bool restore(FirebaseData *fbdo, firebase_mem_storage_type storageType, <string> nodePath, <string> fileName, RTDB_UploadProgressCallback callback = NULL);
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
bool saveErrorQueue(FirebaseData *fbdo, <string> filename, firebase_mem_storage_type storageType);
```



#### Delete file in storage memory.

param **`filename`** File name to delete.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd.

The file systems can be changed in FirebaseFS.h.
    
```cpp
bool deleteStorageFile(<string> filename, firebase_mem_storage_type storageType);
```



#### Restore the Firebase Error Queues from the queue file (flash memory).

param **`fbdo`** The pointer to Firebase Data Object.

param **`filename`** Filename to be read and restore queues.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd.

The file systems can be changed in FirebaseFS.h.
    
```cpp
bool restoreErrorQueue(FirebaseData *fbdo, <string> filename, firebase_mem_storage_type storageType);
```



#### Get the number of Firebase Error Queues stored in a defined file (flash memory).

param **`fbdo`** The pointer to Firebase Data Object.

param **`filename`** Filename to be read and count for queues.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd.

The file systems can be changed in FirebaseFS.h.

return **`Number`** (0-255) of queues store in defined queue file.

```cpp
uint8_t errorQueueCount(FirebaseData *fbdo, <string> filename, firebase_mem_storage_type storageType);
```



#### Get number of queues in Firebase Data object's Error Queues collection.

param **`fbdo`** The pointer to Firebase Data Object.

return **`Number`** (0-255) of queues in Firebase Data object queue collection.

```cpp
uint8_t errorQueueCount(FirebaseData *fbdo);
```



#### Get whether the Firebase Error Queues collection was full or not.

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



#### Get whether the Firebase Error Queue currently exists in the Error Queue collection or not.

param **`fbdo`** The pointer to Firebase Data Object.

param **`errorQueueID`** The Firebase Error Queue ID get from getErrorQueueID.
    
return **`Boolean`** typestatus indicates the queue existence.

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

**queueInfo.currentQueueID()**, get current Error Queue ID that is being processed.

**queueInfo.isQueueFull()**, determine whether Error Queue Collection is full or not.

**queueInfo.dataType()**, get a string of the Firebase call data type that is being processed of current Error Queue.

**queueInfo.method()**, get a string of the Firebase call method that is being processrd of current Error Queue.

**queueInfo.path()**, get a string of the Firebase call path that is being processed of current Error Queue.

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


These functions can be called directly from Firestore object in the Firebase object e.g. Firebase.Firestore.\<function name\>



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
bool exportDocuments(FirebaseData *fbdo, <string> projectId, <string> databaseId, <string> bucketID, <string> storagePath, <string> collectionIds = "");
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
bool importDocuments(FirebaseData *fbdo, <string> projectId, <string> databaseId, <string> bucketID, <string> storagePath, <string> collectionIds = "");
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
bool createDocument(FirebaseData *fbdo, <string> projectId, <string> databaseId, <string> documentPath, <string> content, <string> mask = "");
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
bool createDocument(FirebaseData *fbdo, <string> projectId, <string> databaseId, <string> collectionId, <string> documentId, <string> content, <string> mask = "");
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
bool patchDocument(FirebaseData *fbdo, <string> projectId, <string> databaseId, <string> documentPath, <string> content, <string> updateMask, <string> mask = "", <string> exists = "", <string> updateTime = "");
```



####  Commits a transaction, while optionally updating documents.

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`databaseId`** The Firebase Cloud Firestore database id which is (default) or empty "".

param **`writes`** The dyamic array of write object firebase_firestore_document_write_t.

For the write object, see https://firebase.google.com/docs/firestore/reference/rest/v1/Write

param **`transaction`** A base64-encoded string. If set, applies all writes in this transaction, and commits it.

return **`Boolean`** value, indicates the success of the operation.

Use FirebaseData.payload() to get the returned payload.

This function requires Email/password, Custom token or OAuth2.0 authentication.

```cpp
bool commitDocument(FirebaseData *fbdo, <string> projectId, <string> databaseId, 
std::vector<struct firebase_firestore_document_write_t> writes, <string> transaction = "");

bool commitDocumentAsync(FirebaseData *fbdo, <string> projectId, <string> databaseId, 
std::vector<struct firebase_firestore_document_write_t> writes, <string> transaction = "");
```



####  Applies a batch of write operations.

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`databaseId`** The Firebase Cloud Firestore database id which is (default) or empty "".

param **`writes`** The dyamic array of write object firebase_firestore_document_write_t.

Method does not apply writes atomically and does not guarantee ordering.

Each write succeeds or fails independently.

You cannot write to the same document more than once per request.

For the write object, see https://firebase.google.com/docs/firestore/reference/rest/v1/Write

param **`labels`** The FirebaseJson pointer that represents the Labels (map) associated with this batch write.

return **`Boolean`** value, indicates the success of the operation.

Use FirebaseData.payload() to get the returned payload.

This function requires Email/password, Custom token or OAuth2.0 authentication.

For more description, see https://cloud.google.com/firestore/docs/reference/rest/v1/projects.databases.documents/batchWrite

```cpp
bool batchWriteDocuments(FirebaseData *fbdo, <string> projectId, <string> databaseId, 
std::vector<struct firebase_firestore_document_write_t> writes, FirebaseJson *labels = nullptr);
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
bool getDocument(FirebaseData *fbdo, <string> projectId, <string> databaseId, <string> documentPath, <string> mask = "", <string> transaction = "", <string> readTime = "");
```




#### Gets multiple documents.

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`databaseId`** The Firebase Cloud Firestore database id which is (default) or empty "".

param **`documentPaths`** The list of relative path of documents to get. Use comma (,) to separate between the field names.

param **`mask`** The fields to return. If not set, returns all fields. 

If the document has a field that is not present in this mask, that field will not be returned in the response. 

Use comma (,) to separate between the field names.

param **`batchOperationCallback`** The callback fuction that accepts const char* as argument.

Union field consistency_selector can be only one of the following.

param **`transaction`** Reads the document in a transaction. A base64-encoded string.

param **`newTransaction`** FirebaseJson pointer that represents TransactionOptions object.

Starts a new transaction and reads the documents.

Defaults to a read-only transaction.

The new transaction ID will be returned as the first response in the stream.

param **`readTime`** Reads the version of the document at the given time. This may not be older than 270 seconds.

A timestamp in RFC3339 UTC "Zulu" format, with nanosecond resolution and up to nine fractional digits. 

Examples: "2014-10-02T15:01:23Z" and "2014-10-02T15:01:23.045123456Z".

return **`Boolean`** value, indicates the success of the operation.

Use FirebaseData.payload() to get the returned payload.

This function requires Email/password, Custom token or OAuth2.0 authentication.

```cpp
bool batchGetDocuments(FirebaseData *fbdo, <string> projectId, <string> databaseId, <string> documentPaths, <string> mask, FirebaseData::FirestoreBatchOperationsCallback batchOperationCallback, <string> transaction, 
FirebaseJson *newTransaction, <string> readTime);
```




#### Starts a new transaction.

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`databaseId`** The Firebase Cloud Firestore database id which is (default) or empty "".

param **`transactionOptions`** Optional. The TransactionOptions type data that represents the options for creating a new transaction.

return **`Boolean`** value, indicates the success of the operation.

Use FirebaseData.payload() to get the returned payload.

This function requires OAuth2.0 authentication.

The TransactionOptions struct contains two properties i.e. readOnly and readWrite.

Use readOnly for options for a transaction that can only be used to read documents.

Use readWrite for options for a transaction that can be used to read and write documents.

The readOnly property contains one property, readTime.

The readTime is for reading the documents at the given time. This may not be older than 60 seconds.

A timestamp in RFC3339 UTC "Zulu" format, with nanosecond resolution and up to nine fractional digits.

Examples: "2014-10-02T15:01:23Z" and "2014-10-02T15:01:23.045123456Z".

The readWrite property contains one property, retryTransaction.

The retryTransaction is a base64-encoded string represents a transaction that can be used to read and write documents.

See https://cloud.google.com/firestore/docs/reference/rest/v1/TransactionOptions for transaction options.
    

```cpp
bool beginTransaction(FirebaseData *fbdo, <string> projectId, <string> databaseId, TransactionOptions *transactionOptions = nullptr);
```



#### Rolls back a transaction.

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`databaseId`** The Firebase Cloud Firestore database id which is (default) or empty "".

param **`transaction`** Required. A base64-encoded string of the transaction to roll back.

return **`Boolean`** value, indicates the success of the operation.

Use FirebaseData.payload() to get the returned payload.

This function requires OAuth2.0 authentication.

```cpp
bool rollback(FirebaseData *fbdo, <string> projectId, <string> databaseId, <string> transaction);
```



#### Runs a query.

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`databaseId`** The Firebase Cloud Firestore database id which is (default) or empty "".

param **`documentPath`** The relative path of document to get.

param **`structuredQuery`** The pointer to FirebaseJson object that contains the Firestore query. For the description of structuredQuery, see https://cloud.google.com/firestore/docs/reference/rest/v1/StructuredQuery

param **`consistencyMode`** Optional. The consistency mode for this transaction 
e.g. firebase_firestore_consistency_mode_transaction,
firebase_firestore_consistency_mode_newTransaction
and firebase_firestore_consistency_mode_readTime

param **`consistency`** Optional. The value based on consistency mode e.g. transaction string, TransactionOptions (JSON) and date time string.

For more description, see https://cloud.google.com/firestore/docs/reference/rest/v1/projects.databases.documents/runQuery#body.request_body.FIELDS

return **`Boolean`** value, indicates the success of the operation.

Use FirebaseData.payload() to get the returned payload.

This function requires Email/password, Custom token or OAuth2.0 authentication.

```cpp
bool runQuery(FirebaseData *fbdo, <string> projectId, <string> databaseId, <string> documentPath, <string> structuredQuery, firebase_firestore_consistency_mode consistencyMode, <string> consistency);
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
bool deleteDocument(FirebaseData *fbdo, <string> projectId, <string> databaseId, <string> documentPath, <string> exists = "", <string> updateTime = "");
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
bool listDocuments(FirebaseData *fbdo, <string> projectId, <string> databaseId, <string> collectionId, int pageSize, <string> pageToken, <string> orderBy, <string> mask, bool showMissing);
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
bool listCollectionIds(FirebaseData *fbdo, <string> projectId, <string> databaseId, <string> documentPath, int pageSize, <string> pageToken);
```



#### Creates a composite index.

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`databaseId`** The Firebase Cloud Firestore database id which is (default) or empty "".

param **`collectionId`** The relative path of document colection.

param **`apiScope`** The API scope enum e.g., ANY_API and DATASTORE_MODE_API

param **`queryScope`** The QueryScope enum string e.g., QUERY_SCOPE_UNSPECIFIED, COLLECTION, and COLLECTION_GROUP

See https://cloud.google.com/firestore/docs/reference/rest/Shared.Types/QueryScope

param **`fields`** The FirebaseJsonArray that represents array of fields (IndexField JSON object) of indexes.

A IndexField object contains the keys "fieldPath" and the uinion field "value_mode" of "order" and "arrayConfig"

Where the fieldPath value is the field path string of index.

Where order is the enum string of ORDER_UNSPECIFIED, ASCENDING, and DESCENDING.

And arrayConfig is the ArrayConfig enum string of ARRAY_CONFIG_UNSPECIFIED and CONTAINS

return **`Boolean`** value, indicates the success of the operation.

Note Use FirebaseData.payload() to get the returned payload.

This function requires OAuth2.0 authentication.

For more description, see https://cloud.google.com/firestore/docs/reference/rest/v1/projects.databases.collectionGroups.indexes/create

```cpp
bool createIndex(FirebaseData *fbdo, <string> projectId, <string> databaseId, <string> collectionId, 
<string> apiScope, <string> queryScope, FirebaseJsonArray *fields);
```



#### Deletes an index.

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`databaseId`** The Firebase Cloud Firestore database id which is (default) or empty "".

param **`collectionId`** The relative path of document colection.

param **`indexId`** The index to delete.

Note Use FirebaseData.payload() to get the returned payload.

This function requires OAuth2.0 authentication.

For more description, see https://cloud.google.com/firestore/docs/reference/rest/v1/projects.databases.collectionGroups.indexes/delete

```cpp
bool deleteIndex(FirebaseData *fbdo, <string> projectId, <string> databaseId, <string> collectionId, <string> indexId);
```



#### Lists the indexes that match the specified filters.

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`databaseId`** The Firebase Cloud Firestore database id which is (default) or empty "".

param **`collectionId`** The relative path of document colection.

param **`filter`** The filter to apply to list results.

param **`pageSize`** The number of results to return.

param **`pageToken`** A page token, returned from a previous call to FirestoreAdmin.ListIndexes, that may be used to get the next page of results.

Note Use FirebaseData.payload() to get the returned payload.

This function requires OAuth2.0 authentication.

For more description, see https://cloud.google.com/firestore/docs/reference/rest/v1/projects.databases.collectionGroups.indexes/list

```cpp
bool listIndex(FirebaseData *fbdo, <string> projectId, <string> databaseId, <string> collectionId, <string> filter,
int pageSize, <string> pageToken);
```



#### Get an index.

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`databaseId`** The Firebase Cloud Firestore database id which is (default) or empty "".

param **`collectionId`** The relative path of document colection.

param **`indexId`** The index to get.

Note Use FirebaseData.payload() to get the returned payload.

This function requires OAuth2.0 authentication.

For more description, see https://cloud.google.com/firestore/docs/reference/rest/v1/projects.databases.collectionGroups.indexes/get

```cpp
bool getIndex(FirebaseData *fbdo, <string> projectId, <string> databaseId, <string> collectionId, <string> indexId);
```




## Firebase Cloud Messaging Functions

These functions can be called directly from FCM object in the Firebase object e.g. Firebase.FCM.\<function name\>


#### Set the server key.

param **`serverKey`** Server key found on Console: Project settings > Cloud Messaging

param **`spi_ethernet_module`** SPI_ETH_Module struct data, optional for ESP8266 use with Ethernet module.



note: This server key required for sending message via legacy HTTP API.

SPI_ETH_Module struct data is for ESP8266 Ethernet supported module lwip interface.

The usage example for Ethernet.

```cpp
#include <ENC28J60lwIP.h>

#define ETH_CS_PIN 16 //GPIO 16 connected to Ethernet module (ENC28J60) CS pin
 
ENC28J60lwIP eth(ETH_CS_PIN);

FirebaseData fbdo;

SPI_ETH_Module spi_ethernet_module;

//in setup()

spi_ethernet_module.enc28j60 = &eth;

Firebase.FCM.setServerKey(FIREBASE_FCM_SERVER_KEY, &spi_ethernet_module);

```

The API key created in the Google Cloud console, cannot be used for authorizing FCM requests. 

```cpp
void setServerKey(<string> serverKey, SPI_ETH_Module *spi_ethernet_module = NUL);
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
bool subscribeTopic(FirebaseData *fbdo, <string> topic, <string> IID[], size_t numToken);
```



#### Unsubscribe the devices from the topic.

param **`fbdo`** The pointer to Firebase Data Object.

param **`topic`** The topic to romove the subscription.

param **`IID`** The instance ID tokens or registration tokens array.

param **`numToken`** The size of instance ID tokens array.

return **`Boolean`** value, indicates the success of the operation. 

```cpp
bool unsubscribeTopic(FirebaseData *fbdo, <string> topic, <string> IID[], size_t numToken);
```



#### Get the app instance info.

param **`fbdo`** The pointer to Firebase Data Object.

param **`IID`** The instance ID token of device.

return **`Boolean`** value, indicates the success of the operation. 

```cpp
bool appInstanceInfo(FirebaseData *fbdo, <string> IID);
```



#### Create registration tokens for APNs tokens.

param **`fbdo`** The pointer to Firebase Data Object.

param **`application`** The Bundle id of the app.

param **`sandbox`** The Boolean to indicate sandbox environment (TRUE) or production (FALSE).

param **`APNs`** The iOS APNs tokens array.

param **`numToken`** The size of instance ID tokens array.

return **`Boolean`** value, indicates the success of the operation. 

```cpp
bool regisAPNsTokens(FirebaseData *fbdo, <string> application, bool sandbox, <string> APNs[], size_t numToken);
```



#### Get the server payload.

param **`fbdo`** The pointer to Firebase Data Object.

return **`String`** of payload returned from the server.

```cpp
String payload(FirebaseData *fbdo);
```



## Firebase Storage Functions.

These functions can be called directly from Storage object in the Firebase object e.g. Firebase.Storage.\<function name\>



#### Upload file to the Firebase Storage data bucket.

param **`fbdo`** The pointer to Firebase Data Object.

param **`bucketID`** The Firebase storage bucket ID in the project.

param **`localFileName`** The file path includes its name to upload.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd.

The file systems can be changed in FirebaseFS.h.

param **`remotetFileName`** The file path includes its name of uploaded file in data bucket.

param **`mime`** The file MIME type

param **`callback`** Optional. The callback function that accept FCS_UploadStatusInfo data.

return **`Boolean`** value, indicates the success of the operation. 

Use FirebaseData.downloadURL() to get the download link.

```cpp
bool upload(FirebaseData *fbdo, <string> bucketID, <string> localFileName, firebase_mem_storage_type storageType, <string> remotetFileName, <string> mime, FCS_UploadProgressCallback callback = NULL);
```



#### Upload byte array to the Firebase Storage data bucket.

param **`fbdo`** The pointer to Firebase Data Object.

param **`bucketID`** The Firebase storage bucket ID in the project.

param **`data`** The byte array of data.

param **`len`** The size of byte array data in bytes.

param **`remotetFileName`** The file path includes its name of uploaded file in data bucket.

param **`mime`** The file MIME type

param **`callback`** Optional. The callback function that accept FCS_UploadStatusInfo data.

return **`Boolean`** value, indicates the success of the operation. 

Use FirebaseData.downloadURL() to get the download link.

```cpp
bool upload(FirebaseData *fbdo, <string> bucketID, uint8_t *data, size_t len, <string> remoteFileName, <string> mime, FCS_UploadProgressCallback callback = NULL);
```



#### Download file from the Firebase Storage data bucket.

param **`fbdo`** The pointer to Firebase Data Object.

param **`bucketID`** The Firebase storage bucket ID in the project.

param **`remotetFileName`** The file path includes its name of file in the data bucket to download.

param **`localFileName`** The file path includes its name to save.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd.

The file systems can be changed in FirebaseFS.h.

param **`callback`** Optional. The callback function that accept FCS_DownloadStatusInfo data.

return **`Boolean`** value, indicates the success of the operation. 

```cpp
bool download(FirebaseData *fbdo, <string> bucketID, <string> remoteFileName, <string> localFileName, firebase_mem_storage_type storageType, FCS_DownloadProgressCallback callback = NULL);
```



#### Download a firmware file from the Firebase Storage data bucket for OTA updates.

param **`fbdo`** The pointer to Firebase Data Object.

param **`bucketID`** The Firebase storage bucket ID in the project.

param **`remotetFileName`** The firmware file path includes its name of file in the data bucket to download.

param **`callback`** Optional. The callback function that accept FCS_DownloadStatusInfo data.

return **`Boolean`** value, indicates the success of the operation. 

Note: In ESP8266, this function will allocate 16k+ memory for internal SSL client.

```cpp
bool downloadOTA(FirebaseData *fbdo, <string> bucketID, <string> remoteFileName, FCS_DownloadProgressCallback callback = NULL);
```



#### Get the meta data of file in Firebase Storage data bucket

param **`fbdo`** The pointer to Firebase Data Object.

param **`bucketID`** The Firebase storage bucket ID in the project.

param **`remotetFileName`** The file path includes its name of file in the data bucket.

return **`Boolean`** value, indicates the success of the operation. 

Use the FileMetaInfo type data to get name, bucket, contentType, size, 
generation, etag, crc32, downloadTokens properties from file.

```cpp
bool getMetadata(FirebaseData *fbdo, <string> bucketID, <string> remoteFileName);
```



#### Delete file from Firebase Storage data bucket

param **`fbdo`** The pointer to Firebase Data Object.

param **`bucketID`** The Firebase storage bucket ID in the project.

param **`remotetFileName`** The file path includes its name of file in the data bucket.

return **`Boolean`** value, indicates the success of the operation. 

```cpp
bool deleteFile(FirebaseData *fbdo, <string> bucketID, <string> fileName);
```



#### List all files in the Firebase Storage data bucket.

param **`fbdo`** The pointer to Firebase Data Object.

param **`bucketID`** The Firebase storage bucket ID in the project.

return **`Boolean`** value, indicates the success of the operation. 

Use the FileList type data to get name and bucket properties for each item.

```cpp
bool listFiles(FirebaseData *fbdo, <string> bucketID);
```



#### Run Resumable upload tasks manually.

```cpp
void runResumableUploadTask();
```



## Google Cloud Storage Functions.

These functions can be called directly from GCStorage object in the Firebase object e.g. Firebase.GCStorage.\<function name\>



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
bool upload(FirebaseData *fbdo, <string> bucketID, <string> localFileName, firebase_mem_storage_type storageType, firebase_gcs_upload_type uploadType, <string> remoteFileName, <string> mime, UploadOptions *uploadOptions = nullptr, RequestProperties *requestProps = nullptr, UploadStatusInfo *status = nullptr, ProgressCallback callback = NULL);
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

param **`callback`** Optional. The callback function that accept GCS_DownloadStatusInfo data.

return **`Boolean`** value, indicates the success of the operation. 

This function requires OAuth2.0 authentication.

```cpp
bool download(FirebaseData *fbdo, <string> bucketID, <string> remoteFileName, <string> localFileName, firebase_mem_storage_type storageType, StorageGetOptions *options = nullptr, GCS_DownloadProgressCallback callback = NULL);
```




#### Download a firmware file from the Google Cloud Storage data bucket for OTA updates.

param **`fbdo`** The pointer to Firebase Data Object.

param **`bucketID`** The Firebase or Google Cloud Storage bucket ID.

param **`remotetFileName`** The firmware file path includes its name of file in the data bucket to download.

param **`callback`** Optional. The callback function that accept GCS_DownloadStatusInfo data.

return **`Boolean`** value, indicates the success of the operation. 

Note: In ESP8266, this function will allocate 16k+ memory for internal SSL client.

```cpp
bool downloadOTA(FirebaseData *fbdo, <string> bucketID, <string> remoteFileName, GCS_DownloadProgressCallback callback = NULL);
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
bool getMetadata(FirebaseData *fbdo, <string> bucketID, <string> remoteFileName);
```



#### Delete file from Firebase or Google Cloud Storage data bucket.

param **`fbdo`** The pointer to Firebase Data Object.

param **`bucketID`** The Firebase or Google Cloud Storage bucket ID.

param **`remotetFileName`** The file path includes its name of file in the data bucket.

param **`options`** Optional. The pointer to DeleteOptions data contains the query parameters.

For query parameters options, see https://cloud.google.com/storage/docs/json_api/v1/objects/delete#optional-parameters

return **`Boolean`** value, indicates the success of the operation. 

```cpp
bool deleteFile(FirebaseData *fbdo, <string> bucketID, <string> fileName, DeleteOptions *options = nullptr);
```



#### List all files in the Firebase or Google Cloud Storage data bucket.

param **`fbdo`** The pointer to Firebase Data Object.

param **`bucketID`** The Firebase or Google Cloud Storage bucket ID.

param **`options`** Optional. The pointer to ListOptions data that contains the query parameters

Fore query parameters description, see https://cloud.google.com/storage/docs/json_api/v1/objects/list#optional-parameters

return **`Boolean`** value, indicates the success of the operation. 

Use the FileList type data to get name and bucket properties for each item.

```cpp
bool listFiles(FirebaseData *fbdo, <string> bucketID, ListOptions *options = nullptr);
```



## Cloud Functions for Firebase Functions

These functions can be called directly from Functions object in the Firebase object e.g. Firebase.Functions.\<function name\>



#### Synchronously invokes a deployed Cloud Function. 

To be used for testing purposes as very limited traffic is allowed. 

For more information on the actual limits, refer to Rate Limits. https://cloud.google.com/functions/quotas#rate_limits

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`locationId`** The project location.

param **`functionId`** The name of function.

param **`data`** The Input to be passed to the function (JSON serialized string).

return **`Boolean`** value, indicates the success of the operation. 



Use FirebaseData.payload() to get the returned payload.

Ex. if data is {"info":{"name":"Paul","age":30}}

The values can be obtained from http trigger function request e.g. req as following.

req.body.info.name

req.body.info.age

This function requires OAuth2.0 authentication.

```cpp
bool callFunction(FirebaseData *fbdo, <string> projectId, <string> locationId, <string> functionId, <string> data);
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
bool patchFunction(FirebaseData *fbdo, <string> functionId, FunctionsConfig *patchData);
```




#### Run Functions deploying tasks manually.
  
To manually triggering the deploy task callback function, 
this should call repeatedly in loop().

```cpp
void runDeployTasks();
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
bool setIamPolicy(FirebaseData *fbdo, <string> projectId, <string> locationId, <string> functionId, PolicyBuilder *policy, <string> updateMask = "");
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
bool getIamPolicy(FirebaseData *fbdo, <string> projectId, <string> locationId, <string> functionId, <string> version = "");
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
bool getFunction(FirebaseData *fbdo, <string> projectId, <string> locationId, <string> functionId);
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
bool deleteFunction(FirebaseData *fbdo, <string> projectId, <string> locationId, <string> functionId);
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
bool generateDownloadUrl(FirebaseData *fbdo, <string> projectId, <string> locationId, <string> functionId, <string> versionId = "");
```



#### Returns a signed URL for uploading a function source code. 

param **`fbdo`** The pointer to Firebase Data Object.

param **`projectId`** The Firebase project id (only the name without the firebaseio.com).

param **`locationId`** The project location.

return **`Boolean`** value, indicates the success of the operation. 

Use FirebaseData.payload() to get the returned payload.

This function requires OAuth2.0 authentication.

```cpp
bool generateUploadUrl(FirebaseData *fbdo, <string> projectId, <string> locationId);
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
bool listFunctions(FirebaseData *fbdo, <string> projectId, <string> locationId, size_t pageSize, <string> pageToken = "");
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
bool listOperations(FirebaseData *fbdo, <string> filter, int pageSize, <string> pageToken);
```



## PolicyBuilder and FunctionsConfig classes

The description of PolicyBuilder and FunctionsConfig classes and their functions are available in the header files
[/src/functions/PolicyBuilder.h](/functions/PolicyBuilder.h) and [/src/functions/FunctionsConfig.h](/functions/FunctionsConfig.h).




## Firebase Data Object Functions



### Assign external Arduino generic client.

param **`client`** The pointer to Arduino Client.

param **`networkConnectionCB`** The function that handles the network connection.

param **`networkStatusCB`** The function that handle the network connection status acknowledgement.

```cpp
void setGenericClient(Client *client, FB_NetworkConnectionRequestCallback networkConnectionCB,
                        FB_NetworkStatusRequestCallback networkStatusCB);
```




#### Assign TinyGsm Clients.

param **`client`** The pointer to TinyGsmClient.

param **`modem`** The pointer to TinyGsm modem object. Modem should be initialized and/or set mode before transfering data.

param **`pin`** The SIM pin.

param **`apn`** The GPRS APN (Access Point Name).

param **`user`** The GPRS user.

param **`password`** The GPRS password.

```cpp
void setGSMClient(Client *client, void *modem, const char *pin, const char *apn, const char *user, const char *password);
```





#### Assign external Ethernet Client.

param **`client`** The pointer to Ethernet client object.

param **`macAddress`** The Ethernet MAC address.

param **`csPin`** The Ethernet module SPI chip select pin.

param **`resetPin`** The Ethernet module reset pin.

param **`staticIP`** (Optional) The pointer to `Firebase_StaticIP` object which included these IPAddress properties ipAddress, netMask, defaultGateway and dnsServer.

```cpp
 void setEthernetClient(Client *client, uint8_t macAddress[6], int csPin, int resetPin, Firebase_StaticIP *staticIP = nullptr);
```





#### Set the network status acknowledgement.

param **`status`** The network status. 

```cpp
void setNetworkStatus(bool status);
```




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
ESP_SSLClient *getWiFiClient();
```



#### Close the keep-alive connection of the internal SSL client.

note: This will release the memory used by internal SSL client.

```cpp
void stopWiFiClient();
```




#### Close the internal flash temporary file.

```cpp
void closeFile();
```


#### Set the Root certificate for a FirebaseData object

param **`ca`** PEM format certificate string.

```cpp
void setCert(const char* ca);
```



#### Pause/Unpause WiFiClient from all Firebase operations

param **`pause`** True for pause and False for unpause.

return **`Boolean`** value, indicates the success of the operation.

```cpp
bool pauseFirebase(bool pause);
```



#### Get the data type of payload returned from the server (RTDB only)

return **`The one of these data type e.g. integer, float, string, JSON and blob.`**

```cpp
String dataType();
```



#### Get the data type of payload returned from the server (RTDB only)

return **`The enumeration value of firebase_rtdb_data_type.`**

firebase_rtdb_data_type_null or 1,

firebase_rtdb_data_type_integer or 2,

firebase_rtdb_data_type_float or 3,

firebase_rtdb_data_type_double or 4,

firebase_rtdb_data_type_boolean or 5,

firebase_rtdb_data_type_string or 6,

firebase_rtdb_data_type_json or 7,

firebase_rtdb_data_type_array or 8,

firebase_rtdb_data_type_blob or 9,

firebase_rtdb_data_type_file or 10

```cpp
uint8_t dataTypeEnum();
```



#### Get the event type of stream

return **`The one of these event type String e.g. put, patch, cancel, and auth_revoked.`**



The event type "put" indicated that data at the event path relative to the stream path was completely changed. The event path can be determined by dataPath().

The event type "patch" indicated that data at the event path relative to stream path was updated. The event path can be determined by dataPath().

The event type "cancel" indicated something wrong and cancel by the server.

The event type "auth_revoked" indicated the provided Firebase Authentication Data (Database secret) is no longer valid.

```cpp
String eventType();
```



#### Get the unique identifier (ETag) of current data

return **`String.`** of unique identifier.

```cpp
String ETag();
```



#### Get the current stream path

return **`The database streaming path.`**

```cpp
String streamPath();
```



#### Get the current data path

return **`The node which belongs to server' s returned payload.`**

The node returned from this function in case of stream, also changed upon the child or parent's stream
value changes.

```cpp
String dataPath();
```



#### Get the error reason String from the process

return **`The error description string (String object).`**

```cpp
String errorReason();
```


#### Get the error code from the process

return **`The error code (int).`**

See src/FB_Error.h

```cpp
String errorCode();
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



#### Return the internal Firebase JSON Data object.

return **`FirebaseJsonData object`**.

```cpp
FirebaseJsonData &jsonData();
```



#### Return the pointer to internal Firebase JSON Data object.

return **`FirebaseJsonData object pointer`**.

```cpp
FirebaseJsonData *jsonDataPtr();
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



#### Get the stream connection status

return **`Boolean`** type status indicates whether the Firebase Data object is working with the stream or not.

```cpp
bool isStream();
```



#### Get the server connection status

return **`Boolean`** type status indicates whether the Firebase Data object is connected to the server or not.

```cpp
bool httpConnected();
```



#### Get the timeout event of server's stream (30 sec is the default)

Nothing to do when stream connection timeout, the stream connection will be automatically resumed.

return **`Boolean`** type status indicates whether the stream was time out or not.

```cpp
bool streamTimeout();
```



#### Get the availability of data or payload returned from the server

return **`Boolean`** type status indicates whether the server returns the new payload or not.

```cpp
bool dataAvailable();
```



#### Get the availability of stream event-data payload returned from the server

return **`Boolean`** type status indicates whether the server returns the stream event-data 
payload or not.

```cpp
bool streamAvailable();
```



#### Get the matching between data type that intend to get from/store to database and the server's return payload data type

return **`Boolean`** type status indicates whether the type of data that is being get from or stored to database 
and the server's returned payload is matched or not.

```cpp
bool mismatchDataType();
```



#### Get the HTTP status code return from the server

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



#### Get the name (full path) of backup file in SD card/flash memory

return **`String`** (String object) of a file name that stores on SD card/flash memory after backup operation.

```cpp
String getBackupFilename();
```



#### Get the size of the backup file

return **`Number of byte`** of backup file in byte after backup operation.

```cpp
size_t getBackupFileSize();
```



#### Clear or empty data in the Firebase Data object

```cpp
void clear();
```



#### Get the error description for file transferring (pushFile, setFile, backup and restore)

return **`Error description string* (String object).`**

```cpp
String fileTransferError();
```



#### Return the server's payload data

return **`Payload string* (String object).`**

```cpp
String payload();
```



#### Setup TCP KeepAlive for internal TCP client.

param **`tcpKeepIdleSeconds`** lwIP TCP Keepalive idle in seconds.

param **`tcpKeepIntervalSeconds`** lwIP TCP Keepalive interval in seconds.

param **`tcpKeepCount`** lwIP TCP Keepalive count.

For the TCP (KeepAlive) options, see [this doc](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/lwip.html#tcp-options).

If value of one of these parameters is zero, the TCP KeepAlive will be disabled.

You can check the server connecting status, by exexuting `<FirebaseData>.httpConnected()` which will return true when connection to the server is still alive. 

```cpp
void keepAlive(int tcpKeepIdleSeconds, int tcpKeepIntervalSeconds, int tcpKeepCount);
```



#### Get TCP KeepAlive status.

return **`Boolean`** status of TCP Keepalive.

```cpp
bool isKeepAlive();
```

## FirebaseJSON object Functions



#### Set or deserialize the JSON object data (JSON object literal) as FirebaseJson object.

param **`data`** The JSON object literal string to set or deserialize.

return **`bool`** value represents the successful operation.

Call FirebaseJson.errorPosition to get the error.

```cpp
bool setJsonData(<string> data);
```



#### Clear internal buffer of FirebaseJson object.
    
return **`instance of an object.`**

```cpp
FirebaseJson &clear();
```



#### Set JSON data (Client response) to FirebaseJson object.
    
param **`client`** The pointer to or instance of Client object.

return **`instance of an object.`**

```cpp
bool readFrom(Client *client);

bool readFrom(Client &client);
```



#### Set JSON data (WiFiClient response) to FirebaseJson object.
    
param **`client`** The pointer to or instance of WiFiClient object.

return **`instance of an object.`**

```cpp
bool readFrom(WiFiClient *client);

bool readFrom(WiFiClient &client);
```



#### Set JSON data (WiFiClientSecure response) to FirebaseJson object.
    
param **`client`** The pointer to or instance of WiFiClientSecure object.

return **`instance of an object.`**

```cpp
bool readFrom(WiFiClientSecure *client);

bool readFrom(WiFiClientSecure &client);
```



#### Set JSON data (Seral object) to FirebaseJson object.
    
param **`ser`** The HW or SW Serial object.

param **`timeoutMS`** The timeout in millisecond to wait for Serial data to be completed.

return **`instance of an object.`**

```cpp
bool readFrom(HardwareSerial &ser, uint32_t timeoutMS = 5000);

bool readFrom(SoftwareSerial &ser, uint32_t timeoutMS = 5000);
```



#### Set JSON data (File object) to FirebaseJson object.
    
param **`file`** The File object.

return **`instance of an object.`**

```cpp
bool readFrom(fs::File &file);
```



#### Add null to FirebaseJson object.
    
param **`key`** The new key string that null to be added.

return **`instance of an object.`**

```cpp
FirebaseJson &add(<string> key);
```



#### Add value to FirebaseJson object.
    
param **`key`** The new key string that string value to be added.

param **`value`** The value for the new specified key.

return **`instance of an object.`**

The value that can be added is the following supported types e.g. flash string (PROGMEM and FPSTR), String, C/C++ std::string, const char*, char array, string literal, all integer and floating point numbers, boolean, FirebaseJson object and array.

```cpp
FirebaseJson &add(<string> key, <type> value);
```



#### Get the FirebaseJson object serialized string.

param **`out`** The writable object e.g. String, std::string, char array, Stream e.g ile, WiFi/Ethernet Client and LWMQTT, that accepts the returning string.

param **`topic`** The MQTT topic (LWMQTT).

param **`prettify`** Boolean flag for return the pretty format string i.e. with text indentation and newline. 

```cpp
void toString(<type> out, bool prettify = false);

void toString(<type> out, <string> topic, bool prettify = false);
```



#### Get the value from the specified node path in FirebaseJson object.

param **`result`** The reference of FirebaseJsonData that holds the result.

param **`path`** Relative path to the specific node in FirebaseJson object.

param **`prettify`** The text indentation and new line serialization option.

return **`boolean status of the operation.`**

The FirebaseJsonData object holds the returned data which can be read from the following properties.

jsonData.value - contains the returned string.

jsonData.value - contains the returned integer value.

jsonData.value - contains the returned float value.

jsonData.value - contains the returned double value.

jsonData.value - contains the returned boolean value.

jsonData.success - used to determine the result of the get operation.

jsonData.type - used to determine the type of returned value in string represents 
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
bool get(FirebaseJsonData &result, <string> path, bool prettify = false);
```



#### Search element by key or path in FirebaseJsonArray object.

param **`result`** The reference of FirebaseJsonData that holds the result.

param **`criteria`** The FirebaseJson::SearchCriteria data.

param **`prettify`** The text indentation and new line serialization option.

return **`number`** of elements found from search.

The SearchCriteria data has the properties e.g.

**`path`** - The key of path to search.

Path can be wildcard with * in search path and * should use as key in part and do not mix with any character.

**`value`** - The value string to search.

**`depth`** - The begin depth (int) of element to search, default is 0.

**`endDepth`** - The end depth (int) of element to search, default is -1.

**`searchAll`** - The boolean option to search all occurrences of elements.

```cpp
size_t search(SearchCriteria &criteria);

size_t search(FirebaseJsonData &result, SearchCriteria &criteria, bool prettify = false);
```



#### Get the full path to any element in FirebaseJson object.

param **`path`** The key or path to search in to.

param **`searchAll`** Search all occurrences.

return **`full path string`** in case of found.

```cpp
String getPath(<string> path, bool searchAll = false);
```



#### Check whether key or path to the child element existed in FirebaseJson object or not.

param **`path`** The key or path of child element check.

return **`boolean`** status indicated the existence of element.

```cpp
bool isMember(<string> path);
```



#### Parse and collect all node/array elements in FirebaseJson object.

return **`number`** of child/array elements in FirebaseJson object.

```cpp
size_t iteratorBegin();
```



#### Get child/array elements from FirebaseJson objects at specified index.
    
param **`index`** The element index to get.

param **`type`** The integer which holds the type of data i.e. FirebaseJson::OBJECT and FirebaseJson::ARRAY

param **`key`** The string which holds the key/name of the object, can return empty String if the data type is an array.

param **`value`** The string which holds the value for the element key or array.   

```cpp
void iteratorGet(size_t index, int &type, String &key, String &value);
```



#### Get child/array elements from FirebaseJson objects at specified index.

param **`index`** The element index to get.   

return **` IteratorValue struct`** 

This should call after iteratorBegin.

The IteratorValue struct contains the following members:
int type
String key
String value

```cpp
IteratorValue valueAt(size_t index);
```



#### Clear all iterator buffer (should be called since iteratorBegin was called).

 ```cpp
 void iteratorEnd();
 ```



#### Set null to FirebaseJson object at the specified node path.
    
param **`path`** The relative path that null to be set.

The relative path can be mixed with array index (number placed inside square brackets) and node names e.g. /myRoot/[2]/Sensor1/myData/[3].

```cpp
void set(<string> path);
```



#### Set value to FirebaseJson object at the specified node path.
    
param **`path`** The relative path that string value to be set.

param **`value`** The value to set.



The relative path can be mixed with array index (number placed inside square brackets) and node names 
e.g. /myRoot/[2]/Sensor1/myData/[3].

The value that can be added is the following supported types e.g. flash string (PROGMEM and FPSTR), String, C/C++ std::string, const char*, char array, string literal, all integer and floating point numbers, boolean, FirebaseJson object and array.

```cpp
void set(<string> path, <type> value);
```



#### Remove the specified node and its content.

param **`path`** The relative path to remove its contents/children.

return **`bool`** value represents the successful operation.

```cpp
bool remove(<string> path);
```



#### Get raw JSON.

return **`raw JSON string`**

```cpp
<string> raw();
```



#### Get the error position at the JSON object literal from parsing.

return **`the position of error in JSON object literal`**

Return -1 when for no parsing error.

```cpp
int errorPosition();
```



#### Get the size of serialized JSON object buffer.

param **`prettify`** The text indentation and new line serialization option.

return **`size in byte of buffer`**

```cpp
size_t serializedBufferLength(bool prettify = false);
```



#### Set the precision for float to JSON object.

param **`digits`** The number of decimal places.

```cpp
void setFloatDigits(uint8_t digits);
```



#### Set the precision for double to JSON object.

param **`digits`** The number of decimal places.

```cpp
void setDoubleDigits(uint8_t digits);
```



#### Get http response code of reading JSON data from WiFi/Ethernet Client.

return **`the response code`** of reading JSON data from WiFi/Ethernet Client 

```cpp
int responseCode();
```



### FirebaseJsonArray object functions



#### Set or deserialize the JSON array data (JSON array literal) as FirebaseJsonArray object.

param **`data`** The JSON array literal string to set or deserialize.

return **`bool`** value represents the successful operation.

Call FirebaseJsonArray.errorPosition to get the error.

```cpp
bool setJsonArrayData(<string> data);
```



#### Add null to FirebaseJsonArray object.

return **`instance of an object.`**

```cpp
FirebaseJsonArray &add();
```



#### Add value to FirebaseJsonArray object.

param **`value`** The value to add.

return **`instance of an object.`**

The value that can be added is the following supported types e.g. flash string (PROGMEM and FPSTR), String, C/C++ std::string, const char*, char array, string literal, all integer and floating point numbers, boolean, FirebaseJson object and array.

```cpp
FirebaseJsonArray &add(<type> value);
```



#### Add multiple values to FirebaseJsonArray object.

param **`v`** The value of any type to add.

param **`n`** The consecutive values of any type to add.

return **`instance of an object.`**

e.g. add("a","b",1,2)

```cpp
FirebaseJsonArray &add(First v, Next... n);
```



#### Set JSON data (Client response) to FirebaseJsonArray object.
    
param **`client`** The pointer to or instance of Client object.

return **`instance of an object.`**

```cpp
bool readFrom(Client *client);

bool readFrom(Client &client);
```



#### Set JSON data (WiFiClient response) to FirebaseJsonArray object.
    
param **`client`** The pointer to or instance of WiFiClient object.

return **`instance of an object.`**

```cpp
bool readFrom(WiFiClient *client);

bool readFrom(WiFiClient &client);
```



#### Set JSON data (WiFiClientSecure response) to FirebaseJsonArray object.
    
param **`client`** The pointer to or instance of WiFiClientSecure object.

return **`instance of an object.`**

```cpp
bool readFrom(WiFiClientSecure *client);

bool readFrom(WiFiClientSecure &client);
```



#### Set JSON data (Seral object) to FirebaseJsonArray object.
    
param **`ser`** The HW or SW Serial object.

param **`timeoutMS`** The timeout in millisecond to wait for Serial data to be completed.

return **`instance of an object.`**

```cpp
bool readFrom(HardwareSerial &ser, uint32_t timeoutMS = 5000);

bool readFrom(SoftwareSerial &ser, uint32_t timeoutMS = 5000);
```



#### Set JSON data (File object) to FirebaseJsonArray object.
    
param **`file`** The File object.

return **`instance of an object.`**

```cpp
bool readFrom(fs::File &file);
```



#### Get the array value at the specified index or path from the FirebaseJsonArray object.

param **`result`** The reference of FirebaseJsonData object that holds data at the specified index.

param **`index_or_path`** Index of data or relative path to data in FirebaseJsonArray object.    

return **`boolean`** status of the operation.

The relative path must begin with array index (number placed inside square brackets) followed by
other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2

```cpp
bool get(FirebaseJsonData &result, <int or string> index_or_path);
```



#### Search element by key or path in FirebaseJsonArray object.

param **`result`** The reference of FirebaseJsonData that holds the result.

param **`criteria`** The FirebaseJson::SearchCriteria data.

param **`prettify`** The text indentation and new line serialization option.

return **`number`** of elements found from search.

The SearchCriteria data has the properties e.g.

**`path`** - The key of path to search.

Path can be wildcard with * in search path and * should use as key in part and do not mix with any character.

**`value`** - The value string to search.

**`depth`** - The begin depth (int) of element to search, default is 0.

**`endDepth`** - The end depth (int) of element to search, default is -1.

**`searchAll`** - The boolean option to search all occurrences of elements.

```cpp
size_t search(SearchCriteria &criteria);

size_t search(FirebaseJsonData &result, SearchCriteria &criteria, bool prettify = false);
```



#### Get the full path to any element in FirebaseJsonArray.

param **`path`** The key or path to search in to.

param **`searchAll`** Search all occurrences.

return **`full path string`** in case of found.

```cpp
String getPath(<string> path, bool searchAll = false);
```



#### Check whether key or path to the child element existed in FirebaseJsonArray or not.

param **`path`** The key or path of child element check.

return **`boolean`** status indicated the existence of element.

```cpp
bool isMember(<string> path);
```



#### Parse and collect all node/array elements in FirebaseJsonArray object.

return **`number`** of child/array elements in FirebaseJsonArray object.

```cpp
size_t iteratorBegin();
```



#### Get child/array elements from FirebaseJsonArray objects at specified index.
    
param **`index`** The element index to get.

param **`type`** The integer which holds the type of data i.e. FirebaseJson::OBJECT and FirebaseJson::ARRAY

param **`key`** The string which holds the key/name of the object, can return empty String if the data type is an array.

param **`value`** The string which holds the value for the element key or array.   

```cpp
void iteratorGet(size_t index, int &type, String &key, String &value);
```



#### Get child/array elements from FirebaseJsonArray objects at specified index.

param **`index`** The element index to get.   

return **` IteratorValue struct`** 

This should call after iteratorBegin.

The IteratorValue struct contains the following members:
int type
String key
String value

```cpp
IteratorValue valueAt(size_t index);
```



#### Clear all iterator buffer (should be called since iteratorBegin was called).

```cpp
void iteratorEnd();
```



#### Get the length of array in FirebaseJsonArray object.  

return **`length of the array.`**

```cpp
size_t size();
```



#### Get the FirebaseJsonArray object serialized string.

param **`out`** The writable object e.g. String, std::string, char array, Stream e.g ile, WiFi/Ethernet Client and LWMQTT, that accepts the returning string.

param **`prettify`** Boolean flag for return the pretty format string i.e. with text indentation and newline. 

```cpp
void toString(<type> out, bool prettify = false);
```



#### Get raw JSON Array.

return **`raw JSON Array string`**

```cpp
<string> raw();
```



#### Get the size of serialized JSON array buffer.

param **`prettify`** The text indentation and new line serialization option.

return **`size in byte of buffer`**

```cpp
size_t serializedBufferLength(bool prettify = false);
```



#### Clear all array in FirebaseJsonArray object.

return **`instance of an object.`**

```cpp
FirebaseJsonArray &clear();
```



#### Set null to FirebaseJsonArray object at at specified index or path.
    
param **`index_or_path`** The array index or path that null to be set.

```cpp
void set(<int or string> index_or_path);
```



####  Set String to FirebaseJsonArray object at the specified index.
    
param **`index_or_path`** The array index or path that value to be set.

param **`value`** The value to set.


```cpp
void set(<int or string> index_or_path, <type> value);
```



#### Remove the array value at the specified index or path from the FirebaseJsonArray object.

param **`index_or_path`** The array index or relative path to array to be removed.

return **`bool`** value represents the successful operation.

```cpp
bool remove(<int or string> index_or_path);
```



#### Get the error position at the JSON array literal from parsing.

return **`the position of error in JSON array literal`**

Return -1 when for no parsing error.

```cpp
int errorPosition();
```



#### Set the precision for float to JSON Array object.

param **`digits`** The number of decimal places.

```cpp
void setFloatDigits(uint8_t digits);
```



#### Set the precision for double to JSON Array object.

param **`digits`** The number of decimal places.

```cpp
void setDoubleDigits(uint8_t digits);
```



### FirebaseJsonData object functions


#### Get array data as FirebaseJsonArray object from FirebaseJsonData object.
    
param **`jsonArray`** The returning FirebaseJsonArray object.

return **`bool`** status for successful operation.

This should call after pares or get functions.

```cpp
bool getArray(FirebaseJsonArray &jsonArray);
```



#### Get array data as FirebaseJsonArray object from FirebaseJsonData object.

param **`source`** The JSON array string.
    
param **`jsonArray`** The returning FirebaseJsonArray object.

return **`bool`** status for successful operation.

This should call after pares or get functions.

```cpp
bool getArray(<string> source, FirebaseJsonArray &jsonArray);
```



#### Get array data as FirebaseJson object from FirebaseJsonData object.
    
param **`jsonArray`** The returning FirebaseJson object.

return **`bool`** status for successful operation.

This should call after pares or get functions.

```cpp
bool getJSON(FirebaseJson &json);
```



#### Get JSON data as FirebaseJson object from string.

param **`source`** The JSON string.
    
param **`json`** The returning FirebaseJsonArray object.

return **`bool`** status for successful operation.

This should call after pares or get functions.

```cpp
bool getJSON(<string> source, FirebaseJson &json);
```



#### Cast the FirebaseJsonData object to object or primitive type variable.

return the **`The object or primitive type variable`**.

```cpp
to<type>();

e.g. to<String>(), to<int>(), to<bool>()
```



### FirebaseJsonData object properties

**`value`** The String value of parses data.

**`value`** The int value of parses data.

**`value`** The double value of parses data.

**`value`** The float value of parses data.

**`boolVlue`** The bool value of parses data.

**`success`** used to determine the result of the deserialize operation.

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



## License

The MIT License (MIT)

Copyright (c) 2024 K. Suwatchai (Mobizt)


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

