#include "stdafx.h"
#include "ModuleFunc.h"
#include "Common.h"

#include "ProcessFunc.h"
#include <WinVer.h>

extern BOOL bIsChecking;
extern HANDLE g_hDevice;

ULONG_PTR m_DriverObject;

CListCtrl* g_ListCtrl_Module;

vector<DRIVER_INFO> m_DriverList;
CImageList m_TreeModuleImageList;

COLUMNSTRUCT g_Column_Module[] = 
{
	{	L"������",		130	},
	{	L"����ַ",		125	},
	{	L"��С",			70	},
	{	L"��������",		125	},
	{	L"����·��",		200	},
	{	L"��������",		80	},
	{	L"�������",		125	},
	{	L"����˳��",		65	},
	{	L"�ļ�����",		120	}
};

UINT g_Column_Module_Count  = 9;	  //�����б�����

extern int dpix;
extern int dpiy;



void HsInitModuleList(CListCtrl *m_ListCtrl)
{
	g_ListCtrl_Module = m_ListCtrl;

	m_ListCtrl->SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP);

	UINT i;
	for (i = 0;i<g_Column_Module_Count;i++)
	{
		if (i == 2)
		{
			m_ListCtrl->InsertColumn(i, g_Column_Module[i].szTitle,LVCFMT_RIGHT,(int)(g_Column_Module[i].nWidth*(dpix/96.0)));
		}
		else
		{
			m_ListCtrl->InsertColumn(i, g_Column_Module[i].szTitle,LVCFMT_LEFT,(int)(g_Column_Module[i].nWidth*(dpix/96.0)));
		}
	}
}


DWORD WINAPI HsQueryModuleFunction(CListCtrl *m_ListCtrl)
{
	bIsChecking = TRUE;

	HsQueryModuleList(m_ListCtrl);

	bIsChecking = FALSE;
	return 0;
}

