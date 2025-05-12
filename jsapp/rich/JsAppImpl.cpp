/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "JsAppImpl.h"

#include "CommandParser.h"
#include "FileSystem.h"
#include "JsonReader.h"
#include "PreviewerEngineLog.h"
#include "SharedData.h"
#include "TraceTool.h"
#include "VirtualScreenImpl.h"
#include "external/EventHandler.h"
#include "external/StageContext.h"
#include "viewport_config.h"
#include "glfw_render_context.h"
#if defined(REPLACE_WINDOW_HEADER)
#include "window.h"
#endif
#include "window_model.h"
#include "window_display.h"
#include "ace_preview_helper.h"
#include "ClipboardHelper.h"
#include "CommandLineInterface.h"
#include "ui_content_impl.h"
#if defined(__APPLE__) || defined(_WIN32)
#include "options.h"
#include "simulator.h"
#endif
#include <fstream>

using namespace OHOS;
using namespace OHOS::Ace;

namespace {
ScreenInfo screenInfo;
CommandInfo commandInfo;
}

class PreviewerListener : public OHOS::Rosen::IWindowSystemBarEnableListener {
public:
    OHOS::Rosen::WMError OnSetSpecificBarProperty(OHOS::Rosen::WindowType type,
        const OHOS::Rosen::SystemBarProperty& property)
    {
        JsAppImpl::GetInstance().CalculateAvoidAreaByType(type, property);
        return OHOS::Rosen::WMError::WM_OK;
    }
};

namespace {
OHOS::sptr<PreviewerListener> listener = nullptr;
}

JsAppImpl::JsAppImpl() noexcept : ability(nullptr), isStop(false)
{
#if defined(__APPLE__) || defined(_WIN32)
    windowModel = std::make_shared<OHOS::Previewer::PreviewerWindowModel>();
#endif
}

JsAppImpl::~JsAppImpl() {}

JsAppImpl& JsAppImpl::GetInstance()
{
    static JsAppImpl instance;
    return instance;
}

