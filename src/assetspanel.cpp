/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2015 -2021 Nikolay Akimov
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)

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
 ********************************************************/

#include "assetspanel.h"
#include "assetdialog.h"
#include "attachmentdialog.h"
#include "constants.h"
#include "images_list.h"

#include "model/allmodel.h"
#include <wx/srchctrl.h>

wxBEGIN_EVENT_TABLE(mmAssetsListCtrl, mmListCtrl)
    EVT_RIGHT_DOWN(mmAssetsListCtrl::OnMouseRightClick)
    EVT_LEFT_DOWN(mmAssetsListCtrl::OnListLeftClick)

    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, mmAssetsListCtrl::OnListItemActivated)
    EVT_LIST_ITEM_SELECTED(wxID_ANY,  mmAssetsListCtrl::OnListItemSelected)
    EVT_LIST_END_LABEL_EDIT(wxID_ANY, mmAssetsListCtrl::OnEndLabelEdit)
    EVT_LIST_KEY_DOWN(wxID_ANY,       mmAssetsListCtrl::OnListKeyDown)

    EVT_MENU(MENU_TREEPOPUP_NEW,                  mmAssetsListCtrl::OnNewAsset)
    EVT_MENU(MENU_TREEPOPUP_EDIT,                 mmAssetsListCtrl::OnEditAsset)
    EVT_MENU(MENU_TREEPOPUP_ADDTRANS,             mmAssetsListCtrl::OnAddAssetTrans)
    EVT_MENU(MENU_TREEPOPUP_VIEWTRANS,            mmAssetsListCtrl::OnViewAssetTrans)
    EVT_MENU(MENU_TREEPOPUP_GOTOACCOUNT,          mmAssetsListCtrl::OnGotoAssetAccount)
    EVT_MENU(MENU_TREEPOPUP_DELETE,               mmAssetsListCtrl::OnDeleteAsset)
    EVT_MENU(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, mmAssetsListCtrl::OnOrganizeAttachments)
    EVT_MENU(MENU_ON_DUPLICATE_TRANSACTION,       mmAssetsListCtrl::OnDuplicateAsset)
wxEND_EVENT_TABLE()

const std::vector<ListColumnInfo> mmAssetsListCtrl::LIST_INFO = {
    { LIST_ID_ICON,          true, _n("Icon"),          25,  _FL, false },
    { LIST_ID_ID,            true, _n("ID"),            _WA, _FR, true },
    { LIST_ID_NAME,          true, _n("Name"),          150, _FL, true },
    { LIST_ID_DATE,          true, _n("Date"),          _WH, _FL, true },
    { LIST_ID_TYPE,          true, _n("Type"),          _WH, _FL, true },
    { LIST_ID_VALUE_INITIAL, true, _n("Initial Value"), _WH, _FR, true },
    { LIST_ID_VALUE_CURRENT, true, _n("Current Value"), _WH, _FR, true },
    { LIST_ID_NOTES,         true, _n("Notes"),         450, _FL, true },
};

mmAssetsListCtrl::mmAssetsListCtrl(mmAssetsPanel* cp, wxWindow *parent, wxWindowID winid) :
    mmListCtrl(parent, winid),
    m_panel(cp)
{
    mmThemeMetaColour(this, meta::COLOR_LISTPANEL);

    m_setting_name = "ASSETS";
    o_col_order_prefix = "ASSETS";
    o_col_width_prefix = "ASSETS_COL";
    o_sort_prefix = "ASSETS";
    m_col_info_id = LIST_INFO;
    m_col_id_nr = ListColumnInfo::getListId(LIST_INFO);
    m_sort_col_id = { LIST_ID_DATE };
    createColumns();
}

int mmAssetsListCtrl::getSortIcon(bool asc) const
{
    return asc ? mmAssetsPanel::ICON_UPARROW : mmAssetsPanel::ICON_DOWNARROW;
}

