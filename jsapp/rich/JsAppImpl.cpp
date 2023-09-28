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
#include "window_model.h"
#if defined(__APPLE__) || defined(_WIN32)
#include "options.h"
#include "simulator.h"
#endif

using namespace std;
using namespace OHOS;
using namespace OHOS::Ace;

JsAppImpl::JsAppImpl() noexcept : ability(nullptr), isStop(false)
{
#if defined(__APPLE__) || defined(_WIN32)
    windowModel = std::make_shared<OHOS::Previewer::PreviewerWindowModel>();
#endif
}

JsAppImpl::~JsAppImpl()
{
    if (glfwRenderContext != nullptr) {
        glfwRenderContext->DestroyWindow();
        glfwRenderContext->Terminate();
    }
}

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
    isFinished = true;
}

void JsAppImpl::Restart()
{
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
}

std::string JsAppImpl::GetJSONTree()
{
    std::string jsongTree = ability->GetJSONTree();
    Json::Value jsonData = JsonReader::ParseJsonData(jsongTree);
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    builder["emitUTF8"] = true;
    return Json::writeString(builder, jsonData);
}

std::string JsAppImpl::GetDefaultJSONTree()
{
    ILOG("Start getDefaultJsontree.");
    std::string jsongTree = ability->GetDefaultJSONTree();
    Json::Value jsonData = JsonReader::ParseJsonData(jsongTree);
    ILOG("GetDefaultJsontree finished.");
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    builder["emitUTF8"] = true;
    return Json::writeString(builder, jsonData);
}

void JsAppImpl::OrientationChanged(std::string commandOrientation)
{
    aceRunArgs.deviceWidth = height;
    aceRunArgs.deviceHeight = width;
    VirtualScreenImpl::GetInstance().WidthAndHeightReverse();

    AdaptDeviceType(aceRunArgs, CommandParser::GetInstance().GetDeviceType(),
                    VirtualScreenImpl::GetInstance().GetOrignalWidth());
    AssignValueForWidthAndHeight(VirtualScreenImpl::GetInstance().GetOrignalWidth(),
                                 VirtualScreenImpl::GetInstance().GetOrignalHeight(),
                                 VirtualScreenImpl::GetInstance().GetCompressionWidth(),
                                 VirtualScreenImpl::GetInstance().GetCompressionHeight());
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
}

void JsAppImpl::SetJsAppArgs(OHOS::Ace::Platform::AceRunArgs& args)
{
    SetAssetPath(args, jsAppPath);
    SetProjectModel(args);
    SetPageProfile(args, CommandParser::GetInstance().GetPages());
    SetDeviceWidth(args, width);
    SetDeviceHeight(args, height);
    SetWindowTitle(args, "Ace");
    SetUrl(args, urlPath);
    SetConfigChanges(args, configChanges);
    SetColorMode(args, colorMode);
    SetOrientation(args, orientation);
    SetAceVersionArgs(args, aceVersion);
    SetDeviceScreenDensity(atoi(screenDensity.c_str()),
                           CommandParser::GetInstance().GetDeviceType());
    SetLanguage(args, SharedData<string>::GetData(SharedDataType::LAN));
    SetRegion(args, SharedData<string>::GetData(SharedDataType::REGION));
    SetScript(args, "");
    SetSystemResourcesPath(args);
    SetAppResourcesPath(args, CommandParser::GetInstance().GetAppResourcePath());
    SetFormsEnabled(args, CommandParser::GetInstance().IsCardDisplay());
    SetContainerSdkPath(args, CommandParser::GetInstance().GetContainerSdkPath());
    AdaptDeviceType(args, CommandParser::GetInstance().GetDeviceType(),
                    VirtualScreenImpl::GetInstance().GetOrignalWidth());
    SetOnRender(args);
    SetOnRouterChange(args);
    SetOnError(args);
    SetComponentModeEnabled(args, CommandParser::GetInstance().IsComponentMode());
    ILOG("start abilit: %d %d %f", args.deviceWidth, args.deviceHeight, args.deviceConfig.density);
}

