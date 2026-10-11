#include "hello_imgui/hello_imgui_assets.h"
#include "imgui.h"

#ifdef HELLOIMGUI_INSIDE_APPLE_BUNDLE
#include "hello_imgui/internal/platform/getAppleBundleResourcePath.h"
#include <unistd.h>
#endif

#include "hello_imgui/internal/whereami/whereami_cpp.h"


#ifdef HELLOIMGUI_USE_SDL2
#include "SDL_system.h"
#endif

#include "hello_imgui/hello_imgui_error.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <stdio.h>

#include "vd_path/vd_system_path.h"
namespace FileUtils
{
    /** @brief Validate the declared resource path and probe the same native identity used for reading. */
    bool IsRegularFile(const std::string& filename)
    {
        auto path = vdas::path::parseUtf8(filename, "hello-resource");
        if (!path) return false;
        auto result = vdas::path::isRegularFile(path.value());
        return result && result.value();
    }
    /** @brief Return native CWD as UTF-8, or a controlled error without a fixed buffer. */
    std::string GetCurrentDirectory()
    {
        auto cwd = vdas::path::currentDirectory();
        if (!cwd) throw std::runtime_error("Hello ImGui CWD unavailable");
        return cwd.value().utf8();
    }
}


namespace HelloImGui
{
std::string gAssetsSubfolderFolderName = "assets";

std::string gOverrideAssetsFolder = "";

void overrideAssetsFolder(const char* folder)
{
    gOverrideAssetsFolder = folder;
}

void SetAssetsFolder(const char* folder)
{
    gOverrideAssetsFolder = folder;
}

void SetAssetsFolder(const std::string& folder)
{
    SetAssetsFolder(folder.c_str());
}

struct AssetFolderWithDesignation
{
    std::string folder;
    std::string designation;
};

std::vector<AssetFolderWithDesignation> computePossibleAssetsFolders()
{
    std::vector<AssetFolderWithDesignation> r;

    // Search inside gOverrideAssetsFolder set by SetAssetsFolder()
    if (! gOverrideAssetsFolder.empty())
        r.push_back({gOverrideAssetsFolder, "folder provided by HelloImGui::SetAssetsFolder()"});

    r.push_back({"", "absolute path or current working directory"});

    // For apple bundle, search at the bundle resources folder
    #ifdef HELLOIMGUI_INSIDE_APPLE_BUNDLE
    r.push_back({GetBundlePath() + "/Contents/Resources/" + gAssetsSubfolderFolderName, "AppleAppBundle/Contents/Resources/assets"});
    #endif

    // Search inside a subfolder of the current working directory
    #if !defined(HELLOIMGUI_MOBILEDEVICE)
    {
        auto cwd = vdas::path::currentDirectory();
        if (cwd) r.push_back({cwd.value().join(gAssetsSubfolderFolderName).value().utf8(), "current_folder/assets"});
    }
    #endif

    // Search inside a subfolder of the exe
    #if !defined(HELLOIMGUI_MOBILEDEVICE) && !defined(__EMSCRIPTEN__)
    {
        r.push_back({wai_getExecutableFolder_string() + "/" + gAssetsSubfolderFolderName, "exe_folder/assets"});
        #ifdef _MSC_VER
        {
            // For msvc, we have to deal with the fact that the exe could be in a subfolder "Debug" or "Release" of ${CMAKE_CURRENT_BINARY_DIR}
            // whereas the assets folder is located directly inside ${CMAKE_CURRENT_BINARY_DIR}
            r.push_back({wai_getExecutableFolder_string() + "/../" + gAssetsSubfolderFolderName, "exe_folder/../assets"});
        }
        #endif
    }
    #endif

    // For emscripten, search at "/"
    #ifdef __EMSCRIPTEN__
    {
        r.push_back({"/", "root folder for emscripten"});
        r.push_back({"/" + gAssetsSubfolderFolderName , "root assets folder for emscripten"});
    }
    #endif

    return r;
}

/// Access font files in application bundle or assets/fonts/
std::string AssetFileFullPath(const std::string& assetFilename, bool assertIfNotFound)
{
#if defined(__ANDROID__)
    // Under android, assets can be compressed
    // You cannot use standard file operations!`
    (void)assetFilename;
    IM_ASSERT(false); //assetFileFullPath does not work on android!
    return "";
#else
    auto possibleAssetsFolders = computePossibleAssetsFolders();
    for (const auto& assetsFolder: possibleAssetsFolders)
    {
        auto folder = vdas::path::parseUtf8(assetsFolder.folder, "hello-resource-root");
        if (!folder) continue;
        auto joined = folder.value().join(assetFilename);
        if (!joined) continue;
        const std::string path = joined.value().utf8();
        if (FileUtils::IsRegularFile(path))
            return path;
    }
    #if defined(IOS)
    {
        std::string path = getAppleBundleResourcePath(std::string("assets/") + assetFilename.c_str());
        if (FileUtils::IsRegularFile(path))
            return path;
    }
    #endif

    //
    // Handle failures below
    //

    // Display nice message on error
    if (assertIfNotFound)
    {
        std::string errorMessage;
        errorMessage += "assetFileFullPath(" + assetFilename + ") failed!\n";
        errorMessage += "    Tried the following assets folders:\n";
        for (const auto& assetsFolder: possibleAssetsFolders)
        {
            errorMessage += "        " + assetsFolder.designation + ":\n";
            errorMessage += "            " + assetsFolder.folder + "\n";
        }
        errorMessage += "    (you can call HelloImGui::SetAssetsFolder() to set the default search location)\n";
        throw std::runtime_error(errorMessage);
    }
    return "";
#endif
}

// Returns true if this asset file exists
bool AssetExists(const std::string& assetFilename)
{
#ifdef __ANDROID__
    size_t dataSize;
    void *data = SDL_LoadFile(assetFilename.c_str(), &dataSize);
    bool exists = (data != nullptr);
    if (data != nullptr)
        SDL_free(data);
    return exists;
#else
    std::string fullPath = AssetFileFullPath(assetFilename, false);
    return ! fullPath.empty();
#endif
}


#ifdef HELLOIMGUI_USE_SDL2

AssetFileData DefaultLoadAssetFileData(const char *assetPath)
{
    #ifdef __ANDROID__
    {
        AssetFileData r;
        r.data = SDL_LoadFile(assetPath, &r.dataSize);
        if (!r.data)
            HIMG_ERROR(std::string("LoadAssetFileData: cannot load ") + assetPath);

        return r;
    }
    #else
    {
        std::string assetFullPath = assetFileFullPath(assetPath);

        AssetFileData r;
        r.data = SDL_LoadFile(assetFullPath.c_str(), &r.dataSize);
        if (! r.data)
        {
            std::string errorMessage = std::string("LoadAssetFileData(") + assetPath + ")\n";
            errorMessage += "==> SDL_LoadFile(" + assetFullPath + ") failed !!!!\n";
            throw std::runtime_error(errorMessage);
            IM_ASSERT(false);
        }

        return r;
    }
    #endif
}

void FreeAssetFileData(AssetFileData * assetFileData)
{
    SDL_free(assetFileData->data);
    assetFileData = nullptr;
}


#else

AssetFileData LoadAssetFileData_Impl(const char *assetPath)
{
    AssetFileData r;

    auto path = vdas::path::parseUtf8(assetPath, "hello-asset-read");
    if (!path) return AssetFileData();
    auto opened = vdas::path::openRead(path.value(), std::ios::binary | std::ios::ate);
    if (!opened) throw std::runtime_error("Asset read failed: " + opened.error().nativeError.message());
    auto ifs = std::move(opened).value();

    const auto size = ifs.tellg();
    if (size < 0) return AssetFileData();
    r.dataSize = static_cast<std::size_t>(size);
    ifs.seekg(0, std::ios::beg);
    r.data = malloc(r.dataSize ? r.dataSize : 1);
    if (!r.data) throw std::bad_alloc();

    if (ifs.read((char *)r.data, r.dataSize))
        return r;
    else
    {
        free(r.data);
        return AssetFileData();
    }
}

AssetFileData DefaultLoadAssetFileData(const char *assetPath)
{
    std::string fullPath = assetFileFullPath(assetPath);
    AssetFileData r = LoadAssetFileData_Impl(fullPath.c_str());
    if (!r.data)
    {
        std::stringstream msg;
        msg << "LoadAssetFileData: cannot load " << assetPath << " (also tried " << fullPath << ")\n";
        msg << "(you can call HelloImGui::SetAssetsFolder() to change the assets default location.";
        throw std::runtime_error(msg.str());
    }
    return r;
}

void FreeAssetFileData(AssetFileData * assetFileData)
{
    free(assetFileData->data);
    assetFileData = nullptr;
}

#endif // #ifdef HELLOIMGUI_USE_SDL2

//
// Tooling to make it possible to redirect asset loading
//
static LoadAssetFileDataFunc loadAssetFileDataFunc = DefaultLoadAssetFileData;
AssetFileData LoadAssetFileData(const char *assetPath)
{
    AssetFileData data = loadAssetFileDataFunc(assetPath);
    return data;
}
void SetLoadAssetFileDataFunction(LoadAssetFileDataFunc newLoadAssetFileDataFunc)
{
    loadAssetFileDataFunc = std::move(newLoadAssetFileDataFunc);
}


}  // namespace HelloImGui
