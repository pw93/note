#include <wx/wx.h>
#include <wx/filedlg.h>
#include <wx/image.h>
#include <wx/bitmap.h>
#include <wx/statbmp.h>

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};

class MyFrame : public wxFrame
{
public:
    MyFrame();

private:
    wxStaticBitmap* m_display;
    wxImage m_image;

    void OnOpen(wxCommandEvent& event);
    void OnHistogramEqualize(wxCommandEvent& event);
    void UpdateDisplay();

    wxDECLARE_EVENT_TABLE();
};

// Event IDs
enum
{
    ID_Open = wxID_HIGHEST + 1,
    ID_HistEqualize
};

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(ID_Open, MyFrame::OnOpen)
    EVT_MENU(ID_HistEqualize, MyFrame::OnHistogramEqualize)
wxEND_EVENT_TABLE()

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
    wxInitAllImageHandlers(); // enable PNG, JPG, etc.
    MyFrame* frame = new MyFrame();
    frame->Show(true);
    return true;
}

MyFrame::MyFrame()
    : wxFrame(NULL, wxID_ANY, "Image Viewer with Histogram Equalizer", wxDefaultPosition, wxSize(800, 600))
{
    // Menu bar
    wxMenu* fileMenu = new wxMenu;
    fileMenu->Append(ID_Open, "&Open...\tCtrl+O");

    wxMenu* editMenu = new wxMenu;
    editMenu->Append(ID_HistEqualize, "&Histogram Equalize\tCtrl+H");

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, "&File");
    menuBar->Append(editMenu, "&Edit");
    SetMenuBar(menuBar);

    // Image display
    m_display = new wxStaticBitmap(this, wxID_ANY, wxBitmap(800, 600));
}

void MyFrame::OnOpen(wxCommandEvent& WXUNUSED(event))
{
    wxFileDialog openFile(this, "Open Image", "", "",
                          "Images (*.png;*.jpg;*.bmp)|*.png;*.jpg;*.bmp",
                          wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openFile.ShowModal() == wxID_CANCEL)
        return;

    if (m_image.LoadFile(openFile.GetPath()))
    {
        UpdateDisplay();
    }
}
void MyFrame::OnHistogramEqualize(wxCommandEvent& WXUNUSED(event))
{
    if (!m_image.IsOk())
        return;

    int w = m_image.GetWidth();
    int h = m_image.GetHeight();
    int size = w * h;

    unsigned char* data = m_image.GetData();

    // Convert RGB to YCbCr
    std::vector<unsigned char> Y(size);
    std::vector<unsigned char> Cb(size);
    std::vector<unsigned char> Cr(size);

    for (int i = 0; i < size; i++)
    {
        int r = data[3*i];
        int g = data[3*i+1];
        int b = data[3*i+2];

        // ITU-R BT.601 standard
        Y[i]  = static_cast<unsigned char>(0.299*r + 0.587*g + 0.114*b);
        Cb[i] = static_cast<unsigned char>(128 - 0.168736*r - 0.331264*g + 0.5*b);
        Cr[i] = static_cast<unsigned char>(128 + 0.5*r - 0.418688*g - 0.081312*b);
    }

    // Histogram equalization on Y
    int hist[256] = {0};
    for (int i = 0; i < size; i++) hist[Y[i]]++;

    int cdf[256] = {0};
    cdf[0] = hist[0];
    for (int i = 1; i < 256; i++) cdf[i] = cdf[i-1] + hist[i];

    unsigned char lut[256];
    for (int i = 0; i < 256; i++)
        lut[i] = static_cast<unsigned char>((cdf[i] - cdf[0]) * 255.0 / (size - 1) + 0.5);

    for (int i = 0; i < size; i++)
        Y[i] = lut[Y[i]];

    // Convert back YCbCr → RGB
    for (int i = 0; i < size; i++)
    {
        int y = Y[i];
        int cb = Cb[i] - 128;
        int cr = Cr[i] - 128;

        int r = y + 1.402 * cr;
        int g = y - 0.344136 * cb - 0.714136 * cr;
        int b = y + 1.772 * cb;

        data[3*i]   = std::clamp(r, 0, 255);
        data[3*i+1] = std::clamp(g, 0, 255);
        data[3*i+2] = std::clamp(b, 0, 255);
    }

    UpdateDisplay();
}


void MyFrame::UpdateDisplay()
{
    wxBitmap bitmap(m_image);
    m_display->SetBitmap(bitmap);
    Layout();
}
