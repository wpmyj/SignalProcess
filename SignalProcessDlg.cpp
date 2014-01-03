// SignalProcessDlg.cpp : ʵ���ļ�
#include "stdafx.h"
#include <fstream>
#include <string>
#include <cmath>
#include <algorithm>

#include "SignalProcess.h"
#include "SignalProcessDlg.h"
#include "CSVFile.h"
#include "WaveletTransform.h"
#include "SetAvgDlg.h"
#include "PeakSeak.h"
#include "ShowHide.h"
#include "ConfigArgument.h"

//#include "AnalysisData.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSignalProcessDlg �Ի���
CSignalProcessDlg::CSignalProcessDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSignalProcessDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	pX = NULL;
	pY = NULL;
	pDWTX = NULL;
	pDWTY = NULL;
	dXMinVal = -0.01;
	dXMaxVal = 0.0;
	dYMinVal = -0.01;
	dYMaxVal = 0.0;
	m_OptTimes = 1;
	m_iavgN = 5; //Ĭ���ڲ�ƽ������Ϊ5

}

CSignalProcessDlg::~CSignalProcessDlg()
{
	if (pX!=NULL || pY!=NULL)
	{
		delete [] pX;
		delete [] pY;
	}
}

void CSignalProcessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MovingAverageTimes, m_MovingAvgTimes);
	DDX_Control(pDX, IDC_HALFPEAKWIDTH, m_HalfPeakWidth);
	DDX_Control(pDX, IDC_SLOPE, m_Slope);
	DDX_Control(pDX, IDC_DATAREPORT, m_Report);
	DDX_Control(pDX, IDC_COMBO1, m_avgN);
	DDX_Control(pDX, IDC_PEAKAREA, m_PeakArea);
	DDX_Control(pDX, IDC_EDIT2, m_RunTime);
	DDX_Control(pDX, IDC_SERIES, m_Series);
	DDX_Control(pDX, IDC_LIST3, m_Config);
	DDX_Control(pDX, IDC_EDIT1, m_SlopeFactor);
}

BEGIN_MESSAGE_MAP(CSignalProcessDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()

	ON_NOTIFY(NM_RCLICK, IDC_DATAREPORT, &CSignalProcessDlg::OnRClickDatareport)
	ON_NOTIFY(NM_DBLCLK, IDC_DATAREPORT, &CSignalProcessDlg::OnDblclkDatareport)
	ON_COMMAND(ID_ADDPEAK,AddNewPeak)
	ON_COMMAND(ID_DELPEAK,DelOnePeak)
	ON_WM_TIMER()
	ON_CBN_SELCHANGE(IDC_COMBO1, &CSignalProcessDlg::OnCbnSelchangeMoreAvgPara)
//	ON_BN_CLICKED(IDC_FUNCTEST, &CSignalProcessDlg::OnBnClickedFunctest)
//	ON_BN_CLICKED(IDC_BUTTON1, &CSignalProcessDlg::OnBnClickedDateLoadTest)
	ON_BN_CLICKED(IDC_READSHOW, &CSignalProcessDlg::OnBnClickedReadshow)
//	ON_BN_CLICKED(IDC_DELTEST, &CSignalProcessDlg::OnBnClickedDeltest)
//	ON_BN_CLICKED(IDC_COLTEST, &CSignalProcessDlg::OnBnClickedColtest)
//	ON_BN_CLICKED(IDC_SHOWHIDETEST, &CSignalProcessDlg::OnBnClickedShowhidetest)
//	ON_BN_CLICKED(IDC_BUTTON2, &CSignalProcessDlg::OnRealTimeTest)
//	ON_BN_CLICKED(IDC_PREPROCESS, &CSignalProcessDlg::OnPreprocessConfigArgs)
	ON_BN_CLICKED(IDC_FIRSTSCAN, &CSignalProcessDlg::OnFirstScan)
	ON_BN_CLICKED(IDC_SECONDSCAN, &CSignalProcessDlg::OnSecondScan)
	ON_BN_CLICKED(IDC_THIRDSCAN, &CSignalProcessDlg::OnThirdScan)
	ON_BN_CLICKED(IDC_FOURTHSCAN, &CSignalProcessDlg::OnFourthScan)
	ON_BN_CLICKED(IDC_FIFTHSCAN, &CSignalProcessDlg::OnFifthScan)
	ON_BN_CLICKED(IDC_SIXTHSCAN, &CSignalProcessDlg::OnSixthScan)
	ON_CBN_DROPDOWN(IDC_SERIES, &CSignalProcessDlg::OnCbnDropdownSeries)
	ON_BN_CLICKED(IDC_CLEARRESULT, &CSignalProcessDlg::OnBnClickedClearResult)
	ON_BN_CLICKED(IDC_PROCESSPORT1, &CSignalProcessDlg::OnBnClickedProcessPort1)
	ON_BN_CLICKED(IDC_PREPROCESS, &CSignalProcessDlg::OnBnClickedPreprocess)
END_MESSAGE_MAP()


// CSignalProcessDlg ��Ϣ��������

BOOL CSignalProcessDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE); // ���ô�ͼ��
	SetIcon(m_hIcon, FALSE); // ����Сͼ��
	
	CRect rcPeakSeek;
	GetDlgItem(IDC_PEAKSEEK)->GetWindowRect(rcPeakSeek);
	ScreenToClient(rcPeakSeek);
	m_ChcarCtrlPeakSeek.Create(this, rcPeakSeek, 0, WS_CHILD|WS_VISIBLE);
	m_pLinePeakSeek = m_ChcarCtrlPeakSeek.CreateLineSerie(); //����Ѱ������
	m_pPoint = m_ChcarCtrlPeakSeek.CreatePointsSerie(); //��������յ���
	m_pPeak  = m_ChcarCtrlPeakSeek.CreatePointsSerie(); //����������

	m_pdSaveArray = new double[32];
	for( int i = 0; i < 32; i++ )
	{
		m_pdSaveArray[i] = 0.0;
	}

	m_HalfPeakWidth.SetWindowText(_T("0.00"));
	m_Slope.SetWindowText(_T("5000"));
	m_PeakArea.SetWindowText(_T("���޴�"));
	m_avgN.SetCurSel(4);
	m_Series.SetCurSel(2);
	m_Report.SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);
	m_Report.SetBkColor(RGB(205,205,205));
	m_Report.SetTextBkColor(RGB(205,205,205));
	m_Report.SetTextColor(RGB(45,45,45));
	m_Report.InsertColumn(0, _T("No."),	   LVCFMT_LEFT,38);
	m_Report.InsertColumn(1, _T("����ʱ��"), LVCFMT_LEFT,68);
	m_Report.InsertColumn(2, _T("�����"),   LVCFMT_LEFT,68);
	m_Report.InsertColumn(3, _T("���"),     LVCFMT_LEFT,68);
	m_Report.InsertColumn(4, _T("���"),     LVCFMT_LEFT,68); //new
	m_Report.InsertColumn(5, _T("����"),   LVCFMT_LEFT,68);
	m_Report.InsertColumn(6, _T("������"),   LVCFMT_LEFT,74);
	m_Report.InsertColumn(7, _T("��������"), LVCFMT_LEFT,68);
	m_Report.InsertColumn(8, _T("��β����"), LVCFMT_LEFT,68);
	m_Config.SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);
	m_Config.SetBkColor(RGB(205,205,205));
	m_Config.SetTextBkColor(RGB(205,205,205));
	m_Config.SetTextColor(RGB(45,45,45));
	m_Config.InsertColumn(0, _T("��������"),  LVCFMT_LEFT, 100);
	m_Config.InsertColumn(1, _T("��ʼʱ��"),  LVCFMT_LEFT, 85);
	m_Config.InsertColumn(2, _T("����ʱ��"),  LVCFMT_LEFT, 85);
	m_Config.InsertColumn(3, _T("ֵ"),	    LVCFMT_LEFT, 90);
	
	return TRUE;  
}

void CSignalProcessDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù����ʾ��
HCURSOR CSignalProcessDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSignalProcessDlg::OnClose()
{
	delete[] m_pdSaveArray;
	CDialog::OnClose();
}

void CSignalProcessDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	CDialog::OnMouseMove(nFlags, point);
}

//ͬ��������ͼ����
void CSignalProcessDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	//CChartAxis* pAxisSelectX;
	//CChartAxis* pAxisSelectY;
	//double tempX_MIN;
	//double tempX_MAX; 
	//double tempY_MIN;
	//double tempY_MAX; 

	//if ( 1 /*m_rcChcarCtrl.PtInRect(point)*/ )
	//{
	//	//CSuperChartCtrl* pCtrl     = &m_ChcarCtrl;
	//	//CSuperChartCtrl* pFilter   = &m_ChcarCtrlFilter;
	//	CSuperChartCtrl* pPeakSeek = &m_ChcarCtrlPeakSeek;
	//	pAxisSelectY = pCtrl->GetLeftAxis();   //����Y����
	//	pAxisSelectY->GetMinMax( tempY_MIN, tempY_MAX );
	//	pAxisSelectX = pCtrl->GetBottomAxis(); //����X����
	//	pAxisSelectX->GetMinMax( tempX_MIN, tempX_MAX );
	//	
	//	if (tempY_MAX!=dYMaxVal || tempY_MIN!=dYMinVal || tempX_MAX!=dXMaxVal || tempX_MIN!=dXMinVal)
	//	{	//ͬ���˲��ؼ���ͼ
	//		pAxisSelectY = pFilter->GetLeftAxis();   //�����˲��ؼ�Y����
	//		pAxisSelectY->SetMinMax( tempY_MIN, tempY_MAX );
	//		pAxisSelectX = pFilter->GetBottomAxis(); //�����˲��ؼ�X����
	//		pAxisSelectX->SetMinMax( tempX_MIN, tempX_MAX );
	//		//ͬ��Ѱ��ؼ���ͼ
	//		pAxisSelectY = pPeakSeek->GetLeftAxis();   //����Ѱ��ؼ�Y����
	//		pAxisSelectY->SetMinMax( tempY_MIN, tempY_MAX );
	//		pAxisSelectX = pPeakSeek->GetBottomAxis(); //����Ѱ��ؼ�X����
	//		pAxisSelectX->SetMinMax( tempX_MIN, tempX_MAX );
	//		dYMaxVal = tempY_MAX;
	//		dYMinVal = tempY_MIN; 
	//		dXMaxVal = tempX_MAX;
	//		dXMinVal = tempX_MIN;
	//	}
	//}
	CDialog::OnLButtonUp(nFlags, point);
}

BOOL CSignalProcessDlg::PreTranslateMessage(MSG* pMsg)
{	
	return CDialog::PreTranslateMessage(pMsg);
}

