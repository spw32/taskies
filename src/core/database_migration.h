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

#include <string>

#include <spdlog/spdlog.h>

namespace app::Core
{
class Environment;

struct Migration {
    std::string name;
    std::string sql;
};

class DatabaseMigration
{
public:
    DatabaseMigration(std::shared_ptr<Environment> env, std::shared_ptr<spdlog::logger> logger);
    ~DatabaseMigration();

    bool Migrate();

private:
    void CreateMigrationHistoryTable();
    bool MigrationExists(const std::string& name);

    sqlite3* pDb;
    std::shared_ptr<Environment> pEnv;
    std::shared_ptr<spdlog::logger> pLogger;
};
} // namespace app::Core
