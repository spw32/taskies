// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2022 spw32
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// Contact:
//     spw32 at proton dot me

#include "environment.h"

#include <wx/stdpaths.h>
#include <wx/msw/registry.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif // _WIN32

#include "../utils/utils.h"

namespace app::Core
{
Environment::Environment()
    : mBuildConfig(BuildConfiguration::Undefined)
    , mInstallFolder(InstallFolder::Undefined)
{
#ifdef TKS_DEBUG
    mBuildConfig = BuildConfiguration::Debug;
#else
    mBuildConfig = BuildConfiguration::Release;
#endif // TKS_DEBUG
}

std::filesystem::path Environment::GetApplicationPath()
{
    return wxPathOnly(wxStandardPaths::Get().GetExecutablePath()).ToStdString();
}

std::filesystem::path Environment::GetLogFilePath()
{
    return GetApplicationLogPath() / GetLogName();
}

std::filesystem::path Environment::GetLanguagesPath()
{
    return GetApplicationLanguagesPath();
}

std::string Environment::GetCurrentLocale()
{
#ifdef _WIN32
    LCID lcid = GetThreadLocale();
    wchar_t name[LOCALE_NAME_MAX_LENGTH];
    int res = LCIDToLocaleName(lcid, name, LOCALE_NAME_MAX_LENGTH, 0);
    if (res == 0) {
        return "en-US";
    }

    return std::string(Utils::ToStdString(std::wstring(name)));
#endif // _WIN32

#ifdef __linux__
    setlocale(LC_ALL, NULL);
    return std::string(std::locale().name());
#endif // __linux__
}

void Environment::SetInstallFolder()
{
    const std::string ProgramFilesSubstring = "Program Files";
    const std::string ApplicationPath = GetApplicationPath().string();

    if (ApplicationPath.find(ProgramFilesSubstring) != std::string::npos) {
        mInstallFolder = InstallFolder::ProgramFiles;
    } else {
        mInstallFolder = InstallFolder::Portable;
    }
}

bool Environment::IsInstalled()
{
    wxRegKey key(wxRegKey::HKCU, GetRegistryKey());
    if (key.HasValue("IsSetup")) {
        long value = 0;
        key.QueryValue("IsSetup", &value);

        return !!value;
    }
    return false;
}

bool Environment::SetIsInstalled()
{
    wxRegKey key(wxRegKey::HKCU, GetRegistryKey());
    bool result = key.Exists();
    if (!result) {
        return result;
    }

    return key.SetValue("IsSetup", true);
}

std::filesystem::path Environment::GetApplicationLogPath()
{
    std::filesystem::path appLogPath;
    switch (mBuildConfig) {
    case BuildConfiguration::Debug:
        appLogPath = GetApplicationPath() / "logs";
        break;
    case BuildConfiguration::Release:
        appLogPath = std::filesystem::path(wxStandardPaths::Get().GetUserDataDir().ToStdString()) / "logs";
        break;
    default:
        break;
    }

    {
        // spdlog only creates the file, the directory needs to be handled by us
        std::filesystem::create_directories(appLogPath);
    }

    return appLogPath;
}

std::filesystem::path Environment::GetApplicationLanguagesPath()
{
    std::filesystem::path appLangPath;
    const std::string lang = "lang";

    switch (mBuildConfig) {
    case BuildConfiguration::Debug:
        appLangPath = GetApplicationPath() / lang;
        break;
    case BuildConfiguration::Release:
        appLangPath = std::filesystem::path(wxStandardPaths::Get().GetAppDocumentsDir().ToStdString()) / lang;
        break;
    default:
        break;
    }

    return appLangPath;
}

std::string Environment::GetLogName()
{
    return "taskies.log";
}

std::string Environment::GetRegistryKey()
{
    if (mBuildConfig == BuildConfiguration::Debug) {
        wxRegKey key(wxRegKey::HKCU, "Software\\Taskiesd");
        if (!key.Exists()) {
            key.Create("Software\\Taskiesd");
        }
    }

    switch (mBuildConfig) {
    case BuildConfiguration::Debug:
        return "Software\\Taskiesd";
    case BuildConfiguration::Release:
        return "Software\\Taskies";
    default:
        return "";
    }
}
} // namespace app::Core
