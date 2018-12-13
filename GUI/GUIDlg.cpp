
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

std::list<pprogram> my_list;
std::list<pprogram> delete_list;
std::list<pblackp> black_list;
std::list<punknownp> unknown_list;
std::list<punknownp> del_unknown_list;


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
	DDX_Control(pDX, IDC_SIDE_PIC, sideImg);
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
	ON_WM_CTLCOLOR()
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

	//
	//	list control 초기화
	//

	CRect rect;

	GetClientRect(&rect);
	CPoint pos;
	pos.x = GetSystemMetrics(SM_CXSCREEN) / 2.0f - rect.Width() / 2.0f;
	pos.y = GetSystemMetrics(SM_CYSCREEN) / 2.0f - rect.Height() / 2.0f;;

	SetWindowPos(NULL, pos.x, pos.y, 1200, 570, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);

	LPWSTR szText[COLNUM] = { L" " ,L"이름", L"마지막 사용시간",  L"인증서", L"버젼" };
	int nWidth[COLNUM] = { 25,300,200,200,100 };

	LV_COLUMN iCol;
	iCol.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	iCol.fmt = LVCFMT_LEFT;

	//
	//	사이드 이미지 수정
	//
	sideImg.SetWindowPos(NULL, 10, 10, 300, 450, NULL);
//	sideImg.SetWindowPos(NULL, 870, 10, 300, 450, NULL);
	HBITMAP hBmp = (HBITMAP)::LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BITMAP1), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
	sideImg.SetBitmap(hBmp);

	//
	//	LVS_EX_CHECKBOXES 를 통해 체크박스를 이용할 수 있다.
	//
	m_listView.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
	m_listView.SetWindowPos(NULL, 320, 10, 850, 450, NULL);
//	m_listView.SetWindowPos(NULL, 10, 10, 850, 450, NULL);
	
	//
	//	각각의 Column을 생성하고 크기를 지정
	//
	for (int i = 0; i < COLNUM; i++) {
		iCol.pszText = szText[i];
		iCol.iSubItem = i;
		iCol.cx = nWidth[i];
		iCol.fmt = LVCFMT_CENTER;
		m_listView.InsertColumn(i, &iCol);
	}

	//
	//	버튼들의 위치 조정 (수정 필요)
	//
	GetDlgItem(IDC_RESET_BTN)->SetWindowPos(NULL, 1110 - 65 - 65, 465, 60, 30, NULL);
	GetDlgItem(IDC_SELECT_BTN)->SetWindowPos(NULL, 1110 -65, 465, 60, 30, NULL);
	GetDlgItem(IDC_DELETE_BTN)->SetWindowPos(NULL, 1110, 465, 60, 30, NULL);

	//
	//	로그를 출력하기 위한 static text 추가
	//
//	GetDlgItem(IDC_LOG_STATIC)->SetWindowPos(NULL, 0, 500 , 1200, 20, NULL);
//	SetDlgItemTextW(IDC_LOG_STATIC, L"프로그램이 실행되었습니다. ");
	
	//
	//	일단 창이 실행되면 list control을 출력해준다
	//
	ShowWindow(SW_SHOW);
	RedrawWindow();

	//
	//	loading창을 띄우며 스레드 실행
	//
	run_load_dlg(NULL);

	//
	//	data 입력
	//
	insert_naga_data();

	return TRUE;  // return TRUE  unless you set the focus to a control
}



///	@brief  data를 읽어오는 동안 로딩 다이얼로그를 출력하기 위한 함수
///
///
UINT CGUIDlg::run_load_dlg(LPVOID _mothod) {
	// 
	// Dialog 를 별도의 스레드에서 띄워줍니다.
	//
	CLoadDlg load_dlg;
	boost::thread* dlg_thread = new boost::thread([&load_dlg]() {
		load_dlg.DoModal();
	});

	//
	// 데이터를 가져오고...
	//
	get_naga_data(NULL);

	//
	// 로딩 다이얼로그를 종료합니다. 
	//
	load_dlg.EndDialog(0);

	// 
	// 다이얼로그를 띄워주었던 스레드 객체도 종료해줍니다. 
	// 
	dlg_thread->join();
	delete dlg_thread;

	return 1;
}

