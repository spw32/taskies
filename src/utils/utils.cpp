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

#include "utils.h"

#include <chrono>

#include <date/date.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif // _WIN32

namespace app::Utils
{
#ifdef _WIN32
std::string ToStdString(const std::wstring& input)
{
    int size = WideCharToMultiByte(CP_UTF8, 0, input.data(), static_cast<int>(input.size()), NULL, 0, NULL, NULL);
    std::string result(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, &input[0], (int) input.size(), &result[0], size, NULL, NULL);
    return result;
}
#endif // _WIN32

std::int64_t UnixTimestamp()
{
    auto tp = std::chrono::system_clock::now();
    auto dur = tp.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(dur).count();
    return seconds;
}

std::string ToISODateTime(std::int64_t unixTimestamp)
{
    auto now = std::chrono::system_clock::now();
    auto date = date::format("%F %T", date::floor<std::chrono::seconds>(now));
    return date;
}

int VoidPointerToInt(void* value)
{
    intptr_t p = reinterpret_cast<intptr_t>(value);
    return static_cast<int>(p);
}

void* IntToVoidPointer(int value)
{
    intptr_t p = static_cast<intptr_t>(value);
    return reinterpret_cast<void*>(p);
}
} // namespace app::Utils
