#pragma once
#include "wx/wx.h"

class Client : public wxFrame
{
public:
    void MainFrame(const wxString& title); 
    void sendMsg(wxCommandEvent& evt, SOCKET clientSocket, std::string Name, std::string input, wxTextCtrl* object);
};