///	@brief  실제적으로 data를 읽어오는 함수
///
///
UINT CGUIDlg::get_naga_data(LPVOID _mothod) {
	CGUIDlg *pDlg = (CGUIDlg*)AfxGetApp()->m_pMainWnd;

	my_list.clear();
	black_list.clear();
	unknown_list.clear();

	//
	//	program 부분 (black list 받아옴)
	//
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

	//
	//	prefetch 파싱 부분 추가	(unknown list 받아옴)
	//
	if (!get_prefetch_info(&unknown_list)) {
		log_err "get_prefetch_info() err" log_end;
	}

	return 1;
}

///	@brief 읽어온 데이터를 리스트 컨트롤에 출력하는 함수
///
///
void CGUIDlg::insert_naga_data(void) {

	//
	//	black list 를 화면에 출력하는 부분
	//
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


	//
	//	unknown list 를 화면에 출력하는 부분
	//
	for (auto line : unknown_list) {

		log_err "--- %ws",line->id() log_end;
		//
		//	valid 하지 않을경우 출력하지 않음
		//
		if (!line->isValid()) {
			continue;
		}
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

///	@brief  Delete 버튼을 누를 시 실행되는 함수
///
void CGUIDlg::OnBnClickedDeleteBtn()
{
	delete_list.clear();
	del_unknown_list.clear();

	bool blackFlag = false, unknownFlag = false;

	log_warn "[ 사용자가 선택한 프로그램 ]" log_end;
	// TODO: Add your control notification handler code here

	for (int i = 0; i < m_listView.GetItemCount(); i++) {
		if (m_listView.GetCheck(i) == true) {
			CString p_kind = m_listView.GetItemText(i, 0);

			//
			//	black list인지 unknown list인지 판별
			//
			if ((p_kind.Find(_T("Security")) != -1) || (p_kind.Find(_T("Update")) != -1)) {
				blackFlag = true; unknownFlag = false;
			}
			else if (p_kind.Find(_T("Unknown")) != -1) {
				blackFlag = false; unknownFlag = true;
			}

			CString delete_name = m_listView.GetItemText(i, 1);

			//
			//	black list 인 경우
			//
			if (blackFlag) {
				pprogram temp = find_program(delete_name, my_list);
				log_info "black list : %ws", temp->name() log_end;
				if (temp != NULL) {
					delete_list.push_back(temp);
				}
			}

			//
			//	unknown list 인 경우
			//
			else if (unknownFlag) {
				punknownp temp = find_unknown(delete_name, unknown_list);
				log_info "unknown list : %ws", temp->id() log_end;
				if (temp != NULL) {
					del_unknown_list.push_back(temp);
				}
			}

		}
	}

	// delete_list 들의 uninstaller handle 실행! 
	for (auto mouse : delete_list) {
		STARTUPINFO startupInfo = { 0 };
		PROCESS_INFORMATION processInfo;
		startupInfo.cb = sizeof(STARTUPINFO);
		log_info "uninstaller : %ws", mouse->uninstaller() log_end;
		::CreateProcess(NULL, (LPWSTR)(mouse->uninstaller()), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);
	}

	// unknown_list 들의 uninstaller handle 실행! 
	for (auto mouse : del_unknown_list) {
		wstring full_path = mouse->id();
		wstring dir_path = directory_from_file_pathw(full_path.c_str());
		to_lower_string(dir_path);

		list<pprogram> temp_list;
		get_all_program(&temp_list);

		for (auto installed : temp_list) {
			wstring temp_full_path = installed->uninstaller();
			wstring installed_path = directory_from_file_pathw(temp_full_path.c_str());
			to_lower_string(installed_path);
			
			//
			//	exe 파일이 존재하는 경로와 프로그램 추가/제거에 있는 프로그램의 경로를 비교한다.
			//
			if (installed_path.find(dir_path) != wstring::npos) {
				mouse->setUninstaller(temp_full_path);
			}
		}

		STARTUPINFO startupInfo = { 0 };
		PROCESS_INFORMATION processInfo;
		startupInfo.cb = sizeof(STARTUPINFO);

		//
		//	uninstaller가 존재할 경우 uninstaller를 실행
		//
		if (!::CreateProcess(NULL, (LPWSTR)(mouse->uninstaller()), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo)) {
			wstring temp_path = mouse->id();
			wstring temp_dir = directory_from_file_pathw(temp_path.c_str());
			
			//
			//	uninstaller가 존재하지 않을 경우 해당 경로를 실행해준다.
			//
			ShellExecute(NULL, L"open", L"explorer.exe", temp_dir.c_str(), NULL, SW_SHOW);
		}
	}

}

///	@brief 이름으로 unknown 을 찾아서 반환하는 함수
///
punknownp CGUIDlg::find_unknown(CString program_name, std::list<punknownp> temp_list) {
	log_info "%ws", program_name log_end;
	int result;
	for (auto mine : temp_list) {
		wstring temp_id = mine->id();
		to_lower_string(temp_id);
		if (temp_id.find(program_name) != wstring::npos) {
			return mine;
		}
	}
	return NULL;
}

///	@brief 이름으로 black 을 찾아서 반환하는 함수
///
pprogram CGUIDlg::find_program(CString program_name, list<pprogram> temp_list) {
	log_info "%ws", program_name log_end;
	int result;
	for (auto mine : temp_list) {
		result = wcscmp(program_name, mine->name());
		if (result == 0) return mine;
	}
	return NULL;
}


///	@brief check box를 체크하면 실행되는 함수
///
void CGUIDlg::OnBnClickedSelectBtn()
{
	for (int i = 0; i < m_listView.GetItemCount(); i++)
	{
		m_listView.SetCheck(i, TRUE);
	}
}


///	@brief reset 버튼을 클릭하면 실행되는 함수
///
void CGUIDlg::OnBnClickedResetBtn()
{
	// TODO: Add your control notification handler code here
	m_listView.DeleteAllItems();
	my_list.clear();
	black_list.clear();
	unknown_list.clear();

	run_load_dlg(NULL);
	insert_naga_data();

}


///	@brief 종료될 떄 실행되는 함수
///			메모리 반환을 여기서 하자!
void CGUIDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	std::wstring user_temp_dir;
	get_temp_dirW(user_temp_dir);

	std::wstringstream create_strm;
	create_strm << user_temp_dir << L"Naga\\result";
	// TODO: Add your message handler code here
	delete_all_csv(create_strm.str().c_str(), 1);
	finalize_log();
	for (auto p : my_list)
	{
		delete p;
	}
	my_list.clear();
}

///	@brief  list control을 정렬하기 위한 함수
///
void CGUIDlg::OnLvnColumnclickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	int count = m_listView.GetItemCount();
	int unknown_loc = -1;

	for (int i = 0; i < count; i++)
	{
		//
		//	각각의 행을 읽어와 Unknown이 발견되면 Stop
		//
		CString name = m_listView.GetItemText(i, 0);
		if (name.Find(_T("Unknown")) != -1) {
			log_info "loc : %d", i log_end;
			unknown_loc = i;
			break;
		}
	}

	if (unknown_loc == -1) return;

	for (int j = unknown_loc; j < count; j++) {
		m_listView.DeleteItem(unknown_loc);
	}

	
	unknown_list.sort(unknown_list_sort());
	
	//
	//	unknown list 를 화면에 출력하는 부분
	//
	for (auto line : unknown_list) {
		//
		//	valid 하지 않을경우 출력하지 않음
		//
		if (!line->isValid()) {
			continue;
		}
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

///	@brief  list control의 행 별로 색깔을 입히기 위한 함수
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
		//
		//	black list인 경우
		//	(제거 대상인 금융권 보안 프로그램)
		if (p_type.Find(_T("Security")) != -1)		
		{
			pLVCD->clrText = RGB(33, 151, 216);
			pLVCD->clrTextBk = RGB(237, 255, 255);
		}

		//
		//	update list인 경우
		//	(업데이트가 필요한 금융권 보안 프로그램) like veraport
		else if (p_type.Find(_T("Update")) != -1)	
		{
			pLVCD->clrText = RGB(255, 107, 107);
			pLVCD->clrTextBk = RGB(253, 212, 212);

		}
		else
		{
			//
			//	unknown program의 경우
			//	색칠하지 않음
		}

		*pResult = CDRF_DODEFAULT;
	}

}


///	@brief	list control의 항목을 더블클릭시 실행되는 함수
///
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



HBRUSH CGUIDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here

	// TODO:  Return a different brush if the default is not desired
	if (pWnd->GetDlgCtrlID() == IDC_LOG_STATIC)
	{
		pDC->SetBkColor(RGB(255, 255, 255));
	}

	return hbr;
}
