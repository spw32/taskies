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

#include "application.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

#include "core/environment.h"

namespace app
{
Application::Application()
    : pLogger(nullptr)
{
    SetProcessDPIAware();
}

bool Application::OnInit()
{
    if (!wxApp::OnInit()) {
        return false;
    }

    InitializeLogger();
}

int Application::OnExit()
{
    // Under VisualStudio, this must be called before main finishes to workaround a known VS issue
    spdlog::drop_all();
    return wxApp::OnExit();
}

void Application::InitializeLogger()
{
    pEnv = std::make_shared<Core::Environment>();

    auto logDirectory = pEnv->GetLogFilePath().string();

    auto msvcSink = std::make_shared<spdlog::sinks::msvc_sink_st>();

    auto msvcLogger = std::make_shared<spdlog::logger>("msvc", msvcSink);
    msvcLogger->set_level(spdlog::level::trace);

    auto dialySink = std::make_shared<spdlog::sinks::daily_file_sink_st>(logDirectory, 23, 59);
    dialySink->set_level(spdlog::level::err);

    std::shared_ptr<spdlog::sinks::dist_sink_st> combinedLoggers = std::make_shared<spdlog::sinks::dist_sink_st>();

    combinedLoggers->add_sink(msvcSink);
    combinedLoggers->add_sink(dialySink);

    auto logger = std::make_shared<spdlog::logger>("taskies-logger", combinedLoggers);
    logger->flush_on(spdlog::level::err);
    logger->enable_backtrace(32);

    pLogger = logger;
}
} // namespace app
