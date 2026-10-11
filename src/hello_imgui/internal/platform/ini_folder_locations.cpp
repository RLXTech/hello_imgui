/**
 * @file ini_folder_locations.cpp
 * @brief Native user directory queries delegated to the shared Unicode path layer.
 * Returned strings are explicitly UTF-8 compatibility values, never ACP bytes.
 */
#include "hello_imgui/internal/platform/ini_folder_locations.h"
#include "vd_path/vd_system_path.h"
#include <stdexcept>
namespace HelloImGui
{
/** @brief Return a UTF-8 user directory, or throw a diagnosable bootstrap failure. */
std::string IniFolderLocation(IniFolderType type)
{
    using namespace vdas::path;
    if (type == IniFolderType::CurrentFolder || type == IniFolderType::AbsolutePath) return "";
    auto result = [&]() -> PathResult<Path> {
        switch (type)
        {
            case IniFolderType::AppUserConfigFolder: return userDirectory(UserDirectory::Config);
            case IniFolderType::DocumentsFolder: return userDirectory(UserDirectory::Documents);
            case IniFolderType::HomeFolder: return userDirectory(UserDirectory::Home);
            case IniFolderType::TempFolder: return temporaryDirectory();
            case IniFolderType::AppExecutableFolder:
            {
                auto exe = executablePath();
                if (!exe) return exe.error();
                return exe.value().parent();
            }
            default: return PathError{PathErrorCode::DirectoryUnavailable, "IniFolderLocation", "folder-type", {}};
        }
    }();
    if (!result) throw std::runtime_error("Hello ImGui directory query failed: " + result.error().operation);
    return result.value().utf8();
}
}
