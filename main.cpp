#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include "inputsource.h"
#include "wximagepanel.h"

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
    void OnExit(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(wxID_EXIT,  MyFrame::OnExit)
wxEND_EVENT_TABLE()

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
    wxInitAllImageHandlers();
    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    MyFrame *frame = new MyFrame( "inspectrum", wxPoint(50, 50), wxSize(1024, 500) );

    if (argc < 2)
    {
        printf("Missing filename\n");
        printf("Usage: %s <filename> [fft size]\n", argv[0].mb_str().data());
        return false;
    }

    int fft_size = 1024;
    if (argc > 2) {
        int size = atoi(argv[2]);
        if (size > 0)
            fft_size = size;
    }

    InputSource *is = new InputSource(argv[1], fft_size);

    wxImagePanel *impanel = new wxImagePanel(frame, is);
    sizer->Add(impanel, 1, wxALL | wxEXPAND, 0);
    frame->SetSizer(sizer);

    frame->Show( true );
    return true;
}

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
        : wxFrame(NULL, wxID_ANY, title, pos, size)
{
}

void MyFrame::OnExit(wxCommandEvent& event)
{
    Close( true );
}