void JsAppImpl::RunJsApp()
{
    ILOG("RunJsApp 1");
    AssignValueForWidthAndHeight(VirtualScreenImpl::GetInstance().GetOrignalWidth(),
                                 VirtualScreenImpl::GetInstance().GetOrignalHeight(),
                                 VirtualScreenImpl::GetInstance().GetCompressionWidth(),
                                 VirtualScreenImpl::GetInstance().GetCompressionHeight());
    SetJsAppArgs(aceRunArgs);
    OHOS::Ide::StageContext::GetInstance().SetLoaderJsonPath(aceRunArgs.assetPath);
    OHOS::Ide::StageContext::GetInstance().GetModulePathMapFromLoaderJson();
    InitGlfwEnv();
    if (isDebug && debugServerPort >= 0) {
        RunDebugAbility(); // for debug preview
    } else {
        RunNormalAbility(); // for normal preview
    }
}

void JsAppImpl::RunNormalAbility()
{
    if (ability != nullptr) {
        ability.reset();
    }
    TraceTool::GetInstance().HandleTrace("Launch Js App");
    ability = Platform::AceAbility::CreateInstance(aceRunArgs);
    if (ability == nullptr) {
        ELOG("JsApp::Run ability create failed.");
        return;
    }
    OHOS::Rosen::WMError errCode;
    OHOS::sptr<OHOS::Rosen::WindowOption> sp = nullptr;
    auto window = OHOS::Rosen::Window::Create("previewer", sp, nullptr, errCode);
    window->SetContentInfoCallback(std::move(VirtualScreenImpl::LoadContentCallBack));
    window->CreateSurfaceNode("preview_surface", aceRunArgs.onRender);
    ability->SetWindow(window);
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
    std::string abilitySrcPath = CommandParser::GetInstance().GetAbilityPath();
    debugAbilityId = simulator->StartAbility(abilitySrcPath, [](int64_t abilityId) {});
    if (debugAbilityId < 0) {
        ELOG("JsApp::Run ability start failed. abilitySrcPath:%s", abilitySrcPath.c_str());
        return;
    }
    // set onRender callback
    OHOS::Rosen::Window* window = OHOS::Previewer::PreviewerWindow::GetInstance().GetWindowObject();
    if (!window) {
        ELOG("JsApp::Run get window failed.");
        return;
    }
    window->SetContentInfoCallback(std::move(VirtualScreenImpl::LoadContentCallBack));
    window->CreateSurfaceNode(options.moduleName, std::move(VirtualScreenImpl::CallBack));
}

void JsAppImpl::SetSimulatorParams(OHOS::AbilityRuntime::Options& options)
{
    const string path = CommandParser::GetInstance().GetAppResourcePath() +
                        FileSystem::GetSeparator() + "module.json";
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
    options.resourcePath = CommandParser::GetInstance().GetAppResourcePath() +
                                FileSystem::GetSeparator() + "resources.index";
    options.debugPort = debugServerPort;
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
    OHOS::AbilityRuntime::DeviceConfig deviceCfg;
    deviceCfg.deviceType = SetDevice<OHOS::AbilityRuntime::DeviceType>(aceRunArgs.deviceConfig.deviceType);
    deviceCfg.orientation = SetOrientation<OHOS::AbilityRuntime::DeviceOrientation>(
        aceRunArgs.deviceConfig.orientation);
    deviceCfg.colorMode = SetColorMode<OHOS::AbilityRuntime::ColorMode>(aceRunArgs.deviceConfig.colorMode);
    deviceCfg.density = aceRunArgs.deviceConfig.density;
    options.deviceConfig = deviceCfg;
    string fPath = CommandParser::GetInstance().GetConfigPath();
    options.configuration = UpdateConfiguration(aceRunArgs);
    std::size_t pos = fPath.find(".idea");
    if (pos == std::string::npos) {
        ELOG("previewPath error:%s", fPath.c_str());
    } else {
        options.previewPath = fPath.substr(0, pos) + ".idea" + FileSystem::GetSeparator() + "previewer";
        ILOG("previewPath info:%s", options.previewPath.c_str());
    }
}