//----------------------------------------------------------------------------------------------
// ��д��������ݽṹ���㷨���ֲ�����
//----------------------------------------------------------------------------------------------
void CSignalProcessDlg::ClearLineOn3rdCtrl()
{
	//m_ChcarCtrlPeakSeek.EnableRefresh(false);
	//m_pPoint->ClearSerie();					//����ϴθ�����յ�
	//m_pPeak->ClearSerie();					//����ϴθ����岨�ȵ�
	//m_pLinePeakSeek->ClearSerie();          //����ϴ�����

	//if (m_phemline.size()!=0) //�������
	//{
	//	for (unsigned i=0; i<m_phemline.size(); i++)
	//		m_phemline[i]->ClearSerie();  
	//}

	//if (m_pDividePeakBaseLine.size()!=0) //�����ֱ�ַ������
	//{
	//	typedef std::list<CChartLineSerie*>::iterator LI;
	//	for (LI iter=m_pDividePeakBaseLine.begin(); iter!=m_pDividePeakBaseLine.end(); iter++)
	//		(*iter)->ClearSerie();
	//	m_pDividePeakBaseLine.clear();
	//}

	//if (m_pTagline.size()!=0)//���֮ǰ�ı����
	//{
	//	typedef std::list<CChartLineSerie*>::iterator LI;
	//	for (LI iter=m_pTagline.begin(); iter!=m_pTagline.end(); iter++)
	//		(*iter)->ClearSerie();
	//	m_pTagline.clear();
	//}

	//m_ChcarCtrlPeakSeek.EnableRefresh(true);
	
}
void CSignalProcessDlg::DrawLineOn3rdCtrl()
{
	
	//m_ChcarCtrlPeakSeek.EnableRefresh(false);
	//m_pLinePeakSeek->SetColor(RGB(200, 200, 200)); //�������ߵ���ɫ(�׻�)
	//CChartAxis* m_pAxisSelect;
	//CSuperChartCtrl* pParent = &m_ChcarCtrlPeakSeek;
	//pParent->SetBackColor(RGB(60, 60, 60)); //���ñ�����ɫ
	//m_pAxisSelect = pParent->GetLeftAxis(); //����Y����
	//m_pAxisSelect->SetAutomatic(false);
	//m_pAxisSelect->SetCoordinate(dYMinVal*1.0, dYMaxVal*1.0, COORDINATE_AUTOCALC); //_SetMinMax(dYMinVal*1.0, dYMaxVal*1.1);
	//m_pAxisSelect = pParent->GetBottomAxis(); //����X����
	//m_pAxisSelect->SetAutomatic(false);
	//m_pAxisSelect->SetCoordinate(dXMinVal*1.0, dXMaxVal*1.0, COORDINATE_AUTOCALC); //_SetMinMax(dXMinVal, dXMaxVal);
	//m_pPoint->SetColor(RGB(255, 70, 70)); //���ñ����������ɫ(��ɫ)
	//m_pPeak->SetColor (RGB(0, 138, 138)); //���ñ겨�岨�ȵ����ɫ(��ɫ)
	//if (m_pAxisSelect==NULL) return;
	//m_pAxisSelect->SetVisible(true);

	//m_pLinePeakSeek->m_vPoints.InitPoints(m_OrigSema.size());
	//for (size_t i=0; i<m_OrigSema.size();++i)
	//{//������
	//	//m_pLinePeakSeek->AddPoint(m_OrigSema[i].x, m_OrigSema[i].y);
	//	m_pLinePeakSeek->m_vPoints.AddPoint(m_OrigSema[i].x, m_OrigSema[i].y);
	//}

	//m_pPoint->m_vPoints.InitPoints(m_Peak.size());
	//m_pPeak->m_vPoints.InitPoints(m_Peak.size());
	//for (unsigned i=0; i<m_Peak.size(); ++i)
	//{//���������
	//	double x[]={m_Peak[i].StartSema.x, m_Peak[i].FinSema.x};
	//	double y[]={m_Peak[i].StartSema.y, m_Peak[i].FinSema.y};
	//	m_pPoint->m_vPoints.AddPoints(x, y, 2);
	//	m_pPeak->m_vPoints.AddPoint(m_Peak[i].PeakSema.x, m_Peak[i].PeakSema.y);//���岨��
	//}

	//m_ChcarCtrlPeakSeek.EnableRefresh(true);
	//m_ChcarCtrlPeakSeek.RefreshCtrl();	
}
void CSignalProcessDlg::DrawBaseLineOn3rdCtrl(bool isDividePeak)
{
	//bool isdraw = true;
	////const unsigned nodeSemaSize = NodeSema.size();
	//const unsigned PeakSize = m_Peak.size();
	//if ((!isDividePeak) && (PeakSize!=0)) //�Ǵ�ֱ�ַ�
	//{
	//	for (unsigned i=0; i<PeakSize; ++i)
	//	{//������ֱ��ָ��
	//		CChartLineSerie* line;  
	//		line = m_ChcarCtrlPeakSeek.CreateLineSerie(); 
	//		line->SetColor(RGB(210, 0, 0)); //���û�����ɫ
	//		m_phemline.push_back(line);
	//	}
	//	if (m_phemline.size()==0)
	//	{
	//		isdraw = false;
	//	}
	//	else
	//	{
	//		const unsigned m_phemlineSize = m_phemline.size();
	//		for (unsigned i=0; i<PeakSize; ++i)
	//		{//����ֱ��
	//			m_phemline[i]->m_vPoints.InitPoints(4);
	//			double dx[] = { m_Peak[i].StartSema.x, m_Peak[i].FinSema.x };
	//			double dy[] = { m_Peak[i].StartSema.y, m_Peak[i].FinSema.y };
	//			m_phemline[i]->m_vPoints.AddPoints(dx, dy, 2);   //������
	//		}
	//	}
	//}

	//else if (PeakSize!=0 && isdraw)
	//{
	//	unsigned nodeIndex = 0;  //����ֱ�ַ��ߵ�����
	//	unsigned bg = 0;
	//	unsigned ed = bg;
	//	bool isSignalWave = false;
	//	for (; bg<PeakSize; ++bg)
	//	{
	//		if (m_Peak[bg].Type==1 || m_Peak[bg].Type==-1)
	//		{
	//			ed = bg;
	//			isSignalWave = true;
	//		}
	//		else /*(Peak[bg].Type>1)*/ //�ص�����
	//		{
	//			ed = bg;
	//			int mark = m_Peak[bg].Type;
	//			while (ed<PeakSize && m_Peak[ed].Type==mark)
	//			{
	//				if (ed+1==PeakSize && m_Peak[PeakSize-1].Type==mark)
	//				{
	//					break;
	//				}
	//				ed++;
	//				if (m_Peak[ed].Type!=mark)
	//				{
	//					ed--;
	//					break;
	//				}
	//			}
	//		}
	//		if (isSignalWave && nodeIndex!=PeakSize) //������ַ���
	//		{
	//			CChartLineSerie* line;  
	//			line = m_ChcarCtrlPeakSeek.CreateLineSerie();
	//			line->SetColor(RGB(210, 0, 0)); //���û�����ɫ
	//			double dx[] = { m_Peak[nodeIndex].StartSema.x, m_Peak[nodeIndex].FinSema.x };
	//			double dy[] = { m_Peak[nodeIndex].StartSema.y, m_Peak[nodeIndex].FinSema.y };
	//			nodeIndex++;
	//			line->m_vPoints.InitPoints(4);
	//			line->m_vPoints.AddPoints(dx, dy, 2);   
	//			isSignalWave = false;
	//			m_phemline.push_back(line);
	//		}
	//		else if (!isSignalWave && nodeIndex!=PeakSize) //��������ַ���
	//		{
	//			for (; bg!=ed; ++bg)
	//			{
	//				CChartLineSerie* line1/*, *line2, *line3*/;  
	//				line1 = m_ChcarCtrlPeakSeek.CreateLineSerie(); 
	//				//line2 = m_ChcarCtrlPeakSeek.CreateLineSerie();
	//				double dx[] = { m_Peak[nodeIndex].StartSema.x, m_Peak[nodeIndex].FinSema.x };
	//				double dy[] = { m_Peak[nodeIndex].StartSema.y, m_Peak[nodeIndex].FinSema.y };
	//				nodeIndex++;
	//				line1->SetColor(RGB(210, 0, 0)); //���û�����ɫ
	//				line1->m_vPoints.InitPoints(4);
	//				line1->m_vPoints.AddPoints(dx, dy, 2);
	//				m_phemline.push_back(line1);

	//				if (bg+1==ed)
	//				{
	//					//line3 = m_ChcarCtrlPeakSeek.CreateLineSerie();
	//					double dxx[] = { m_Peak[nodeIndex].StartSema.x, m_Peak[nodeIndex].FinSema.x };
	//					double dyy[] = { m_Peak[nodeIndex].StartSema.y, m_Peak[nodeIndex].FinSema.y };
	//					nodeIndex++;
	//					line1->m_vPoints.InitPoints(4);
	//					line1->m_vPoints.AddPoints(dxx, dyy, 2);
	//					m_phemline.push_back(line1);
	//				}
	//			}
	//		}
	//	}
	//}

	//m_ChcarCtrlPeakSeek.EnableRefresh(true);
	//m_ChcarCtrlPeakSeek.RefreshCtrl();	
}
void CSignalProcessDlg::DrawDividePeakBaseLine(bool isDividePeak)
{
	if (!isDividePeak)
	{
		return;
	}

	m_ChcarCtrlPeakSeek.EnableRefresh(false);

	const unsigned PeakSize = m_Peak.size();
	for (unsigned i=0; i<PeakSize-1; ++i)
	{
		if (abs(m_Peak[i].Type)!=1)
		{
			int PeakType = m_Peak[i].Type;
			while (m_Peak[i].Type == PeakType && m_Peak[i+1].Type == PeakType )
			{
				CChartLineSerie* line;  
				line = m_ChcarCtrlPeakSeek.CreateLineSerie(); //����һ�����ߵ�������
				line->SetColor(RGB(210, 0, 0)); //���û�����ɫ
				m_pDividePeakBaseLine.push_back(line);
				line->m_vPoints.InitPoints(4);
				double dx[] = { m_Peak[i].FinSema.x, m_Peak[i].FinSema.x };
				double dy[] = { m_OrigSema[m_Peak[i].FinSema.idx].y, m_Peak[i].FinSema.y };
				line->m_vPoints.AddPoints(dx, dy, 2);
				i++;
			}
		}
	}
	m_ChcarCtrlPeakSeek.EnableRefresh(true);

}
void CSignalProcessDlg::DrawTagLineOn3rdCtrl()
{	
	//�������
	const unsigned TagPointSize = m_TagPoint.size();
	for (unsigned i=0; i<TagPointSize/2; ++i)
	{//�����������ָ��
		CChartLineSerie* line; 
		line = m_ChcarCtrlPeakSeek.CreateLineSerie(); 
		line->SetColor(RGB(74, 165, 255)); //���ñ������ɫ
		m_pTagline.push_back(line);
	}
	const unsigned m_pTaglineSize = m_pTagline.size();
	typedef std::list<CChartLineSerie*>::iterator LI;
	unsigned i = 0;
	for (LI iter=m_pTagline.begin(); iter!=m_pTagline.end(); iter++)
	{//���ӱ����
		double dx[] = { m_TagPoint[i].x,  m_TagPoint[i+1].x };
		double dy[] = { m_TagPoint[i].y,  m_TagPoint[i+1].y };
		i+=2;
		(*iter)->AddPoints(dx, dy, 2);   //�����
	}
	m_ChcarCtrlPeakSeek.RefreshCtrl();	
}
void CSignalProcessDlg::OutputReport()
{
	// ������
	m_Report.DeleteAllItems(); //ȫ���һ��
	const int PeakSize = m_Peak.size();
	for (int i=0; i<PeakSize; ++i)
	{
		CString szRepo[9]; //szNo,szT,szArea,szHeight,szHalf,szType,szRatio,szTF;
		szRepo[0].Format(_T("%d"), i+1);
		szRepo[1].Format(_T("%0.4f"),m_Peak[i].PeakSema.x);       //��ı���ʱ��
		szRepo[2].Format(_T("%0.4f"),m_Peak[i].PeakArea);		  //�����
		szRepo[3].Format(_T("%0.4f"),m_Peak[i].PeakHeight);		  //���
		szRepo[4].Format(_T("%0.3f"),m_Peak[i].PeakWidth);        //���
		szRepo[5].Format(_T("%0.4f"),m_Peak[i].HalfPeakWidth);    //����

		CString szPeakType;
		szPeakType.Format(_T("%d"), m_Peak[i].Type);
		if (m_Peak[i].Type==1 )	//������
			szRepo[6] = _T("������") + szPeakType + _T("(+)");
		else if (m_Peak[i].Type==-1)
			szRepo[6] = _T("������") + szPeakType + _T("(-)");
		else if (m_Peak[i].Type>1)
			szRepo[6] = _T("������") + szPeakType + _T("(+)");
		else if (m_Peak[i].Type<-1)
			szRepo[6] = _T("������") + szPeakType + _T("(-)");
		else
			szRepo[6] = _T("�˹���");

		szRepo[7].Format(_T("%0.4f"),m_Peak[i].Disfactor);			//��������
		szRepo[8].Format(_T("%0.4f"),m_Peak[i].Tailing);			//��β����

		m_Report.InsertItem(i, szRepo[0], i); //���ӷ����
		for (int j=1; j<9; ++j)
		{
			m_Report.SetItemText(i, j, szRepo[j]);
		}
	}
}
void CSignalProcessDlg::OutputConfigArgs()
{
	m_Config.DeleteAllItems(); //ȫ���һ��
	const unsigned ArgsItemSize = m_ConfigArgs.size();
	for (unsigned i=0; i<ArgsItemSize; ++i)
	{
		CString szConfigArgs[4]; 
		szConfigArgs[0] = m_ConfigArgs[i].ArgName;  //��������
		szConfigArgs[1].Format(_T("%0.4f"),m_ConfigArgs[i].BeginTime);  //��ʼʱ��
		szConfigArgs[2].Format(_T("%0.4f"),m_ConfigArgs[i].EndTime);	//����ʱ��
		szConfigArgs[3].Format(_T("%0.2f"),m_ConfigArgs[i].InputValue); //ֵ

		if (m_Config.InsertItem(i, szConfigArgs[0]) != -1)
		{
			for (int j=1; j<4; ++j)
				if (!m_Config.SetItemText(i, j, szConfigArgs[j]))
					TRACE(_T("m_Config.SetItemText false"));
		}
		else
			TRACE(_T("m_Config.InsertItem false"));
	}
}
void CSignalProcessDlg::OutputResultConfigArgs() //������ȷ�����ò���
{
	const unsigned ListCtrlCount = m_Config.GetItemCount(); //��������б�����
	const unsigned ArgsItemSize = m_ConfigArgs.size();
	CString szLabel(_T("----���----"));
	m_Config.InsertItem(ListCtrlCount, szLabel);

	for (unsigned i=0; i<ArgsItemSize; ++i)
	{
		CString szConfigArgs[4]; 
		szConfigArgs[0] = m_ConfigArgs[i].ArgName;  //��������
		szConfigArgs[1].Format(_T("%0.4f"),m_ConfigArgs[i].BeginTime);  //��ʼʱ��
		szConfigArgs[2].Format(_T("%0.4f"),m_ConfigArgs[i].EndTime);	//����ʱ��
		if (szConfigArgs[0] == _T("��ֹ����"))
		{
			szConfigArgs[3] = _T("--");
		}
		if (szConfigArgs[0] == _T("�ֹ��ַ�"))
		{
			szConfigArgs[2] = _T("--");
			szConfigArgs[3] = _T("--");
		}
		if (szConfigArgs[0] == _T("�ϲ���"))
		{
			szConfigArgs[3] = _T("--");
		}
		if (szConfigArgs[0] == _T("�ص��崦��"))
		{
			if (m_ConfigArgs[i].InputValue == 1)
				szConfigArgs[3] = _T("��ֱ�ַ�");
			else if (m_ConfigArgs[i].InputValue == 2)
				szConfigArgs[3] = _T("���зַ�");
			else if (m_ConfigArgs[i].InputValue == 3)
				szConfigArgs[3] = _T("���зַ�");
			else
				szConfigArgs[3] = _T("��Ȼ���");
		}
		else if(szConfigArgs[0] == _T("�ֹ�����"))
		{
			if (m_ConfigArgs[i].InputValue == 11)
				szConfigArgs[3] = _T("���");
			else if (m_ConfigArgs[i].InputValue == 22)
				szConfigArgs[3] = _T("���");
			else if(m_ConfigArgs[i].InputValue == 33)
				szConfigArgs[3] = _T("�յ�");

			szConfigArgs[2] = _T("--");
		}
		else
		{
			szConfigArgs[3].Format(_T("%0.2f"),m_ConfigArgs[i].InputValue); //ֵ
		}
		

		if (m_Config.InsertItem(i+ListCtrlCount+1, szConfigArgs[0]) != -1)
		{
			for (int j=1; j<4; ++j)
				if (!m_Config.SetItemText(i+ListCtrlCount+1, j, szConfigArgs[j]))
					TRACE(_T("m_Config.SetItemText false"));
		}
		else
			TRACE(_T("m_Config.InsertItem false"));
	}
}
void CSignalProcessDlg::AddStringsOn3rdCtrl()
{
//	const unsigned PeakSize = m_Peak.size();
//	for (unsigned i=0; i<PeakSize; ++i)
//	{//������㲢����ֱ��
//		std::wstring szPeakInfo(L"");
//		wchar_t szNo[10];   //�����
//		_itow_s(i+1, szNo, 10, 10);
//		szPeakInfo += szNo;
//		wchar_t szTime[20]; //����ʱ��
//		double peakTime = m_Peak[i].PeakSema.x;
//		swprintf(szTime, 20, L"%.4lf", peakTime); 
//		szPeakInfo += L"��t";
//		szPeakInfo += szTime;
//		wchar_t szPeakH[20]; //���
//		double peakH = m_Peak[i].PeakHeight;
//		swprintf(szPeakH, 20, L"%.4lf", peakH); 
//		szPeakInfo += L"��h";
//		szPeakInfo += szPeakH;
//		szPeakInfo += L"��";
////		m_ChcarCtrlPeakSeek.AddString(m_Peak[i].PeakSema.x,m_Peak[i].PeakSema.y, szPeakInfo);  //�����ַ���
//	}
}
void CSignalProcessDlg::OnRClickDatareport(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	//CMenu mainMenu, popMenu;
	//mainMenu.CreatePopupMenu();
	//popMenu.CreatePopupMenu();
	//mainMenu.AppendMenu(MF_POPUP,(UINT)popMenu.m_hMenu, _T("�Ҽ�����ʽ�˵�"));
	////popMenu.AppendMenu(MF_STRING, ID_BASELINE, _T("���ߵ���"));
	//popMenu.AppendMenu(MF_STRING, ID_DELPEAK,  _T("����ɾ��"));
	//popMenu.AppendMenu(MF_STRING, ID_ADDPEAK,  _T("��������"));
	////SetMenu(&popMenu);
	//DWORD dwPos = GetMessagePos();
	//CPoint pt(LOWORD(dwPos), HIWORD(dwPos));  
	//mainMenu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_VERTICAL, pt.x, pt.y, this);
	//mainMenu.DestroyMenu();
	//*pResult = 0;
}
void CSignalProcessDlg::OnDblclkDatareport(NMHDR *pNMHDR, LRESULT *pResult)  
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	POSITION pos = m_Report.GetFirstSelectedItemPosition(); //��õ�ǰѡ�б����ڵ�����
	if(pos!=NULL)
	{
		//int Item = m_Report.GetNextSelectedItem(pos);
		//CString szPeakNo = m_Report.GetItemText(Item, 0); //��1���ݲ�����
		//CParaAdjust ParaAdjDlg;
		//ParaAdjDlg.m_DlgStyle = 1;
		//int PeakNo = _tstoi(szPeakNo);
		//ParaAdjDlg.m_PeakNo = PeakNo;
		//ParaAdjDlg.m_StartSema = m_Peak[PeakNo-1].StartSema.x;
		//ParaAdjDlg.m_FinSema = m_Peak[PeakNo-1].FinSema.x;
		//if (ParaAdjDlg.DoModal()==IDOK) 
		//{
		//	int a = ParaAdjDlg.m_PeakNo;
		//	double s = ParaAdjDlg.m_StartSema;
		//	double f = ParaAdjDlg.m_FinSema;
		//	Para para(a, s, f);
		//	neo_BaseLineAdj(m_OrigSema, m_Peak, para);
		//	CString opt(_T("modify"));
		//	SaveConfigParameter(opt, para);
		//}
		//ClearLineOn3rdCtrl();
		//DrawBaseLineOn3rdCtrl(false);
		//DrawLineOn3rdCtrl();
		//OutputReport();
	}
	
	*pResult = 0;
}

