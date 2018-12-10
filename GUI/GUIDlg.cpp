
// GUIDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GUI.h"
#include "GUIDlg.h"
#include "program.h"
#include "afxdialogex.h"
#include "prefetch.h"
#include "update.h"
#include "phverify/verify.h"
#include "Win32Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

list<pprogram> my_list;
list<pprogram> delete_list;
std::list<pblackp> black_list;
std::list<punknownp> unknown_list;



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

														// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}


void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST1, CGUIDlg::OnNMCustomdrawList1)
END_MESSAGE_MAP()


// CGUIDlg dialog



CGUIDlg::CGUIDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GUI_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}


void CGUIDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_LIST1, m_listView);
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CGUIDlg, CDialogEx)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, &CGUIDlg::OnLvnItemchangedList1)
	ON_BN_CLICKED(IDC_DELETE_BTN, &CGUIDlg::OnBnClickedDeleteBtn)
	ON_BN_CLICKED(IDC_SELECT_BTN, &CGUIDlg::OnBnClickedSelectBtn)
	ON_BN_CLICKED(IDC_RESET_BTN, &CGUIDlg::OnBnClickedResetBtn)
	ON_WM_DESTROY()
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST1, &CGUIDlg::OnLvnColumnclickList1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST1, &CGUIDlg::OnNMCustomdrawList1)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CGUIDlg::OnNMDblclkList1)
END_MESSAGE_MAP()


// CGUIDlg message handlers

BOOL CGUIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	SetWindowPos(NULL, -1, -1, 900, 550, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	LPWSTR szText[COLNUM] = { L" " ,L"이름", L"마지막 사용시간",  L"인증서", L"버젼" };
	int nWidth[COLNUM] = { 25,300,200,200,100 };

	LV_COLUMN iCol;
	iCol.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	iCol.fmt = LVCFMT_LEFT;
	


	m_listView.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
	m_listView.SetWindowPos(NULL, 10, 10, 860, 440, NULL);
	for (int i = 0; i < COLNUM; i++) {
		iCol.pszText = szText[i];
		iCol.iSubItem = i;
		iCol.cx = nWidth[i];
		iCol.fmt = LVCFMT_CENTER;
		m_listView.InsertColumn(i, &iCol);
	}

	GetDlgItem(IDC_RESET_BTN)->SetWindowPos(NULL, 640, 460, 60, 30, NULL);
	GetDlgItem(IDC_SELECT_BTN)->SetWindowPos(NULL, 720, 460, 60, 30, NULL);
	GetDlgItem(IDC_DELETE_BTN)->SetWindowPos(NULL, 800, 460, 60, 30, NULL);

	//run_load_dlg(NULL);
	get_naga_data();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

UINT CGUIDlg::run_load_dlg(LPVOID _mothod) {
	CLoadDlg load_dlg;
	load_dlg.DoModal();

	return 1;
}

void CGUIDlg::get_naga_data(void) {
	my_list.clear();
	black_list.clear();
	unknown_list.clear();

	compare_lists(&my_list);
	wstring null_string = L"";
	for (auto mine : my_list) {
		pblackp temp = new blackp(mine->id(), mine->name(), mine->vendor(), mine->version(), mine->uninstaller(), null_string.c_str(), mine->vendor());
		black_list.push_back(temp);
	}

	//
	//	update 부분 추가
	//
	if (!get_update_info(&black_list)) {
		log_err "get_update_info err" log_end;
	}
	for (auto black : black_list)
	{
		wstring name = black->name();
		if (name.find(L"Veraport") != std::string::npos) {
			insertData(
				L"Update", const_cast<LPWSTR>(black->name()), const_cast<LPWSTR>(black->version()), L"", const_cast<LPWSTR>(black->bank()));
		}
		else {
			insertData(
				LPWSTR(L"Security"), const_cast<LPWSTR>(black->name()), const_cast<LPWSTR>(black->version()), L"", const_cast<LPWSTR>(black->bank()));
		}

	}

	if (!get_prefetch_info(&unknown_list)) {
		log_err "get_prefetch_info() err" log_end;
	}

	for (auto line : unknown_list) {
		wstring	lastuse = line->lastuse();

		wstringstream last_stm;
		int loc = lastuse.find(L" ");
		last_stm << lastuse.substr(0, loc);

		wstring file_name = (file_name_from_file_pathw(line->id())).c_str();
		to_lower_string(file_name);

		insertData(
			LPWSTR(L"Unknown"),
			LPWSTR(file_name.c_str()),
			(LPWSTR)(line->version()),
			(LPWSTR)(last_stm.str().c_str()),
			(LPWSTR)line->cert());
	}

}

/// @brief	리스트 컨트롤에 데이터를 추가하기 위한 함수
///	이름, 마지막사용시간, 버젼, 인증서의 순서로 삽입한다.
void CGUIDlg::insertData(LPWSTR type, LPWSTR name, LPWSTR version, LPWSTR lastuse, LPWSTR cert) {
	LV_ITEM lvitem;
	int count = m_listView.GetItemCount();

	// 체크박스를 위한 공백
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = count;
	lvitem.iSubItem = 0;
	lvitem.pszText = type;
	m_listView.InsertItem(&lvitem);

	// 이름 입력
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = count;
	lvitem.iSubItem = 1;
	lvitem.pszText = name;
	m_listView.SetItem(&lvitem);

	//	마지막 사용시간 입력
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = count;
	lvitem.iSubItem = 2;
	lvitem.pszText = lastuse;
	m_listView.SetItem(&lvitem);

	// 버젼 입력
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = count;
	lvitem.iSubItem = 4;
	lvitem.pszText = version;
	m_listView.SetItem(&lvitem);

	//	인증서 유무 검증
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = count;
	lvitem.iSubItem = 3;
	lvitem.pszText = cert;
	m_listView.SetItem(&lvitem);

}

void CGUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{


	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}

}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CGUIDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CGUIDlg::OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

}