void JsAppImpl::Start()
{
    VirtualScreenImpl::GetInstance().InitVirtualScreen();
    VirtualScreenImpl::GetInstance().InitAll(pipeName, pipePort);
    isFinished = false;
    ILOG("Start run js app");
    OHOS::AppExecFwk::EventHandler::SetMainThreadId(std::this_thread::get_id());
    RunJsApp();
    ILOG("Js app run finished");
    while (!isStop) {
        // Execute all tasks in the main thread
        OHOS::AppExecFwk::EventHandler::Run();
        glfwRenderContext->PollEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    StopAbility(); // start and stop ability at the same thread
    ILOG("JsAppImpl::Interrupt finished");
    isFinished = true;
}

void JsAppImpl::Restart()
{
    StopAbility();
}

std::string JsAppImpl::GetJSONTree()
{
    std::string jsonTree = "";
    if (isDebug && debugServerPort >= 0) {
        auto uiContent = GetWindow()->GetUIContent();
        jsonTree = uiContent->GetJSONTree();
    } else {
        jsonTree = ability->GetJSONTree();
    }
    return jsonTree;
}

std::string JsAppImpl::GetDefaultJSONTree()
{
    ILOG("Start getDefaultJsontree.");
    std::string jsonTree = ability->GetDefaultJSONTree();
    ILOG("GetDefaultJsontree finished.");
    return jsonTree;
}

void JsAppImpl::OrientationChanged(std::string commandOrientation)
{
    aceRunArgs.deviceWidth = height;
    aceRunArgs.deviceHeight = width;
    VirtualScreenImpl::GetInstance().WidthAndHeightReverse();
    AdaptDeviceType(aceRunArgs, commandInfo.deviceType, aceRunArgs.deviceWidth);
    if (commandOrientation == "portrait") {
        aceRunArgs.deviceConfig.orientation = DeviceOrientation::PORTRAIT;
    } else {
        aceRunArgs.deviceConfig.orientation = DeviceOrientation::LANDSCAPE;
    }

    orientation = commandOrientation;
    ILOG("OrientationChanged: %s %d %d %f", orientation.c_str(), aceRunArgs.deviceWidth,
         aceRunArgs.deviceHeight, aceRunArgs.deviceConfig.density);
    if (ability != nullptr) {
        OHOS::AppExecFwk::EventHandler::PostTask([this]() {
            glfwRenderContext->SetWindowSize(width, height);
        });
        ability->SurfaceChanged(aceRunArgs.deviceConfig.orientation, aceRunArgs.deviceConfig.density,
                                aceRunArgs.deviceWidth, aceRunArgs.deviceHeight);
    }
}

void JsAppImpl::ColorModeChanged(const std::string commandColorMode)
{
    if (commandColorMode == "light") {
        aceRunArgs.deviceConfig.colorMode = ColorMode::LIGHT;
    } else {
        aceRunArgs.deviceConfig.colorMode = ColorMode::DARK;
    }

    if (ability != nullptr) {
        ability->OnConfigurationChanged(aceRunArgs.deviceConfig);
    }
}

void JsAppImpl::Interrupt()
{
    isStop = true;
}

void JsAppImpl::SetJsAppArgs(OHOS::Ace::Platform::AceRunArgs& args)
{
    ILOG("foldStatus:%s foldWidth:%d foldHeight:%d",
        screenInfo.foldStatus.c_str(), screenInfo.foldWidth, screenInfo.foldHeight);
    SetAssetPath(args, jsAppPath);
    SetProjectModel(args);
    SetPageProfile(args, commandInfo.pages);
    SetDeviceWidth(args, ConvertFoldStatus(screenInfo.foldStatus) ==
        OHOS::Rosen::FoldStatus::FOLDED ? screenInfo.foldWidth : screenInfo.orignalResolutionWidth);
    SetDeviceHeight(args, ConvertFoldStatus(screenInfo.foldStatus) ==
        OHOS::Rosen::FoldStatus::FOLDED ? screenInfo.foldHeight : screenInfo.orignalResolutionHeight);
    SetWindowTitle(args, "Ace");
    SetUrl(args, urlPath);
    SetConfigChanges(args, configChanges);
    SetColorMode(args, colorMode);
    SetOrientation(args, orientation);
    SetAceVersionArgs(args, aceVersion);
    SetDeviceScreenDensity(atoi(screenDensity.c_str()), commandInfo.deviceType);
    SetLanguage(args, SharedData<std::string>::GetData(SharedDataType::LAN));
    SetRegion(args, SharedData<std::string>::GetData(SharedDataType::REGION));
    SetScript(args, "");
    SetSystemResourcesPath(args);
    SetAppResourcesPath(args, commandInfo.appResourcePath);
    SetFormsEnabled(args, commandInfo.isCardDisplay);
    SetContainerSdkPath(args, commandInfo.containerSdkPath);
    AdaptDeviceType(args, commandInfo.deviceType, ConvertFoldStatus(screenInfo.foldStatus) ==
        OHOS::Rosen::FoldStatus::FOLDED ? screenInfo.foldWidth : screenInfo.orignalResolutionWidth);
    SetOnRouterChange(args);
    SetOnError(args);
    SetComponentModeEnabled(args, commandInfo.isComponentMode);
    SetPkgContextInfo();
#ifdef COMPONENT_TEST_ENABLED
    SetComponentTestConfig(args, componentTestModeConfig);
#endif // COMPONENT_TEST_ENABLED
    ILOG("start ability: %d %d %f", args.deviceWidth, args.deviceHeight, args.deviceConfig.density);
}

void JsAppImpl::RunJsApp()
{
    ILOG("RunJsApp 1");
    InitScreenInfo();
    AssignValueForWidthAndHeight(screenInfo.orignalResolutionWidth, screenInfo.orignalResolutionHeight,
                                 screenInfo.compressionResolutionWidth, screenInfo.compressionResolutionHeight);
    SetJsAppArgs(aceRunArgs);
    OHOS::Ide::StageContext::GetInstance().SetLoaderJsonPath(commandInfo.loaderJsonPath);
    OHOS::Ide::StageContext::GetInstance().SetHosSdkPath(commandInfo.containerSdkPath);
    OHOS::Ide::StageContext::GetInstance().GetModulePathMapFromLoaderJson();
    OHOS::Previewer::PreviewerDisplay::GetInstance().SetFoldable(screenInfo.foldable);
    OHOS::Previewer::PreviewerDisplay::GetInstance().SetFoldStatus(ConvertFoldStatus(screenInfo.foldStatus));
    InitGlfwEnv();
    Platform::AcePreviewHelper::GetInstance()->SetCallbackOfPostTask(AppExecFwk::EventHandler::PostTask);
    Platform::AcePreviewHelper::GetInstance()->
        SetCallbackOfIsCurrentRunnerThread(AppExecFwk::EventHandler::IsCurrentRunnerThread);
    Platform::AcePreviewHelper::GetInstance()->SetCallbackOfSetClipboardData(ClipboardHelper::SetClipboardData);
    Platform::AcePreviewHelper::GetInstance()->SetCallbackOfGetClipboardData(ClipboardHelper::GetClipboardData);
    listener = new(std::nothrow) PreviewerListener();
    if (!listener) {
        ELOG("Memory allocation failed: listener.");
        return;
    }
    if (isDebug && debugServerPort >= 0) {
        RunDebugAbility(); // for debug preview
    } else {
        RunNormalAbility(); // for normal preview
    }
}

void JsAppImpl::RunNormalAbility()
{
    Platform::AcePreviewHelper::GetInstance()->SetCallbackOfHspBufferTracker(
        [](const std::string& inputPath, uint8_t** buff, size_t* buffSize, std::string &errorMsg) -> bool {
            if (!buff || !buffSize || inputPath.empty()) {
                return false;
            }
            auto data = OHOS::Ide::StageContext::GetInstance().GetModuleBuffer(inputPath);
            if (!data) {
                return false;
            }
            *buff = data->data();
            *buffSize = data->size();
            return true;
        });
    if (ability != nullptr) {
        ability.reset();
    }
    TraceTool::GetInstance().HandleTrace("Launch Js App");
    ability = Platform::AceAbility::CreateInstance(aceRunArgs);
    if (ability == nullptr) {
        ELOG("JsApp::Run ability create failed.");
        return;
    }
    SetMockJsonInfo();
    OHOS::Rosen::WMError errCode;
    OHOS::sptr<OHOS::Rosen::WindowOption> sp = nullptr;
    auto window = OHOS::Rosen::Window::Create("previewer", sp, nullptr, errCode);
    window->RegisterSystemBarEnableListener(sptr<OHOS::Rosen::IWindowSystemBarEnableListener>(listener));
    window->SetContentInfoCallback(std::move(VirtualScreenImpl::LoadContentCallback));
    window->CreateSurfaceNode("preview_surface", std::move(VirtualScreenImpl::Callback));
    ability->SetWindow(window);
    InitAvoidAreas(window);
    ability->InitEnv();
}

#if defined(__APPLE__) || defined(_WIN32)
void JsAppImpl::RunDebugAbility()
{
    // init window params
    SetWindowParams();
    OHOS::Previewer::PreviewerWindow::GetInstance().SetWindowParams(*windowModel);
    // start ability
    OHOS::AbilityRuntime::Options options;
    SetSimulatorParams(options);
    simulator = OHOS::AbilityRuntime::Simulator::Create(options);
    if (!simulator) {
        ELOG("JsApp::Run simulator create failed.");
        return;
    }
    simulator->SetHostResolveBufferTracker(
        [](const std::string &inputPath, uint8_t **buff, size_t *buffSize, std::string &errorMsg) -> bool {
            if (inputPath.empty() || buff == nullptr || buffSize == nullptr) {
                ELOG("Param invalid.");
                return false;
            }

            DLOG("Get module buffer, input path: %{public}s.", inputPath.c_str());
            auto data = Ide::StageContext::GetInstance().GetModuleBuffer(inputPath);
            if (data == nullptr) {
                ELOG("Get module buffer failed, input path: %{public}s.", inputPath.c_str());
                return false;
            }

            *buff = data->data();
            *buffSize = data->size();
            return true;
        });
    SetMockJsonInfo();
    std::string abilitySrcPath = commandInfo.abilityPath;
    std::string abilityName = commandInfo.abilityName;
    debugAbilityId = simulator->StartAbility(abilitySrcPath, [](int64_t abilityId) {}, abilityName);
    if (debugAbilityId < 0) {
        ELOG("JsApp::Run ability start failed. abilitySrcPath:%s abilityName:%s", abilitySrcPath.c_str(),
            abilityName.c_str());
        return;
    }
    // set onRender callback
    OHOS::Rosen::Window* window = OHOS::Previewer::PreviewerWindow::GetInstance().GetWindowObject();
    if (!window) {
        ELOG("JsApp::Run get window failed.");
        return;
    }
    window->RegisterSystemBarEnableListener(sptr<OHOS::Rosen::IWindowSystemBarEnableListener>(listener));
    window->SetContentInfoCallback(std::move(VirtualScreenImpl::LoadContentCallback));
    window->CreateSurfaceNode(options.moduleName, std::move(VirtualScreenImpl::Callback));
    InitAvoidAreas(window);
}

void JsAppImpl::SetSimulatorParams(OHOS::AbilityRuntime::Options& options)
{
    const std::string path = commandInfo.appResourcePath + FileSystem::GetSeparator() + "module.json";
    if (!FileSystem::IsFileExists(path)) {
        ELOG("The module.json file is not exist.");
        return;
    }
    std::optional<std::vector<uint8_t>> ctx = OHOS::Ide::StageContext::GetInstance().ReadFileContents(path);
    if (ctx.has_value()) {
        options.moduleJsonBuffer = ctx.value();
    } else {
        ELOG("get module.json content failed");
    }
    SetSimulatorCommonParams(options);
    ILOG("setted bundleName:%s moduleName:%s", options.modulePath.c_str(), options.resourcePath.c_str());
}

void JsAppImpl::SetSimulatorCommonParams(OHOS::AbilityRuntime::Options& options)
{
    options.modulePath = aceRunArgs.assetPath + FileSystem::GetSeparator() + "modules.abc";
    options.resourcePath = commandInfo.appResourcePath + FileSystem::GetSeparator() + "resources.index";
    if (debugServerPort > 0) {
        options.debugPort = debugServerPort;
    }
    options.assetPath = aceRunArgs.assetPath;
    options.systemResourcePath = aceRunArgs.systemResourcesPath;
    options.appResourcePath = aceRunArgs.appResourcesPath;
    options.containerSdkPath = aceRunArgs.containerSdkPath;
    options.url = aceRunArgs.url;
    options.language = aceRunArgs.language;
    options.region = aceRunArgs.region;
    options.script = aceRunArgs.script;
    options.themeId = aceRunArgs.themeId;
    options.deviceWidth = aceRunArgs.deviceWidth;
    options.deviceHeight = aceRunArgs.deviceHeight;
    options.isRound = aceRunArgs.isRound;
    options.onRouterChange = aceRunArgs.onRouterChange;
    options.pkgContextInfoJsonStringMap = aceRunArgs.pkgContextInfoJsonStringMap;
    options.packageNameList = aceRunArgs.packageNameList;
    OHOS::AbilityRuntime::DeviceConfig deviceCfg;
    deviceCfg.deviceType = SetDevice<OHOS::AbilityRuntime::DeviceType>(aceRunArgs.deviceConfig.deviceType);
    deviceCfg.orientation = SetOrientation<OHOS::AbilityRuntime::DeviceOrientation>(
        aceRunArgs.deviceConfig.orientation);
    deviceCfg.colorMode = SetColorMode<OHOS::AbilityRuntime::ColorMode>(aceRunArgs.deviceConfig.colorMode);
    deviceCfg.density = aceRunArgs.deviceConfig.density;
    options.deviceConfig = deviceCfg;
    std::string fPath = commandInfo.configPath;
    options.configuration = UpdateConfiguration(aceRunArgs);
    std::size_t pos = fPath.find(".idea");
    if (pos == std::string::npos) {
        ELOG("previewPath error:%s", fPath.c_str());
    } else {
        options.previewPath = fPath.substr(0, pos) + ".idea" + FileSystem::GetSeparator() + "previewer";
        ILOG("previewPath info:%s", options.previewPath.c_str());
    }
    options.postTask = AppExecFwk::EventHandler::PostTask;
}

std::shared_ptr<AppExecFwk::Configuration> JsAppImpl::UpdateConfiguration(OHOS::Ace::Platform::AceRunArgs& args)
{
    std::shared_ptr<AppExecFwk::Configuration> configuration = std::make_shared<AppExecFwk::Configuration>();
    configuration->AddItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE,
        SharedData<std::string>::GetData(SharedDataType::LANGUAGE));
    std::string colorMode = "light";
    if (aceRunArgs.deviceConfig.colorMode == ColorMode::DARK) {
        colorMode = "dark";
    }
    configuration->AddItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, colorMode);
    std::string direction = "portrait";
    if (aceRunArgs.deviceConfig.orientation == DeviceOrientation::LANDSCAPE) {
        orientation = "landscape";
    }
    configuration->AddItem(OHOS::AppExecFwk::ConfigurationInner::APPLICATION_DIRECTION, direction);
    std::string density = std::to_string(aceRunArgs.deviceConfig.density);
    configuration->AddItem(OHOS::AppExecFwk::ConfigurationInner::APPLICATION_DENSITYDPI, density);
    return configuration;
}