VOID CSignalProcessDlg::AddNewPeak()  //��������
{
	//CParaAdjust ParaAdjDlg;
	//ParaAdjDlg.m_DlgStyle = 2;
	//ParaAdjDlg.m_PeakNo = 0;
	//ParaAdjDlg.m_StartSema = 0;
	//ParaAdjDlg.m_FinSema = 0;
	//if (ParaAdjDlg.DoModal()==IDOK) 
	//{
	//	int a = ParaAdjDlg.m_PeakNo;
	//	double s = ParaAdjDlg.m_StartSema;
	//	double f = ParaAdjDlg.m_FinSema;
	//	Para para(a, s, f);
	//	neo_AddNewPeak(m_OrigSema, m_Peak, para);
	//	CString opt(_T("add"));
	//	SaveConfigParameter(opt, para);
	//}
	//ClearLineOn3rdCtrl();
	//DrawBaseLineOn3rdCtrl(false);
	//DrawLineOn3rdCtrl();
	//OutputReport();
}
VOID CSignalProcessDlg::DelOnePeak()
{
	//CParaAdjust ParaAdjDlg;
	//ParaAdjDlg.m_DlgStyle = 3;
	//ParaAdjDlg.m_StartSema = 0;
	//ParaAdjDlg.m_FinSema = 0;
	////ParaAdjDlg.m_iPeakNo.SetReadOnly(TRUE);
	//if (ParaAdjDlg.DoModal()==IDOK) 
	//{
	//	//int a = ParaAdjDlg.m_PeakNo;
	//	double s = ParaAdjDlg.m_StartSema;
	//	double f = ParaAdjDlg.m_FinSema;
	//	Para para(s, f);
	//	neo_DelOnePeak(m_OrigSema, m_Peak, para);
	//	CString opt(_T("delete"));
	//	SaveConfigParameter(opt, para);
	//}
	//ClearLineOn3rdCtrl();
	//DrawBaseLineOn3rdCtrl(false);
	//DrawLineOn3rdCtrl();
	//OutputReport();
}
VOID CSignalProcessDlg::SaveConfigParameter(CString &opt, const Para &para)
{
	//CString szConfig[5]; 
	//szConfig[0].Format(_T("%d"), m_OptTimes);
	//szConfig[1] = opt;       //��������
	//szConfig[2].Format(_T("%d"),para.pn);		  //���
	//szConfig[3].Format(_T("%0.4f"),para.st);		  //���ʱ��
	//szConfig[4].Format(_T("%0.4f"),para.ft);    //�յ�ʱ��

	//m_Config.InsertItem(m_OptTimes, szConfig[0], 1); //���ӷ����
	//for (int j=1; j<5; ++j)
	//{
	//	m_Config.SetItemText(m_OptTimes-1, j, szConfig[j]);
	//}
	//m_OptTimes++;

}
VOID CSignalProcessDlg::LoadConfigParameter()
{
	//int line = m_Config.GetItemCount();
	//for (int i=1; i<=line; ++i)
	//{
	//	CString szOpation = m_Config.GetItemText(i-1, 1); //��i�У���2�� ��0��ʼ������
	//	CString szPeakNo = m_Config.GetItemText(i-1, 2);
	//	CString szSTime = m_Config.GetItemText(i-1, 3);
	//	CString szFTime = m_Config.GetItemText(i-1, 4);
	//	int PeakNo = _wtoi(szPeakNo);
	//	double StartSema = _wtof(szSTime);
	//	double FinSema = _wtof(szFTime);
	//	Para p(PeakNo, StartSema, FinSema);

	//	if (szOpation=="modify")
	//		neo_BaseLineAdj(m_OrigSema, m_Peak, p);
	//	else if (szOpation=="add")
	//		neo_AddNewPeak(m_OrigSema, m_Peak, p);
	//	else if (szOpation=="delete")
	//		neo_DelOnePeak(m_OrigSema, m_Peak, p);
	//}
}
int  CSignalProcessDlg::GetMoreAvgPara()
{
	int avg = 5;
	CSetAvgDlg SetAvgDlg;
	if (SetAvgDlg.DoModal()==IDOK)
	{
		avg = SetAvgDlg.m_iParaAvgTimes;
	}
	m_avgN.UpdateData(FALSE);
	CString szContent(_T("�ڲ�")), szParaAvgTimes;//�ڲ�1��ƽ��
	szParaAvgTimes.Format(_T("%d"), avg);
	szContent += szParaAvgTimes + _T("��ƽ��");
	m_avgN.InsertString(m_avgN.GetCount()-1, szContent);
	m_avgN.SetCurSel(m_avgN.GetCount()-2);
	return avg;
}
void CSignalProcessDlg::OnCbnSelchangeMoreAvgPara()
{
	m_iavgN = m_avgN.GetCurSel()+1;  //����ƽ�����ڲ����ڲ���

	if (m_avgN.GetCount()!=21)
	{
		m_avgN.DeleteString(20);
		m_iavgN--;
	}
	if (m_iavgN==m_avgN.GetCount()) //����...
	{
		m_iavgN = GetMoreAvgPara();
	}
}
void CSignalProcessDlg::OnCbnDropdownSeries() //�ڲ������е��CCombox����ʱ�¼�
{
	if (m_Series.GetCount()>40)
	{
		m_Series.DeleteString(m_Series.GetCount()-1);
	}
}
//void CSignalProcessDlg::OnBnClickedFunctest()
//{
//	if (m_PeakSeak.m_OrigSema.size()==0)
//		return;
//
//	bool isDividePeak = false; //�Ƿ�����ֱ�ַ�
//	CButton *pBtnVDP = static_cast<CButton*>(GetDlgItem(IDC_VDIVIDEPEAK));
//	if (pBtnVDP!=NULL)
//	{
//		if (pBtnVDP->GetCheck()==BST_CHECKED)
//			isDividePeak = true;  //����������Ƿ�����ֱ�ַ�
//		else
//			isDividePeak = false;
//	}
//	else
//		return;
//	bool hasNagetiveSpike = true;     //�����Ƿ��и������
//	CButton *pBtnHNS = static_cast<CButton*>(GetDlgItem(IDC_NAGETIVESPIKE));
//	if (pBtnHNS!=NULL)
//	{
//		if (pBtnHNS->GetCheck()==BST_CHECKED)
//			hasNagetiveSpike = true;
//		else 
//			hasNagetiveSpike = false;
//	}
//	else
//		return;
//	CString szSlope;    //Ҫ����������б��
//	m_Slope.GetWindowText(szSlope);
//	double threshold = _wtof(szSlope);
//	if (szSlope==_T("5000")||szSlope.IsEmpty())
//		threshold = 5000/*37800*/;         //Ĭ������б��
//	CString szHalfPeakWidth;    //Ҫ���������
//	m_HalfPeakWidth.GetWindowText(szHalfPeakWidth);
//	double halfPeakWidth = _wtof(szHalfPeakWidth);
//	if (szHalfPeakWidth==_T("0.00")||szHalfPeakWidth.IsEmpty())
//		halfPeakWidth = 0.00;  //Ĭ�ϰ���
//	CString szPeakArea;    //Ҫ����������
//	m_PeakArea.GetWindowText(szPeakArea);
//	double peakArea = _wtof(szPeakArea);
//	if (szPeakArea==_T("���޴�")||szPeakArea.IsEmpty())
//		peakArea = 0.0;         //Ĭ����С�����
//	int series = m_Series.GetCurSel()+1;  //����ƽ�����ڲ����ڲ���
//	if (series == 0) series = 3;
//	m_PeakSeak.SetParameter(isDividePeak,hasNagetiveSpike,threshold,halfPeakWidth,peakArea,series,m_iavgN);	// ����Ѱ�����
//
//	DWORD StartTime = ::GetTickCount();
////	m_PeakSeak.ClearLine();	
//	DWORD FinishTime = ::GetTickCount();
//	DWORD  RunTime = FinishTime - StartTime;
//	CString szRunTime;
//	szRunTime.Format(_T("%u"), RunTime);
//	m_RunTime.SetWindowText(szRunTime);
////	m_PeakSeak.SetChartCtrl(&m_ChcarCtrlPeakSeek);
////	m_PeakSeak.SignalProc();			//(�ӿ�)�źŴ���
////	m_PeakSeak.SignalDisplay();
//	m_PeakSeak.GetPeakResult(m_Peak);   //��ý������
//	OutputReport();
//}
//void CSignalProcessDlg::OnBnClickedDateLoadTest()
//{
//	CShowHide ShowHide;
////	m_PeakSeak.GetShowHide(ShowHide.g1_IsShow, ShowHide.g2_IsShow);
//	if (ShowHide.DoModal()==IDOK)
//	{
//		DWORD StartTime = ::GetTickCount();
//
////		m_PeakSeak.SetShowHide(ShowHide.g1_IsShow, ShowHide.g2_IsShow);
////		m_PeakSeak.SetChartCtrl(&m_ChcarCtrlPeakSeek);	// ����ؼ�ָ��
////		m_PeakSeak.ClearLine();
////		m_PeakSeak.SignalDisplay();
//
//		DWORD FinishTime = ::GetTickCount();
//		DWORD  RunTime = FinishTime - StartTime;
//		CString szRunTime;
//		szRunTime.Format(_T("%u"), RunTime);
//		m_RunTime.SetWindowText(szRunTime);
//	}
//}
/*void CSignalProcessDlg::OnBnClickedDeltest()
{   */                                       
	//m_PeakSeak.ClearOrigCurve();      
	//m_PeakSeak.ClearStartFinishPoints();
	//m_PeakSeak.ClearBaseLine();
