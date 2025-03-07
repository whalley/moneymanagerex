/*************************************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011, 2013 Nikolay & Stefano Giorgio

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *************************************************************************/

#include "mmhelppanel.h"
#include "paths.h"
#include "constants.h"
#include "mmex.h"
#include "mmframe.h"
#include "model/Model_Usage.h"

BEGIN_EVENT_TABLE(mmHelpPanel, wxPanel)
    EVT_BUTTON(wxID_BACKWARD, mmHelpPanel::OnHelpPageBack)
    EVT_BUTTON(wxID_FORWARD, mmHelpPanel::OnHelpPageForward)
END_EVENT_TABLE()

mmHelpPanel::mmHelpPanel(wxWindow *parent, mmGUIFrame* frame, wxWindowID winid
    , const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : m_frame(frame)
{
    Create(parent, winid, pos, size, style, name);
}

bool mmHelpPanel::Create( wxWindow *parent, wxWindowID winid,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create(parent, winid, pos, size, style, name);

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Model_Usage::instance().pageview(this);

    return true;
}

void mmHelpPanel::CreateControls()
{    
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer2);

    wxPanel* itemPanel3 = new wxPanel(this, wxID_ANY
        , wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemBoxSizer2->Add(itemPanel3, 0, wxGROW | wxALL, 5);

    wxBoxSizer* itemBoxSizerHeader = new wxBoxSizer(wxHORIZONTAL);
    itemPanel3->SetSizer(itemBoxSizerHeader);

    wxButton* buttonBack = new wxButton(itemPanel3, wxID_BACKWARD, _t("&Back"));
    wxButton* buttonFordward = new wxButton(itemPanel3, wxID_FORWARD, _t("&Forward"));

    wxStaticText* itemStaticText9 = new wxStaticText(itemPanel3, wxID_ANY
        , mmex::getCaption(_t("Help")));
    itemStaticText9->SetFont(this->GetFont().Larger().Bold());

    itemBoxSizerHeader->Add(buttonBack, 0, wxLEFT, 5);
    itemBoxSizerHeader->Add(buttonFordward, 0, wxLEFT | wxRIGHT, 5);
    itemBoxSizerHeader->Add(itemStaticText9, 0, wxLEFT | wxTOP, 5);

    /**************************************************************************
    Allows help files for a specific language.

    Main default help file name: ./help/index.html
    Default filename names can be found in mmex::getPathDoc(fileIndex)

    Default help files will be used when the language help file are not found.
    **************************************************************************/

    int helpFileIndex = m_frame->getHelpFileIndex();
    const wxString help_file = mmex::getPathDoc(static_cast<mmex::EDocFile>(helpFileIndex));
    m_frame->setHelpFileIndex();
    //wxLogDebug("%s", help_file);
    browser_ = wxWebView::New(this, wxID_ANY, help_file);

    Bind(wxEVT_WEBVIEW_NEWWINDOW, &mmHelpPanel::OnNewWindow, this, browser_->GetId());

    itemBoxSizer2->Add(browser_, 1, wxGROW | wxALL, 1);
}

void mmHelpPanel::sortList()
{
}

void mmHelpPanel::OnHelpPageBack(wxCommandEvent& /*event*/)
{
    if (browser_->CanGoBack())
    {
        browser_->GoBack();
        browser_->SetFocus();
    }
}

void mmHelpPanel::OnHelpPageForward(wxCommandEvent& /*event*/)
{
    if (browser_->CanGoForward() )
    {
        browser_->GoForward();
        browser_->SetFocus();
    }
}

void mmHelpPanel::PrintPage()
{
    browser_->Print();
}

void mmHelpPanel::OnNewWindow(wxWebViewEvent& evt)
{
    const wxString uri = evt.GetURL();
    wxLaunchDefaultBrowser(uri);

    evt.Skip();
}
