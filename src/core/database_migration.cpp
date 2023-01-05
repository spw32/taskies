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
const std::string DatabaseMigration::BeginTransactionQuery = "BEGIN TRANSACTION";
const std::string DatabaseMigration::CommitTransactionQuery = "COMMIT";
const std::string DatabaseMigration::CreateMigrationHistoryQuery =
    "CREATE TABLE IF NOT EXISTS migration_history("
    "id INTEGER PRIMARY KEY NOT NULL,"
    "name TEXT NOT NULL"
    ");";
const std::string DatabaseMigration::SelectMigrationExistsQuery = "SELECT count(*) FROM migration_history WHERE name = ?";
const std::string DatabaseMigration::InsertMigrationHistoryQuery = "INSERT INTO migration_history (name) VALUES (?);";

DatabaseMigration::DatabaseMigration(std::shared_ptr<Environment> env, std::shared_ptr<spdlog::logger> logger)
    : pEnv(env)
    , pLogger(logger)
    , pDb(nullptr)
{
    auto databaseFile = pEnv->GetDatabasePath().string();
    int rc = sqlite3_open(databaseFile.c_str(), &pDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Failed to open database {0}", std::string(err));
    }

    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(pDb, "PRAGMA foreign_keys = ON;", -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Failed to prepare statement {0}", std::string(err));
        sqlite3_finalize(stmt);
        return;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_OK) {
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

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(pDb, BeginTransactionQuery.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Failed to prepare statement {0}", std::string(err));
        sqlite3_finalize(stmt);
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Failed to step statement {0}", std::string(err));
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);

    for (const auto& migration : migrations) {
        if (MigrationExists(migration.name)) {
            continue;
        }

        sqlite3_stmt* migrationStmt = nullptr;
        const char* sql = migration.sql.c_str();

        do {
            int mrc = sqlite3_prepare_v2(pDb, sql, -1, &migrationStmt, nullptr);
            if (mrc != SQLITE_OK) {
                const char* err = sqlite3_errmsg(pDb);
                pLogger->error("Error when executing statement {0}", std::string(err));
                sqlite3_finalize(migrationStmt);
                return false;
            }

            if (migrationStmt == nullptr) {
                break;
            }

            mrc = sqlite3_step(migrationStmt);
            if (mrc != SQLITE_OK && mrc != SQLITE_DONE) {
                const char* err = sqlite3_errmsg(pDb);
                pLogger->error("Failed to step/execute migration {0} - ({1})", migration.name, std::string(err));
                sqlite3_finalize(migrationStmt);
                return false;
            }

            sqlite3_finalize(migrationStmt);
        } while (sql && sql[0] != '\0');

        sqlite3_stmt* migrationHistoryStmt = nullptr;

        int mhrc = sqlite3_prepare_v2(pDb, InsertMigrationHistoryQuery.c_str(), -1, &migrationHistoryStmt, nullptr);
        if (mhrc != SQLITE_OK) {
            const char* err = sqlite3_errmsg(pDb);
            pLogger->error("Failed to prepare statement {0}", std::string(err));
            sqlite3_finalize(migrationHistoryStmt);
            return false;
        }

        mhrc = sqlite3_bind_text(
            migrationHistoryStmt, 1, migration.name.c_str(), static_cast<int>(migration.name.size()), SQLITE_TRANSIENT);
        if (mhrc != SQLITE_OK) {
            const char* err = sqlite3_errmsg(pDb);
            pLogger->error("Failed to bind {0}", std::string(err));
            sqlite3_finalize(migrationHistoryStmt);
            return false;
        }

        mhrc = sqlite3_step(migrationHistoryStmt);
        if (mhrc != SQLITE_OK) {
            const char* err = sqlite3_errmsg(pDb);
            pLogger->error("Failed to step through statement {0}", std::string(err));
            sqlite3_finalize(migrationHistoryStmt);
            return false;
        }
    }

    rc = sqlite3_prepare_v2(pDb, CommitTransactionQuery.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Failed to prepare statement {0}", std::string(err));
        sqlite3_finalize(stmt);
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Failed to step statement {0}", std::string(err));
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);

    return true;
}

void DatabaseMigration::CreateMigrationHistoryTable()
{
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(pDb, CreateMigrationHistoryQuery.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Failed to prepare statement {0}", std::string(err));
        sqlite3_finalize(stmt);
        return;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_OK) {
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
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb, SelectMigrationExistsQuery.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Failed to prepare statement {0}", std::string(err));
        sqlite3_finalize(stmt);
        return false;
    }

    rc = sqlite3_bind_text(stmt, 1, name.c_str(), static_cast<int>(name.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Failed to prepare statement {0}", std::string(err));
        sqlite3_finalize(stmt);
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Error when stepping into result {0}", std::string(err));
        sqlite3_finalize(stmt);
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        pLogger->warn("Count statement returned more than 1 row");
    }

    int count = sqlite3_column_int(stmt, 0);

    return count > 0;
}
} // namespace app::Core