void mmAssetsListCtrl::OnMouseRightClick(wxMouseEvent& event)
{
    if (m_selected_row > -1)
        SetItemState(m_selected_row, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
    int Flags = wxLIST_HITTEST_ONITEM;
    m_selected_row = HitTest(wxPoint(event.m_x, event.m_y), Flags);

    if (m_selected_row >= 0)
    {
        SetItemState(m_selected_row, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(m_selected_row, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
    }
    m_panel->updateExtraAssetData(m_selected_row);
    wxMenu menu;
    menu.Append(MENU_TREEPOPUP_NEW, _tu("&New Asset…"));
    menu.AppendSeparator();
    menu.Append(MENU_ON_DUPLICATE_TRANSACTION, _tu("D&uplicate Asset…"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_ADDTRANS, _tu("&Add Asset Transaction…"));
    menu.Append(MENU_TREEPOPUP_VIEWTRANS, _t("&View Asset Transactions"));
    menu.Append(MENU_TREEPOPUP_GOTOACCOUNT, _tu("&Open Asset Account…"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_EDIT, _tu("&Edit Asset…"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_DELETE, _tu("&Delete Asset…"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, _tu("&Organize Attachments…"));
    if (m_selected_row < 0)
    {
        menu.Enable(MENU_ON_DUPLICATE_TRANSACTION, false);
        menu.Enable(MENU_TREEPOPUP_ADDTRANS, false);
        menu.Enable(MENU_TREEPOPUP_VIEWTRANS, false);
        menu.Enable(MENU_TREEPOPUP_EDIT, false);
        menu.Enable(MENU_TREEPOPUP_DELETE, false);
        menu.Enable(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, false);
    }
    else
    {
        auto asset_account = Model_Account::instance().get(m_panel->m_assets[m_selected_row].ASSETNAME);  // ASSETNAME <=> ACCOUNTNAME
        if (!asset_account) asset_account = Model_Account::instance().get(m_panel->m_assets[m_selected_row].ASSETTYPE);  // ASSETTYPE <=> ACCOUNTNAME
        menu.Enable(MENU_TREEPOPUP_GOTOACCOUNT, asset_account);
        menu.Enable(MENU_TREEPOPUP_VIEWTRANS, asset_account);
    }

    PopupMenu(&menu, event.GetPosition());
}

void mmAssetsListCtrl::OnListLeftClick(wxMouseEvent& event)
{
    int Flags = wxLIST_HITTEST_ONITEM;
    long index = HitTest(wxPoint(event.m_x, event.m_y), Flags);
    if (index == -1)
    {
        m_selected_row = -1;
        m_panel->updateExtraAssetData(m_selected_row);
    }
    event.Skip();
}

wxString mmAssetsListCtrl::OnGetItemText(long item, long col_nr) const
{
    return m_panel->getItem(item, getColId_Nr(col_nr));
}

void mmAssetsListCtrl::OnListItemSelected(wxListEvent& event)
{
    m_selected_row = event.GetIndex();
    m_panel->updateExtraAssetData(m_selected_row);
}

int mmAssetsListCtrl::OnGetItemImage(long item) const
{
    return Model_Asset::type_id(m_panel->m_assets[item]);
}

void mmAssetsListCtrl::OnListKeyDown(wxListEvent& event)
{
    if (event.GetKeyCode() == WXK_DELETE)
    {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_DELETE);
        OnDeleteAsset(evt);
    }
    else
    {
        event.Skip();
    }
}

void mmAssetsListCtrl::OnNewAsset(wxCommandEvent& /*event*/)
{
    mmAssetDialog dlg(this, static_cast<Model_Asset::Data*>(nullptr));
    if (dlg.ShowModal() == wxID_OK)
    {
        doRefreshItems(dlg.m_asset->ASSETID);
        m_panel->m_frame->RefreshNavigationTree();
    }
}

void mmAssetsListCtrl::doRefreshItems(int64 trx_id)
{
    int selectedIndex = m_panel->initVirtualListControl(trx_id);

    long cnt = static_cast<long>(m_panel->m_assets.size());

    if (selectedIndex >= cnt || selectedIndex < 0)
        selectedIndex = getSortAsc() ? cnt - 1 : 0;

    if (cnt>0)
        RefreshItems(0, cnt > 0 ? --cnt : 0);
    else
        selectedIndex = -1;

    if (selectedIndex >= 0 && cnt>0)
    {
        SetItemState(selectedIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(selectedIndex, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
        EnsureVisible(selectedIndex);
    }
    m_selected_row = selectedIndex;
}

void mmAssetsListCtrl::OnDeleteAsset(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0)    return;

    wxMessageDialog msgDlg(this
        , _t("Do you want to delete the asset?")
        , _t("Confirm Asset Deletion")
        , wxYES_NO | wxNO_DEFAULT | wxICON_ERROR);

    if (msgDlg.ShowModal() == wxID_YES)
    {
        const Model_Asset::Data& asset = m_panel->m_assets[m_selected_row];
        Model_Asset::instance().remove(asset.ASSETID);
        mmAttachmentManage::DeleteAllAttachments(Model_Asset::refTypeName, asset.ASSETID);
        Model_Translink::RemoveTransLinkRecords<Model_Asset>(asset.ASSETID);

        m_panel->initVirtualListControl();
        m_selected_row = -1;
        m_panel->updateExtraAssetData(m_selected_row);
    }
}

void mmAssetsListCtrl::OnEditAsset(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0)     return;

    wxListEvent evt(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxID_ANY);
    AddPendingEvent(evt);
}

void mmAssetsListCtrl::OnDuplicateAsset(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0)     return;

    const Model_Asset::Data& asset = m_panel->m_assets[m_selected_row];
    Model_Asset::Data* duplicate_asset = Model_Asset::instance().clone(&asset);

    if (EditAsset(duplicate_asset))
    {
        m_panel->initVirtualListControl();
        doRefreshItems(duplicate_asset->ASSETID);
    }
}

void mmAssetsListCtrl::OnAddAssetTrans(wxCommandEvent& WXUNUSED(event))
{
    if (m_selected_row < 0) return;

    m_panel->AddAssetTrans(m_selected_row);
}

void mmAssetsListCtrl::OnViewAssetTrans(wxCommandEvent& WXUNUSED(event))
{
    if (m_selected_row < 0) return;

    m_panel->ViewAssetTrans(m_selected_row);
}

void mmAssetsListCtrl::OnGotoAssetAccount(wxCommandEvent& WXUNUSED(event))
{
    if (m_selected_row < 0) return;

    m_panel->GotoAssetAccount(m_selected_row);
}

void mmAssetsListCtrl::OnOrganizeAttachments(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0) return;

    wxString RefType = Model_Asset::refTypeName;
    int64 RefId = m_panel->m_assets[m_selected_row].ASSETID;

    mmAttachmentDialog dlg(this, RefType, RefId);
    dlg.ShowModal();

    doRefreshItems(RefId);
}

void mmAssetsListCtrl::OnOpenAttachment(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0) return;

    wxString RefType = Model_Asset::refTypeName;
    int64 RefId = m_panel->m_assets[m_selected_row].ASSETID;

    mmAttachmentManage::OpenAttachmentFromPanelIcon(this, RefType, RefId);
    doRefreshItems(RefId);
}

void mmAssetsListCtrl::OnListItemActivated(wxListEvent& event)
{
    if (m_selected_row < 0)
    {
        m_selected_row = event.GetIndex();
    }
    EditAsset(&(m_panel->m_assets[m_selected_row]));
}

bool mmAssetsListCtrl::EditAsset(Model_Asset::Data* pEntry)
{
    mmAssetDialog dlg(this, pEntry);
    bool edit = true;
    if (dlg.ShowModal() == wxID_OK)
    {
        doRefreshItems(dlg.m_asset->ASSETID);
        m_panel->updateExtraAssetData(m_selected_row);
    }
    else edit = false;

    return edit;
}

void mmAssetsListCtrl::OnColClick(wxListEvent& event)
{
    int col_nr = (event.GetId() == MENU_HEADER_SORT) ? m_sel_col_nr : event.GetColumn();
    if (!isValidColNr(col_nr))
        return;
    int col_id = getColId_Nr(col_nr);
    if (!m_col_info_id[col_id].sortable)
        return;

    if (m_sort_col_id[0] != col_id)
        m_sort_col_id[0] = col_id;
    else if (event.GetId() != MENU_HEADER_SORT)
        m_sort_asc[0] = !m_sort_asc[0];
    updateSortIcon();
    savePreferences();

    int64 trx_id = -1;
    if (m_selected_row>=0) trx_id = m_panel->m_assets[m_selected_row].ASSETID;

    doRefreshItems(trx_id);
}

void mmAssetsListCtrl::OnEndLabelEdit(wxListEvent& event)
{
    if (event.IsEditCancelled()) return;
    Model_Asset::Data* asset = &m_panel->m_assets[event.GetIndex()];
    asset->ASSETNAME = event.m_item.m_text;
    Model_Asset::instance().save(asset);
    RefreshItems(event.GetIndex(), event.GetIndex());
}

BEGIN_EVENT_TABLE(mmAssetsPanel, wxPanel)
    EVT_BUTTON(wxID_NEW, mmAssetsPanel::OnNewAsset)
    EVT_BUTTON(wxID_EDIT, mmAssetsPanel::OnEditAsset)
    EVT_BUTTON(wxID_ADD, mmAssetsPanel::OnAddAssetTrans)
    EVT_BUTTON(wxID_VIEW_DETAILS , mmAssetsPanel::OnViewAssetTrans)
    EVT_BUTTON(wxID_DELETE, mmAssetsPanel::OnDeleteAsset)
    EVT_BUTTON(wxID_FILE, mmAssetsPanel::OnOpenAttachment)
    EVT_BUTTON(wxID_FILE2, mmAssetsPanel::OnMouseLeftDown)
    EVT_MENU(wxID_ANY, mmAssetsPanel::OnViewPopupSelected)
    EVT_SEARCHCTRL_SEARCH_BTN(wxID_FIND, mmAssetsPanel::OnSearchTxtEntered)
END_EVENT_TABLE()

mmAssetsPanel::mmAssetsPanel(mmGUIFrame* frame, wxWindow *parent, wxWindowID winid, const wxString& name)
    : m_frame(frame)
    , m_filter_type(Model_Asset::TYPE_ID(-1))
    , tips_()
{
    Create(parent, winid, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, name);
}

bool mmAssetsPanel::Create(wxWindow *parent
    , wxWindowID winid
    , const wxPoint &pos
    , const wxSize &size
    , long style
    , const wxString &name)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    if (!wxPanel::Create(parent, winid, pos, size, style, name)) return false;

    this->windowsFreezeThaw();

    auto start = wxDateTime::UNow();
    tips_ = _t("MMEX allows fixed assets like cars, houses, land, and others to be tracked. Each asset can have its value appreciate by a certain rate per year, depreciate by a certain rate per year, or not change in value. The total assets are added to your total financial worth.");
    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    initVirtualListControl();
    if (!this->m_assets.empty())
        m_lc->EnsureVisible(this->m_assets.size() - 1);

    this->windowsFreezeThaw();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    Model_Usage::instance().pageview(this, (wxDateTime::UNow() - start).GetMilliseconds().ToLong());

    return true;
}

void mmAssetsPanel::CreateControls()
{
    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer9);

    wxPanel* headerPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition
        , wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
    itemBoxSizer9->Add(headerPanel, g_flagsBorder1V);

    wxBoxSizer* itemBoxSizerVHeader = new wxBoxSizer(wxVERTICAL);
    headerPanel->SetSizer(itemBoxSizerVHeader);

    wxStaticText* itemStaticText9 = new wxStaticText(headerPanel, wxID_STATIC, _t("Assets"));
    itemStaticText9->SetFont(this->GetFont().Larger().Bold());
    itemBoxSizerVHeader->Add(itemStaticText9, g_flagsBorder1V);

    wxBoxSizer* itemBoxSizerHHeader2 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizerVHeader->Add(itemBoxSizerHHeader2);

    m_bitmapTransFilter = new wxButton(headerPanel, wxID_FILE2);
    m_bitmapTransFilter->SetBitmap(mmBitmapBundle(png::TRANSFILTER, mmBitmapButtonSize));
    m_bitmapTransFilter->SetLabel(_t("All"));
    m_bitmapTransFilter->SetMinSize(wxSize(150, -1));
    itemBoxSizerHHeader2->Add(m_bitmapTransFilter, g_flagsBorder1H);

    header_text_ = new wxStaticText(headerPanel, wxID_STATIC, "");
    itemBoxSizerVHeader->Add(header_text_, g_flagsBorder1V);

    /* ---------------------- */

    wxSplitterWindow* itemSplitterWindow10 = new wxSplitterWindow( this, wxID_STATIC,
        wxDefaultPosition, wxSize(200, 200), wxSP_3DBORDER|wxSP_3DSASH|wxNO_BORDER);

    m_lc = new mmAssetsListCtrl(this, itemSplitterWindow10, wxID_ANY);

    wxVector<wxBitmapBundle> images;
    images.push_back(mmBitmapBundle(png::PROPERTY));
    images.push_back(mmBitmapBundle(png::CAR));
    images.push_back(mmBitmapBundle(png::HOUSEHOLD_OBJ));
    images.push_back(mmBitmapBundle(png::ART));
    images.push_back(mmBitmapBundle(png::JEWELLERY));
    images.push_back(mmBitmapBundle(png::CASH));
    images.push_back(mmBitmapBundle(png::OTHER));
    images.push_back(mmBitmapBundle(png::UPARROW));
    images.push_back(mmBitmapBundle(png::DOWNARROW));

    m_lc->SetSmallImages(images);

    wxPanel* assets_panel = new wxPanel(itemSplitterWindow10, wxID_ANY
        , wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
    mmThemeMetaColour(assets_panel, meta::COLOR_LISTPANEL);

    itemSplitterWindow10->SplitHorizontally(m_lc, assets_panel);
    itemSplitterWindow10->SetMinimumPaneSize(100);
    itemSplitterWindow10->SetSashGravity(1.0);
    itemBoxSizer9->Add(itemSplitterWindow10, g_flagsExpandBorder1);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    assets_panel->SetSizer(itemBoxSizer4);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(itemBoxSizer5, g_flagsBorder1V);

    wxButton* itemButton6 = new wxButton( assets_panel, wxID_NEW, _t("&New "));
    mmToolTip(itemButton6, _t("New Asset"));
    itemBoxSizer5->Add(itemButton6, 0, wxRIGHT, 5);

    wxButton* add_trans_btn = new wxButton(assets_panel, wxID_ADD, _t("&Add Trans "));
    mmToolTip(add_trans_btn, _t("Add Asset Transaction"));
    itemBoxSizer5->Add(add_trans_btn, 0, wxRIGHT, 5);
    add_trans_btn->Enable(false);

    wxButton* view_trans_btn = new wxButton(assets_panel, wxID_VIEW_DETAILS, _t("&View Trans "));
    mmToolTip(view_trans_btn, _t("View Asset Transactions"));
    itemBoxSizer5->Add(view_trans_btn, 0, wxRIGHT, 5);
    view_trans_btn->Enable(false);

    wxButton* itemButton81 = new wxButton( assets_panel, wxID_EDIT, _t("&Edit "));
    mmToolTip(itemButton81, _t("Edit Asset"));
    itemBoxSizer5->Add(itemButton81, 0, wxRIGHT, 5);
    itemButton81->Enable(false);

    wxButton* itemButton7 = new wxButton( assets_panel, wxID_DELETE, _t("&Delete "));
    mmToolTip(itemButton7, _t("Delete Asset"));
    itemBoxSizer5->Add(itemButton7, 0, wxRIGHT, 5);
    itemButton7->Enable(false);

    wxBitmapButton* attachment_button_ = new wxBitmapButton(assets_panel
        , wxID_FILE, mmBitmapBundle(png::CLIP, mmBitmapButtonSize), wxDefaultPosition,
        wxSize(30, itemButton7->GetSize().GetY()));
    mmToolTip(attachment_button_, _t("Open attachments"));
    itemBoxSizer5->Add(attachment_button_, 0, wxRIGHT, 5);
    attachment_button_->Enable(false);

    wxSearchCtrl* searchCtrl = new wxSearchCtrl(assets_panel
        , wxID_FIND, wxEmptyString, wxDefaultPosition
        , wxSize(100, itemButton7->GetSize().GetHeight())
        , wxTE_PROCESS_ENTER, wxDefaultValidator, _t("Search"));
    searchCtrl->SetHint(_t("Search"));
    itemBoxSizer5->Add(searchCtrl, 0, wxCENTER, 1);
    mmToolTip(searchCtrl,
        _t("Enter any string to find related assets") + "\n\n" +
        _tu("Tips: Wildcard characters—question mark (?), asterisk (*)—can be used in search criteria.") + "\n" +
        _tu("Use the question mark (?) to find any single character—for example, “s?t” finds “sat” and “set”.") + "\n" +
        _tu("Use the asterisk (*) to find any number of characters—for example, “s*d” finds “sad” and “started”.") + "\n" +
        _tu("Use the asterisk (*) at the beginning to find any string in the middle of the sentence.")
    );

    //Infobar-mini
    wxStaticText* itemStaticText44 = new wxStaticText(assets_panel, IDC_PANEL_ASSET_STATIC_DETAILS_MINI, "");
    itemBoxSizer5->Add(itemStaticText44, 1, wxGROW | wxTOP | wxLEFT, 5);

    //Infobar
    wxStaticText* itemStaticText33 = new wxStaticText(assets_panel
        , IDC_PANEL_ASSET_STATIC_DETAILS, "", wxDefaultPosition, wxSize(200, -1), wxTE_MULTILINE | wxTE_WORDWRAP);
    itemBoxSizer4->Add(itemStaticText33, g_flagsExpandBorder1);

    updateExtraAssetData(-1);
}

void mmAssetsPanel::sortList()
{
    std::sort(this->m_assets.begin(), this->m_assets.end());
    std::stable_sort(this->m_assets.begin(), this->m_assets.end(), SorterBySTARTDATE());
    switch (this->m_lc->getSortColId())
    {
    case mmAssetsListCtrl::LIST_ID_ID:
        std::stable_sort(this->m_assets.begin(), this->m_assets.end(), SorterByASSETID());
        break;
    case mmAssetsListCtrl::LIST_ID_NAME:
        std::stable_sort(this->m_assets.begin(), this->m_assets.end(), SorterByASSETNAME());
        break;
    case mmAssetsListCtrl::LIST_ID_TYPE:
        std::stable_sort(this->m_assets.begin(), this->m_assets.end(), SorterByASSETTYPE());
        break;
    case mmAssetsListCtrl::LIST_ID_VALUE_INITIAL:
        std::stable_sort(this->m_assets.begin(), this->m_assets.end()
            , [](const Model_Asset::Data& x, const Model_Asset::Data& y)
            {
                return Model_Asset::value(x).first < Model_Asset::value(y).first;
            });
        break;
    case mmAssetsListCtrl::LIST_ID_VALUE_CURRENT:
        std::stable_sort(this->m_assets.begin(), this->m_assets.end()
            , [](const Model_Asset::Data& x, const Model_Asset::Data& y)
            {
                return Model_Asset::value(x).second < Model_Asset::value(y).second;
            });
        break;
    case mmAssetsListCtrl::LIST_ID_DATE:
        break;
    case mmAssetsListCtrl::LIST_ID_NOTES:
        std::stable_sort(this->m_assets.begin(), this->m_assets.end(), SorterByNOTES());
    default:
        break;
    }

    if (!this->m_lc->getSortAsc()) std::reverse(this->m_assets.begin(), this->m_assets.end());
}

int mmAssetsPanel::initVirtualListControl(int64 id)
{
    /* Clear all the records */
    m_lc->DeleteAllItems();

    if (this->m_filter_type == Model_Asset::TYPE_ID(-1)) // ALL
        this->m_assets = Model_Asset::instance().all();
    else
        this->m_assets = Model_Asset::instance().find(Model_Asset::ASSETTYPE(m_filter_type));
    this->sortList();

    m_lc->SetItemCount(this->m_assets.size());

    double initial = 0.0, balance = 0.0;
    for (const auto& asset: this->m_assets)
    {
        auto bal = Model_Asset::value(asset);
        initial += bal.first;
        balance += bal.second;
    }
    header_text_->SetLabelText(wxString::Format("%s, %s", wxString::Format(_t("Total: %s"), Model_Currency::toCurrency(balance)),  wxString::Format(_t("Initial: %s"), Model_Currency::toCurrency(initial)))); // balance

    int selected_item = 0;
    for (const auto& asset: this->m_assets)
    {
        if (asset.ASSETID == id) return selected_item;
        ++ selected_item;
    }
    return -1;
}

void mmAssetsPanel::OnDeleteAsset(wxCommandEvent& event)
{
    m_lc->OnDeleteAsset(event);
}

void mmAssetsPanel::OnNewAsset(wxCommandEvent& event)
{
    m_lc->OnNewAsset(event);
}

void mmAssetsPanel::OnEditAsset(wxCommandEvent& event)
{
    m_lc->OnEditAsset(event);
}

void mmAssetsPanel::OnAddAssetTrans(wxCommandEvent& event)
{
    m_lc->OnAddAssetTrans(event);
}

void mmAssetsPanel::OnViewAssetTrans(wxCommandEvent& event)
{
    m_lc->OnViewAssetTrans(event);
}

void mmAssetsPanel::OnOpenAttachment(wxCommandEvent& event)
{
    m_lc->OnOpenAttachment(event);
}

wxString mmAssetsPanel::getItem(long item, int col_id)
{
    const Model_Asset::Data& asset = this->m_assets[item];
    switch (col_id) {
    case mmAssetsListCtrl::LIST_ID_ICON:
        return "";
    case mmAssetsListCtrl::LIST_ID_ID:
        return wxString::Format("%lld", asset.ASSETID).Trim();
    case mmAssetsListCtrl::LIST_ID_NAME:
        return asset.ASSETNAME;
    case mmAssetsListCtrl::LIST_ID_TYPE:
        return wxGetTranslation(asset.ASSETTYPE);
    case mmAssetsListCtrl::LIST_ID_VALUE_INITIAL:
        return Model_Currency::toCurrency(Model_Asset::value(asset).first);
    case mmAssetsListCtrl::LIST_ID_VALUE_CURRENT:
        return Model_Currency::toCurrency(Model_Asset::value(asset).second);
    case mmAssetsListCtrl::LIST_ID_DATE:
        return mmGetDateTimeForDisplay(asset.STARTDATE);
    case mmAssetsListCtrl::LIST_ID_NOTES: {
        wxString full_notes = asset.NOTES;
        full_notes.Replace("\n", " ");
        if (Model_Attachment::NrAttachments(Model_Asset::refTypeName, asset.ASSETID))
            full_notes = full_notes.Prepend(mmAttachmentManage::GetAttachmentNoteSign());
        return full_notes;
    }
    default:
        return "";
    }
}

void mmAssetsPanel::updateExtraAssetData(int selIndex)
{
    wxStaticText* st = static_cast<wxStaticText*>(FindWindow(IDC_PANEL_ASSET_STATIC_DETAILS));
    wxStaticText* stm = static_cast<wxStaticText*>(FindWindow(IDC_PANEL_ASSET_STATIC_DETAILS_MINI));
    if (selIndex > -1)
    {
        const Model_Asset::Data& asset = this->m_assets[selIndex];
        enableEditDeleteButtons(true);
        const auto& change_rate = (Model_Asset::change_id(asset) != Model_Asset::CHANGE_ID_NONE)
            ? wxString::Format("%.2f %%", asset.VALUECHANGERATE) : "";
        const wxString& miniInfo = " " + wxString::Format(_t("Change in Value: %1$s %2$s")
            , wxGetTranslation(asset.VALUECHANGE), change_rate);

        st->SetLabelText(asset.NOTES);
        stm->SetLabelText(miniInfo);
    }
    else
    {
        stm->SetLabelText("");
        st->SetLabelText(this->tips_);
        enableEditDeleteButtons(false);
    }
}

void mmAssetsPanel::enableEditDeleteButtons(bool enable)
{
    wxButton* btn = static_cast<wxButton*>(FindWindow(wxID_EDIT));
    if (btn) btn->Enable(enable);

    btn = static_cast<wxButton*>(FindWindow(wxID_NEW));
    if (btn) btn->Enable(!enable);

    btn = static_cast<wxButton*>(FindWindow(wxID_ADD));
    if (btn) btn->Enable(enable);

    btn = static_cast<wxButton*>(FindWindow(wxID_VIEW_DETAILS));
    if (btn) btn->Enable(enable);

    btn = static_cast<wxButton*>(FindWindow(wxID_DELETE));
    if (btn) btn->Enable(enable);

    btn = static_cast<wxButton*>(FindWindow(wxID_FILE));
    if (btn) btn->Enable(enable);
}

void mmAssetsPanel::OnMouseLeftDown (wxCommandEvent& event)
{
    int i = 0;
    wxMenu menu;
    menu.Append(++i, _t("All"));

    for (int typeId = 0; typeId < Model_Asset::TYPE_ID_size; ++typeId) {
        wxString type = Model_Asset::type_name(typeId);
        menu.Append(++i, wxGetTranslation(type));
    }
    PopupMenu(&menu);

    event.Skip();
}

void mmAssetsPanel::OnViewPopupSelected(wxCommandEvent& event)
{
    int evt = std::max(event.GetId() - 1, 0);

    if (evt == 0)
    {
        m_bitmapTransFilter->SetLabel(_t("All"));
        m_bitmapTransFilter->SetBitmap(mmBitmapBundle(png::TRANSFILTER, mmBitmapButtonSize));
        this->m_filter_type = Model_Asset::TYPE_ID(-1);
    }
    else
    {
        this->m_filter_type = Model_Asset::TYPE_ID(evt - 1);
        m_bitmapTransFilter->SetBitmap(mmBitmapBundle(png::TRANSFILTER_ACTIVE, mmBitmapButtonSize));
        m_bitmapTransFilter->SetLabel(wxGetTranslation(Model_Asset::type_name(evt - 1)));
    }

    int64 trx_id = -1;
    m_lc->doRefreshItems(trx_id);
    updateExtraAssetData(-1);
}

void mmAssetsPanel::OnSearchTxtEntered(wxCommandEvent& event)
{
    const wxString search_string = event.GetString().Lower();
    if (search_string.IsEmpty()) return;

    long last = m_lc->GetItemCount();
    if (last == 0) return;

    long selectedItem = m_lc->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selectedItem == wxNOT_FOUND)
        selectedItem = 0;
    else
        selectedItem = (selectedItem + 1) % last;

    long startItem = selectedItem;

    SetEvtHandlerEnabled(false);

    do {
        const wxString t = getItem(selectedItem, mmAssetsListCtrl::LIST_ID_NOTES).Lower();
        if (t.Contains(search_string))
        {
            long cursel = m_lc->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            if (cursel != wxNOT_FOUND)
                m_lc->SetItemState(cursel, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);

            m_lc->SetItemState(selectedItem, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED,
                                              wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
            m_lc->EnsureVisible(selectedItem);
            break;
        }

        selectedItem = (selectedItem + 1) % last;

    } while (selectedItem != startItem);

    SetEvtHandlerEnabled(true);
}

void mmAssetsPanel::AddAssetTrans(const int selected_index)
{
    Model_Asset::Data* asset = &m_assets[selected_index];
    mmAssetDialog asset_dialog(this, asset, true);
    Model_Account::Data* account = Model_Account::instance().get(asset->ASSETNAME);
    Model_Account::Data* account2 = Model_Account::instance().get(asset->ASSETTYPE);
    if (account || account2)
    {
        asset_dialog.SetTransactionAccountName(account ? asset->ASSETNAME : asset->ASSETTYPE);
    }
    else
    {
        Model_Translink::Data_Set translist = Model_Translink::TranslinkList<Model_Asset>(asset->ASSETID);
        if (translist.empty())
        {
            wxMessageBox(_t(
                "This asset does not have its own account\n\n"
                "Multiple transactions for this asset are not recommended.")
                , _t("Asset Management"), wxOK | wxICON_INFORMATION);

            return; // abort process
        }
    }

    if (asset_dialog.ShowModal() == wxID_OK)
    {
        m_lc->doRefreshItems(selected_index);
        updateExtraAssetData(selected_index);
    }
}

void mmAssetsPanel::ViewAssetTrans(int selectedIndex)
{
    Model_Asset::Data* asset = &m_assets[selectedIndex];

    wxDialog dlg(this, wxID_ANY,
                 _t("View Asset Transactions") + ": " + asset->ASSETNAME,
                 wxDefaultPosition, wxDefaultSize,
                 wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

    dlg.SetIcon(mmex::getProgramIcon());
    wxWindow* parent = dlg.GetMainWindowOfCompositeControl();
    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);

    // Initialize list control
    wxListCtrl* assetTxnListCtrl = this->InitAssetTxnListCtrl(parent);
    topsizer->Add(assetTxnListCtrl, wxSizerFlags(g_flagsExpand).TripleBorder());

    // Bind events here
    BindAssetListEvents(assetTxnListCtrl);

    // Load asset transactions
    LoadAssetTransactions(assetTxnListCtrl, asset->ASSETID);

    // Add buttons
    wxSizer* buttonSizer = dlg.CreateSeparatedButtonSizer(wxOK);
    if (buttonSizer) {
        topsizer->Add(buttonSizer, wxSizerFlags().Expand().DoubleBorder(wxLEFT | wxRIGHT | wxBOTTOM));
    }

    dlg.SetSizerAndFit(topsizer);
    dlg.SetInitialSize(wxSize(600, 400)); // Set default size
    dlg.Center();
    dlg.ShowModal();
}

// Initialize the list control
wxListCtrl* mmAssetsPanel::InitAssetTxnListCtrl(wxWindow* parent)
{
    wxListCtrl* listCtrl = new wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                          wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_AUTOARRANGE);

    listCtrl->AppendColumn(_t("Account"), wxLIST_FORMAT_LEFT, 120);
    listCtrl->AppendColumn(_t("Date"), wxLIST_FORMAT_LEFT, 100);
    listCtrl->AppendColumn(_t("Trade Type"), wxLIST_FORMAT_LEFT, 100);
    listCtrl->AppendColumn(_t("Value"), wxLIST_FORMAT_RIGHT, 120);

    return listCtrl;
}

// Load asset transactions into the list control
void mmAssetsPanel::LoadAssetTransactions(wxListCtrl* listCtrl, int64 assetId)
{
    Model_Translink::Data_Set assetList = Model_Translink::TranslinkList<Model_Asset>(assetId);

    int row = 0;
    for (const auto& assetEntry : assetList)
    {
        auto* assetTrans = Model_Checking::instance().get(assetEntry.CHECKINGACCOUNTID);
        if (!assetTrans) continue;

        long index = listCtrl->InsertItem(row++, "");
        listCtrl->SetItemData(index, assetTrans->TRANSID.GetValue());
        FillAssetListRow(listCtrl, index, *assetTrans);
    }
}

void mmAssetsPanel::FillAssetListRow(wxListCtrl* listCtrl, long index, const Model_Checking::Data& txn)
{
    listCtrl->SetItem(index, 0, Model_Account::get_account_name(txn.ACCOUNTID));
    listCtrl->SetItem(index, 1, mmGetDateTimeForDisplay(txn.TRANSDATE));
    listCtrl->SetItem(index, 2, Model_Checking::trade_type_name(Model_Checking::type_id(txn.TRANSCODE)));
    listCtrl->SetItem(index, 3, Model_Currency::toString(txn.TRANSAMOUNT));
//    listCtrl->SetItem(index, 3, Model_Currency::get_currency_symbol(txn.CURRENCYID));
}

void mmAssetsPanel::BindAssetListEvents(wxListCtrl* listCtrl)
{
    listCtrl->Bind(wxEVT_LIST_ITEM_ACTIVATED, [listCtrl, this](wxListEvent& event) {
        long index = event.GetIndex();
        auto* txn = Model_Checking::instance().get(event.GetData());
        if (!txn) return;

        auto link = Model_Translink::TranslinkRecord(txn->TRANSID);
        mmAssetDialog dlg(listCtrl, &link, txn);
        dlg.ShowModal();

        this->FillAssetListRow(listCtrl, index, *txn);

        listCtrl->SortItems([](wxIntPtr item1, wxIntPtr item2, wxIntPtr) -> int {
            auto date1 = Model_Checking::TRANSDATE(Model_Checking::instance().get(item1));
            auto date2 = Model_Checking::TRANSDATE(Model_Checking::instance().get(item2));
            return date1.IsEarlierThan(date2) ? -1 : (date1.IsLaterThan(date2) ? 1 : 0);
        }, 0);
    });

    listCtrl->Bind(wxEVT_CHAR, [listCtrl, this](wxKeyEvent& event) {
        if (event.GetKeyCode() == WXK_CONTROL_C) {
            CopySelectedRowsToClipboard(listCtrl);
        } else if (event.GetKeyCode() == WXK_CONTROL_A) {
            for (int row = 0; row < listCtrl->GetItemCount(); row++)
                listCtrl->SetItemState(row, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        }
    });
}

void mmAssetsPanel::CopySelectedRowsToClipboard(wxListCtrl* listCtrl)
{
    if (!wxTheClipboard->Open()) return;

    wxString data;
    const wxString separator = "\t";

    for (int row = 0; row < listCtrl->GetItemCount(); row++) {
        if (listCtrl->GetItemState(row, wxLIST_STATE_SELECTED) != wxLIST_STATE_SELECTED)
            continue;

        for (int col = 0; col < listCtrl->GetColumnCount(); col++) {
            if (listCtrl->GetColumnWidth(col) > 0) {
                data += listCtrl->GetItemText(row, col) + separator;
            }
        }
        data += "\n";
    }

    wxTheClipboard->SetData(new wxTextDataObject(data));
    wxTheClipboard->Close();
}

void mmAssetsPanel::GotoAssetAccount(const int selected_index)
{
    Model_Asset::Data* asset = &m_assets[selected_index];
    const Model_Account::Data* account = Model_Account::instance().get(asset->ASSETNAME);
    if (account)
    {
        SetAccountParameters(account);
    }
    else
    {
        Model_Translink::Data_Set asset_list = Model_Translink::TranslinkList<Model_Asset>(asset->ASSETID);
        for (const auto &asset_entry : asset_list)
        {
            Model_Checking::Data* asset_trans = Model_Checking::instance().get(asset_entry.CHECKINGACCOUNTID);
            if (asset_trans)
            {
                account = Model_Account::instance().get(asset_trans->ACCOUNTID);
                SetAccountParameters(account);
            }
        }
    }
}

void mmAssetsPanel::SetAccountParameters(const Model_Account::Data* account)
{
    m_frame->setNavTreeAccount(account->ACCOUNTNAME);
    m_frame->setGotoAccountID(account->ACCOUNTID);
    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_GOTOACCOUNT);
    m_frame->GetEventHandler()->AddPendingEvent(evt);
}
