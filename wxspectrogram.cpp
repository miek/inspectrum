#include "wxspectrogram.h"

wxBEGIN_EVENT_TABLE(wxSpectrogram, wxScrolled<wxPanel>)
    EVT_MOUSEWHEEL(wxSpectrogram::OnMouseWheel)
    EVT_SIZE(wxSpectrogram::OnSize)
wxEND_EVENT_TABLE()

wxSpectrogram::wxSpectrogram(wxFrame *parent, InputSource *input_source) : wxScrolled<wxPanel>(parent)
{
    this->input_source = input_source;
    SetVirtualSize(input_source->GetWidth(), input_source->GetHeight());
    AllocateBuffers();
    SetScrollRate(10, 10);
}

wxSpectrogram::~wxSpectrogram()
{
    free(input_data);
}

void wxSpectrogram::AllocateBuffers()
{
    int width, height;
    GetSize(&width, &height);

    // TODO: maybe change this
    width = input_source->GetWidth();

    input_data = (float*)realloc(input_data, width * height * sizeof(float));

    image.reset(new wxBitmap(width, height, 24));
}

int clamp(int a, int b, int c) {
    if (a < b) return b;
    if (a > c) return c;
    return a;
}

void wxSpectrogram::OnDraw(wxDC &dc)
{
    int x, y;
    int xunit, yunit;
    int width, height;
    GetViewStart(&x, &y);
    GetScrollPixelsPerUnit(&xunit, &yunit);
    GetSize(&width, &height);

    // TODO: maybe change this
    width = input_source->GetWidth();

    x *= xunit;
    y *= yunit;

    input_source->GetViewport(input_data, 0, y, width, height, 0);

    wxNativePixelData pixel_data(*image);
    wxNativePixelData::Iterator pix(pixel_data);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            pix.Blue() = clamp((input_data[i*width + j] + 20) * 5 + 255, 0, 255);
            pix.Red() = 0;
            pix.Green() = 0;
            pix++;
        }
    }

    dc.DrawBitmap(*image, 0, y, false);
}

void wxSpectrogram::OnMouseWheel(wxMouseEvent &event)
{
    if (event.ControlDown() && event.GetWheelRotation() != 0) {
        bool forward = event.GetWheelRotation() > 0;
        int x, y;
        int xunit, yunit;
        int width, height;
        GetViewStart(&x, &y);
        GetScrollPixelsPerUnit(&xunit, &yunit);
        GetClientSize(&width, &height);
        int centre_offset = height / 2 / yunit;
        if (forward) {
            if (input_source->ZoomIn()) {
                Scroll(x, y*2 + centre_offset);
            }
        } else {
            if (input_source->ZoomOut()) {
                Scroll(x, (y - centre_offset)/2);
            }
        }

        SetVirtualSize(input_source->GetWidth(), input_source->GetHeight());
        Refresh();
    } else {
        event.Skip();
    }
}

void wxSpectrogram::OnSize(wxSizeEvent &event)
{
    AllocateBuffers();
}