void CGUIDlg::OnBnClickedDeleteBtn()
{
	log_warn "[ 사용자가 선택한 프로그램 ]" log_end;
	// TODO: Add your control notification handler code here
	for (int i = 0; i < m_listView.GetItemCount(); i++) {
		if (m_listView.GetCheck(i) == true) {
			CString delete_name = m_listView.GetItemText(i, 1);
			pprogram temp = find_program(delete_name, my_list);
			log_info "%ws", temp->name() log_end;
			if (temp != NULL) {
				delete_list.push_back(temp);
			}
		}
	}
	// delete_list 들의 uninstaller handle 실행! 
	for (auto mouse : delete_list) {
	//	MessageBox(mouse->uninstaller());
		STARTUPINFO startupInfo = { 0 };
		PROCESS_INFORMATION processInfo;
		startupInfo.cb = sizeof(STARTUPINFO);
		::CreateProcess(NULL, (LPWSTR)(mouse->uninstaller()), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);
	}

}


punknownp CGUIDlg::find_unknown(CString program_name, std::list<punknownp> temp_list) {
	log_info "%ws", program_name log_end;
	int result;
	for (auto mine : temp_list) {
		result = wcscmp(program_name, mine->id());
		if (result == 0) return mine;
	}
	return NULL;
}

pprogram CGUIDlg::find_program(CString program_name, list<pprogram> temp_list) {
	log_info "%ws", program_name log_end;
	int result;
	for (auto mine : temp_list) {
		result = wcscmp(program_name, mine->name());
		if (result == 0) return mine;
	}
	return NULL;
}

void CGUIDlg::OnBnClickedSelectBtn()
{
	for (int i = 0; i < m_listView.GetItemCount(); i++)
	{
		m_listView.SetCheck(i, TRUE);
	}
}

void CGUIDlg::OnBnClickedResetBtn()
{
	// TODO: Add your control notification handler code here
	m_listView.DeleteAllItems();
	my_list.clear();
	black_list.clear();
	unknown_list.clear();
	get_naga_data();
}

void CGUIDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here
	delete_all_csv(L"C:\\Temp\\result", 1);
	finalize_log();
	for (auto p : my_list)
	{
		delete p;
	}
	my_list.clear();
}


void CGUIDlg::OnLvnColumnclickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	m_listView.DeleteAllItems();
	my_list.clear();
	delete_list.clear();
	unknown_list.clear();
	black_list.clear();
	if (!get_prefetch_info(&unknown_list)) {
		log_err "힝" log_end;
	}

}

///
///
///
void CGUIDlg::OnNMCustomdrawList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	BOOL bUpdateFlag = FALSE;
	BOOL bSecurityFlag = FALSE;
	NMLVCUSTOMDRAW* pLVCD = (NMLVCUSTOMDRAW*)pNMHDR;

	CString p_type = m_listView.GetItemText(pLVCD->nmcd.dwItemSpec, 0);

	*pResult = 0;

	if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
		*pResult = CDRF_NOTIFYITEMDRAW;

	else if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		if (p_type.Find(_T("Security")) != -1)			//	금융관련 프로그램
		{
			pLVCD->clrText = RGB(33, 151, 216);  
			pLVCD->clrTextBk = RGB(237, 255, 255);
		}
		else if (p_type.Find(_T("Update")) != -1)	//	update가 필요한 금융관련 프로그램
		{
			pLVCD->clrText = RGB(255, 107, 107);
			pLVCD->clrTextBk = RGB(253, 212, 212);

		}
		else
		{
			//
			//	unknown program의 경우
			//
		}

		*pResult = CDRF_DODEFAULT;
	}

}




void CGUIDlg::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
	*pResult = 0;
	int row = pNMItemActivate->iItem;
	int col = pNMItemActivate->iSubItem;
	log_info "clicked!! %d %d", row, col log_end;

	CString name = m_listView.GetItemText(row, 0);
	if (name.Find(_T("Update")) != -1) {
		CBankDlg dlg;
		dlg.DoModal();
	}
}

