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

#ifndef GLOBAL_VARIABLES_H
#define GLOBAL_VARIABLES_H

// MockJsAppImpl
extern bool g_getOrientation;
extern bool g_getColorMode;
extern bool g_memoryRefresh;
extern bool g_getJSONTree;
extern bool g_getDefaultJSONTree;
extern bool g_loadDocument;
extern bool g_reloadRuntimePage;
extern bool g_restart;

// MockKeyInputImpl
extern bool g_dispatchOsInputMethodEvent;
extern bool g_dispatchOsKeyEvent;

// MockMouseInputImpl
extern bool g_dispatchOsTouchEvent;
extern bool g_dispatchOsBackEvent;

// MockVirtualMessageImpl
extern bool g_sendVirtualMessage;

// MockLocalSocket
extern bool g_input;
extern bool g_output;
extern bool g_disconnectFromServer;

// MockWebSocketServer
extern bool g_run;

// MockVirtualScreen
extern bool g_getCurrentWidth;
extern bool g_getCurrentHeight;
extern bool g_getCurrentRouter;
extern bool g_getAbilityCurrentRouter;
extern bool g_getFastPreviewMsg;
extern bool g_getFoldStatus;

#endif // GLOBAL_VARIABLES_H