void JsAppImpl::SetWindowParams() const
{
    windowModel->isRound = aceRunArgs.isRound;
    windowModel->originWidth = aceRunArgs.deviceWidth;
    windowModel->originHeight = aceRunArgs.deviceHeight;
    windowModel->compressWidth = aceRunArgs.deviceWidth;
    windowModel->compressHeight = aceRunArgs.deviceHeight;
    windowModel->density = aceRunArgs.deviceConfig.density;
    windowModel->deviceType = SetDevice<OHOS::Previewer::DeviceType>(aceRunArgs.deviceConfig.deviceType);
    windowModel->orientation = SetOrientation<OHOS::Previewer::Orientation>(aceRunArgs.deviceConfig.orientation);
    windowModel->colorMode = SetColorMode<OHOS::Previewer::ColorMode>(aceRunArgs.deviceConfig.colorMode);
}
#else
    void JsAppImpl::RunDebugAbility()
    {
        ELOG("JsApp::Run ability start failed.Linux is not supported.");
        return;
    }
#endif

void JsAppImpl::AdaptDeviceType(Platform::AceRunArgs& args, const std::string type,
                                const int32_t realDeviceWidth, double screenDendity) const
{
    if (type == "wearable") {
        args.deviceConfig.deviceType = DeviceType::WATCH;
        double density = screenDendity > 0 ? screenDendity : watchScreenDensity;
        double adaptWidthWatch = realDeviceWidth * BASE_SCREEN_DENSITY / density;
        args.deviceConfig.density = args.deviceWidth / adaptWidthWatch;
        return;
    }
    if (type == "tv") {
        args.deviceConfig.deviceType = DeviceType::TV;
        double density = screenDendity > 0 ? screenDendity : tvScreenDensity;
        double adaptWidthTv = realDeviceWidth * BASE_SCREEN_DENSITY / density;
        args.deviceConfig.density = args.deviceWidth / adaptWidthTv;
        return;
    }
    if (type == "phone" || type == "default") {
        args.deviceConfig.deviceType = DeviceType::PHONE;
        double density = screenDendity > 0 ? screenDendity : phoneScreenDensity;
        double adaptWidthPhone = realDeviceWidth * BASE_SCREEN_DENSITY / density;
        args.deviceConfig.density = args.deviceWidth / adaptWidthPhone;
        return;
    }
    if (type == "2in1") {
        args.deviceConfig.deviceType = DeviceType::TWO_IN_ONE;
        double density = screenDendity > 0 ? screenDendity : twoInOneScreenDensity;
        double adaptWidthPhone = realDeviceWidth * BASE_SCREEN_DENSITY / density;
        args.deviceConfig.density = args.deviceWidth / adaptWidthPhone;
        return;
    }
    if (type == "tablet") {
        args.deviceConfig.deviceType = DeviceType::TABLET;
        double density = screenDendity > 0 ? screenDendity : tabletScreenDensity;
        double adaptWidthTablet = realDeviceWidth * BASE_SCREEN_DENSITY / density;
        args.deviceConfig.density = args.deviceWidth / adaptWidthTablet;
        return;
    }
    if (type == "car") {
        args.deviceConfig.deviceType = DeviceType::CAR;
        double density = screenDendity > 0 ? screenDendity : carScreenDensity;
        double adaptWidthCar = realDeviceWidth * BASE_SCREEN_DENSITY / density;
        args.deviceConfig.density = args.deviceWidth / adaptWidthCar;
        return;
    }
    ELOG("DeviceType not supported : %s", type.c_str());
    return;
}