//}
//void CSignalProcessDlg::OnBnClickedColtest()
//{
	//CColorDialog coldlg;
//	//if (coldlg.DoModal()==IDOK)
//	{
////		m_PeakSeak.m_BaseLineColor = coldlg.GetColor();
//	}
////	m_PeakSeak.SetBaseLineColor(coldlg.GetColor());
//}
//void CSignalProcessDlg::OnBnClickedShowhidetest()
//{
//}
//void CSignalProcessDlg::OnRealTimeTest()
//{
	//static bool onlyOne = true;
	//static int InputNums = 0;
	//static double threshold = 1.0f;
	//static int series = 3;
	//static bool hasMinusPeak = false; 
	//static DWORD totalRunTime = 0;
	//const int STEPNUM = 100;


	//if (onlyOne)
	//{
	//	CFileDialog flDlg(TRUE,_T("csv"),_T(""),OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
	//		_T("csv��ʽɫ���ź��ļ�|*.csv||"));
	//	if (flDlg.DoModal()==IDOK)
	//	{
	//		m_PeakSeak.ClearSemaDate();
	//		m_CsvFilePathName = flDlg.GetPathName();
	//		onlyOne = !onlyOne;
	//		m_RealTime.SetSemaDate(m_CsvFilePathName);		// �����ź���
	//		m_RealTime.SetChartCtrl(&m_ChcarCtrlPeakSeek);	// ����ؼ�ָ��
	//		m_RealTime.RT_SetProcessParameter(true, true, 5, 3, threshold, 0.0f, 0.0f);

	//		hasMinusPeak = true;     //�����Ƿ��и������
	//		CButton *pBtnHNS = static_cast<CButton*>(GetDlgItem(IDC_NAGETIVESPIKE));
	//		if (pBtnHNS!=NULL)
	//		{
	//			if (pBtnHNS->GetCheck()==BST_CHECKED)
	//				hasMinusPeak = true;
	//			else 
	//				hasMinusPeak = false;
	//		}
	//		else
	//			return;

	//		CString szSlope;    //Ҫ����������б��
	//		m_Slope.GetWindowText(szSlope);
	//		threshold = _wtof(szSlope);
	//		if (szSlope==_T("5000")||szSlope.IsEmpty())
	//			threshold = 1.0f;         //Ĭ������б��
	//		series = m_Series.GetCurSel()+1;  //����ƽ�����ڲ����ڲ���
	//		if (series == 0) series = 3;

	//		SetTimer(1, 200, NULL);
	//	}
	//	else
	//		return;
	//}

	//if (static_cast<unsigned>(InputNums+STEPNUM) > m_RealTime.m_OrigSema.size()) //�߽���ƣ�ֹͣ��ʱ��
	//{
	//	KillTimer(1);
	//	Sleep(500);
	//	CString szTotalRunTime;
	//	szTotalRunTime.Format(_T("%u"), totalRunTime);
	//	CString szMsg(_T("�ܹ������ˣ�"));
	//	MessageBox(szMsg+szTotalRunTime);
	//	onlyOne = true;
	//	InputNums = 0;
	//	return;
	//}

	//DWORD StartTime = ::GetTickCount(); //ʱ��ͳ�ƿ�ʼ

	//m_RealTime.ClearRealTimeCurve();
	//m_RealTime.SetRealTimeData(InputNums, InputNums+STEPNUM);
	//InputNums += STEPNUM;
	//m_RealTime.RT_Averaging();
	////RealTime_Averaging(m_RealTime.m_InputSema, m_RealTime.m_OutputSema, m_RealTime.m_RTTotalAvgSema, m_RealTime.m_RTTotalSema, m_iavgN);

	//static std::vector<double> firstOrder;	//б�ʴ������
	//static double maxSlope = 0.0;
	//m_RealTime.RT_AvgDataSlope();
	//m_RealTime.RT_PeakSearch();
	//m_RealTime.RT_PeakReturn();
	////RealTime_GetAvgDataSlope(m_RealTime.m_OutputSema, m_RealTime.m_RTTotalAvgSema, firstOrder, maxSlope);
	////RealTime_GetpeakSearch_v2(m_RealTime.m_OutputSema, firstOrder, m_RealTime.m_RTTotalAvgSema, m_RealTime.m_RTPeak,threshold, hasMinusPeak, false, series);
	////RealTime_RestorePeakData(m_RealTime.m_OutputSema,m_RealTime.m_RTTotalSema, m_RealTime.m_RTPeak, m_iavgN, threshold);  
	//m_RealTime.DrawRealTimeCurve();
	//m_RealTime.DrawRealTimeResult();

	//DWORD FinishTime = ::GetTickCount(); //ʱ��ͳ�ƽ���
	//DWORD  RunTime = FinishTime - StartTime;
	//totalRunTime += RunTime;
	//CString szRunTime;
	//szRunTime.Format(_T("%u"), RunTime);
	//m_RunTime.SetWindowText(szRunTime);
