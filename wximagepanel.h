#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/rawbmp.h>

#include "inputsource.h"

class wxImagePanel : public wxScrolled<wxPanel>
{
private:
    InputSource *input_source = nullptr;
    float *input_data = nullptr;
    std::unique_ptr<wxBitmap> image;

public:
    wxImagePanel(wxFrame *parent, InputSource *input_source);
    ~wxImagePanel();

    void AllocateBuffers();

    void OnDraw(wxDC &dc);
    void OnMouseWheel(wxMouseEvent &event);
    void OnSize(wxSizeEvent &event);

    wxDECLARE_EVENT_TABLE();
};