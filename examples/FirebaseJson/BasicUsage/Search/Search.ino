/**
 * Created by K. Suwatchai (Mobizt)
 * 
 * Email: k_suwatchai@hotmail.com
 * 
 * Github: https://github.com/mobizt/FirebaseJson
 * 
 * Copyright (c) 2022 mobizt
 *
*/

#include <Arduino.h>
#include <FirebaseJson.h>

void setup()
{

    Serial.begin(115200);
    Serial.println();
    Serial.println();

    //Search the element in JSON object

    FirebaseJson json;
    FirebaseJsonData result;

    json.set("destination_addresses/[0]", "Washington, DC, USA");
    json.set("destination_addresses/[1]", "Philadelphia, PA, USA");
    json.set("destination_addresses/[2]", "Santa Barbara, CA, USA");
    json.set("destination_addresses/[3]", "Miami, FL, USA");
    json.set("destination_addresses/[4]", "Austin, TX, USA");
    json.set("destination_addresses/[5]", "Napa County, CA, USA");

    json.set("origin_addresses/[0]", "New York, NY, USA");

    json.set("rows/[0]/elements/[0]/distance/text", "27 mi");
    json.set("rows/[0]/elements/[0]/distance/value", 365468);
    json.set("rows/[0]/elements/[1]/duration/text", "3 hours 54 mins");
    json.set("rows/[0]/elements/[1]/duration/value", 14064);
    json.set("rows/[0]/elements/[2]/status", "OK");

    json.set("rows/[1]/elements/[0]/distance/text", "94.6 mi");
    json.set("rows/[1]/elements/[0]/distance/value", 152193);
    json.set("rows/[1]/elements/[1]/duration/text", "1 hour 44 mins");
    json.set("rows/[1]/elements/[1]/duration/value", 6227);
    json.set("rows/[1]/elements/[2]/status", "OK");

    json.set("rows/[2]/elements/[0]/distance/text", "2,878 mi");
    json.set("rows/[2]/elements/[0]/distance/value", 4632197);
    json.set("rows/[2]/elements/[1]/duration/text", "1 day 18 hours");
    json.set("rows/[2]/elements/[1]/duration/value", 151772);
    json.set("rows/[2]/elements/[2]/status", "OK");

    json.set("rows/[3]/elements/[0]/distance/text", "1,286 mi");
    json.set("rows/[3]/elements/[0]/distance/value", 2069031);
    json.set("rows/[3]/elements/[1]/duration/text", "18 hours 43 mins");
    json.set("rows/[3]/elements/[1]/duration/value", 67405);
    json.set("rows/[3]/elements/[2]/status", "OK");

    json.set("rows/[4]/elements/[0]/distance/text", "1,742 mi");
    json.set("rows/[4]/elements/[0]/distance/value", 2802972);
    json.set("rows/[4]/elements/[1]/duration/text", "1 day 2 hours");
    json.set("rows/[4]/elements/[1]/duration/value", 93070);
    json.set("rows/[4]/elements/[2]/status", "OK");

    json.set("rows/[5]/elements/[0]/distance/text", "2,871 mi");
    json.set("rows/[5]/elements/[0]/distance/value", 4620514);
    json.set("rows/[5]/elements/[1]/duration/text", "1 day 18 hours");
    json.set("rows/[5]/elements/[1]/duration/value", 152913);
    json.set("rows/[5]/elements/[2]/status", "OK");

    json.set("status", "OK");

    json.toString(Serial, true);

    //Assign the search criteria
    FirebaseJson::SearchCriteria criteria;

    criteria.path = "*/[3]/*/text"; /* use * as wildcard in path */
    //criteria.path = "rows/[3]/elements/[0]/distance/text"; /* without wildcard in path */
    //criteria.value = "1,286 mi";
    //criteria.depth = 0; /* begin dept to search, default is 0 */
    //criteria.endDepth = 10; /* end dept to search, default is -1 */
    criteria.searchAll = true; /* search all occurrences of elements */

    int count = json.search(result /*result */, criteria, true /*prettify option */);
    /* To search without result */
    //int count = json.search(criteria);

    Serial.println("\n\n===============================");
    Serial.println("Searching for " + criteria.path + "...\n");
    Serial.print("Search result: ");
    if (count > 0)
    {
        Serial.printf(" found %d item(s)", count);
        if (result.success)
        {
            Serial.println("\n" + result.to<String>());
            Serial.println("\nPath: " + result.searchPath);

            if (criteria.searchAll) //with search all, result will be an array
            {
                //To get array from search result
                FirebaseJsonArray arr;
                result.getArray(arr);

                //parse or print the array here
                count = arr.iteratorBegin();
                Serial.println("\nIterate Search result item...");
                Serial.println("-------------------------------");
                for (size_t i = 0; i < arr.size(); i++)
                {
                    arr.get(result, i);
                    if (result.typeNum == FirebaseJson::JSON_STRING)
                        Serial.printf("Array index %d, String Val: %s\n", i, result.to<String>().c_str());
                    else if (result.typeNum == FirebaseJson::JSON_INT)
                        Serial.printf("Array index %d, Int Val: %d\n", i, result.to<int>());
                    else if (result.typeNum == FirebaseJson::JSON_FLOAT)
                        Serial.printf("Array index %d, Float Val: %f\n", i, result.to<float>());
                    else if (result.typeNum == FirebaseJson::JSON_DOUBLE)
                        Serial.printf("Array index %d, Double Val: %f\n", i, result.to<double>());
                    else if (result.typeNum == FirebaseJson::JSON_BOOL)
                        Serial.printf("Array index %d, Bool Val: %d\n", i, result.to<bool>());
                    else if (result.typeNum == FirebaseJson::JSON_OBJECT)
                        Serial.printf("Array index %d, Object Val: %s\n", i, result.to<String>().c_str());
                    else if (result.typeNum == FirebaseJson::JSON_ARRAY)
                        Serial.printf("Array index %d, Array Val: %s\n", i, result.to<String>().c_str());
                    else if (result.typeNum == FirebaseJson::JSON_NULL)
                        Serial.printf("Array index %d, Null Val: %s\n", i, result.to<String>().c_str());
                }
            }
        }
    }
    else
        Serial.println(" not found");

    //Get the fullpath for element
    String path = json.getPath("*/[4]/*/value" /* key or path with wildcard */, false /* get all occurrences */);
    Serial.println("\n\n===============================");
    Serial.print("Get full path for */[4]/*/value, ");
    Serial.println(path);

    //check for a member in json
    Serial.println("\n\n===============================");
    Serial.print("Is \"rows/[3]/elements/[0]/distance/text\" the json member? ");
    if (json.isMember("rows/[3]/elements/[0]/distance/text" /* key or path without wildcard */))
        Serial.println("yes");
    else
        Serial.println("no");
}

void loop()
{
}