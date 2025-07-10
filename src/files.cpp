/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * files.cpp
 * Copyright (C) 2019-2025 Emilien Kia <Emilien.Kia+dev@gmail.com>
 *
 * logviewer is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * logviewer is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <wx/wx.h>

#include "files.hpp"

#include "app.hpp"
#include "helpers.hpp"
#include "parser.hpp"

#include <wx/artprov.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/filepicker.h>




//
// File Dialog List Model
//

FileDialogListModel::FileDialogListModel(FileData& data) :
    _data(data)
{
    data.AddListener(this);
}

FileDialogListModel::~FileDialogListModel()
{
    _data.RemListener(this);
}


unsigned int FileDialogListModel::GetColumnCount()const
{
    return FileDialogListModel::COLUMN_COUNT;
}

wxString FileDialogListModel::GetColumnType(unsigned int col)const
{
    return "string";
}

void FileDialogListModel::GetValueByRow(wxVariant &variant, unsigned int row, unsigned int col) const
{
    switch (col)
    {
    case FileDialogListModel::STATUS:
        variant = Formatter::StatusToString(GetData().GetSource(row).status);
        return;
    case FileDialogListModel::FILENAME:
        variant = std::string(GetData().GetSource(row).source->GetName());
        return;
    case FileDialogListModel::READER:
        variant = "<reader>";
        return;
    case FileDialogListModel::LAYOUT:
        variant = "<layout>";
        return;
    default:
        return;
    }
}

bool FileDialogListModel::GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr &attr)const
{
    return false;
}

bool FileDialogListModel::SetValueByRow(const wxVariant &variant, unsigned int row, unsigned int col)
{
    return false;
}

void FileDialogListModel::Update()
{
    long count = GetData().GetSourceCount();
    Reset(count);
}

void FileDialogListModel::Updated(FileData& data)
{
    Update();
}

uint16_t FileDialogListModel::GetFileId(wxDataViewItem item)const
{
    return GetRow(item);
}

FileDescriptor* FileDialogListModel::GetFile(wxDataViewItem item)
{
    return _data.FindSource(GetFileId(item));
}

const FileDescriptor* FileDialogListModel::GetFile(wxDataViewItem item)const
{
    return _data.FindSource(GetFileId(item));
}



//
// FileManagementDialog
// 


BEGIN_EVENT_TABLE(FileManagementDialog, wxDialog)
    EVT_UPDATE_UI(wxID_REVERT, FileManagementDialog::OnUpdateReloadFilesButton)
    EVT_UPDATE_UI(wxID_CLOSE, FileManagementDialog::OnUpdateRemoveFilesButton)

    EVT_BUTTON(wxID_OPEN, FileManagementDialog::OnOpenFilesButton)
    EVT_BUTTON(wxID_REVERT, FileManagementDialog::OnReloadFilesButton)
    EVT_BUTTON(wxID_CLOSE, FileManagementDialog::OnRemoveFilesButton)

    EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, FileManagementDialog::OnFileSelectionChanged)
    EVT_CHOICE(wxID_ANY, FileManagementDialog::OnLogFormatChoice)
    EVT_CHOICE(wxID_ANY, FileManagementDialog::OnDateFormatChoice)
    EVT_TEXT(wxID_ANY, FileManagementDialog::OnCustomLogFormatChanged)
    EVT_TEXT(wxID_ANY, FileManagementDialog::OnCustomDateFormatChanged)

END_EVENT_TABLE()


