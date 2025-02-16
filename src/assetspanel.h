/*******************************************************
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

#pragma once
#include "mmpanelbase.h"
#include "model/Model_Asset.h"
#include "model/Model_Account.h"
#include "mmframe.h"

class wxListEvent;
class mmAssetsPanel;
class wxButton;

/* Custom ListCtrl class that implements virtual LC style */
class mmAssetsListCtrl: public mmListCtrl
{
    DECLARE_NO_COPY_CLASS(mmAssetsListCtrl)
    wxDECLARE_EVENT_TABLE();

public:
    enum LIST_COL
    {
        LIST_COL_ICON = 0,
        LIST_COL_ID,
        LIST_COL_NAME,
        LIST_COL_DATE,
        LIST_COL_TYPE,
        LIST_COL_VALUE_INITIAL,
        LIST_COL_VALUE_CURRENT,
        LIST_COL_NOTES,
        LIST_COL_size, // number of columns
    };

private:
    static const std::vector<ListColumnInfo> col_info_all();
    int col_size() { return LIST_COL_size; }
    int col_sort() { return LIST_COL_DATE; }

public:
    mmAssetsListCtrl(mmAssetsPanel* cp, wxWindow *parent, wxWindowID winid = wxID_ANY);

    void OnNewAsset(wxCommandEvent& event);
    void OnEditAsset(wxCommandEvent& event);
    void OnDeleteAsset(wxCommandEvent& event);
    void OnDuplicateAsset(wxCommandEvent& event);
    void OnOrganizeAttachments(wxCommandEvent& event);
    void OnOpenAttachment(wxCommandEvent& event);
    void OnAddAssetTrans(wxCommandEvent& WXUNUSED(event));
    void OnViewAssetTrans(wxCommandEvent& WXUNUSED(event));
    void OnGotoAssetAccount(wxCommandEvent& WXUNUSED(event));

    void doRefreshItems(int64 trx_id = -1);

protected:
    virtual void OnColClick(wxListEvent& event);

private:
    mmAssetsPanel* m_panel = nullptr;

    /* required overrides for virtual style list control */
    virtual wxString OnGetItemText(long item, long column) const;
    virtual int OnGetItemImage(long item) const;

    void OnMouseRightClick(wxMouseEvent& event);
    void OnListLeftClick(wxMouseEvent& event);
    void OnListItemActivated(wxListEvent& event);
    void OnListKeyDown(wxListEvent& event);
    void OnListItemSelected(wxListEvent& event);
    void OnEndLabelEdit(wxListEvent& event);
    bool EditAsset(Model_Asset::Data* pEntry);

    enum {
        MENU_TREEPOPUP_NEW = wxID_HIGHEST + 1200,
        MENU_TREEPOPUP_ADDTRANS,
        MENU_TREEPOPUP_VIEWTRANS,
        MENU_TREEPOPUP_GOTOACCOUNT,
        MENU_TREEPOPUP_EDIT,
        MENU_TREEPOPUP_DELETE,
        MENU_ON_DUPLICATE_TRANSACTION,
        MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS,
    };
};

class mmAssetsPanel : public mmPanelBase
{
    wxDECLARE_EVENT_TABLE();

public:
    enum EIcons
    {
        ICON_PROPERTY,
        ICON_CAR,
        ICON_HOUSEHOLD_OBJ,
        ICON_ART,
        ICON_JEWELLERY,
        ICON_CASH,
        ICON_OTHER,
        ICON_UPARROW,   // the 'ARROW's need to be last in the list
        ICON_DOWNARROW
    };

    mmAssetsPanel(mmGUIFrame* frame, wxWindow *parent, wxWindowID winid, const wxString& name="mmAssetsPanel");
    mmGUIFrame* m_frame = nullptr;

    void updateExtraAssetData(int selIndex);
    int initVirtualListControl(int64 trx_id = -1, int col = 0, bool asc = true);
    wxString getItem(long item, long column);

    Model_Asset::Data_Set m_assets;
    Model_Asset::TYPE_ID m_filter_type;

    wxString BuildPage() const { return m_listCtrlAssets->BuildPage(_("Assets")); }

    void AddAssetTrans(const int selected_index);
    void ViewAssetTrans(const int selected_index);
    void GotoAssetAccount(const int selected_index);
    void RefreshList();

private:
    void enableEditDeleteButtons(bool enable);
    void OnSearchTxtEntered(wxCommandEvent& event);
    
    mmAssetsListCtrl* m_listCtrlAssets = nullptr;
    wxButton* m_bitmapTransFilter = nullptr;
    wxStaticText* header_text_ = nullptr;

    bool Create(wxWindow *parent
        , wxWindowID winid
        , const wxPoint& pos
        , const wxSize& size
        , long style
        , const wxString &name);
    void CreateControls();

    /* Event handlers for Buttons */
    void OnNewAsset(wxCommandEvent& event);
    void OnDeleteAsset(wxCommandEvent& event);
    void OnEditAsset(wxCommandEvent& event);
    void OnOpenAttachment(wxCommandEvent& event);
    void OnMouseLeftDown(wxCommandEvent& event);
    void OnAddAssetTrans(wxCommandEvent& event);
    void OnViewAssetTrans(wxCommandEvent& event);

    void OnViewPopupSelected(wxCommandEvent& event);
    void sortTable();
    void SetAccountParameters(const Model_Account::Data* account);

private:
    wxString tips_;
    enum {
        IDC_PANEL_ASSET_STATIC_DETAILS = wxID_HIGHEST + 1220,
        IDC_PANEL_ASSET_STATIC_DETAILS_MINI,
    };
};

inline void mmAssetsPanel::RefreshList(){ m_listCtrlAssets->doRefreshItems(); }
