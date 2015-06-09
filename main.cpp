#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <sys/mman.h>
#include <fftw3.h>

#define FFT_SIZE 1024

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
    MyFrame *frame = new MyFrame( "Hello World", wxPoint(50, 50), wxSize(450, 340) );
    frame->Show( true );
    return true;
}

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
        : wxFrame(NULL, wxID_ANY, title, pos, size)
{
    int fd;
    fftwf_complex *input;

    fd = open("/home/mike/projects/hubsan/name-f2.431000e+09-s2.000000e+07-t20150524002153.cfile", O_RDONLY);
    input = (fftwf_complex*)mmap(NULL, 2022340, PROT_READ, MAP_SHARED, fd, 0);

    fftwf_complex *in, *out;
    fftwf_plan p;

    in = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * FFT_SIZE);
    out = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * FFT_SIZE);
    p = fftwf_plan_dft_1d(FFT_SIZE, in, out, FFTW_FORWARD, FFTW_MEASURE);

    for (int i = 0; i < 252792 - FFT_SIZE; i += FFT_SIZE) {
        memcpy(in, input[i], sizeof(fftwf_complex) * FFT_SIZE);
        fftwf_execute(p);
    }

    printf("done\n");

    fftwf_destroy_plan(p);
    fftwf_free(in); fftwf_free(out);

    CreateStatusBar();
    SetStatusText( tmpnam(0) );
}

void MyFrame::OnExit(wxCommandEvent& event)
{
    Close( true );
}