//}
void CSignalProcessDlg::OnTimer(UINT_PTR nIDEvent)
{
//	OnRealTimeTest();
	CDialog::OnTimer(nIDEvent);
}
void CSignalProcessDlg::OnConfigArgsTest()
{

}


//���ݶ���
void CSignalProcessDlg::OnBnClickedReadshow()
{
	CFileDialog flDlg(TRUE,_T("csv"),_T(""),OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		_T("csv��ʽɫ���ź��ļ�|*.csv||"));
	if (flDlg.DoModal()==IDOK)
	{
		m_PeakSeak.ClearSemaDate();
		m_CsvFilePathName = flDlg.GetPathName();
	}
	else
		return;

	DWORD StartTime = ::GetTickCount();
	m_PeakDraw.ClearLine();	
	m_PeakSeak.SetSemaDate(m_CsvFilePathName);		// �����ź���
	m_PeakDraw.SetChartCtrl(&m_ChcarCtrlPeakSeek);	// ����ؼ�ָ��
	m_PeakDraw.SetCPeakSeakData(m_PeakSeak);
	m_PeakDraw.DrawCurve();

	DWORD FinishTime = ::GetTickCount();
	DWORD  RunTime = FinishTime - StartTime;
	CString szRunTime;
	szRunTime.Format(_T("%u"), RunTime);
	m_RunTime.SetWindowText(szRunTime);

//	InitSliderCtrl();
	GetDlgItem(IDC_PREPROCESS)->EnableWindow(TRUE); //����active
}

