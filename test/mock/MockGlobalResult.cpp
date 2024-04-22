/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "MockGlobalResult.h"

// MockJsAppImpl
bool g_getOrientation = false;
bool g_getColorMode = false;
bool g_memoryRefresh = false;
bool g_getJSONTree = false;
bool g_getDefaultJSONTree = false;
bool g_loadDocument = false;
bool g_reloadRuntimePage = false;
bool g_restart = false;

// MockKeyInputImpl
bool g_dispatchOsInputMethodEvent = false;
bool g_dispatchOsKeyEvent = false;

// MockMouseInputImpl
bool g_dispatchOsTouchEvent = false;
bool g_dispatchOsBackEvent = false;

// MockVirtualMessageImpl
bool g_sendVirtualMessage = false;

// MockLocalSocket
bool g_input = false;
bool g_output = false;
bool g_disconnectFromServer = false;

// MockWebSocketServer
bool g_run = false;

// MockVirtualScreen
bool g_getCurrentWidth = false;
bool g_getCurrentHeight = false;
bool g_getCurrentRouter = false;
bool g_getAbilityCurrentRouter = false;
bool g_getFastPreviewMsg = false;
bool g_getFoldStatus = false;