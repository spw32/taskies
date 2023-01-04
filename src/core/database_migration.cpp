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

#include "database_migration.h"

#include <sqlite3.h>

#include "environment.h"
#include "../utils/utils.h"

namespace
{
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
static BOOL CALLBACK EnumMigrations(HMODULE hModule, LPCTSTR lpszType, LPTSTR lpszName, LONG_PTR lParam)
{
    std::vector<app::Core::Migration>* migrations = reinterpret_cast<std::vector<app::Core::Migration>*>(lParam);

    HRSRC rc = FindResource(hModule, lpszName, lpszType);
    DWORD size = SizeofResource(hModule, rc);
    HGLOBAL data = LoadResource(hModule, rc);
    const char* buffer = reinterpret_cast<const char*>(LockResource(data));

    app::Core::Migration m;
    m.name = app::Utils::ToStdString(lpszName);
    m.sql = std::string(buffer, size);

    migrations->push_back(m);

    return TRUE;
}
} // namespace

namespace app::Core
{
DatabaseMigration::DatabaseMigration(std::shared_ptr<Environment> env, std::shared_ptr<spdlog::logger> logger)
    : pEnv(env)
    , pLogger(logger)
    , pDb(nullptr)
{
    auto databaseFile = pEnv->GetDatabasePath().string();
    int rc = sqlite3_open(databaseFile.c_str(), &pDb);
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Failed to open database {0}", std::string(err));
    }

    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(pDb, "PRAGMA foreign_keys = ON;", -1, &stmt, nullptr);
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Failed to prepare statement {0}", std::string(err));
        sqlite3_finalize(stmt);
        return;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Error when executing statement {0}", std::string(err));
        sqlite3_finalize(stmt);
        return;
    }

    if (rc != SQLITE_DONE) {
        pLogger->error("Error when running statement");
        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_finalize(stmt);
}

DatabaseMigration::~DatabaseMigration()
{
    sqlite3_close(pDb);
}

bool DatabaseMigration::Migrate()
{
    CreateMigrationHistoryTable();

    std::vector<Migration> migrations;

    // clang-format off
    EnumResourceNames(
        nullptr,
        TEXT("MIGRATION"),
        &EnumMigrations,
        reinterpret_cast<LONG_PTR>(&migrations)
    );
    // clang-format on

    bool sqlTransactionSuccessful = true;

    return false;
}

void DatabaseMigration::CreateMigrationHistoryTable()
{
    std::string createMigrationHistory = "CREATE TABLE IF NOT EXISTS migration_history("
                                         "id INTEGER PRIMARY KEY NOT NULL,"
                                         "name TEXT NOT NULL"
                                         ");";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(pDb, createMigrationHistory.c_str(), -1, &stmt, nullptr);
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Failed to prepare statement {0}", std::string(err));
        sqlite3_finalize(stmt);
        return;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Error when executing statement {0}", std::string(err));
        sqlite3_finalize(stmt);
        return;
    }

    if (rc != SQLITE_DONE) {
        pLogger->error("Error when running statement");
        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_finalize(stmt);
}

bool DatabaseMigration::MigrationExists(const std::string& name)
{
    return false;
}
} // namespace app::Core