void JsAppImpl::SetAssetPath(Platform::AceRunArgs& args, const std::string assetPath) const
{
    args.assetPath = assetPath;
}

void JsAppImpl::SetProjectModel(Platform::AceRunArgs& args) const
{
    int idxVal = CommandParser::GetInstance().GetProjectModelEnumValue();
    ILOG("ProjectModel: %s", CommandParser::GetInstance().GetProjectModelEnumName(idxVal).c_str());
    args.projectModel = Platform::ProjectModel(idxVal);
}

void JsAppImpl::SetPageProfile(Platform::AceRunArgs& args, const std::string pageProfile) const
{
    args.pageProfile = pageProfile;
}

void JsAppImpl::SetDeviceWidth(Platform::AceRunArgs& args, const int32_t deviceWidth) const
{
    args.deviceWidth = deviceWidth;
}

void JsAppImpl::SetDeviceHeight(Platform::AceRunArgs& args, const int32_t deviceHeight) const
{
    args.deviceHeight = deviceHeight;
}

void JsAppImpl::SetWindowTitle(Platform::AceRunArgs& args, const std::string windowTitle) const
{
    args.windowTitle = windowTitle;
}

void JsAppImpl::SetUrl(Platform::AceRunArgs& args, const std::string urlPath) const
{
    args.url = urlPath;
}

void JsAppImpl::SetConfigChanges(Platform::AceRunArgs& args, const std::string configChanges) const
{
    args.configChanges = configChanges;
}

void JsAppImpl::SetColorMode(Platform::AceRunArgs& args, const std::string colorMode) const
{
    ILOG("JsAppImpl::RunJsApp SetColorMode: %s", colorMode.c_str());
    if (colorMode == "dark") {
        args.deviceConfig.colorMode = ColorMode::DARK;
    } else {
        args.deviceConfig.colorMode = ColorMode::LIGHT;
    }
}

void JsAppImpl::SetOrientation(Platform::AceRunArgs& args, const std::string orientation) const
{
    ILOG("JsAppImpl::RunJsApp SetOrientation: %s", orientation.c_str());
    if (orientation == "landscape") {
        args.deviceConfig.orientation = DeviceOrientation::LANDSCAPE;
    } else {
        args.deviceConfig.orientation = DeviceOrientation::PORTRAIT;
    }
}

void JsAppImpl::SetAceVersionArgs(Platform::AceRunArgs& args, const std::string aceVersion) const
{
    ILOG("JsAppImpl::RunJsApp SetAceVersionArgs: %s", aceVersion.c_str());
    if (aceVersion == "ACE_2_0") {
        args.aceVersion = Platform::AceVersion::ACE_2_0;
    } else {
        args.aceVersion = Platform::AceVersion::ACE_1_0;
    }
}

