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
bool g_dispatchBackPressedEvent = false;
bool g_dispatchPointerEvent = false;
bool g_dispatchAxisEvent = false;
bool g_dispatchInputMethodEvent = false;
bool g_dispatchKeyEvent = false;

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
bool g_writeData = false;

// MockVirtualScreen
bool g_getCurrentWidth = false;
bool g_getCurrentHeight = false;
bool g_getCurrentRouter = false;
bool g_getAbilityCurrentRouter = false;
bool g_getFastPreviewMsg = false;
bool g_getFoldStatus = false;

// MockAceAbility
bool g_setMockModuleList = false;
bool g_initEnv = false;
bool g_onBackPressed = false;
bool g_onInputEvent = false;
bool g_onInputMethodEvent = false;
bool g_onConfigurationChanged = false;
bool g_surfaceChanged = false;
bool g_replacePage = false;
bool g_loadAceDocument = false;
bool g_getAceJSONTree = false;
bool g_operateComponent = false;
bool g_setWindow = false;

// MockWindowDisplay
bool g_execStatusChangedCallback = false;

// MockGlfwRenderContext
bool g_glfwInit = false;
bool g_createGlfwWindow = false;
bool g_destroyWindow = false;
bool g_terminate = false;
bool g_pollEvents = false;

// MockWindow
bool g_setContentInfoCallback = false;
bool g_createSurfaceNode = false;
bool g_getSystemBarPropertyByType = false;
bool g_updateAvoidArea = false;

// MockSimulator
bool g_terminateAbility = false;
bool g_updateConfiguration = false;
bool g_setAbilityMockList = false;
bool g_setHostResolveBufferTracker = false;

// MockJsAbility
bool g_jsAbilityLaunch = false;
bool g_jsAbilityShow = false;
bool g_jsAbilityHide = false;
bool g_jsAbilityTransferToDestroy = false;

// MockUIFont
bool g_uiFontSetFont = false;
bool g_uiFontSetPsramMemory = false;

// MockProductAdapter
bool g_setDefaultFontStyle = false;

// MockDebugger
bool g_configEngineDebugger = false;

// MockGraphicStartUp
bool g_graphicStartUpInit = false;

// MockInputDeviceManager
bool g_inputDeviceManagerAdd = false;

// MockTaskManager
bool g_taskHandler = false;

// MockGlobal
bool g_globalConfigLanguage = false;