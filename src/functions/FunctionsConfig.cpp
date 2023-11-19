
/**
 * Google's Cloud Functions Config class, FunctionsConfig.cpp version 1.0.10
 *
 * Created April 5, 2023
 *
 * The MIT License (MIT)
 * Copyright (c) 2023 K. Suwatchai (Mobizt)
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

#include "./FirebaseFS.h"

#if defined(ENABLE_FB_FUNCTIONS) || defined(FIREBASE_ENABLE_FB_FUNCTIONS)

#ifndef _FB_FUNCTIONS_CONFIG_CPP_
#define _FB_FUNCTIONS_CONFIG_CPP_

#include "FunctionsConfig.h"

FunctionsConfig::~FunctionsConfig()
{
    _projectId.clear();
    _locationId.clear();
    _bucketId.clear();
    clear();
}

void FunctionsConfig::mSetProjectId(MB_StringPtr projectId)
{
    _projectId = projectId;
}
void FunctionsConfig::mSetLocationId(MB_StringPtr locationId)
{
    _locationId = locationId;
}
void FunctionsConfig::mSetBucketId(MB_StringPtr bucketId)
{
    _bucketId = bucketId;
}

void FunctionsConfig::addUpdateMasks(const char *key)
{
    for (size_t i = 0; i < _updateMask.size(); i++)
    {
        if (strcmp(_updateMask[i].c_str(), key) == 0)
            return;
    }
    MB_String k = key;
    _updateMask.push_back(k);
}

void FunctionsConfig::mFunctionsConfig(MB_StringPtr projectId, MB_StringPtr locationId, MB_StringPtr bucketId)
{
    _projectId = projectId;
    _locationId = locationId;
    _bucketId = bucketId;
    _triggerType = firebase_functions_trigger_type_https;
    _updateMask.clear();
}

void FunctionsConfig::removeUpdateMasks(const char *key)
{
    for (size_t i = 0; i < _updateMask.size(); i++)
    {
        if (strcmp(_updateMask[i].c_str(), key) == 0)
        {
            _updateMask.erase(_updateMask.begin() + i);
            return;
        }
    }
}

void FunctionsConfig::mSetName(MB_StringPtr name)
{
    MB_String t = firebase_func_pgm_str_47; // "projects/"
    t += _projectId;
    t += firebase_func_pgm_str_27; // "/locations/"
    t += _locationId;
    t += firebase_func_pgm_str_28; // "/functions"
    t += firebase_pgm_str_1;   // "/"
    t += name;
    _funcCfg.set(pgm2Str(firebase_pgm_str_66 /* "name" */), t.c_str());
    _name = name;
}

void FunctionsConfig::mSetDescription(MB_StringPtr description)
{
    MB_String _description = description;
    _funcCfg.set(pgm2Str(firebase_func_pgm_str_48 /* "description" */), _description);
    addUpdateMasks(pgm2Str(firebase_func_pgm_str_48 /* "description" */));
}

void FunctionsConfig::mSetEntryPoint(MB_StringPtr entry)
{
    MB_String _entry = entry;
    _funcCfg.set(pgm2Str(firebase_func_pgm_str_49 /* "entryPoint" */), _entry);
    addUpdateMasks(pgm2Str(firebase_func_pgm_str_49 /* "entryPoint" */));
    _entryPoint = entry;
}

void FunctionsConfig::mSetRuntime(MB_StringPtr runtime)
{
    MB_String _runtime = runtime;
    _funcCfg.set(pgm2Str(firebase_func_pgm_str_50 /* "runtime" */), _runtime);
    addUpdateMasks(pgm2Str(firebase_func_pgm_str_50 /* "runtime" */));
}

void FunctionsConfig::mSetTimeout(MB_StringPtr seconds)
{
    MB_String s = seconds;
    s += firebase_func_pgm_str_51; // "s"
    _funcCfg.set(pgm2Str(firebase_func_pgm_str_52 /* "timeout" */), s.c_str());
    addUpdateMasks(pgm2Str(firebase_func_pgm_str_52 /* "timeout" */));
}

