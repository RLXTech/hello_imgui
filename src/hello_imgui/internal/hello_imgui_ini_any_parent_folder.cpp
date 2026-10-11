#include "hello_imgui/internal/hello_imgui_ini_any_parent_folder.h"
#include "hello_imgui/internal/inicpp.h"

#include "vd_path/vd_system_path.h"
#include <vector>
#include <filesystem>
#include <string>
#include <optional>


namespace HelloImGui
{
    namespace HelloImGuiIniAnyParentFolder
    {
        //
        // Utilities that read any settings in a hello_imgui.ini file located in the current folder or any of its parents
        //
        std::vector<std::string> _folderAndAllParents(const std::string& folder)
        {
            std::vector<std::string> result;
            auto admitted = vdas::path::parseUtf8(folder, "hello-parent-ini");
            if (!admitted) return result;
            auto currentFolder = admitted.value();
            while (true)
            {
                result.push_back(currentFolder.utf8());
                auto parent = currentFolder.parent();
                if (parent.empty() || parent == currentFolder) break;
                currentFolder = std::move(parent);
            }
            return result;
        };

        std::vector<std::string> _allHelloImGuiIniFilesToSearch()
        {
            std::vector<std::string> allIniFileToSearch;

            auto cwd = vdas::path::currentDirectory();
            if (!cwd) return allIniFileToSearch;
            const std::string currentFolder = cwd.value().utf8();
            for (const auto& folder : _folderAndAllParents(currentFolder))
                allIniFileToSearch.push_back(folder + "/hello_imgui.ini");

            return allIniFileToSearch;
        };

        template<typename T>
        std::optional<T> _readIniValue(const std::string& iniFilePath, const std::string& sectionName, const std::string& valueName)
        {
            auto path = vdas::path::parseUtf8(iniFilePath, "hello-parent-ini");
            if (!path) return std::nullopt;
            auto regular = vdas::path::isRegularFile(path.value());
            if (!regular || !regular.value()) return std::nullopt;

            try
            {

                std::optional<T> result = std::nullopt;
                ini::IniFile ini;
                ini.load(iniFilePath);
                if (ini.find(sectionName) != ini.end())
                {
                    auto& section = ini.at(sectionName);
                    if (section.find(valueName) != section.end())
                        result =  ini[sectionName][valueName].as<T>();
                }
                return result;
            }
            catch(...)
            {
                return std::nullopt;
            }
        };

        template <typename T>
        std::optional<T> _readIniValueInParentFolders(const std::string& sectionName, const std::string& valueName)
        {
            for (const auto& iniFile: _allHelloImGuiIniFilesToSearch())
            {
                auto value = _readIniValue<T>(iniFile, sectionName, valueName);
                if (value.has_value())
                    return value;
            }
            return std::nullopt;
        };


        std::optional<float> readFloatValue(const std::string& sectionName, const std::string& valueName)
        {
            return _readIniValueInParentFolders<float>(sectionName, valueName);
        }

        std::optional<bool> readBoolValue(const std::string& sectionName, const std::string& valueName)
        {
            return _readIniValueInParentFolders<bool>(sectionName, valueName);
        }

        std::optional<std::string> readStringValue(const std::string& sectionName, const std::string& valueName)
        {
            return _readIniValueInParentFolders<std::string>(sectionName, valueName);
        }

        std::optional<int> readIntValue(const std::string &sectionName, const std::string &valueName)
        {
            return _readIniValueInParentFolders<int>(sectionName, valueName);
        }

    } // namespace HelloImGuiIniAnyParentFolder

} // namespace HelloImGui
