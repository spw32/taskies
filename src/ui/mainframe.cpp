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

#include "mainframe.h"

#include <wx/persist/toplevel.h>

#include "../common/common.h"
#include "../core/environment.h"
#include "../core/configuration.h"

namespace app::UI
{
// clang-format off
wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(std::shared_ptr<Core::Environment> env,
    std::shared_ptr<Core::Configuration> cfg,
    std::shared_ptr<spdlog::logger> logger,
    const wxString& name)
// clang-format on
{
    if (!wxPersistenceManager::Get().RegisterAndRestore(this)) {
        SetSize(FromDIP(wxSize(800, 600)));
    }

    wxIconBundle iconBundle("TASKIES_ICO", 0);
    SetIcons(iconBundle);
    Create();
}

bool MainFrame::Create()
{
    return false;
}

void MainFrame::CreateControls() {
    /* Menu Controls */
    /* Menubar */
    /* File */
    auto fileMenu = new wxMenu();
    fileMenu->AppendSeparator();

    auto exitMenuItem = fileMenu->Append(wxID_EXIT, "Exit", "Exit the program");

    /* Menu bar */
    auto menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, "File");

    SetMenuBar(menuBar);

    auto mainPanel = new wxPanel(this, wxID_ANY);

    auto mainSizer = new wxBoxSizer(wxVERTICAL);
    mainPanel->SetSizer(mainSizer);

    auto starterLabel = new wxStaticText(mainPanel, wxID_ANY, "wxWidgets Quick Starter");
    mainSizer->Add(starterLabel, wxSizerFlags().Center());
}
} // namespace app::UI
