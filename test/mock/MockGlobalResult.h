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
extern bool g_dispatchBackPressedEvent;
extern bool g_dispatchPointerEvent;
extern bool g_dispatchAxisEvent;
extern bool g_dispatchInputMethodEvent;
extern bool g_dispatchKeyEvent;

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
extern bool g_writeData;

// MockVirtualScreen
extern bool g_getCurrentWidth;
extern bool g_getCurrentHeight;
extern bool g_getCurrentRouter;
extern bool g_getAbilityCurrentRouter;
extern bool g_getFastPreviewMsg;
extern bool g_getFoldStatus;

// MockAceAbility
extern bool g_setMockModuleList;
extern bool g_initEnv;
extern bool g_onBackPressed;
extern bool g_onInputEvent;
extern bool g_onInputMethodEvent;
extern bool g_onConfigurationChanged;
extern bool g_surfaceChanged;
extern bool g_replacePage;
extern bool g_loadAceDocument;
extern bool g_getAceJSONTree;
extern bool g_operateComponent;
extern bool g_setWindow;

// MockWindowDisplay
extern bool g_execStatusChangedCallback;

// MockGlfwRenderContext
extern bool g_glfwInit;
extern bool g_createGlfwWindow;
extern bool g_destroyWindow;
extern bool g_terminate;
extern bool g_pollEvents;

// MockWindow
extern bool g_setContentInfoCallback;
extern bool g_createSurfaceNode;

// MockSimulator
extern bool g_terminateAbility;
extern bool g_updateConfiguration;
extern bool g_setAbilityMockList;
extern bool g_setHostResolveBufferTracker;


#endif // GLOBAL_VARIABLES_H