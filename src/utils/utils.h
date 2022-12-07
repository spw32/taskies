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

#include <cstdint>
#include <string>

namespace app::Utils
{
#ifdef _WIN32
std::string ToStdString(const std::wstring& input);
#endif // _WIN32

std::int64_t UnixTimestamp();

std::string ToISODateTime(std::int64_t unixTimestamp);

int VoidPointerToInt(void* value);

void* IntToVoidPointer(int value);
} // namespace app::Utils