void JsAppImpl::SetLanguage(Platform::AceRunArgs& args, const std::string language) const
{
    args.language = language;
}

void JsAppImpl::SetRegion(Platform::AceRunArgs& args, const std::string region) const
{
    args.region = region;
}

void JsAppImpl::SetScript(Platform::AceRunArgs& args, const std::string script) const
{
    args.script = script;
}

void JsAppImpl::SetSystemResourcesPath(Platform::AceRunArgs& args) const
{
    std::string sep = FileSystem::GetSeparator();
    std::string rPath = FileSystem::GetApplicationPath();
    rPath = FileSystem::NormalizePath(rPath);
    int idx = rPath.find_last_of(sep);
    rPath = rPath.substr(0, idx + 1) + "resources";
    args.systemResourcesPath = rPath;
}

void JsAppImpl::SetAppResourcesPath(Platform::AceRunArgs& args, const std::string appResourcesPath) const
{
    args.appResourcesPath = appResourcesPath;
}

void JsAppImpl::SetFormsEnabled(Platform::AceRunArgs& args, bool formsEnabled) const
{
    args.formsEnabled = formsEnabled;
}

void JsAppImpl::SetContainerSdkPath(Platform::AceRunArgs& args, const std::string containerSdkPath) const
{
    args.containerSdkPath = containerSdkPath;
}

void JsAppImpl::SetOnRouterChange(Platform::AceRunArgs& args) const
{
    args.onRouterChange = std::move(VirtualScreenImpl::PageCallback);
}

void JsAppImpl::SetOnError(Platform::AceRunArgs& args) const
{
    args.onError = std::move(VirtualScreenImpl::FastPreviewCallback);
}

void JsAppImpl::SetComponentModeEnabled(Platform::AceRunArgs& args, bool isComponentMode) const
{
    args.isComponentMode = isComponentMode;
}

void JsAppImpl::AssignValueForWidthAndHeight(const int32_t origWidth,
                                             const int32_t origHeight,
                                             const int32_t compWidth,
                                             const int32_t compHeight)
{
    orignalWidth = origWidth;
    orignalHeight = origHeight;
    width = compWidth;
    height = compHeight;
    ILOG("AssignValueForWidthAndHeight: %d %d %d %d", orignalWidth, orignalHeight, width, height);
}

void JsAppImpl::ResolutionChanged(ResolutionParam& param, int32_t screenDensity, std::string reason)
{
    SetResolutionParams(param.orignalWidth, param.orignalHeight, param.compressionWidth,
        param.compressionHeight, screenDensity);
    if (isDebug && debugServerPort >= 0) {
#if defined(__APPLE__) || defined(_WIN32)
        SetWindowParams();
        OHOS::Ace::ViewportConfig config;
        config.SetSize(windowModel->originWidth, windowModel->originHeight);
        config.SetPosition(0, 0);
        config.SetOrientation(static_cast<int32_t>(
            OHOS::Previewer::PreviewerWindow::TransOrientation(windowModel->orientation)));
        config.SetDensity(windowModel->density);
        OHOS::Rosen::Window* window = OHOS::Previewer::PreviewerWindow::GetInstance().GetWindowObject();
        if (!window) {
            ELOG("JsApp::Run get window failed.");
            return;
        }
        InitAvoidAreas(window);
        OHOS::AppExecFwk::EventHandler::PostTask([this]() {
            glfwRenderContext->SetWindowSize(aceRunArgs.deviceWidth, aceRunArgs.deviceHeight);
        });
        simulator->UpdateConfiguration(*(UpdateConfiguration(aceRunArgs).get()));
        window->SetViewportConfig(config);
#endif
    } else {
        if (ability != nullptr) {
            InitAvoidAreas(ability->GetWindow());
            OHOS::AppExecFwk::EventHandler::PostTask([this]() {
                glfwRenderContext->SetWindowSize(aceRunArgs.deviceWidth, aceRunArgs.deviceHeight);
            });
            ability->SurfaceChanged(aceRunArgs.deviceConfig.orientation, aceRunArgs.deviceConfig.density,
                aceRunArgs.deviceWidth, aceRunArgs.deviceHeight, ConvertResizeReason(reason));
        }
    }
}

WindowSizeChangeReason JsAppImpl::ConvertResizeReason(std::string reason)
{
    if (reason == "undefined") {
        return WindowSizeChangeReason::UNDEFINED;
    } else if (reason == "rotation") {
        return WindowSizeChangeReason::ROTATION;
    } else {
        return WindowSizeChangeReason::RESIZE;
    }
}

void JsAppImpl::SetResolutionParams(int32_t changedOriginWidth, int32_t changedOriginHeight, int32_t changedWidth,
    int32_t changedHeight, int32_t screenDensity)
{
    SetDeviceWidth(aceRunArgs, changedWidth);
    SetDeviceHeight(aceRunArgs, changedHeight);
    orignalWidth = changedOriginWidth;
    orignalHeight = changedOriginHeight;
    SetDeviceScreenDensity(screenDensity, commandInfo.deviceType);
    AdaptDeviceType(aceRunArgs, commandInfo.deviceType, changedOriginWidth);
    AssignValueForWidthAndHeight(changedOriginWidth, changedOriginHeight, changedWidth, changedHeight);
    if (changedWidth <= changedHeight) {
        JsAppImpl::GetInstance().SetDeviceOrentation("portrait");
    } else {
        JsAppImpl::GetInstance().SetDeviceOrentation("landscape");
    }
    SetOrientation(aceRunArgs, orientation);
    VirtualScreenImpl::GetInstance().SetCurrentResolution(aceRunArgs.deviceWidth, aceRunArgs.deviceHeight);
    ILOG("ResolutionChanged: %s %d %d %f", orientation.c_str(), aceRunArgs.deviceWidth,
         aceRunArgs.deviceHeight, aceRunArgs.deviceConfig.density);
}

void JsAppImpl::SetArgsColorMode(const std::string& value)
{
    colorMode = value;
}

void JsAppImpl::SetArgsAceVersion(const std::string& value)
{
    aceVersion = value;
}

