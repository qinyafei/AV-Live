// RtmpClientPlayerDlg.h : 头文件
//

#pragma once

#include "H264Render.h"
#include "RtmpPlayer.h"
#include "RtmpPublish.h"


// CRtmpClientPlayerDlg 对话框
class CRtmpClientPlayerDlg : public CDialog
{
// 构造
public:
	CRtmpClientPlayerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_RTMPCLIENTPLAYER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

public:
	H264Render* render_;
	RtmpPlayer* player_;
	RtmpPublish* publish_;

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonPublish();
	afx_msg void OnBnClickedButtonPlayfile();
	afx_msg void OnBnClickedButtonSelect();
};
