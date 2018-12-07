// CStartDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CStartDlg.h"
#include "afxdialogex.h"
#include "resource.h"


// CStartDlg dialog

IMPLEMENT_DYNAMIC(CStartDlg, CDialogEx)

CStartDlg::CStartDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_START_DIALOG, pParent)
{

}

CStartDlg::~CStartDlg()
{
}

void CStartDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_START_BTN, m_btnStart);
}


BEGIN_MESSAGE_MAP(CStartDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_BN_CLICKED(IDC_START_BTN, &CStartDlg::OnBnClickedStartBtn)
END_MESSAGE_MAP()


// CStartDlg message handlers


void CStartDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: Add your message handler code here and/or call default

	CDialogEx::OnSysCommand(nID, lParam);
}


BOOL CStartDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

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

	CPngImage logo_image;
	VERIFY(logo_image.Load(IDB_PNG1));
	CStatic* m_logo = (CStatic*)GetDlgItem(IDC_START_LOGO);
	m_logo->SetBitmap(logo_image);
		

	m_btnStart.LoadStdImage(IDC_START_PNG, _T("PNG"));	//	대기상태일 때 이미지
//	m_btnStart.LoadAltImage(IDC_START_PNG, _T("PNG"));	//	커서를 올려놓았을 때 이미지
	m_btnStart.EnableToggle(FALSE);
	
	m_btnStart.MoveWindow(300,300,120, 60);					// x, y, width, height
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}




void CStartDlg::OnBnClickedStartBtn()
{
	// TODO: Add your control notification handler code here
	CGUIDlg dlg;
	dlg.DoModal();
}
