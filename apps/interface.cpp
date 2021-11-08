// SPDX-FileCopyrightText: 2014 Félix Robles <felrobelv@gmail.com>
// SPDX-FileCopyrightText: 2021 Eduardo Robles <edulix@nvotes.com>
//
// SPDX-License-Identifier: AGPL-3.0-only

#include "interface.h"

#include <agora-airgap/encrypt.h>
#include <agora-airgap/screen.h>
#include <agora-airgap/sha256.h>
#include <wx/filename.h>
#include <wx/generic/statbmpg.h>

using namespace AgoraAirgap;

bool MyApp::OnInit()
{
    MainFrame * frame =
        new MainFrame("Audit Your Ballot", wxPoint(50, 50), wxSize(450, 840));
    frame->Show(true);
    return true;
}

void MainFrame::OnBallotClick(wxMouseEvent & event)
{
    event.Skip(true);
    string ballot = ballot_text->GetValue().ToStdString();
    if (0 == ballot.compare(string("Paste your ballot here")))
    {
        ballot_text->Clear();
    }
}

void MainFrame::OnBallotLostFocus(wxFocusEvent & e)
{
    e.Skip(true);
    string ballot = ballot_text->GetValue().ToStdString();
    if (0 == ballot.length())
    {
        ballot_text->WriteText(string("Paste your ballot here"));
    }
}

MainFrame::MainFrame(
    const wxString & title,
    const wxPoint & pos,
    const wxSize & size)
    : wxFrame(NULL, wxID_ANY, title, pos, size)
{
    wxBoxSizer * overSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer * mainSizer = new wxBoxSizer(wxHORIZONTAL);

    wxBoxSizer * leftSizer = new wxBoxSizer(wxVERTICAL);

    ballot_text = new wxTextCtrl(
        this, -1, wxT(""), wxDefaultPosition, wxSize(220, 250), wxTE_MULTILINE);

    ballot_text->SetEditable(true);

    *ballot_text << wxString(string("Paste your ballot here"));

    ballot_text->Connect(
        wxEVT_LEFT_UP,
        wxMouseEventHandler(MainFrame::OnBallotClick),
        NULL,
        this);
    ballot_text->Connect(
        wxEVT_KILL_FOCUS,
        wxFocusEventHandler(MainFrame::OnBallotLostFocus),
        NULL,
        this);

    leftSizer->Add(ballot_text, wxSizerFlags(5).Expand().Border(wxALL, 5));

    mainSizer->Add(
        leftSizer,
        wxSizerFlags(1).Left().Border(wxALL, 5));  // set border width to 10

    wxBoxSizer * rightSizer = new wxBoxSizer(wxVERTICAL);

    // rightSizer->Add(new wxButton( this, ID_ADVANCED_SETTINGS, "Advanced
    // settings", wxDefaultPosition, wxSize(400,40)),
    // wxSizerFlags(0).Expand().Border(wxALL, 5));

    rightSizer->Add(
        new wxStaticText(
            this,
            -1,
            wxT("Get the ballot from the voting booth, which looks "
                "like in the picture below. Copy the full text from "
                "the voting booth and paste it on the right white "
                "box that says 'Paste your ballot here'"),
            wxDefaultPosition,
            wxSize(400, 50)),
        wxSizerFlags(0).Top().Border(wxALL, 5));

    wxInitAllImageHandlers();

    wxBitmap bmp = wxBITMAP_PNG_FROM_DATA(
        screen);  //( wxT("screen.png"), wxBITMAP_TYPE_ANY );
    bmp.SetHeight(165);
    bmp.SetWidth(400);

    wxGenericStaticBitmap * sb = new wxGenericStaticBitmap(
        this, -1, bmp, wxDefaultPosition, wxSize(400, 165));

    rightSizer->Add(sb, wxSizerFlags(0).Left().Border(wxALL, 5));

    state_text = new wxStaticText(
        this, -1, wxT("State: -"), wxDefaultPosition, wxSize(100, 20));

    rightSizer->Add(state_text, wxSizerFlags(0).Left().Border(wxALL, 5));

    mainSizer->Add(
        rightSizer,
        wxSizerFlags(1).Expand().Border(wxALL, 5));  // set border width to 10

    overSizer->Add(mainSizer, wxSizerFlags(0).Expand().Border(wxALL, 0));

    overSizer->Add(
        new wxButton(
            this,
            ID_VERIFY_ONLINE,
            "Verify Ballot",
            wxDefaultPosition,
            wxSize(220, 40)),
        wxSizerFlags(0).Expand().Border(wxALL, 5));

    overSizer->Add(
        new wxStaticText(
            this, -1, wxT("Details:"), wxDefaultPosition, wxSize(80, 20)),
        wxSizerFlags(0).Top().Border(wxALL, 5));

    console_text = new wxTextCtrl(
        this, -1, "", wxDefaultPosition, wxSize(440, 300), wxTE_MULTILINE);

    console_text->SetEditable(false);

    overSizer->Add(console_text, wxSizerFlags(5).Expand().Border(wxALL, 5));

    overSizer->SetSizeHints(this);
    SetSizer(overSizer);  // use the sizer for layout and size window
                          // accordingly and prevent it from being resized
                          // to smaller size
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
void MainFrame::OnAdvancedSettings(wxCommandEvent & event)
#pragma clang diagnostic pop
{
    AdvancedSettingsFrame * frame = new AdvancedSettingsFrame(
        this, "Advanced Settings", wxPoint(100, 100), wxSize(350, 400));
    frame->Show(true);
}

AdvancedSettingsFrame::AdvancedSettingsFrame(
    wxFrame * parent,
    const wxString & title,
    const wxPoint & pos,
    const wxSize & size)
    : wxFrame(parent, wxID_ANY, title, pos, size)
{
    wxBoxSizer * main = new wxBoxSizer(wxVERTICAL);
    main->Add(
        new wxButton(
            this,
            ID_VERIFY_ON_AIRGAP,
            "Verify on airgap computer",
            wxDefaultPosition,
            wxSize(350, 40)),
        wxSizerFlags(0).Expand().Border(wxALL, 5));
    main->SetSizeHints(this);
    SetSizer(main);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
void MainFrame::OnVerify(wxCommandEvent & event)
#pragma clang diagnostic pop
{
    string ballot = ballot_text->GetValue().ToStdString();
    stringstream sstext;

    bool passed = false;
    try
    {
        download_audit_text(sstext, ballot);
        passed = true;
    } catch (runtime_error & e)
    {
        sstext.str(string());
        sstext << e.what();
        state_text->SetLabelText("State: ERROR");
    } catch (exception & e)
    {
        sstext.str(string());
        sstext << e.what();
        state_text->SetLabelText("State: ERROR");
    }
    sstext.flush();
    console_text->Clear();
    console_text->WriteText(wxString(sstext.str().c_str(), wxConvUTF8));
    if (passed)
    {
        state_text->SetLabelText("State: VERIFIED");
    }
}
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
void MainFrame::OnClose(wxCloseEvent & event) { Destroy(); }
#pragma clang diagnostic pop
