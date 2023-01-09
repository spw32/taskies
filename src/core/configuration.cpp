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

#include "configuration.h"

#include "environment.h"

namespace app::Core
{
const std::string Configuration::Sections::GeneralSection = "general";
const std::string Configuration::Sections::DatabaseSection = "database";

Configuration::Configuration(std::shared_ptr<Environment> env, std::shared_ptr<spdlog::logger> logger)
    : pEnv(env)
    , pLogger(logger)
{
    LoadConfigFile();
}

void Configuration::Save()
{
    const toml::value data{
        { Sections::GeneralSection, { { "lang", mSettings.UserInterfaceLanguage } } },
        { Sections::DatabaseSection, { { "backupPath", mSettings.DatabasePath } } },
    };

    const std::string configString = toml::format(data);

    const std::string configFilePath = pEnv->GetConfigurationPath().u8string();
    std::ofstream configFile;
    configFile.open(configFilePath, std::ios_base::out);
    if (!configFile) {
        pLogger->error("Failed to open config file at specified location {0}", configFilePath);
        return;
    }

    configFile << configString;

    configFile.close();
}

std::string Configuration::GetUserInterfaceLanguage()
{
    return mSettings.UserInterfaceLanguage;
}

void Configuration::SetUserInterfaceLanguage(const std::string& value)
{
    mSettings.UserInterfaceLanguage = value;
}

std::string Configuration::GetDatabasePath() const
{
    return mSettings.DatabasePath;
}

void Configuration::SetDatabasePath(const std::string& value)
{
    mSettings.DatabasePath = value;
}

void Configuration::LoadConfigFile()
{
    auto data = toml::parse(pEnv->GetConfigurationPath().string());
    GetGeneralConfig(data);
    GetDatabaseConfig(data);
}

void Configuration::GetGeneralConfig(const toml::value& config)
{
    const auto& generalSection = toml::find(config, Sections::GeneralSection);

    mSettings.UserInterfaceLanguage = toml::find<std::string>(generalSection, "lang");
}

void Configuration::GetDatabaseConfig(const toml::value& config)
{
    const auto& databaseSection = toml::find(config, Sections::DatabaseSection);

    mSettings.DatabasePath = toml::find<std::string>(databaseSection, "databasePath");
}

} // namespace app::Core
