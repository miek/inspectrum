#include "wximagepanel.h"

wxBEGIN_EVENT_TABLE(wxImagePanel, wxScrolled<wxPanel>)
    EVT_MOUSEWHEEL(wxImagePanel::OnMouseWheel)
    EVT_SIZE(wxImagePanel::OnSize)
wxEND_EVENT_TABLE()

wxImagePanel::wxImagePanel(wxFrame *parent, InputSource *input_source) : wxScrolled<wxPanel>(parent)
{
    this->input_source = input_source;
    SetVirtualSize(input_source->GetWidth(), input_source->GetHeight());
    AllocateBuffers();
    SetScrollRate(10, 10);
}

wxImagePanel::~wxImagePanel()
{
    free(input_data);
}

void wxImagePanel::AllocateBuffers()
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

void wxImagePanel::OnDraw(wxDC &dc)
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

void wxImagePanel::OnMouseWheel(wxMouseEvent &event)
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

void wxImagePanel::OnSize(wxSizeEvent &event)
{
    AllocateBuffers();
}