//����Ԥ����
void CSignalProcessDlg::OnPreprocessConfigArgs()
{
	if (m_PeakSeak.m_OrigSema.size()==0)
		return;

	bool isDividePeak = false; //�Ƿ�����ֱ�ַ�
	CButton *pBtnVDP = static_cast<CButton*>(GetDlgItem(IDC_VDIVIDEPEAK));
	if (pBtnVDP!=NULL)
	{
		if (pBtnVDP->GetCheck()==BST_CHECKED)
			isDividePeak = true;  //����������Ƿ�����ֱ�ַ�
		else
			isDividePeak = false;
	}
	else
		return;
	bool hasNagetiveSpike = true;     //�����Ƿ��и������
	CButton *pBtnHNS = static_cast<CButton*>(GetDlgItem(IDC_NAGETIVESPIKE));
	if (pBtnHNS!=NULL)
	{
		if (pBtnHNS->GetCheck()==BST_CHECKED)
			hasNagetiveSpike = true;
		else 
			hasNagetiveSpike = false;
	}
	else
		return;
	CString szSlope;    //Ҫ����������б��
	m_Slope.GetWindowText(szSlope);
	double threshold = _wtof(szSlope);
	if (szSlope==_T("5000")||szSlope.IsEmpty())
		threshold = 5000.0f;      //Ĭ������б��

	CString szSlopeFactor;
	m_SlopeFactor.GetWindowText(szSlopeFactor);
	double thresholdfactor = _wtof(szSlopeFactor);
	if (szSlopeFactor.IsEmpty())
		thresholdfactor = 1.0f;

	CString szPeakArea;		   //Ҫ����������
	m_PeakArea.GetWindowText(szPeakArea);
	double peakArea = _wtof(szPeakArea);
	if (szPeakArea==_T("���޴�")||szPeakArea.IsEmpty())
		peakArea = 0.0;         //Ĭ����С�����
	CString szHalfPeakWidth;    //Ҫ���������
	m_HalfPeakWidth.GetWindowText(szHalfPeakWidth);
	double halfPeakWidth = _wtof(szHalfPeakWidth);
	if (szHalfPeakWidth==_T("0.00")||szHalfPeakWidth.IsEmpty())
		halfPeakWidth = 0.00;  //Ĭ�ϰ���
	//���������Ĭ����С����������2���ڲ������������е㣬�ڲ�ƽ��������
	//�����е㷶Χ[1,10] �ڲ�ƽ��������Χ[1,20]
	//һ��Ĭ�������е�=3���ڲ�ƽ������=5
	//֮ǰ�ó��Ľ����ǣ��������3/4��Ϊ���������*100=�ڲ�ƽ��������
	//�������е��򲻱仯���̶���ĳһ����ֵ
	//�������Ժ������÷��*100=�����е�����ܴﵽ���ѵ�Ч��
	int series = 0;
	if (halfPeakWidth == 0.00)
	{
		series = m_Series.GetCurSel()+1;  //����ƽ�����ڲ����ڲ���
		if (series == 0) series = 3;
	}
	else
	{
		series = static_cast<int>(halfPeakWidth * 200);
		CString szContent(_T("�Զ�����"));
		CString szSeries;
		szSeries.Format(_T("%d"), series);
		szContent += szSeries;
		m_Series.InsertString(m_Series.GetCount(), szContent);
		m_Series.SetCurSel(m_Series.GetCount()-1);
	} 
	m_PeakSeak.SetParameter(isDividePeak, hasNagetiveSpike, threshold, thresholdfactor, halfPeakWidth, peakArea, 0.0f, series, m_iavgN);	// ����Ѱ�����	
	m_PeakDraw.SetChartCtrl(&m_ChcarCtrlPeakSeek);
	
	//ConfigArgument testCA(_T("�ֹ��ַ�"), 2.980000);   //////////////////////////////////////////////////
	//m_ConfigArgs.push_back(testCA);
	//��txt�ļ��ж�ȡ���ò���
//	CReadSaveArgs ReadSaveArgs;
//	ReadSaveArgs.ReadFromTxt(m_ConfigArgs); // E:\\PWB\\chem2009\\SignalProcess\\ConfigArgument.txt



	m_PeakSeak.m_AnalysisConfigArg.SetArgs(m_ConfigArgs);
	m_PeakSeak.m_AnalysisConfigArg.PreprocessConfigArgs(); //���ò���Ԥ����
	OutputConfigArgs();

	//double threshold,minHalfPeakWidth,minPeakArea,minPeakHeight;
	//CString szAnalysisData = pSample_Parameter_PeakProcess->GetXMLString();
	//if (pSample_Parameter_PeakProcess->GetValue_Double(XMLPATH_PEAKPROC_TRD, threshold)!=IAERROR_SUCCESS)  return FALSE;
	//if (pSample_Parameter_PeakProcess->GetValue_Double(XMLPATH_PEAKPROC_MPW, minHalfPeakWidth)!=IAERROR_SUCCESS) return FALSE;
	//if (pSample_Parameter_PeakProcess->GetValue_Double(XMLPATH_PEAKPROC_MPA, minPeakArea)!=IAERROR_SUCCESS) return FALSE;
	//if (pSample_Parameter_PeakProcess->GetValue_Double(XMLPATH_PEAKPROC_HEIGHT, minPeakHeight)!=IAERROR_SUCCESS) return FALSE;

	//// ����Ѱ����� (Ĭ��series, m_iavgN��Ҫ�޸�
	////��1-2���������Ƿ�����ֱ�ַ壬�Ƿ��и������
	////��3-5������������б�ʣ���С��������С���������С���
	//SetParameter(true, false, threshold, minHalfPeakWidth, minPeakArea, minPeakHeight);	

	//// ��ȡ���÷崦��������
	//CIADataTable cPPTimeTable; 
	//if (pSample_Parameter_PeakProcess->GetValue_Table(XMLPATH_PEAKPROC_PPT, cPPTimeTable) != IAERROR_SUCCESS) return FALSE;;
	//const long ConfigArgumentNum = cPPTimeTable.GetRowsNumber(); //�������������������

	//// ��Ҫ����Ĳ���������pAnalysisData��������
	//deque<ConfigArgument> cArgs;
	//for (long i=0; i<ConfigArgumentNum; ++i)
	//{
	//	//�ж��Ƿ񿪹�
	//	CString szState;
	//	if(!cPPTimeTable.GetCellValue(i, _T("State"), szState)) return FALSE;
	//	if (szState == _T("��")) continue;

	//	CString szName, szBegainTime, szEndTime, szValue;
	//	if(!cPPTimeTable.GetCellValue(i, _T("Name"), szName)) return FALSE; 
	//	if(!cPPTimeTable.GetCellValue(i, _T("BegainTime"), szBegainTime)) return FALSE;
	//	if(!cPPTimeTable.GetCellValue(i, _T("EndTime"), szEndTime)) return FALSE;
	//	if(!cPPTimeTable.GetCellValue(i, _T("Value"), szValue)) return FALSE;

	//	double BegainTime = _tstof(szBegainTime);
	//	double EndTime = _tstof(szEndTime);
	//	double Value = _tstof(szValue);

	//	if (szValue==_T("ǰ��"))
	//		Value = -1.0f;
	//	if (szValue==_T("����"))
	//		Value = 1.0f;

	//	ConfigArgument cArg(szName, BegainTime, EndTime,  Value);
	//	cArgs.push_back(cArg);
	//}
	////������Ҫ��������ת������XML�ϵ��ֶ�ת���ɷ崦������

	//m_AnalysisConfigArg.TransformArgs(cArgs);
	////.........................................................

	//m_AnalysisConfigArg.SetArgs(cArgs);
	//m_AnalysisConfigArg.PreprocessConfigArgs(); //���ò���Ԥ����



	GetDlgItem(IDC_FIRSTSCAN)->EnableWindow(TRUE); //��һ��ɨ�谴��active
	GetDlgItem(IDC_PREPROCESS)->EnableWindow(FALSE); //��ǰ����inactive
	GetDlgItem(IDC_READSHOW)->EnableWindow(FALSE);
}

