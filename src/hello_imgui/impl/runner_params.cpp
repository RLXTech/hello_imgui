#include "hello_imgui/runner_params.h"

#include "vd_path/vd_path.h"
#include <stdexcept>

namespace HelloImGui
{
    RunnerParams SimpleRunnerParams::ToRunnerParams() const
    {
        auto& self = *this;
        RunnerParams r;

        r.callbacks.ShowGui = self.guiFunction;

        r.appWindowParams.windowGeometry.size = self.windowSize;
        r.appWindowParams.windowGeometry.sizeAuto = self.windowSizeAuto;
        r.appWindowParams.restorePreviousGeometry = self.windowRestorePreviousGeometry;

        r.appWindowParams.windowTitle = self.windowTitle;

        r.fpsIdling.fpsIdle = self.fpsIdle;
        r.fpsIdling.enableIdling = self.enableIdling;

        return r;
    }


    // IniSettingsLocation returns the path to the ini file for the application settings.
    std::string IniSettingsLocation(const RunnerParams& runnerParams)
    {
        auto _getIniFileName = [& runnerParams]() -> std::string
        {
            auto _stringToSaneFilename=[](const std::string& s, const std::string& extension) -> std::string
            {
                std::string filenameSanitized;

                for (char c : s)
                {
                    if (isalnum(static_cast<unsigned char>(c)))
                        filenameSanitized += c;
                    else
                        filenameSanitized += "_";
                }

                filenameSanitized += extension;
                return filenameSanitized;
            };

            if (! runnerParams.iniFilename.empty())
                return runnerParams.iniFilename;

            if (runnerParams.iniFilename_useAppWindowTitle && !runnerParams.appWindowParams.windowTitle.empty())
                return _stringToSaneFilename(runnerParams.appWindowParams.windowTitle, ".ini");

            return "imgui.ini";

        };

        auto filename = vdas::path::parseUtf8(_getIniFileName(), "hello-ini-filename");
        if (!filename) throw std::runtime_error("Invalid Hello ImGui ini filename");
        const auto folderText = HelloImGui::IniFolderLocation(runnerParams.iniFolderType);
        auto folder = vdas::path::parseUtf8(folderText, "hello-ini-directory");
        if (!folder) throw std::runtime_error("Invalid Hello ImGui ini directory");
        auto fullPath = folder.value().join(filename.value());
        if (!fullPath) throw std::runtime_error("Hello ImGui ini join failed");
        auto parent = fullPath.value().parent();
        if (!parent.empty())
        {
            auto created = vdas::path::createDirectories(parent);
            if (!created) throw std::runtime_error("Hello ImGui ini directory inaccessible: " + created.error().nativeError.message());
        }
        const std::string iniFullFilename = fullPath.value().utf8();

        return iniFullFilename;
    }

    // DeleteIniSettings deletes the ini file for the application settings.
    void DeleteIniSettings(const RunnerParams& runnerParams)
    {
        std::string iniFullFilename = IniSettingsLocation(runnerParams);

        if (iniFullFilename.empty())
            return;

        auto path = vdas::path::parseUtf8(iniFullFilename, "hello-delete-ini");
        if (!path) throw std::runtime_error("Invalid Hello ImGui ini filename");
        auto removed = vdas::path::remove(path.value());
        if (!removed) throw std::runtime_error("Failed to delete Hello ImGui ini: " + removed.error().nativeError.message());
    }

    // HasIniSettings returns true if the ini file for the application settings exists.
    bool HasIniSettings(const RunnerParams& runnerParams)
    {
        std::string iniFullFilename = IniSettingsLocation(runnerParams);

        if (iniFullFilename.empty())
            return false;

        auto path = vdas::path::parseUtf8(iniFullFilename, "hello-has-ini");
        if (!path) throw std::runtime_error("Invalid Hello ImGui ini filename");
        auto exists = vdas::path::exists(path.value());
        if (!exists) throw std::runtime_error("Failed to query Hello ImGui ini: " + exists.error().nativeError.message());
        return exists.value();
    }

}  // namespace HelloImGui