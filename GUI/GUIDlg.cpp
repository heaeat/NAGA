
// GUIDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GUI.h"
#include "GUIDlg.h"
#include "program.h"
#include "afxdialogex.h"
#include "prefetch.h"
#include "phverify/verify.h"
#include "Win32Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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
	ON_BN_CLICKED(IDC_BTN_VERIFY, &CGUIDlg::OnBnClickedBtnVerify)
END_MESSAGE_MAP()


// CGUIDlg message handlers

BOOL CGUIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//
	//	파일 로그를 초기화한다. 
	// 
	std::wstring current_dir = get_current_module_dirEx();
	std::wstringstream strm;
	strm << current_dir << L"\\MouseTrap.log";

	if (true != initialize_log(log_mask_all,
							   log_level_debug,
							   log_to_file | log_to_con | log_to_ods,
							   strm.str().c_str()))
	{
		fwprintf(stderr, L"initialize_log() fail. give up! \n");
	}

	//
	//	로그의 출력 형식을 지정한다. 
	//
	set_log_format(false, false, false, false);


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
	LPWSTR szText[3] = { L"삭제여부" ,L"이름", L"버젼" };
	int nWidth[3] = { 60,300,100 };

	LV_COLUMN iCol;
	iCol.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	iCol.fmt = LVCFMT_LEFT;
	m_listView.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

	for (int i = 0; i < 3; i++) {
		iCol.pszText = szText[i];
		iCol.iSubItem = i;
		iCol.cx = nWidth[i];
		iCol.fmt = LVCFMT_CENTER;
		m_listView.InsertColumn(i, &iCol);
	}

	if (true != get_prefetch_info())
	{
		MessageBox(L"Can not get prefetch information", L"Error", 0);
		return TRUE;
	}

	my_list = compare_lists();
	for (auto mine : my_list)
	{
		insertData(const_cast<LPWSTR>(mine->name()), const_cast<LPWSTR>(mine->version()));
	}

	//	delete_all_csv(L"C:\\Temp\\result", 1);
	return TRUE;  // return TRUE  unless you set the focus to a control

}

void CGUIDlg::insertData(LPWSTR name, LPWSTR version) {
	LV_ITEM lvitem;
	int count = m_listView.GetItemCount();

	// 이름 입력
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = count;
	lvitem.iSubItem = 0;
	lvitem.pszText = L"";
	m_listView.InsertItem(&lvitem);

	// 이름 입력
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = count;
	lvitem.iSubItem = 1;
	lvitem.pszText = name;
	m_listView.SetItem(&lvitem);

	// 버젼 입력
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = count;
	lvitem.iSubItem = 2;
	lvitem.pszText = version;
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
		MessageBox(mouse->uninstaller());
		STARTUPINFO startupInfo = { 0 };
		PROCESS_INFORMATION processInfo;
		startupInfo.cb = sizeof(STARTUPINFO);
		::CreateProcess(NULL, (LPWSTR)(mouse->uninstaller()), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);
	}

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
	my_list = compare_lists();

	for (auto mine : my_list)
	{
		insertData(const_cast<LPWSTR>(mine->name()), const_cast<LPWSTR>(mine->version()));
	}

}

void CGUIDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here
//	delete_all_csv(L"C:\\Temp\\result", 1);
	finalize_log();
	for (auto p : my_list)
	{
		delete p;
	}
	my_list.clear();
}


void CGUIDlg::OnBnClickedBtnVerify()
{
	//
	// README - test 코드
	//
	
	const wchar_t* csv = L"c:\\Users\\unsor\\Downloads\\20181112123633_PECmd_Output_Timeline.csv";
	std::map<std::string, std::string> data;
	read_csv((wchar_t*)csv, &data);

	for (auto line : data)
	{
		// README
		//
		// UTF8 인코딩된 문자열을 MFC 콘트롤에 (기본설정으로) 안보일껄? (모르겠음)
		// 암튼 그래서 utf8 인코딩 된 문자열을 Wide char 문자열로 변경해서 
		// 컨트롤에 출력하기 
		insertData(L"pf", (LPWSTR)Utf8MbsToWcsEx(line.second.c_str()).c_str());
	}
	return;







	//
	// c:\windows\system32\ 아래 있는 exe 파일들의 전자서명을 검증한다.
	//

	std::list<std::wstring> files;
	log_info "trying to find exe/dll files..." log_end;
	bool ret = find_files(L"c:\\windows\\", [](_In_ DWORD_PTR tag, _In_ const wchar_t* path)->bool
		{
			auto files = (std::list<std::wstring>*)(tag);

			if (true == rstrnicmp(path, L".exe") || true == rstrnicmp(path, L".dll"))
			{
				log_info "found. %ws", path log_end;
				files->push_back(path);
			}

			if (files->size() == 10)
			{
				// 10 개의 실행파일만... 
				// false 를 리턴하면 파일 찾기를 중지한다. 
				log_info "Done..." log_end;
				return false;
			}
			else
			{
				return true;
			}

		},
		(DWORD_PTR)&files,	// tag
		false);				// recursive

	if (true != PhpVerifyInitialize())
	{
		MessageBoxW(L"전자서명 검증 모듈을 초기화 할 수 없습니다.",
					L"오류",
					0);
		return;
	}

	for (auto file : files)
	{
		std::wstring signer_name;
		VERIFY_RESULT vr = PhVerifyFile(file.c_str(), &signer_name);

		log_info "file=%ws, verify result=%u, signer=%ws",
			file.c_str(),
			vr,
			signer_name.c_str()
			log_end;

		if (vr == VrTrusted)
		{
			insertData((LPWSTR)file.c_str(), (LPWSTR)signer_name.c_str());
		}
		else
		{
			insertData((LPWSTR)file.c_str(), L"Not verified");
		}
	}

	PhpVerifyFinalize();
}