std::shared_ptr<AppExecFwk::Configuration> JsAppImpl::UpdateConfiguration(OHOS::Ace::Platform::AceRunArgs& args)
{
    std::shared_ptr<AppExecFwk::Configuration> configuration = make_shared<AppExecFwk::Configuration>();
    configuration->AddItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE,
        SharedData<string>::GetData(SharedDataType::LANGUAGE));
    string colorMode = "light";
    if (aceRunArgs.deviceConfig.colorMode == ColorMode::DARK) {
        colorMode = "dark";
    }
    configuration->AddItem(OHOS::AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, colorMode);
    string direction = "portrait";
    if (aceRunArgs.deviceConfig.orientation == DeviceOrientation::LANDSCAPE) {
        orientation = "landscape";
    }
    configuration->AddItem(OHOS::AppExecFwk::ConfigurationInner::APPLICATION_DIRECTION, direction);
    string density = std::to_string(aceRunArgs.deviceConfig.density);
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
        args.deviceConfig.deviceType = DeviceType::TABLET;
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
    string sep = FileSystem::GetSeparator();
    string rPath = FileSystem::GetApplicationPath();
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

void JsAppImpl::SetOnRender(Platform::AceRunArgs& args) const
{
    args.onRender = std::move(VirtualScreenImpl::CallBack);
}

void JsAppImpl::SetOnRouterChange(Platform::AceRunArgs& args) const
{
    args.onRouterChange = std::move(VirtualScreenImpl::PageCallBack);
}

void JsAppImpl::SetOnError(Platform::AceRunArgs& args) const
{
    args.onError = std::move(VirtualScreenImpl::FastPreviewCallBack);
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

void JsAppImpl::ResolutionChanged(int32_t changedOriginWidth, int32_t changedOriginHeight, int32_t changedWidth,
                                  int32_t changedHeight, int32_t screenDensity)
{
    SetResolutionParams(changedOriginWidth, changedOriginHeight, changedWidth, changedHeight, screenDensity);
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
        OHOS::AppExecFwk::EventHandler::PostTask([this]() {
            glfwRenderContext->SetWindowSize(width, height);
        });
        simulator->UpdateConfiguration(*(UpdateConfiguration(aceRunArgs).get()));
        window->SetViewportConfig(config);
#endif
    } else {
        if (ability != nullptr) {
            OHOS::AppExecFwk::EventHandler::PostTask([this]() {
                glfwRenderContext->SetWindowSize(width, height);
            });
            ability->SurfaceChanged(aceRunArgs.deviceConfig.orientation, aceRunArgs.deviceConfig.density,
                                    aceRunArgs.deviceWidth, aceRunArgs.deviceHeight);
        }
    }
}

void JsAppImpl::SetResolutionParams(int32_t changedOriginWidth, int32_t changedOriginHeight, int32_t changedWidth,
    int32_t changedHeight, int32_t screenDensity)
{
    SetDeviceWidth(aceRunArgs, changedWidth);
    SetDeviceHeight(aceRunArgs, changedHeight);
    orignalWidth = changedOriginWidth;
    orignalHeight = changedOriginHeight;
    VirtualScreenImpl::GetInstance().SetVirtualScreenWidthAndHeight(changedOriginWidth, changedOriginHeight,
                                                                    changedWidth, changedHeight);
    SetDeviceScreenDensity(screenDensity,
                           CommandParser::GetInstance().GetDeviceType());
    AdaptDeviceType(aceRunArgs, CommandParser::GetInstance().GetDeviceType(),
                    VirtualScreenImpl::GetInstance().GetOrignalWidth());
    AssignValueForWidthAndHeight(VirtualScreenImpl::GetInstance().GetOrignalWidth(),
                                 VirtualScreenImpl::GetInstance().GetOrignalHeight(),
                                 VirtualScreenImpl::GetInstance().GetCompressionWidth(),
                                 VirtualScreenImpl::GetInstance().GetCompressionHeight());
    if (changedWidth <= changedHeight) {
        JsAppImpl::GetInstance().SetDeviceOrentation("portrait");
    } else {
        JsAppImpl::GetInstance().SetDeviceOrentation("landscape");
    }
    SetOrientation(aceRunArgs, orientation);
    ILOG("ResolutionChanged: %s %d %d %f", orientation.c_str(), aceRunArgs.deviceWidth,
         aceRunArgs.deviceHeight, aceRunArgs.deviceConfig.density);
}