//��һ��ɨ�裺Ѱ��ؼ���
void CSignalProcessDlg::OnFirstScan()
{
	m_PeakDraw.ClearLine();	
	//m_PeakSeak.ConfigArgs_FirstScan(); //(�ӿ�)�źŴ���:Ѱ��ؼ���
	m_PeakSeak.SP_FirstScan();
	m_PeakDraw.SetCPeakSeakData(m_PeakSeak);
	m_PeakDraw.SignalDisplay();
	GetDlgItem(IDC_SECONDSCAN)->EnableWindow(TRUE); //�ڶ���ɨ�谴��active
	GetDlgItem(IDC_FIRSTSCAN)->EnableWindow(FALSE); //��һ��ɨ�谴��inactive
}

//�ڶ���ɨ�裺ȥ����Ҫ�ķ�
void CSignalProcessDlg::OnSecondScan()
{
	m_PeakDraw.ClearLine();	
	//m_PeakSeak.ConfigArgs_SecondScan(); //(�ӿ�)�źŴ���:ȥ����Ҫ�ķ�
	m_PeakSeak.SP_SecondScan();
	m_PeakDraw.SetCPeakSeakData(m_PeakSeak);
	m_PeakDraw.SignalDisplay();

	m_PeakSeak.GetPeakResult(m_Peak);
	OutputReport();
	GetDlgItem(IDC_THIRDSCAN)->EnableWindow(TRUE);  //�����ΰ���active
	GetDlgItem(IDC_SECONDSCAN)->EnableWindow(FALSE); //�ڶ���ɨ�谴��inactive
}

//������ɨ�裺�ص��崦��
void CSignalProcessDlg::OnThirdScan()
{
	m_PeakDraw.ClearLine();
	//(�ӿ�)�źŴ���:�ص��崦��(��ֱ�ַ壬������ߣ����߷ַ�
	m_PeakSeak.SP_ThirdScan();
	m_PeakDraw.SetCPeakSeakData(m_PeakSeak);
	m_PeakDraw.SignalDisplay();
	
	GetDlgItem(IDC_FOURTHSCAN)->EnableWindow(TRUE);
	GetDlgItem(IDC_THIRDSCAN)->EnableWindow(FALSE);  //�����ΰ���inactive
	m_PeakSeak.GetPeakResult(m_Peak);
	OutputReport();
}

//��4��ɨ�裺���ߵ�У��
void CSignalProcessDlg::OnFourthScan()
{
	m_PeakDraw.ClearLine();
    //(�ӿ�)�źŴ���:����У��
	m_PeakSeak.SP_FourthScan();
	m_PeakSeak.SP_FourthScan(); // 1 more time
	m_PeakDraw.SetCPeakSeakData(m_PeakSeak);
	m_PeakDraw.SignalDisplay();

	GetDlgItem(IDC_FIFTHSCAN)->EnableWindow(TRUE);
	GetDlgItem(IDC_FOURTHSCAN)->EnableWindow(FALSE);  //���Ĵΰ���inactive
	m_PeakSeak.GetPeakResult(m_Peak);
	OutputReport();
}

//�����ɨ�裺�ֹ�������
void CSignalProcessDlg::OnFifthScan()
{
	m_PeakDraw.ClearLine();
	//m_PeakSeak.ConfigArgs_FourthScan(); //(�ӿ�)�źŴ���:�ֹ������壨�ϲ��塢�ֹ��ַ塢�ֹ����ߡ��ֹ��ⶥ
	m_PeakSeak.SP_FifthScan();
	m_PeakDraw.SetCPeakSeakData(m_PeakSeak);
	m_PeakDraw.SignalDisplay();

	GetDlgItem(IDC_SIXTHSCAN)->EnableWindow(TRUE);
	GetDlgItem(IDC_FIFTHSCAN)->EnableWindow(FALSE);  //����ΰ���inactive
	
	m_PeakSeak.GetPeakResult(m_Peak);
	OutputReport();
}

//������ɨ�裺���ֲ�����
void CSignalProcessDlg::OnSixthScan()
{
	m_PeakDraw.ClearLine();
	//m_PeakSeak.ConfigArgs_SixthScan(); //(�ӿ�)�źŴ���:��������
	m_PeakSeak.SP_SixthScan();
	m_PeakDraw.SetCPeakSeakData(m_PeakSeak);
	m_PeakDraw.SignalDisplay();
	
	m_Peak.clear();
	m_PeakSeak.GetPeakResult(m_Peak);
	OutputReport();
	m_ConfigArgs.clear();
	m_ConfigArgs = m_PeakSeak.m_AnalysisConfigArg.GetResultArgument();

	//�����ս�������б����浽txt�ļ���ȥ
//	CReadSaveArgs ReadSaveArgs;
//	ReadSaveArgs.Write2Txt(&m_ConfigArgs);

	OutputResultConfigArgs(); //�������ս�������б�

	GetDlgItem(IDC_SIXTHSCAN)->EnableWindow(FALSE);  //�����ΰ���inactive

	CString szMsg;
	m_PeakSeak.GetPeakTypeStr(szMsg);
	MessageBox(szMsg);

}

//������
void CSignalProcessDlg::OnBnClickedClearResult() 
{
	GetDlgItem(IDC_READSHOW)  ->EnableWindow(TRUE);
	GetDlgItem(IDC_PREPROCESS)->EnableWindow(FALSE);  //����Ԥ����inactive
	GetDlgItem(IDC_FIRSTSCAN) ->EnableWindow(FALSE);  //��һ�ΰ���inactive
	GetDlgItem(IDC_SECONDSCAN)->EnableWindow(FALSE);  //�ڶ��ΰ���inactive
	GetDlgItem(IDC_THIRDSCAN) ->EnableWindow(FALSE);  //�����ΰ���inactive
	GetDlgItem(IDC_FOURTHSCAN)->EnableWindow(FALSE);  //���Ĵΰ���inactive
	GetDlgItem(IDC_FIFTHSCAN) ->EnableWindow(FALSE);  //����ΰ���inactive
	GetDlgItem(IDC_SIXTHSCAN) ->EnableWindow(FALSE);  //�����ΰ���inactive

	m_PeakSeak.ClearSemaDate();
	m_PeakDraw.ClearLine();
}


void CSignalProcessDlg::OnBnClickedProcessPort1()  //���ֽӿ�1
{
	//��ȡXML��Ʒ�ļ��ڵĻ��ֲ���
	CFileDialog flDlg(TRUE,_T("cg.smp"),_T(""),OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		_T("��Ʒ�ļ�|*.cg.smp||"));
	CString XMLFilePath;
	if (flDlg.DoModal()==IDOK)
		XMLFilePath = flDlg.GetPathName();
	else
		return;

//	CAnalysisData analysisData;
//	analysisData.LoadFromFile(XMLFilePath);
//	CString cAllXML = analysisData.GetXMLString();
//	CString cRoot = analysisData.GetXMLPathString();
//	CString cPath = analysisData.m_strXMLPath;
//	CIADataTable cPeakListTable(_T("Sample"), _T("<PeakListTable/>"));
//	
//	m_PeakDraw.SetChartCtrl(&m_ChcarCtrlPeakSeek);
//	
//	//m_PeakSeak.SignalProcess(&analysisData, &cPeakListTable); //���ֽӿ�1
//	//m_PeakSeak.GetPeakResult(m_Peak);
//
//	//m_Peak =  m_PeakSeak.SignalProcessResult(&analysisData);//���ֽӿ�2
//
////	m_PeakSeak.ClearLine();	
////	m_PeakSeak.CA_SignalDisplay();
//	
//	OutputReport();
//
//	CString Observer = cPeakListTable.GetXMLString();

}


void CSignalProcessDlg::OnBnClickedPreprocess()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
}