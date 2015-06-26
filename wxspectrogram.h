#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/rawbmp.h>

#include "inputsource.h"

class wxSpectrogram : public wxScrolled<wxPanel>
{
private:
    InputSource *input_source = nullptr;
    float *input_data = nullptr;
    std::unique_ptr<wxBitmap> image;

public:
    wxSpectrogram(wxFrame *parent, InputSource *input_source);
    ~wxSpectrogram();

    void AllocateBuffers();

    void OnDraw(wxDC &dc);
    void OnMouseWheel(wxMouseEvent &event);
    void OnSize(wxSizeEvent &event);

    wxDECLARE_EVENT_TABLE();
};