void HsQueryModuleList(CListCtrl *m_ListCtrl)
{
	DWORD dwReturnSize = 0;
	DWORD dwRet = 0;
	int ModuleCount = 0;

	while (m_TreeModuleImageList.Remove(0));
	m_ListCtrl->DeleteAllItems();

	ULONG ulReturnSize = 0;
	BOOL bRet = FALSE;


	m_DriverList.clear();

	ULONG ulCount = 1000;
	PALL_DRIVERS Drivers = NULL;

	do 
	{
		ULONG ulSize = 0;

		if (Drivers)
		{
			free(Drivers);
			Drivers = NULL;
		}

		ulSize = sizeof(ALL_DRIVERS) + ulCount * sizeof(DRIVER_INFO);

		Drivers = (PALL_DRIVERS)malloc(ulSize);
		if (!Drivers)
		{
			break;
		}

		memset(Drivers,0,ulSize);


		bRet = DeviceIoControl(g_hDevice,HS_IOCTL(HS_IOCTL_MODU_MODULELIST),
			NULL,
			0,
			Drivers,
			ulSize,
			&ulReturnSize,
			NULL);

		ulCount = (ULONG)Drivers->ulCount + 100;

	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && Drivers->ulCount > 0)
	{
		for (ULONG i = 0;i<Drivers->ulCount; i++)
		{
			FixDriverPath(&Drivers->Drivers[i]);
			m_DriverList.push_back(Drivers->Drivers[i]);
		}
	}

	if (Drivers)
	{
		free(Drivers);
		Drivers = NULL;
	}

	//////////////////////////////////////////////////////////////////////////

	if (m_DriverList.empty())
	{
		return;
	}


	for (vector <DRIVER_INFO>::iterator Iter = m_DriverList.begin( ); 
		Iter != m_DriverList.end( ); 
		Iter++ )
	{
		DRIVER_INFO DriverInfor = *Iter;

		WCHAR wzDriverName[MAX_PATH] = {0};
		WCHAR szWindowsText[MAX_PATH] = {0};


		CString  strDriverName;
		CString  strBase;
		CString  strSize;
		CString  strObject;
		CString  strLoadCount;
		CString  strComp;
		CString  strDriverStart;


		//���ϵͳ��ȫ��������Ϣ
		//GetServiceKeys();


		wcsncpy_s(wzDriverName, DriverInfor.wzDriverPath,wcslen(DriverInfor.wzDriverPath));


		//
		// ���������
		//
		WCHAR* Temp = wcsrchr(wzDriverName, '\\');

		


		strBase.Format(L"0x%p",DriverInfor.Base);
		strSize.Format(L"0x%X",DriverInfor.Size);
		if (DriverInfor.DriverObject)
		{
			strObject.Format(L"0x%p",DriverInfor.DriverObject);
		}
		else
		{
			strObject = L"-";
		}
		strLoadCount.Format(L"%d",DriverInfor.LodeOrder);
		if (DriverInfor.DriverObject)
		{
			strDriverStart.Format(L"0x%p",DriverInfor.DirverStartAddress);
		}
		else
		{
			strDriverStart = L"-";
		}
		
		int n = g_ListCtrl_Module->GetItemCount();

		int j = 0;

		LPCTSTR lpModuleName;

		if (Temp!=NULL)
		{
			lpModuleName = Temp+1;
		}
		else if (wcslen(DriverInfor.wzDriverPath)>0)
		{
			lpModuleName = DriverInfor.wzDriverPath;
			
		} else
		{
			continue;
		}


		AddModuleFileIcon(DriverInfor.wzDriverPath);
		int nImageCount = m_TreeModuleImageList.GetImageCount()-1;
		j = g_ListCtrl_Module->InsertItem(n,lpModuleName, nImageCount);

		
		
		ModuleCount++;
		CString CSStatusDetail;
		CSStatusDetail.Format( L"����ģ�����ڼ��ء� ��������%d",ModuleCount);
		HsSendStatusDetail(CSStatusDetail.GetBuffer());
		


		//����ļ�����
		if (PathFileExists(DriverInfor.wzDriverPath))
		{
			strComp = HsGetFileCompanyName(DriverInfor.wzDriverPath);
		}

		else
		{
			strComp = L"�ļ�������";
		}


		g_ListCtrl_Module->SetItemText(j, 1, strBase);
		g_ListCtrl_Module->SetItemText(j, 2, strSize);
		g_ListCtrl_Module->SetItemText(j, 3, strObject);
		g_ListCtrl_Module->SetItemText(j, 4, DriverInfor.wzDriverPath);
		g_ListCtrl_Module->SetItemText(j, 5, DriverInfor.wzKeyName);
		g_ListCtrl_Module->SetItemText(j, 6, strDriverStart);
		g_ListCtrl_Module->SetItemText(j, 7, strLoadCount);
		g_ListCtrl_Module->SetItemText(j, 8, strComp);
		//g_ListCtrl_Module->SetItemData(j,j);
		if (_wcsnicmp(strComp,L"Microsoft Corporation", wcslen(L"Microsoft Corporation"))==0)
		{
			g_ListCtrl_Module->SetItemData(j,1);
		}
	}

	CString CSStatusDetail;
	CSStatusDetail.Format( L"����ģ�������ɡ� ��������%d",ModuleCount);
	HsSendStatusDetail(CSStatusDetail.GetBuffer());
}




