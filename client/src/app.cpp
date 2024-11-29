#include "../include/app.h"
#include "../include/main.h"
#include "wx/wx.h"

wxIMPLEMENT_APP(App);

bool App::OnInit() {
    Client* mainFrame = new Client(); 
    mainFrame->MainFrame("Chat App"); 
    mainFrame->SetClientSize(800, 600);
    mainFrame->Center();
    mainFrame->Show();
    return true;
}