void JsAppImpl::SetDeviceOrentation(const std::string& value)
{
    orientation = value;
}

std::string JsAppImpl::GetOrientation() const
{
    return orientation;
}

std::string JsAppImpl::GetColorMode() const
{
    return colorMode;
}

void JsAppImpl::SetDeviceScreenDensity(const int32_t screenDensity, const std::string type)
{
    if (type == "wearable" && screenDensity != 0) {
        watchScreenDensity = screenDensity;
        return;
    }
    if (type == "tv" && screenDensity != 0) {
        tvScreenDensity = screenDensity;
        return;
    }
    if ((type == "phone" || type == "default") && screenDensity != 0) {
        phoneScreenDensity = screenDensity;
        return;
    }
    if (type == "2in1" && screenDensity != 0) {
        twoInOneScreenDensity = screenDensity;
        return;
    }
    if (type == "tablet" && screenDensity != 0) {
        tabletScreenDensity = screenDensity;
        return;
    }
    if (type == "car" && screenDensity != 0) {
        carScreenDensity = screenDensity;
        return;
    }
    ILOG("DeviceType not supported to SetDeviceScreenDensity: %s", type.c_str());
    return;
}

void JsAppImpl::ReloadRuntimePage(const std::string currentPage)
{
    std::string params = "";
    if (ability != nullptr) {
        ability->ReplacePage(currentPage, params);
    }
}

void JsAppImpl::SetScreenDensity(const std::string value)
{
    screenDensity = value;
}

void JsAppImpl::SetConfigChanges(const std::string value)
{
    configChanges = value;
}

bool JsAppImpl::MemoryRefresh(const std::string memoryRefreshArgs) const
{
    ILOG("MemoryRefresh.");
    if (ability != nullptr) {
        return ability->OperateComponent(memoryRefreshArgs);
    } else {
        auto uiContent = GetWindow()->GetUIContent();
        return uiContent->OperateComponent(memoryRefreshArgs);
    }
    return false;
}

void JsAppImpl::ParseSystemParams(OHOS::Ace::Platform::AceRunArgs& args, const Json2::Value& paramObj)
{
    if (paramObj.IsNull()) {
        SetDeviceWidth(args, VirtualScreenImpl::GetInstance().GetCompressionWidth());
        SetDeviceHeight(args, VirtualScreenImpl::GetInstance().GetCompressionHeight());
        AssignValueForWidthAndHeight(args.deviceWidth, args.deviceHeight,
                                     args.deviceWidth, args.deviceHeight);
        SetColorMode(args, colorMode);
        SetOrientation(args, orientation);
        SetDeviceScreenDensity(atoi(screenDensity.c_str()), commandInfo.deviceType);
        AdaptDeviceType(args, commandInfo.deviceType, aceRunArgs.deviceWidth);
        SetLanguage(args, SharedData<std::string>::GetData(SharedDataType::LAN));
        SetRegion(args, SharedData<std::string>::GetData(SharedDataType::REGION));
    } else {
        SetDeviceWidth(args, paramObj["width"].AsInt());
        SetDeviceHeight(args, paramObj["height"].AsInt());
        AssignValueForWidthAndHeight(args.deviceWidth, args.deviceHeight,
                                     args.deviceWidth, args.deviceHeight);
        SetColorMode(args, paramObj["colorMode"].AsString());
        SetOrientation(args, paramObj["orientation"].AsString());
        std::string deviceType = paramObj["deviceType"].AsString();
        SetDeviceScreenDensity(atoi(screenDensity.c_str()), deviceType);
        AdaptDeviceType(args, deviceType, args.deviceWidth, paramObj["dpi"].AsDouble());
        std::string lanInfo = paramObj["locale"].AsString();
        SetLanguage(args, lanInfo.substr(0, lanInfo.find("_")));
        SetRegion(args, lanInfo.substr(lanInfo.find("_") + 1, lanInfo.length() - 1));
    }
}

void JsAppImpl::SetSystemParams(OHOS::Ace::Platform::SystemParams& params, const Json2::Value& paramObj)
{
    ParseSystemParams(aceRunArgs, paramObj);
    params.deviceWidth = aceRunArgs.deviceWidth;
    params.deviceHeight = aceRunArgs.deviceHeight;
    params.language = aceRunArgs.language;
    params.region = aceRunArgs.region;
    params.colorMode = aceRunArgs.deviceConfig.colorMode;
    params.orientation = aceRunArgs.deviceConfig.orientation;
    params.deviceType = aceRunArgs.deviceConfig.deviceType;
    params.density = aceRunArgs.deviceConfig.density;
    params.isRound = (paramObj.IsNull()) ? (commandInfo.screenShape == "circle") :
        paramObj["roundScreen"].AsBool();
}

void JsAppImpl::LoadDocument(const std::string filePath,
                             const std::string componentName,
                             const Json2::Value& previewContext)
{
    ILOG("LoadDocument.");
    OHOS::Ace::Platform::SystemParams params;
    SetSystemParams(params, previewContext);
    ILOG("LoadDocument params is density: %f region: %s language: %s deviceWidth: %d\
         deviceHeight: %d isRound:%d colorMode:%s orientation: %s deviceType: %s",
         params.density,
         params.region.c_str(),
         params.language.c_str(),
         params.deviceWidth,
         params.deviceHeight,
         (params.isRound ? "true" : "false"),
         ((params.colorMode == ColorMode::DARK) ? "dark" : "light"),
         ((params.orientation == DeviceOrientation::LANDSCAPE) ? "landscape" : "portrait"),
        GetDeviceTypeName(params.deviceType).c_str());
    OHOS::AppExecFwk::EventHandler::PostTask([this]() {
        glfwRenderContext->SetWindowSize(aceRunArgs.deviceWidth, aceRunArgs.deviceHeight);
    });

    if (ability != nullptr) {
        ability->LoadDocument(filePath, componentName, params);
    } else {
        auto uiContent = GetWindow()->GetUIContent();
        uiContent->LoadDocument(filePath, componentName, params);
    }
}