void FixDriverPath(PDRIVER_INFO DriverInfor)
{
	if (!DriverInfor || wcslen(DriverInfor->wzDriverPath) == 0)
	{
		return;
	}

	WCHAR wzWindowsDirectory[MAX_PATH] = {0};
	WCHAR wzDriverDirectory[MAX_PATH] = {0};	
	WCHAR wzDriver[] = L"\\System32\\Drivers\\";

	GetWindowsDirectory(wzWindowsDirectory, MAX_PATH - 1);
	wcscpy_s(wzDriverDirectory, wzWindowsDirectory);
	wcscat_s(wzDriverDirectory, wzDriver);

	WCHAR* wzOriginPath = DriverInfor->wzDriverPath;
	WCHAR  wzPath[MAX_PATH] = {0};
	WCHAR* wzTemp = wcschr(wzOriginPath, L'\\');

	// û��Ŀ¼��Ϣ��ֻ��һ���������ֵģ�ֱ��ƴ��DriverĿ¼��
	if (!wzTemp)
	{
		wcscpy_s(wzPath, wzDriverDirectory);
		wcscat_s(wzPath, wzOriginPath);
		wcscpy_s(DriverInfor->wzDriverPath, wzPath);

		wzOriginPath[wcslen(wzPath)] = L'\0';
	}
	else
	{
		WCHAR wzUnknow[] = L"\\??\\";
		WCHAR wzSystemRoot[] = L"\\SystemRoot";
		WCHAR wzWindows[] = L"\\Windows";
		WCHAR wzWinnt[] = L"\\Winnt";
		size_t nOrigin = wcslen(wzOriginPath);

		if ( nOrigin >= wcslen(wzUnknow) && !_wcsnicmp(wzOriginPath, wzUnknow, wcslen(wzUnknow)) )
		{
			wcscpy_s(wzPath, wzOriginPath + wcslen(wzUnknow));
			wcscpy_s(DriverInfor->wzDriverPath, wzPath);
			wzOriginPath[wcslen(wzPath)] = L'\0';
		}
		else if (nOrigin >= wcslen(wzSystemRoot) && !_wcsnicmp(wzOriginPath, wzSystemRoot, wcslen(wzSystemRoot)))
		{
			wcscpy_s(wzPath, wzWindowsDirectory);
			wcscat_s(wzPath, wzOriginPath + wcslen(wzSystemRoot));
			wcscpy_s(DriverInfor->wzDriverPath, wzPath);
			wzOriginPath[wcslen(wzPath)] = L'\0';
		}
		else if (nOrigin >= wcslen(wzWindows) && !_wcsnicmp(wzOriginPath, wzWindows, wcslen(wzWindows)))
		{
			wcscpy_s(wzPath, wzWindowsDirectory);
			wcscat_s(wzPath, wzOriginPath + wcslen(wzWindows));
			wcscpy_s(DriverInfor->wzDriverPath, wzPath);
			wzOriginPath[wcslen(wzPath)] = L'\0';
		}
		else if (nOrigin >= wcslen(wzWinnt) && !_wcsnicmp(wzOriginPath, wzWinnt, wcslen(wzWinnt)))
		{
			wcscpy_s(wzPath, wzWindowsDirectory);
			wcscat_s(wzPath, wzOriginPath + wcslen(wzWinnt));
			wcscpy_s(DriverInfor->wzDriverPath, wzPath);
			wzOriginPath[wcslen(wzPath)] = L'\0';
		}
	}

	// ����Ƕ��ļ���
	if (wcschr(wzOriginPath, '~'))
	{
		WCHAR wzLongPath[MAX_PATH] = {0};
		DWORD nRet = GetLongPathName(wzOriginPath, wzLongPath, MAX_PATH);
		if ( !(nRet >= MAX_PATH || nRet == 0) )
		{
			wcscpy_s(DriverInfor->wzDriverPath, wzLongPath);
			wzOriginPath[wcslen(wzLongPath)] = L'\0';
		}
	}
}



void AddModuleFileIcon(WCHAR* ModulePath)
{
	SHFILEINFO shInfo;
	memset(&shInfo, 0, sizeof(shInfo));
	CString CSmPath = ModulePath;
	SHGetFileInfo(CSmPath, FILE_ATTRIBUTE_NORMAL, 
		&shInfo, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_USEFILEATTRIBUTES);

	HICON  hIcon = shInfo.hIcon;

	m_TreeModuleImageList.Add(hIcon);
}



void HsRemoveDriverModule(CListCtrl* m_ListCtrl)
{
	m_DriverObject = 0;

	int Index = m_ListCtrl->GetSelectionMark();

	if (Index<0)
	{
		return;
	}

	CString Temp = m_ListCtrl->GetItemText(Index,3);

	swscanf_s(Temp.GetBuffer()+2,L"%p",&m_DriverObject);

	if (m_DriverObject==0)
	{
		return;
	}

	if (SendIoControlUnloadDriver())
	{
		HsQueryModuleList(m_ListCtrl);
	}

	bIsChecking = FALSE;
}


BOOL SendIoControlUnloadDriver()
{
	ULONG ulReturnSize = 0;
	BOOL bRet = FALSE;

	bRet = DeviceIoControl(g_hDevice,HS_IOCTL(HS_IOCTL_MODU_REMOVEMODULE),
		&m_DriverObject,
		sizeof(ULONG_PTR),
		NULL,
		0,
		&ulReturnSize,
		NULL);

	return bRet;
}