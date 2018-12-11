
// GUIDlg.h : header file
//

#pragma once
#include "list.h"
#include "CBankDlg.h"
#include "CLoadDlg.h"
#include <thread>

// CGUIDlg dialog
class CGUIDlg : public CDialogEx
{
// Construction
public:
	CGUIDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GUI_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
	void insertData(LPWSTR type, LPWSTR name, LPWSTR version, LPWSTR lastuse, LPWSTR cert);
	CListCtrl m_listView;
	int m_nSelected;
	afx_msg void OnBnClickedDeleteBtn();
	pprogram find_program(CString program_name, std::list<pprogram> temp_list);
	afx_msg void OnBnClickedSelectBtn();
	afx_msg void OnBnClickedResetBtn();
	punknownp find_unknown(CString program_name, std::list<punknownp> temp_list);
	void get_naga_data(void);
	UINT run_load_dlg(LPVOID _mothod);
	#define COLNUM 5

private:
	std::list<pprogram> my_list;
	std::list<pprogram> delete_list;
public:
	afx_msg void OnDestroy();
	afx_msg void OnLvnColumnclickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnUpdate();
	afx_msg void OnNMRDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
};


