#include "../include/main.h"
#include "../include/app.h"
#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <string>
#include <thread>
#include <vector>
#include <wx/wx.h>           
#include <wx/textdlg.h>     
#include <wx/colour.h>        
#include <wx/region.h>       
#include <wx/image.h>         
#include <wx/bitmap.h>        
#include <wx/bmpbuttn.h>
#include <wx/event.h>
#include <wx/richtext/richtextbuffer.h>



#pragma comment(lib, "WS2_32")

void reciveMsg(SOCKET clientSocket, wxTextCtrl* object);

void Client::MainFrame(const wxString& title) {
    wxImage::AddHandler(new wxPNGHandler());
    wxString name;
    wxTextEntryDialog textdlg(this, _("Enter name"));
    if (textdlg.ShowModal() == wxID_OK)
    {
          name = textdlg.GetValue();
    }
    wxColor* backgroundColor = new wxColor(137, 168, 178);
    wxColor* messegeBackgroundColor = new wxColor(179, 200, 207);
    wxColor* inputField = new wxColor(179, 200, 207);
    wxWindow::SetForegroundColour(*wxWHITE);
    Create(nullptr, wxID_ANY, title + " | " + name, wxDefaultPosition, wxSize(800, 600));
    Show(true);
    this->SetBackgroundColour(*backgroundColor);
    wxBoxSizer* main = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* messegesSizer = new wxBoxSizer(wxHORIZONTAL);
    wxTextCtrl* msgDisplay = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
         wxTE_MULTILINE | wxTE_READONLY | wxHSCROLL | wxTE_AUTO_URL | wxTE_MULTILINE | wxNO_BORDER);
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxTextCtrl* textField = new wxTextCtrl(this, wxID_ANY,"", wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTE_MULTILINE | wxTE_WORDWRAP | wxTE_PROCESS_ENTER | wxTE_RICH);
    wxBitmap originalBitmap("sendIcon.png", wxBITMAP_TYPE_PNG);
    wxImage scaledImage = originalBitmap.ConvertToImage().Rescale(50, 50, wxIMAGE_QUALITY_HIGH);
    wxBitmapButton* sendButton = new wxBitmapButton(this, wxID_ANY, wxBitmap(scaledImage), wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxBU_AUTODRAW);    
    int dynamicFontSize = textField->GetSize().GetHeight() * 0.9;
    wxFont dynamicFont = wxFont(dynamicFontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    textField->SetFont(dynamicFont);
    msgDisplay->SetFont(dynamicFont);
    msgDisplay->SetMargins(5);
    textField->SetMargins(5);
    msgDisplay->SetBackgroundColour(*messegeBackgroundColor);
    textField->SetBackgroundColour(*inputField);
    sendButton->SetBackgroundColour(*inputField);
    buttonSizer->Add(textField,4, wxEXPAND);
    buttonSizer->Add(sendButton, 1, wxEXPAND);
    messegesSizer->Add(msgDisplay, 4, wxEXPAND);
    main->Add(messegesSizer, 4, wxEXPAND | wxALL, 20);
    main->Add(buttonSizer, 1, wxEXPAND | wxALL, 20);
    SetSizerAndFit(main);

    //init dlls and shi
    WSADATA wsadata;
    int wsaerr;
    WORD wVersionRequested = MAKEWORD(2, 2);
    wsaerr = WSAStartup(wVersionRequested, &wsadata);
    if (wsaerr != 0) {
        exit(0);
    }
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        WSACleanup();
        exit(0);
    }
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(4369); // random port number cuz why not
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    int connectResult = connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if (connectResult == SOCKET_ERROR) {
        closesocket(clientSocket);
        WSACleanup();
        exit(0);
    }
    std::string nameString = std::string(name);
    send(clientSocket, nameString.c_str(), sizeof(nameString.c_str()), 0);

    //threads for reciving and sending msgs
    // thread go vroom
    std::thread t_recv([clientSocket, msgDisplay]() {
        while (true) {
            reciveMsg(clientSocket, msgDisplay);
        }
        });

    sendButton->Bind(wxEVT_BUTTON, [this, clientSocket, textField, name, msgDisplay](wxCommandEvent& event) {
        std::string input = std::string(textField->GetValue().ToStdString());
        sendMsg(event, clientSocket, std::string(name), input, msgDisplay);
        textField->Clear();
        });

    textField->Bind(wxEVT_TEXT_ENTER, [this, clientSocket, textField, name, msgDisplay](wxCommandEvent& event) {
        std::string input = std::string(textField->GetValue().ToStdString());
        sendMsg(event, clientSocket, std::string(name), input, msgDisplay);
        textField->Clear();
        });

    t_recv.detach();

    Bind(wxEVT_SIZE, [this, textField, msgDisplay](wxSizeEvent& event) {
        int dynamicFontSize = textField->GetSize().GetHeight() * 0.3;
        wxFont dynamicFont = wxFont(dynamicFontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
        textField->SetFont(dynamicFont);
        msgDisplay->SetFont(dynamicFont);
        event.Skip();
        });

    

    Bind(wxEVT_CLOSE_WINDOW, [clientSocket, name](wxCloseEvent& event) {
        std::string NameString = std::string(name) + " has left";
        send(clientSocket, NameString.c_str(), NameString.length(), 0);
        closesocket(clientSocket);
        WSACleanup();
        event.Skip();
        });
}

void reciveMsg(SOCKET clientSocket, wxTextCtrl* object) {
    char buffer[1024] = { 0 };

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(clientSocket, &readfds);

    timeval timeout;
    timeout.tv_sec = 1;  
    timeout.tv_usec = 0;

    int selectResult = select(0, &readfds, NULL, NULL, &timeout);
    if (selectResult > 0) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            if (bytesReceived > 0 && buffer[bytesReceived - 1] == '\n') {
                buffer[bytesReceived - 1] = '\0';
            }
            object->AppendText(std::string(buffer) + "\n");
        }
        else if (bytesReceived == 0) {
            // connection go boom
        }
        else {
            // error.mp4
        }
    }
}

void Client::sendMsg(wxCommandEvent& evt, SOCKET clientSocket, std::string Name, std::string input, wxTextCtrl* object) {
    if (input.empty()) {
        return;
    }
    std::string message = Name + ": " + input;
    const char* message_c_str = message.c_str();
    int sendResult = send(clientSocket, message_c_str, strlen(message_c_str), 0);
    if (sendResult == SOCKET_ERROR) {
        //error
    }
    object->SetDefaultStyle(wxTextAttr(*wxBLUE));
    object->AppendText("You: ");
    object->SetDefaultStyle(wxTextAttr(*wxBLACK));
    object->AppendText(std::string(input));
}