# FireSense for Firebase ESP Client.


<br/>

# The Programmable Data Logging and IO Control library v1.0.13


This library supports ESP8266 and ESP32 MCU from Espressif. 


## Functions and Usages

<br/>

#### Initiate the FireSense Class.

param **`config`** The pointer to Firesense_Config data.

param **`databaseSecret`** The database secret.

return **`Boolean`** value, indicates the success of the operation.

note: Due to query index need to be assign to the database rules, the admin rights access is needed.

The database secret can be empty string if the sign-in sign-in method is OAuth2.0.

```cpp
bool begin(struct firesense_config_t *config, const char *databaseSecret);
```

<br/>

#### Load the device configurations.

param **`defaultDataLoadCallback`** The callback function that called when no config found in the database.

note: The callback function should add the channals or conditions manually or load from the device storage.

```cpp
void loadConfig(callback_function_t defaultDataLoadCallback);
```

<br/>

#### Save the current config to the device storage.

param **`filename`** The file path includes its name of file that will be saved.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.

return **`Boolean`** value, indicates the success of the operation.

```cpp
bool backupConfig(const String &filename, fb_esp_mem_storage_type storageType);
```

<br/>

#### Read the config from the device storage.

param **`filename`** The file path includes its name of file that will be read.

param **`storageType`** The enum of memory storage type e.g. mem_storage_type_flash and mem_storage_type_sd. The file systems can be changed in FirebaseFS.h.

return **`Boolean`** value, indicates the success of the operation.

```cpp
bool restoreConfig(const String &filename, fb_esp_mem_storage_type storageType);
```

<br/>

#### Enable (run) or disable (stop) the conditions checking tasks.

param **`enable`** The boolean value to enable/disable.

```cpp
void enableController(bool enable);
```

<br/>

#### Add a channel to device config.

param **`channel`** The FireSense_Channel data to add.

param **`addToDatabase`** The boolean option, set to true to add the data to database.

```cpp
void addChannel(struct channel_info_t &channel, bool addToDatabase = true);
```

<br/>

#### Add a condition to device config.

param **`cond`** The FireSense_Condition data to add.

param **`addToDatabase`** The boolean option, set to true to add the data to database.


<br/>

The format of conditions (IF) and its expression.

CONDITION1 + && or || LOGICAL OPERATOR + CONDITION2 + LOGICAL OPERATOR + CONDITION3 +...

The condition checking and expression evaluation are from left to right

The valid left, right operands and syntaxes are

| Operand and Syntaxes  | Usages |
| ------------- | ------------- |
| \<channel id\>  | LED1 == false && STATUS == LED1  |
| values e.g. boolean, integer and float  | HUMID1 > 70 \|\| LAMP1 == false  |
| millis | millis > 200000 + VALUE1 |
| micros | VALUE1 < micros - 1000000 |
| time e.g. hour:min:sec | time > 12:00:00 && time < 15:30:00 |
| date e.g. month/day/year where month start with 0 | date == 5/28/2021 |
| weekday e.g. 1 for monday and 7 for sunday | weekday == 5 |
| day e.g. 1 to 31 | day > 24 |
| month e.g. 0 to 11 | month < 11 |
| year e.g. 2021 | year == 2021 |
| hour e.g. 0 to 23 | hour == 18 |
| min  e.g. 0 to 59 | min == 30 |
| sec  e.g. 0 to 59 | sec == 20 |
| change e.g the value of channel changed | change(VALUE1) |
| ! e.g. the opposite of expresion result | !LED1 \|\| !(time > 15:20:06) |
| year e.g. 2021 | year == 2021 |


<br/>

The format of statements (THEN and ELSE) and its expression.

STATEMENT1 + COMMA + STATEMENT2 +...

The statement processing and expression evaluation are from left to right.

The valid left, right operands and syntaxes are

| Operand and Syntaxes  | Usages |
| ------------- | ------------- |
| \<channel id\>  | LED1 = false, STATUS = 5 * 10  |
| values e.g. boolean, integer and float  | HUMID1 = 70  |
| millis | VALUE1 += millis |
| micros | VALUE1 \*= micros |
| delay | delay(1000), LED1 = true |
|  | ;do non-blocking delay until timed out and set LED1 to true |
| func  e.g. func(x,y,z) | func(0,10,'hello world') |
| where x is the index of callback function added with FireSense.addCallbackFunction | ;send the hello world text 10 times to function with index 0 |
| y is the number of iteration that function will be called as long as the conditions is true |  |
| z is the message payload sent to the callback. |  |
| The content of payload other than alphabet (A-Z, a-z and 1-9) should be in ''. |  |
| Use {CHANNEL_ID} to insert the channel value into the text payload. |  |


<br/>

The supported assignment operators are
+=, -=, *=, /=, &=, |=


The supported comparision operators are
==, !=, >, <, >=, <=


```cpp
void addCondition(struct firesense_condition_t cond, bool addToDatabase = true);
```

<br/>

#### Add a callback function used with func syntax in the conditions.

param **`func`** The FireSense_Function callback.

```cpp
void addCallbackFunction(FireSense_Function func);
```

<br/>

#### Clear all callback functions used with func syntax in the conditions.

```cpp
void clearAllCallbackFunctions();
```

<br/>

#### Add a pointer of uint8_t (byte) variable that bind to the channels.

param **`value`** The uint8_t variable.

```cpp
void addUserValue(uint8_t *value);
```

<br/>

#### Add a pointer of bool variable that bind to the channels.

param **`value`** The bool variable.

```cpp
void addUserValue(bool *value);
```

<br/>

#### Add a pointer of int variable that bind to the channels.

param **`value`** The int variable.

```cpp
void addUserValue(int *value);
```

<br/>

#### Add a pointer of float variable that bind to the channels.

param **`value`** The float variable.

```cpp
void addUserValue(float *value);
```

<br/>

#### Clear all user variable pointers that bound to the channels.

```cpp
void clearAllUserValues();
```

<br/>

#### Get the devivce id string.

return **`String`** The unique id String of device.

```cpp
 String getDeviceId();
```

<br/><br/>

## License

The MIT License (MIT)

Copyright (c) 2023 K. Suwatchai (Mobizt)


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