void FunctionsConfig::mSetAvailableMemoryMb(MB_StringPtr mb)
{
    MB_String m = mb;
    int _mb = atoi(m.c_str());

    if (_mb > 4096)
        _mb = 4096;
    else if (_mb > 2048 && _mb <= 4096)
        _mb = 4096;
    else if (_mb > 1024 && _mb <= 2048)
        _mb = 2048;
    else if (_mb > 512 && _mb <= 1024)
        _mb = 1024;
    else if (_mb > 256 && _mb <= 512)
        _mb = 512;
    else if (_mb > 128 && _mb <= 256)
        _mb = 256;
    else
        _mb = 128;

    _funcCfg.set(pgm2Str(firebase_func_pgm_str_53 /* "availableMemoryMb" */), _mb);
    addUpdateMasks(pgm2Str(firebase_func_pgm_str_53 /* "availableMemoryMb" */));
}

void FunctionsConfig::mSetMaxInstances(MB_StringPtr maxInstances)
{
    _funcCfg.set(pgm2Str(firebase_func_pgm_str_54 /* "maxInstances"*/), atoi(stringPtr2Str(maxInstances)));
    addUpdateMasks(pgm2Str(firebase_func_pgm_str_54 /* "maxInstances" */));
}

void FunctionsConfig::mSetSource(MB_StringPtr path, firebase_functions_sources_type sourceType, firebase_mem_storage_type storageType)
{
    MB_String _path = path;
    MB_String t;
    switch (sourceType)
    {
    case functions_sources_type_storage_bucket_archive:

        Core.uh.addGStorageURL(t, _bucketId, path);
        _funcCfg.set(pgm2Str(firebase_func_pgm_str_58 /* "sourceArchiveUrl" */), t.c_str());
        addUpdateMasks(pgm2Str(firebase_func_pgm_str_58 /* "sourceArchiveUrl" */));

        _sourceType = sourceType;
        break;

    case functions_sources_type_storage_bucket_sources:
        _bucketSourcesPath = path;
        _sourceType = sourceType;
        break;

    case functions_sources_type_local_archive:

        _uploadArchiveFile = path;
        _uploadArchiveStorageType = storageType;
        _sourceType = sourceType;
        break;

    case functions_sources_type_repository:
        _funcCfg.set(pgm2Str(firebase_func_pgm_str_59 /* "sourceRepository" */), _path);
        addUpdateMasks(pgm2Str(firebase_func_pgm_str_59 /* "sourceRepository" */));
        _sourceType = sourceType;
        break;

    default:
        break;
    }
}
void FunctionsConfig::setSource(const uint8_t *pgmArchiveData, size_t len)
{
    _pgmArc = pgmArchiveData;
    _pgmArcLen = len;
    _sourceType = functions_sources_type_flash_data;
}

void FunctionsConfig::mSetIngressSettings(MB_StringPtr settings)
{
    MB_String _settings = settings;
    _funcCfg.set(pgm2Str(firebase_func_pgm_str_57 /* "ingressSettings" */), _settings);
    addUpdateMasks(pgm2Str(firebase_func_pgm_str_57 /* "ingressSettings" */));
}

void FunctionsConfig::mAddLabel(MB_StringPtr key, MB_StringPtr value)
{
    MB_String _value = value;
    MB_String t = firebase_pgm_str_64; // "labels"
    addUpdateMasks(t.c_str());
    t += firebase_pgm_str_1; // "/"
    t += key;
    _funcCfg.set(t.c_str(), _value);
}
void FunctionsConfig::clearLabels()
{
    _funcCfg.remove(pgm2Str(firebase_pgm_str_64 /* "labels" */));
    removeUpdateMasks(pgm2Str(firebase_pgm_str_64 /* labels */));
}
void FunctionsConfig::mAddEnvironmentVariable(MB_StringPtr key, MB_StringPtr value)
{
    MB_String str = firebase_func_pgm_str_12; // "environmentVariables"
    addUpdateMasks(str.c_str());
    str += firebase_pgm_str_1; // "/"
    str += key;
    _funcCfg.set(str.c_str(), stringPtr2Str(value));
}
void FunctionsConfig::clearEnvironmentVariables()
{
    _funcCfg.remove(pgm2Str(firebase_func_pgm_str_12 /* "environmentVariables" */));
    removeUpdateMasks(pgm2Str(firebase_func_pgm_str_12 /* "environmentVariables" */));
}
void FunctionsConfig::mAddBuildEnvironmentVariable(MB_StringPtr key, MB_StringPtr value)
{
    MB_String _value = value;
    MB_String str = firebase_func_pgm_str_60; // "buildEnvironmentVariables"
    addUpdateMasks(str.c_str());
    str += firebase_pgm_str_1; // "/"
    str += key;
    _funcCfg.set(str.c_str(), stringPtr2Str(value));
}

