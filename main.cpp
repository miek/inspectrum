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
    float *image_data;

public:
    wxImagePanel(wxFrame *parent, float *image_data);

    void paintEvent(wxPaintEvent &evt);
    void paintNow();

    void OnDraw(wxDC &dc);

    wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(wxImagePanel, wxScrolled<wxPanel>)
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

wxImagePanel::wxImagePanel(wxFrame *parent, float *image_data) : wxScrolled<wxPanel>(parent)
{
    this->image_data = image_data;
    SetScrollRate(10, 10);
    SetVirtualSize(FFT_SIZE, 25885879);
}

void wxImagePanel::OnDraw(wxDC &dc)
{
    int x, y;
    int xunit, yunit;
    int width, height;
    GetViewStart(&x, &y);
    GetScrollPixelsPerUnit(&xunit, &yunit);
    GetClientSize(&width, &height);

    x *= xunit;
    y *= yunit;

    wxBitmap image(FFT_SIZE, height, 24);
    wxNativePixelData data(image);
    wxNativePixelData::Iterator pix(data);

    int row_start = y * FFT_SIZE;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < FFT_SIZE; j++) {
            pix.Blue() = image_data[row_start + j] * -2;
            pix.Red() = 0;
            pix.Green() = 0;
            pix++;
        }
        row_start += FFT_SIZE;
    }

    dc.DrawBitmap(image, 0, y, false);
}


bool MyApp::OnInit()
{
    wxInitAllImageHandlers();
    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    MyFrame *frame = new MyFrame( "merry supernova", wxPoint(50, 50), wxSize(1024, 500) );

    int fd;
    fftwf_complex *input;
    float *output;

    fd = open("/home/mike/projects/hubsan/name-f2.431000e+09-s2.000000e+07-t20150524002153.cfile", O_RDONLY);
    input = (fftwf_complex*)mmap(NULL, 2070870320, PROT_READ, MAP_SHARED, fd, 0);
    output = (float*)malloc(258858790 * sizeof(float));

    fftwf_complex *in, *out;
    fftwf_plan p;

    in = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * FFT_SIZE);
    out = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * FFT_SIZE);
    p = fftwf_plan_dft_1d(FFT_SIZE, in, out, FFTW_FORWARD, FFTW_MEASURE);

    float *output_ptr = output;
    for (int i = 0; i < 25885879 - FFT_SIZE; i += FFT_SIZE) {
        memcpy(in, input[i], sizeof(fftwf_complex) * FFT_SIZE);
        fftwf_execute(p);

        for (int j = 0; j < FFT_SIZE; j++) {
            float re, im, mag, magdb;
            re = out[j][0];
            im = out[j][1];
            mag = sqrt(re * re + im * im) / FFT_SIZE;
            magdb = 10 * log(mag);
            *output_ptr = magdb;
            output_ptr++;
        }
    }

    fftwf_destroy_plan(p);
    fftwf_free(in); fftwf_free(out);

    wxImagePanel *impanel = new wxImagePanel(frame, output);
    sizer->Add(impanel, 1, wxALL | wxEXPAND, 0);
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