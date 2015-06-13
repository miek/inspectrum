#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/rawbmp.h>

#include <cstdlib>
#include <sys/mman.h>
#include <fftw3.h>

#define FFT_SIZE 1024

class wxImagePanel : public wxScrolled<wxPanel>
{
private:
    wxBitmap *image;

public:
    wxImagePanel(wxFrame *parent, wxBitmap *image);

    void paintEvent(wxPaintEvent &evt);
    void paintNow();

    void render(wxDC &dc);

    wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(wxImagePanel, wxScrolled<wxPanel>)
    EVT_PAINT(wxImagePanel::paintEvent)
wxEND_EVENT_TABLE()

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

wxImagePanel::wxImagePanel(wxFrame *parent, wxBitmap *image) : wxScrolled<wxPanel>(parent)
{
    this->image = image;
}

void wxImagePanel::paintEvent(wxPaintEvent &evt)
{
    wxPaintDC dc(this);
    render(dc);
}

void wxImagePanel::paintNow()
{
    wxClientDC dc(this);
    render(dc);
}

void wxImagePanel::render(wxDC &dc)
{
    dc.DrawBitmap(*image, 0, 0, false);
}

wxBitmap image(FFT_SIZE, 252792, 24);

bool MyApp::OnInit()
{
    wxInitAllImageHandlers();

    MyFrame *frame = new MyFrame( "Hello World", wxPoint(50, 50), wxSize(1024, 500) );

    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);

    int fd;
    fftwf_complex *input;

    fd = open("/home/mike/projects/hubsan/name-f2.431000e+09-s2.000000e+07-t20150524002153.cfile", O_RDONLY);
    input = (fftwf_complex*)mmap(NULL, 2022340, PROT_READ, MAP_SHARED, fd, 0);

    fftwf_complex *in, *out;
    fftwf_plan p;

    in = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * FFT_SIZE);
    out = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * FFT_SIZE);
    p = fftwf_plan_dft_1d(FFT_SIZE, in, out, FFTW_FORWARD, FFTW_MEASURE);

    wxNativePixelData data(image);
    wxNativePixelData::Iterator pix(data);
    for (int i = 0; i < 252792 - FFT_SIZE; i += FFT_SIZE) {
        memcpy(in, input[i], sizeof(fftwf_complex) * FFT_SIZE);
        fftwf_execute(p);

        for (int j = 0; j < FFT_SIZE; j++) {
            float re, im, mag, magdb;
            re = out[j][0];
            im = out[j][1];
            mag = sqrt(re * re + im * im) / FFT_SIZE;
            magdb = 10 * log(mag);
            pix.Blue() = magdb;
            pix++;
        }
    }

    fftwf_destroy_plan(p);
    fftwf_free(in); fftwf_free(out);

    wxImagePanel *impanel = new wxImagePanel(frame, &image);
    sizer->Add(impanel, 1, wxEXPAND);

    frame->SetSizer(sizer);

    frame->Show( true );
    return true;
}

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
        : wxFrame(NULL, wxID_ANY, title, pos, size)
{
    CreateStatusBar();
}

void MyFrame::OnExit(wxCommandEvent& event)
{
    Close( true );
}