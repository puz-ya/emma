#include "stdafx.h"
#include "C2DBalloonOperation.h"


IOIMPL(C2DBalloonOperation, C2D_BALLOON_OPERATION)

C2DBalloonOperation::C2DBalloonOperation(CString strName) : CGen2DOperation(strName){
	SetIconName(IDS_ICON_OPERATION_FORMING);
	//������ ���������� ��� ����������� � CGen2DOperation
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//AddTool();	//��������� ������ (����� �����)
}


C2DBalloonOperation::~C2DBalloonOperation()
{
}