bool FileManagementDialog::Create(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size)
{
    if(!wxDialog::Create(parent, id, title, pos, size, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX|wxMINIMIZE_BOX)) {
        return false;
    }
    SetMinSize(wxSize(600, 400));

    _filesModel = new FileDialogListModel(wxGetApp().GetFileData());

    wxSizer* gsz = new wxBoxSizer(wxVERTICAL);

    wxSizer* tbsz = new wxBoxSizer(wxHORIZONTAL);

    //
    // Management buttons:
    //

    wxButton* btn;
    btn = new wxButton(this, wxID_OPEN, "Open files...");
    btn->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_BUTTON));
    tbsz->Add(btn , 0, wxALL, 4);

    btn = new wxButton(this, wxID_REVERT, "Load");
    btn->SetBitmap(wxArtProvider::GetBitmap(wxART_TICK_MARK, wxART_BUTTON));
    tbsz->Add(btn , 0, wxALL, 4);

    btn = new wxButton(this, wxID_CLOSE, "Remove");
    btn->SetBitmap(wxArtProvider::GetBitmap(wxART_CLOSE, wxART_BUTTON));
    tbsz->Add(btn , 0, wxALL, 4);

    gsz->Add(tbsz, 0, wxEXPAND);

    //
    // Files list:
    //

    _files = new wxDataViewCtrl(this, ID_LV_FILEBOX_FILE_DVCTRL, wxDefaultPosition, wxDefaultSize, wxDV_HORIZ_RULES);
    _files->AppendTextColumn("Status",    FileDialogListModel::STATUS,  wxDATAVIEW_CELL_INERT, 64, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);
    _files->AppendTextColumn("File",     FileDialogListModel::FILENAME, wxDATAVIEW_CELL_INERT, 300, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE)
        ->GetRenderer()->EnableEllipsize(wxELLIPSIZE_START);
//	_files->AppendTextColumn("Reader",    FileDialogListModel::READER,  wxDATAVIEW_CELL_INERT, 128, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);
//	_files->AppendTextColumn("Layout", FileDialogListModel::LAYOUT, 	wxDATAVIEW_CELL_INERT, 128, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE);
    _files->AssociateModel(_filesModel);

    gsz->Add(_files, 1, wxALL|wxEXPAND, 4);

    //
    // Format management
    //
    wxFlexGridSizer* formatSizer = new wxFlexGridSizer(2, 3, 5, 10);
    formatSizer->AddGrowableCol(2); // La troisième colonne s'étend

    // Log format:
    wxStaticText* logFormatLabel = new wxStaticText(this, wxID_ANY, _("Log format:"));
    _logFormatChoice = new wxChoice(this, wxID_ANY);
    for (const auto& def : LogFormat::LOG_FORMAT_DEFINITIONS) {
        _logFormatChoice->Append(def.displayName);
    }
    _logFormatChoice->SetSelection(0);

    _customLogFormatText = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);

    formatSizer->Add(logFormatLabel, 0, wxALIGN_CENTER_VERTICAL);
    formatSizer->Add(_logFormatChoice, 0, wxALIGN_CENTER_VERTICAL);
    formatSizer->Add(_customLogFormatText, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL);

    // Date format:
    wxStaticText* dateFormatLabel = new wxStaticText(this, wxID_ANY, _("Date format:"));
    _dateFormatChoice = new wxChoice(this, wxID_ANY);
    for (const auto& def : LogFormat::DATE_FORMAT_DEFINITIONS) {
        _dateFormatChoice->Append(def.displayName);
    }
    _dateFormatChoice->SetSelection(0); // Sélection par défaut

    _customDateFormatText = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);

    formatSizer->Add(dateFormatLabel, 0, wxALIGN_CENTER_VERTICAL);
    formatSizer->Add(_dateFormatChoice, 0, wxALIGN_CENTER_VERTICAL);
    formatSizer->Add(_customDateFormatText, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL);

    gsz->Add(formatSizer, 0, wxEXPAND | wxALL, 10);


    //
    // Close buttons::
    //
    gsz->Add(CreateSeparatedButtonSizer(wxOK|wxCANCEL), 0, wxALL|wxEXPAND, 4);
    SetSizer(gsz);

    _filesModel->Update();

    return true;
}


void FileManagementDialog::OnOpenFilesButton(wxCommandEvent& event)
{
    wxArrayString paths;
    int res = wxGetApp().OpenFileDialog(this, paths);
    if(res == wxID_CANCEL)
        return;

    wxVector<uint16_t> fids;
    for (const wxString& path : paths)
    {
        fids.push_back(_filesModel->GetData().AddFile(std::string(path)).id);
    }

    _filesModel->Update();
}

