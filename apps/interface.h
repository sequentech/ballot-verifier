#ifndef INTERFACE_H
#define INTERFACE_H
// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/wizard.h>
#include <string>

using namespace std;

namespace AgoraAirgap {

class MyApp: public wxApp
{
public:
  virtual bool OnInit();
};

class MainFrame: public wxFrame
{
public:
  MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
private:
  wxTextCtrl* console_text;
  wxTextCtrl* ballot_text;
  wxStaticText* state_text;
  void OnVerify(wxCommandEvent& event);
  void OnAdvancedSettings(wxCommandEvent& event);
  void OnClose(wxCloseEvent& event);
  void OnBallotClick(wxMouseEvent& event );
  void OnBallotLostFocus(wxFocusEvent& e);
  wxDECLARE_EVENT_TABLE();
};

class AdvancedSettingsFrame: public wxFrame
{
public:
  AdvancedSettingsFrame(wxFrame* parent, const wxString& title, const wxPoint& pos, const wxSize& size);
private:
};


enum
{
    ID_Hello = 1,
    ID_Audit,
    ID_Encrypt,
    ID_A,
    ID_B,
    ID_BALLOT_PATH_TXT,
    ID_BALLOT_PATH_BUTTON,
    ID_BALLOT_PATH_LABEL,
    ID_ELECTION_PATH_TXT,
    ID_ELECTION_PATH_BUTTON,
    ID_ELECTION_PATH_LABEL,
    ID_VERIFY_ONLINE,
    ID_DOWNLOAD_ELECTION,
    ID_VERIFY_OFFLINE,
    ID_ADVANCED_SETTINGS,
    ID_VERIFY_ON_AIRGAP
};

} // using namespace AgoraAirgap;

using namespace AgoraAirgap;

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_BUTTON(ID_VERIFY_ONLINE,      MainFrame::OnVerify)
    EVT_BUTTON(ID_ADVANCED_SETTINGS,  MainFrame::OnAdvancedSettings)
    EVT_CLOSE(MainFrame::OnClose)
wxEND_EVENT_TABLE()
wxIMPLEMENT_APP(MyApp);

#endif //INTERFACE_H