void JsAppImpl::SetArgsColorMode(const string& value)
{
    colorMode = value;
}

void JsAppImpl::SetArgsAceVersion(const string& value)
{
    aceVersion = value;
}

void JsAppImpl::SetDeviceOrentation(const string& value)
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
    }
    return false;
}

void JsAppImpl::ParseSystemParams(OHOS::Ace::Platform::AceRunArgs& args, Json::Value paramObj)
{
    if (paramObj == Json::nullValue) {
        SetDeviceWidth(args, VirtualScreenImpl::GetInstance().GetCompressionWidth());
        SetDeviceHeight(args, VirtualScreenImpl::GetInstance().GetCompressionHeight());
        AssignValueForWidthAndHeight(args.deviceWidth, args.deviceHeight,
                                     args.deviceWidth, args.deviceHeight);
        SetColorMode(args, colorMode);
        SetOrientation(args, orientation);
        SetDeviceScreenDensity(atoi(screenDensity.c_str()),
                               CommandParser::GetInstance().GetDeviceType());
        AdaptDeviceType(args, CommandParser::GetInstance().GetDeviceType(),
                        VirtualScreenImpl::GetInstance().GetOrignalWidth());
        SetLanguage(args, SharedData<string>::GetData(SharedDataType::LAN));
        SetRegion(args, SharedData<string>::GetData(SharedDataType::REGION));
    } else {
        SetDeviceWidth(args, paramObj["width"].asInt());
        SetDeviceHeight(args, paramObj["height"].asInt());
        AssignValueForWidthAndHeight(args.deviceWidth, args.deviceHeight,
                                     args.deviceWidth, args.deviceHeight);
        SetColorMode(args, paramObj["colorMode"].asString());
        SetOrientation(args, paramObj["orientation"].asString());
        string deviceType = paramObj["deviceType"].asString();
        SetDeviceScreenDensity(atoi(screenDensity.c_str()), deviceType);
        AdaptDeviceType(args, deviceType, args.deviceWidth, paramObj["dpi"].asDouble());
        string lanInfo = paramObj["locale"].asString();
        SetLanguage(args, lanInfo.substr(0, lanInfo.find("_")));
        SetRegion(args, lanInfo.substr(lanInfo.find("_") + 1, lanInfo.length() - 1));
    }
}

void JsAppImpl::SetSystemParams(OHOS::Ace::Platform::SystemParams& params, Json::Value paramObj)
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
    params.isRound = (paramObj == Json::nullValue) ?
                     (CommandParser::GetInstance().GetScreenShape() == "circle") :
                     paramObj["roundScreen"].asBool();
}

void JsAppImpl::LoadDocument(const std::string filePath,
                             const std::string componentName,
                             Json::Value previewContext)
{
    ILOG("LoadDocument.");
    if (ability != nullptr) {
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
            glfwRenderContext->SetWindowSize(width, height);
        });
        ability->LoadDocument(filePath, componentName, params);
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

string JsAppImpl::GetDeviceTypeName(const OHOS::Ace::DeviceType type) const
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
    glfwRenderContext = OHOS::Rosen::GlfwRenderContext::GetGlobal();
    if (!glfwRenderContext->Init()) {
        ELOG("Could not create window: InitGlfwEnv failed.");
        return;
    }
    glfwRenderContext->CreateGlfwWindow(aceRunArgs.deviceWidth, aceRunArgs.deviceHeight, false);
}