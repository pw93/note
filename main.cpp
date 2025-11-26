#include <wx/wx.h>
#include <wx/image.h>

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};

class MyFrame : public wxFrame
{
public:
    MyFrame();
};

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
    wxInitAllImageHandlers(); // enable PNG, JPG, etc.
    MyFrame *frame = new MyFrame();
    frame->Show(true);
    return true;
}

MyFrame::MyFrame()
    : wxFrame(NULL, wxID_ANY, "Open Image Example", wxDefaultPosition, wxSize(800,600))
{
    wxImage image;
    if(image.LoadFile("test.png")) // make sure test.png exists
    {
        wxBitmap bitmap(image);
        wxStaticBitmap* display = new wxStaticBitmap(this, wxID_ANY, bitmap);
    }
}