void FileManagementDialog::OnReloadFilesButton(wxCommandEvent& event)
{
    if(_files->HasSelection())
    {
        FileDescriptor* fd = const_cast<FileDescriptor*>(_filesModel->GetFile(_files->GetSelection()));
        if(fd)
        {
            fd->status = FileDescriptor::FILE_RELOAD;
            _filesModel->Update();
        }
    }
}

void FileManagementDialog::OnRemoveFilesButton(wxCommandEvent& event)
{
    if(_files->HasSelection())
    {
        FileDescriptor* fd = const_cast<FileDescriptor*>(_filesModel->GetFile(_files->GetSelection()));
        if(fd)
        {
            fd->status = FileDescriptor::FILE_REMOVED;
            _filesModel->Update();
        }
    }
}

void FileManagementDialog::OnUpdateReloadFilesButton(wxUpdateUIEvent& event)
{
    event.Enable(_files->HasSelection());
}

void FileManagementDialog::OnUpdateRemoveFilesButton(wxUpdateUIEvent& event)
{
    event.Enable(_files->HasSelection());
}

void FileManagementDialog::OnFileSelectionChanged(wxDataViewEvent& event)
{
    SaveCurrentFormatSettings();

    wxDataViewItem item = _files->GetSelection();
    if (item.IsOk()) {
        _selectedDescriptor = _filesModel->GetFile(item);
        LoadFormatSettingsFromDescriptor(*_selectedDescriptor);
    } else {
        _selectedDescriptor = nullptr;
        _logFormatChoice->Enable(false);
        _dateFormatChoice->Enable(false);
        _customLogFormatText->Enable(false);
        _customDateFormatText->Enable(false);
    }
}

void FileManagementDialog::OnLogFormatChoice(wxCommandEvent& event)
{
    UpdateFormatControls();
    SaveCurrentFormatSettings();
}

void FileManagementDialog::OnDateFormatChoice(wxCommandEvent& event)
{
    UpdateFormatControls();
    SaveCurrentFormatSettings();
}

void FileManagementDialog::OnCustomLogFormatChanged(wxCommandEvent& event)
{
    // Validate the regex
    int logSelection = _logFormatChoice->GetSelection();
    if (logSelection >= 0 && logSelection < LogFormat::LOG_FORMAT_DEFINITIONS.size()) {
        const auto& logDef = LogFormat::LOG_FORMAT_DEFINITIONS[logSelection];
        if (logDef.isEditable) {
            ValidateRegex(_customLogFormatText, _customLogFormatText->GetValue());
        }
    }

    SaveCurrentFormatSettings();
}

void FileManagementDialog::OnCustomDateFormatChanged(wxCommandEvent& event)
{
    // Validate the regex
    int dateSelection = _dateFormatChoice->GetSelection();
    if (dateSelection >= 0 && dateSelection < LogFormat::DATE_FORMAT_DEFINITIONS.size()) {
        const auto& dateDef = LogFormat::DATE_FORMAT_DEFINITIONS[dateSelection];
        if (dateDef.isEditable) {
            ValidateRegex(_customDateFormatText, _customDateFormatText->GetValue());
        }
    }

    SaveCurrentFormatSettings();
}