void JsAppImpl::DispatchBackPressedEvent() const
{
    ability->OnBackPressed();
}
void JsAppImpl::DispatchKeyEvent(const std::shared_ptr<OHOS::MMI::KeyEvent>& keyEvent) const
{
    if (isDebug && debugServerPort >= 0) {
#if defined(__APPLE__) || defined(_WIN32)
        OHOS::Rosen::Window* window = OHOS::Previewer::PreviewerWindow::GetInstance().GetWindowObject();
        if (!window) {
            ELOG("JsApp::Run get window failed.");
            return;
        }
        window->ConsumeKeyEvent(keyEvent);
#endif
    } else {
        ability->OnInputEvent(keyEvent);
    }
}
void JsAppImpl::DispatchPointerEvent(const std::shared_ptr<OHOS::MMI::PointerEvent>& pointerEvent) const
{
    if (isDebug && debugServerPort >= 0) {
#if defined(__APPLE__) || defined(_WIN32)
        OHOS::Rosen::Window* window = OHOS::Previewer::PreviewerWindow::GetInstance().GetWindowObject();
        if (!window) {
            ELOG("JsApp::Run get window failed.");
            return;
        }
        window->ConsumePointerEvent(pointerEvent);
#endif
    } else {
        ability->OnInputEvent(pointerEvent);
    }
}
void JsAppImpl::DispatchAxisEvent(const std::shared_ptr<OHOS::MMI::AxisEvent>& axisEvent) const
{
    ability->OnInputEvent(axisEvent);
}
void JsAppImpl::DispatchInputMethodEvent(const unsigned int codePoint) const
{
    ability->OnInputMethodEvent(codePoint);
}

std::string JsAppImpl::GetDeviceTypeName(const OHOS::Ace::DeviceType type) const
{
    switch (type) {
        case DeviceType::WATCH:
            return "watch";
        case DeviceType::TV:
            return "tv";
        case DeviceType::PHONE:
            return "phone";
        case DeviceType::TABLET:
            return "tablet";
        case DeviceType::CAR:
            return "car";
        default:
            return "";
    }
}

void JsAppImpl::InitGlfwEnv()
{
    ILOG("InitGlfwEnv started");
    glfwRenderContext = OHOS::Rosen::GlfwRenderContext::GetGlobal();
    if (!glfwRenderContext->Init()) {
        ELOG("Could not create window: InitGlfwEnv failed.");
        return;
    }
    glfwRenderContext->CreateGlfwWindow(aceRunArgs.deviceWidth, aceRunArgs.deviceHeight, false);
    ILOG("InitGlfwEnv finished");
}

void JsAppImpl::SetMockJsonInfo()
{
    std::string filePath = commandInfo.appResourcePath + FileSystem::GetSeparator() + "mock-config.json";
    if (isDebug && debugServerPort >= 0) {
#if defined(__APPLE__) || defined(_WIN32)
        simulator->SetMockList(Ide::StageContext::GetInstance().ParseMockJsonFile(filePath));
#endif
    } else {
        ability->SetMockModuleList(Ide::StageContext::GetInstance().ParseMockJsonFile(filePath));
    }
}

void JsAppImpl::SetPkgContextInfo()
{
    Ide::StageContext::GetInstance().SetPkgContextInfo(aceRunArgs.pkgContextInfoJsonStringMap,
        aceRunArgs.packageNameList);
}

void JsAppImpl::FoldStatusChanged(const std::string commandFoldStatus, int32_t currentWidth, int32_t currentHeight)
{
    std::string reason = "resize";
    ILOG("FoldStatusChanged commandFoldStatus:%s", commandFoldStatus.c_str());
    VirtualScreenImpl::GetInstance().SetFoldStatus(commandFoldStatus);
    OHOS::Rosen::FoldStatus status = ConvertFoldStatus(commandFoldStatus);
    // execute callback
    OHOS::Previewer::PreviewerDisplay::GetInstance().SetFoldStatus(status);
    OHOS::Previewer::PreviewerDisplay::GetInstance().ExecStatusChangedCallback();
    if (status == OHOS::Rosen::FoldStatus::UNKNOWN) {
        return; // unknown status do nothing
    }
    // change resolution
    ResolutionParam param(currentWidth, currentHeight, currentWidth, currentHeight);
    ResolutionChanged(param, atoi(screenDensity.c_str()), reason);
}

OHOS::Rosen::FoldStatus JsAppImpl::ConvertFoldStatus(std::string value) const
{
    OHOS::Rosen::FoldStatus foldStatus = OHOS::Rosen::FoldStatus::EXPAND;
    if (value == "fold") {
        foldStatus = OHOS::Rosen::FoldStatus::FOLDED;
    } else if (value == "unfold") {
        foldStatus = OHOS::Rosen::FoldStatus::EXPAND;
    } else if (value == "half_fold") {
        foldStatus = OHOS::Rosen::FoldStatus::HALF_FOLD;
    } else {
        foldStatus = OHOS::Rosen::FoldStatus::UNKNOWN;
    }
    return foldStatus;
}

void JsAppImpl::SetAvoidArea(const AvoidAreas& areas)
{
    avoidInitialAreas = areas;
}