void FunctionsConfig::clearBuildEnvironmentVariables()
{
    _funcCfg.remove(pgm2Str(firebase_func_pgm_str_60 /* "buildEnvironmentVariables" */));
    removeUpdateMasks(pgm2Str(firebase_func_pgm_str_60 /* "buildEnvironmentVariables" */));
}
void FunctionsConfig::mSetNetwork(MB_StringPtr network)
{
    MB_String _network = network;
    _funcCfg.set(pgm2Str(firebase_func_pgm_str_61 /* "network" */), _network);
    addUpdateMasks(pgm2Str(firebase_func_pgm_str_61 /* "network" */));
}
void FunctionsConfig::mSetVpcConnector(MB_StringPtr vpcConnector)
{

    MB_String _vpcConnector = vpcConnector;
    _funcCfg.set(pgm2Str(firebase_func_pgm_str_55 /* "vpcConnector" */), _vpcConnector);
    addUpdateMasks(pgm2Str(firebase_func_pgm_str_55 /* "vpcConnector" */));
}
void FunctionsConfig::mSetVpcConnectorEgressSettings(MB_StringPtr e)
{

    MB_String _e = e;
    _funcCfg.set(pgm2Str(firebase_func_pgm_str_56 /* "vpcConnectorEgressSettings" */), _e);
    addUpdateMasks(pgm2Str(firebase_func_pgm_str_56 /* "vpcConnectorEgressSettings" */));
}

void FunctionsConfig::mSetEventTrigger(MB_StringPtr eventType, MB_StringPtr resource,
                                       MB_StringPtr service, MB_StringPtr failurePolicy)
{

    MB_String _eventType = eventType, _resource = resource, _service = service, _failurePolicy = failurePolicy;
    _triggerType = firebase_functions_trigger_type_event;

    if (_eventType.length() > 0)
        _funcCfg.set(pgm2Str(firebase_func_pgm_str_62 /* "eventTrigger/eventType" */), _eventType);

    if (_resource.length() > 0)
        _funcCfg.set(pgm2Str(firebase_func_pgm_str_63 /* "eventTrigger/resource" */), _resource);

    if (_service.length() > 0)
        _funcCfg.set(pgm2Str(firebase_func_pgm_str_64 /* "eventTrigger/service" */), _service);

    if (_failurePolicy.length() > 0)
    {

        static FirebaseJson js(firebase_func_pgm_str_66 /* "{\"retry\":{}}" */);
        _funcCfg.set(pgm2Str(firebase_func_pgm_str_65 /* "eventTrigger/failurePolicy" */), js);
    }
    addUpdateMasks(pgm2Str(firebase_func_pgm_str_67 /* "eventTrigger" */));
}

void FunctionsConfig::setIamPolicy(PolicyBuilder *policy)
{
    _policy = policy;
    addUpdateMasks(pgm2Str(firebase_func_pgm_str_68 /* "policy" */));
}

String FunctionsConfig::getTriggerUrl()
{
    return _httpsTriggerUrl.c_str();
}

void FunctionsConfig::clear()
{
    _updateMask.clear();
    _funcCfg.clear();
    _entryPoint.clear();
    _name.clear();
    _httpsTriggerUrl.clear();
    _bucketSourcesPath.clear();
    _uploadArchiveFile.clear();
    _pgmArc = nullptr;
    _pgmArcLen = 0;
    _uploadArchiveStorageType = mem_storage_type_undefined;
    _sourceType = functions_sources_type_undefined;
    _triggerType = firebase_functions_trigger_type_undefined;
    _policy = nullptr;
}

#endif

#endif // ENABLE