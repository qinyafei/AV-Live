// RtmpClientPlayerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "RtmpClientPlayer.h"
#include "RtmpClientPlayerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CRtmpClientPlayerDlg 对话框




CRtmpClientPlayerDlg::CRtmpClientPlayerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRtmpClientPlayerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

}

void CRtmpClientPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CRtmpClientPlayerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_START, &CRtmpClientPlayerDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_PUBLISH, &CRtmpClientPlayerDlg::OnBnClickedButtonPublish)
	ON_BN_CLICKED(IDC_BUTTON_PLAYFILE, &CRtmpClientPlayerDlg::OnBnClickedButtonPlayfile)
	ON_BN_CLICKED(IDC_BUTTON_SELECT, &CRtmpClientPlayerDlg::OnBnClickedButtonSelect)
END_MESSAGE_MAP()


// CRtmpClientPlayerDlg 消息处理程序

BOOL CRtmpClientPlayerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	render_ = new H264Render();
	player_ = new RtmpPlayer();
	publish_ = new RtmpPublish();

	CString rtmpUrl = "rtmp://192.168.8.240/myapp/livestream";
	GetDlgItem(IDC_EDIT_RTMP_URL)->SetWindowText(rtmpUrl);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRtmpClientPlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRtmpClientPlayerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}


//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CRtmpClientPlayerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CRtmpClientPlayerDlg::OnBnClickedButtonStart()
{
	// TODO: 在此添加控件通知处理程序代码
	CString url;
	GetDlgItem(IDC_EDIT_RTMP_URL)->GetWindowText(url);
	CWnd* pWnd = GetDlgItem(IDC_STATIC_VIDEO);
	if (player_ != NULL && pWnd != NULL)
	{
		player_->init(pWnd->m_hWnd);
		player_->setupRtmp((LPCTSTR)url);
	}

}


void CRtmpClientPlayerDlg::OnBnClickedButtonPublish()
{
	// TODO: 在此添加控件通知处理程序代码
	CString url;
	GetDlgItem(IDC_EDIT_RTMP_URL)->GetWindowText(url);
	CString file;
	GetDlgItem(IDC_EDIT_FILEPATH)->GetWindowText(file);
	if (publish_ != NULL)
	{
		publish_->publishH264((LPCTSTR)url, (LPCTSTR)file);
	}
}


void CRtmpClientPlayerDlg::OnBnClickedButtonPlayfile()
{
	// TODO: 在此添加控件通知处理程序代码
	CWnd* pWnd = GetDlgItem(IDC_STATIC_VIDEO);
	CString file;
	GetDlgItem(IDC_EDIT_FILEPATH)->GetWindowText(file);
	if (render_ != NULL && pWnd != NULL)
	{
		render_->init(pWnd->m_hWnd);
		render_->renderFile((LPCTSTR)file);
	}
}


void CRtmpClientPlayerDlg::OnBnClickedButtonSelect()
{
	// TODO: 在此添加控件通知处理程序代码
	CString filePath;
	CFileDialog dlg(TRUE, "Bsr", NULL, 0,
		"Bmpeg Files (*.h264)|*.bsr|All Files (*.*)|*.*||", AfxGetMainWnd());
	//dlg.m_ofn.lpstrInitialDir = gSysParamerter.GetVideoPath();
	if (dlg.DoModal() == IDOK)
	{
		filePath = dlg.GetPathName();
	}
	else
	{
		return ;
	}

	GetDlgItem(IDC_EDIT_FILEPATH)->SetWindowText(filePath);
}
