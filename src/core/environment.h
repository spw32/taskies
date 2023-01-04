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

#pragma once

#include <filesystem>
#include <memory>
#include <string>

namespace app::Core
{
class Environment final
{
public:
    enum class BuildConfiguration { Undefined, Debug, Release };

    enum class InstallFolder { Undefined, Portable, ProgramFiles };

    Environment();
    Environment(const Environment&) = delete;
    ~Environment() = default;

    Environment& operator=(const Environment&) = delete;

    std::filesystem::path GetLogFilePath();
    std::filesystem::path GetLanguagesPath();
    std::filesystem::path GetConfigurationPath();
    std::filesystem::path GetDatabasePath();

    std::string GetCurrentLocale();

    void SetInstallFolder();
    bool IsInstalled();
    bool SetIsInstalled();

private:
    std::filesystem::path GetApplicationPath();
    std::filesystem::path GetApplicationLogPath();
    std::filesystem::path GetApplicationLanguagesPath();
    std::filesystem::path GetApplicationConfigurationPath();
    std::filesystem::path GetApplicationDatabasePath();

    std::string GetLogName();
    std::string GetConfigName();
    std::string GetDatabaseName();

    std::string GetRegistryKey();

    BuildConfiguration mBuildConfig;
    InstallFolder mInstallFolder;
};
} // namespace app::Core