void FileManagementDialog::UpdateFormatControls()
{
    int logSelection = _logFormatChoice->GetSelection();
    if (logSelection >= 0 && logSelection < LogFormat::LOG_FORMAT_DEFINITIONS.size()) {
        const auto& logDef = LogFormat::LOG_FORMAT_DEFINITIONS[logSelection];
        _customLogFormatText->Enable(logDef.isEditable);
        _customLogFormatText->SetEditable(logDef.isEditable);
        if (!logDef.isEditable) {
            _customLogFormatText->SetValue(logDef.defaultRegex);
            SetRegexDecoration(_customLogFormatText, true);
        } else {
            ValidateRegex(_customLogFormatText, _customLogFormatText->GetValue());
        }
        _customLogFormatText->Refresh();
    }

    int dateSelection = _dateFormatChoice->GetSelection();
    if (dateSelection >= 0 && dateSelection < LogFormat::DATE_FORMAT_DEFINITIONS.size()) {
        const auto& dateDef = LogFormat::DATE_FORMAT_DEFINITIONS[dateSelection];
        _customDateFormatText->Enable(dateDef.isEditable);
        _customDateFormatText->SetEditable(dateDef.isEditable);
        if (!dateDef.isEditable) {
            _customDateFormatText->SetValue(dateDef.defaultRegex);
            SetRegexDecoration(_customDateFormatText, true);
        } else {
            // Valider la regex actuelle si éditable
            ValidateRegex(_customDateFormatText, _customDateFormatText->GetValue());
        }
        _customDateFormatText->Refresh();
    }
}

void FileManagementDialog::SaveCurrentFormatSettings()
{
    if (_selectedDescriptor != nullptr) {
        int logSelection = _logFormatChoice->GetSelection();
        if (logSelection >= 0 && logSelection < LogFormat::LOG_FORMAT_DEFINITIONS.size()) {
            _selectedDescriptor->logFormat = LogFormat::LOG_FORMAT_DEFINITIONS[logSelection].enumValue;
            if (LogFormat::LOG_FORMAT_DEFINITIONS[logSelection].isEditable) {
                _selectedDescriptor->logRegex = _customLogFormatText->GetValue().ToStdString();
            }
        }

        int dateSelection = _dateFormatChoice->GetSelection();
        if (dateSelection >= 0 && dateSelection < LogFormat::DATE_FORMAT_DEFINITIONS.size()) {
            _selectedDescriptor->dateFormat = static_cast<FileDescriptor::LOG_DATE>(LogFormat::DATE_FORMAT_DEFINITIONS[dateSelection].enumValue);
            if (LogFormat::DATE_FORMAT_DEFINITIONS[dateSelection].isEditable) {
                _selectedDescriptor->dateRegex = _customDateFormatText->GetValue().ToStdString();
            }
        }
    }
}

void FileManagementDialog::LoadFormatSettingsFromDescriptor(const FileDescriptor& descriptor)
{
    _logFormatChoice->Enable(true);
    _dateFormatChoice->Enable(true);
    _customLogFormatText->Enable(true);
    _customDateFormatText->Enable(true);

    _logFormatChoice->SetSelection(static_cast<int>(descriptor.logFormat));

    if (descriptor.logFormat == FileDescriptor::LOG_FORMAT_CUSTOM) {
        _customLogFormatText->SetValue(descriptor.logRegex);
    }

    _dateFormatChoice->SetSelection(static_cast<int>(descriptor.dateFormat));

    if (descriptor.dateFormat == FileDescriptor::DATE_FORMAT_CUSTOM) {
        _customDateFormatText->SetValue(descriptor.dateRegex);
    }

    UpdateFormatControls();
}

void FileManagementDialog::ValidateRegex(wxTextCtrl* textCtrl, const wxString& regex)
{
    if (regex.IsEmpty()) {
        SetRegexDecoration(textCtrl, true);
        return;
    } else {
        try {
            auto regexObj = Regex::createRegex(regex.ToStdString());
            SetRegexDecoration(textCtrl, regexObj != nullptr);
        } catch (...) {
            SetRegexDecoration(textCtrl, false);
        }
    }
    textCtrl->Refresh();
}

void FileManagementDialog::SetRegexDecoration(wxTextCtrl* textCtrl, bool valid) {
    if (valid) {
//        textCtrl->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
        textCtrl->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
    } else {
//        textCtrl->SetBackgroundColour(*wxRED);
        textCtrl->SetForegroundColour(*wxRED);
    }
    textCtrl->Refresh();
}
