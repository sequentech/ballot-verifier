// wxWidgets "Hello world" Program
// For compilers that support precompilation, includes "wx/wx.h".
#if defined(_WIN32)
 # define CURL_STATICLIB 1 
#endif 
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/wizard.h>
#include <string>

using namespace std;

class MyApp: public wxApp
{
public:
  virtual bool OnInit();
};

class MyFrame: public wxFrame
{
public:
  MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
private:
  wxTextCtrl* console_text;
  wxTextCtrl* ballot_text;
  wxStaticText* state_text;
  void OnVerify(wxCommandEvent& event);
  void OnClose(wxCloseEvent& event);
  void OnBallotClick(wxMouseEvent& event );
  void OnBallotLostFocus(wxFocusEvent& e);
  wxDECLARE_EVENT_TABLE();
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
    ID_VERIFY_OFFLINE
};



wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_BUTTON(ID_VERIFY_ONLINE,   MyFrame::OnVerify)
    EVT_CLOSE(MyFrame::OnClose)
wxEND_EVENT_TABLE()
wxIMPLEMENT_APP(MyApp);