void JsAppImpl::CalculateAvoidAreaByType(OHOS::Rosen::WindowType type,
    const OHOS::Rosen::SystemBarProperty& property)
{
    uint32_t deviceWidth = static_cast<uint32_t>(aceRunArgs.deviceWidth);
    uint32_t deviceHeight = static_cast<uint32_t>(aceRunArgs.deviceHeight);
    OHOS::Rosen::Window* window = GetWindow();
    if (!window) {
        ELOG("GetWindow failed");
        return;
    }
    sptr<OHOS::Rosen::AvoidArea> statusArea(new(std::nothrow) OHOS::Rosen::AvoidArea());
    if (!statusArea) {
        ELOG("new OHOS::Rosen::AvoidArea failed");
        return;
    }
    if (OHOS::Rosen::WindowType::WINDOW_TYPE_STATUS_BAR == type) {
        if (property.enable_) {
            statusArea->topRect_ = {0, 0, deviceWidth, avoidInitialAreas.topRect.height};
            window->UpdateAvoidArea(statusArea, OHOS::Rosen::AvoidAreaType::TYPE_SYSTEM);
        } else {
            statusArea->topRect_ = {0, 0, 0, 0};
            window->UpdateAvoidArea(statusArea, OHOS::Rosen::AvoidAreaType::TYPE_SYSTEM);
        }
        UpdateAvoidArea2Ide("topRect", statusArea->topRect_);
    } else if (OHOS::Rosen::WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR == type) {
        if (property.enable_) {
            statusArea->bottomRect_ = {0, deviceHeight - avoidInitialAreas.bottomRect.height,
                deviceWidth, avoidInitialAreas.bottomRect.height};
            window->UpdateAvoidArea(statusArea, OHOS::Rosen::AvoidAreaType::TYPE_NAVIGATION_INDICATOR);
        } else {
            statusArea->bottomRect_ = {0, 0, 0, 0};
            window->UpdateAvoidArea(statusArea, OHOS::Rosen::AvoidAreaType::TYPE_NAVIGATION_INDICATOR);
        }
        UpdateAvoidArea2Ide("bottomRect", statusArea->bottomRect_);
    } else {
        return; // currently not support
    }
}

void JsAppImpl::UpdateAvoidArea2Ide(const std::string& key, const OHOS::Rosen::Rect& value)
{
    Json2::Value son = JsonReader::CreateObject();
    son.Add("posX", value.posX_);
    son.Add("posY", value.posY_);
    son.Add("width", value.width_);
    son.Add("height", value.height_);
    Json2::Value val = JsonReader::CreateObject();
    val.Add(key.c_str(), son);
    CommandLineInterface::GetInstance().CreatCommandToSendData("AvoidAreaChanged", val, "get");
}

OHOS::Rosen::Window* JsAppImpl::GetWindow() const
{
    if (isDebug && debugServerPort >= 0) {
#if defined(__APPLE__) || defined(_WIN32)
        return OHOS::Previewer::PreviewerWindow::GetInstance().GetWindowObject();
#else
        return nullptr;
#endif
    } else {
        return ability->GetWindow();
    }
}

void JsAppImpl::InitAvoidAreas(OHOS::Rosen::Window* window)
{
    CalculateAvoidAreaByType(OHOS::Rosen::WindowType::WINDOW_TYPE_STATUS_BAR,
        window->GetSystemBarPropertyByType(OHOS::Rosen::WindowType::WINDOW_TYPE_STATUS_BAR));
    CalculateAvoidAreaByType(OHOS::Rosen::WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR,
        window->GetSystemBarPropertyByType(OHOS::Rosen::WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR));
}

void JsAppImpl::InitJsApp()
{
    CommandParser& parser = CommandParser::GetInstance();
    InitCommandInfo();
    SetJsAppPath(parser.Value("j"));
    if (parser.IsSet("s")) {
        SetPipeName(parser.Value("s"));
    }
    if (parser.IsSet("url")) {
        SetUrlPath(parser.Value("url"));
    }
    if (parser.IsSet("lws")) {
        SetPipePort(parser.Value("lws"));
    }
    if (parser.IsSet("cm")) {
        SetArgsColorMode(parser.Value("cm"));
    }
    if (parser.IsSet("av")) {
        SetArgsAceVersion(parser.Value("av"));
    }
    if (parser.IsSet("sd")) {
        SetScreenDensity(parser.Value("sd"));
    }
    if (parser.IsSet("cc")) {
        SetConfigChanges(parser.Value("cc"));
    }
    if (commandInfo.compressionResolutionWidth <= commandInfo.compressionResolutionHeight) {
        SetDeviceOrentation("portrait");
    } else {
        SetDeviceOrentation("landscape");
    }
    if (parser.IsSet("d")) {
        SetIsDebug(true);
        if (parser.IsSet("p")) {
            SetDebugServerPort(static_cast<uint16_t>(atoi(parser.Value("p").c_str())));
        }
    }
#ifdef COMPONENT_TEST_ENABLED
    if (parser.IsSet("componentTest")) {
        SetComponentTestModeConfig(parser.Value("componentTest"));
    }
#endif // COMPONENT_TEST_ENABLED
    VirtualScreenImpl::GetInstance().InitFoldParams();
    Start();
}

#ifdef COMPONENT_TEST_ENABLED
void JsAppImpl::SetComponentTestConfig(Platform::AceRunArgs& args, const std::string componentTest) const
{
    args.componentTestConfig = componentTest;
    args.isComponentTestMode = true;
}

void JsAppImpl::SetComponentTestModeConfig(const std::string value)
{
    componentTestModeConfig = value;
}
#endif // COMPONENT_TEST_ENABLED

void JsAppImpl::StopAbility()
{
    if (listener) {
        OHOS::Rosen::Window* window = GetWindow();
        if (window) {
            window->UnRegisterSystemBarEnableListener(sptr<OHOS::Rosen::IWindowSystemBarEnableListener>(listener));
            listener = nullptr;
        }
    }
    if (isDebug && debugServerPort >= 0) {
#if defined(__APPLE__) || defined(_WIN32)
        if (simulator) {
            simulator->TerminateAbility(debugAbilityId);
        }
#endif
    } else {
        ability = nullptr;
    }
    OHOS::Ide::StageContext::GetInstance().ReleaseHspBuffers();
    if (glfwRenderContext != nullptr) {
        glfwRenderContext->DestroyWindow();
        glfwRenderContext->Terminate();
        ILOG("glfw Terminate finished");
    }
}

void JsAppImpl::InitCommandInfo()
{
    CommandParser::GetInstance().GetCommandInfo(commandInfo);
}

void JsAppImpl::InitScreenInfo()
{
    screenInfo = VirtualScreenImpl::GetInstance().GetScreenInfo();
}
