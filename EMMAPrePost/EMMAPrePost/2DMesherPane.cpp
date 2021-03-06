﻿#include "StdAfx.h"
#include "2DMesherPane.h"


C2DMeshPane::~C2DMeshPane(void)
{
}

BEGIN_MESSAGE_MAP(C2DMeshPane, CEMMARightPane)
	ON_WM_CREATE()
	//SIZE, PAINT, ERASEBKGND are in EMMARightPane
	
	////TOOLBAR
	ON_COMMAND(ID_CLEAR_MESH, &C2DMeshPane::OnClearMesh)
	ON_UPDATE_COMMAND_UI(ID_CLEAR_MESH, &C2DMeshPane::OnUpdateClearMesh)

	ON_COMMAND(ID_NEW_MESH, &C2DMeshPane::OnNewMesh)
	ON_UPDATE_COMMAND_UI(ID_NEW_MESH, &C2DMeshPane::OnUpdateNewMesh)

	ON_COMMAND(ID_NEW_MESHPOINT, &C2DMeshPane::OnNewMeshPoint)
	ON_UPDATE_COMMAND_UI(ID_NEW_MESHPOINT, &C2DMeshPane::OnUpdateNewMeshPoint)

	ON_COMMAND(ID_IMPORT_SKETCH, &C2DMeshPane::OnImportSketch)
	//ON_UPDATE_COMMAND_UI(ID_IMPORT_SKETCH, &C2DMeshPane::OnUpdateImportSketch)

	//Buttons
	ON_BN_CLICKED(ID_RIGHTPANE_BUTTON_APPLY, C2DMeshPane::ButtonApply)

END_MESSAGE_MAP()

int C2DMeshPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CEMMADockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Создание Тулбара {{
	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_2DMESH_TB);
	m_wndToolBar.LoadToolBar(IDR_2DMESH_TB, 0, 0, 1 /* Is locked */);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	
	m_wndToolBar.SetOwner(this);
	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(0);
	// }}

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 1))
	{
		CDlgShowError cError(ID_ERROR_2DMESHERPANE_FAIL_PROPGRID); //_T("Failed to create Properties Grid \n"));
		return -1;      // fail to create
	}
	
	//creating Apply button for PropertyGrid
	int nButton = CreateApplyButton(rectDummy);
	if (nButton == -1) {
		return -1;	//тут же выходим
	}

	//! Устанавливаем имена колонок Таблицы
	SetColumnNames();
	//! Устанавливаем размеры Тулбара и Таблицы и Кнопок
	AdjustLayout();

	m_cursor = 0;

	return 0;
}

//! Устанавливаем размеры Тулбара и Таблицы
void C2DMeshPane::AdjustLayout(void){
	if (GetSafeHwnd() == nullptr)		return;

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(0, 1).cy;

	m_wndToolBar.SetWindowPos(nullptr, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndPropList.SetWindowPos(nullptr, rectClient.left, rectClient.top + cyTlb, rectClient.Width(), (rectClient.Height() - cyTlb) / 2, SWP_NOACTIVATE | SWP_NOZORDER);

	//устанавливаем размеры Кнопки
	int cyPropList = rectClient.top + cyTlb + (rectClient.Height() - cyTlb) / 2 + 10;
	int nFromLeft = rectClient.left + int(rectClient.Width() / 5.0);
	int nFromRight = rectClient.Width() - int(rectClient.Width() / 5.0) * 2;	//обязательное умножение, чтобы был отступ справа
	m_buttonApply.SetWindowPos(nullptr, nFromLeft, cyPropList, nFromRight, 20, SWP_NOOWNERZORDER | SWP_SHOWWINDOW);
}

C2DOutline* C2DMeshPane::GetOutline(){
	
	C2DSketch* parent = dynamic_cast<C2DSketch*>(m_pDoc->GetParent()->SubDoc(0));
	if (parent){
		return parent->GetOutline();
	}

	return nullptr;
}

////////TOOLBAR
void C2DMeshPane::OnClearMesh(){
	m_pDoc->GetView()->SendMessage(WMR_SETCURSOR,0);
	m_cursor = 0;

	CMesh* pMesh = GetMesher()->GetMesh();
	if (pMesh){
		pMesh->DeleteMesh();
		pMesh->DeleteMeshPoint();
	}
	m_pDoc->GetView()->Invalidate();
}

void C2DMeshPane::OnUpdateClearMesh(CCmdUI *pCmdUI){
	// TODO: Add your command update UI handler code here
}


void C2DMeshPane::OnNewMesh(){
	
	C2DMesher* pMesher = GetMesher();
	//TODO: продумать проверки

	if (pMesher->GetOutline()->GetCurveCount()>0) {
		CMesh* pMesh = GetMesher()->GetMesh();
		if (pMesh != nullptr) {
			pMesh->DeleteMesh();
		}
		
		//TODO: проверка на чертёж (возможно не нужна, если Женин вариант будет работать)
		pMesh->GenerateMesh(pMesh->m_dRibLen, GetMesher()->GetOutline());
		pMesh->ConstructMap();

	} else {
		//Нету документа (Mesher-a) или не загружен
		CDlgShowError Diag_err(ID_ERROR_2DMESHERPANE_NO_SKETCH);
	}

	m_pDoc->GetView()->Invalidate();
	//UpdatePane();	
}

void C2DMeshPane::OnUpdateNewMesh(CCmdUI *pCmdUI){

}

void C2DMeshPane::OnNewMeshPoint(){

	if(m_cursor != 300){
		m_pDoc->GetView()->SendMessage(WMR_SETCURSOR,300);
		m_cursor = 300;
	}
	else{
		m_pDoc->GetView()->SendMessage(WMR_SETCURSOR,0);
		m_cursor = 0;
	}
	//UpdatePane();
}

void C2DMeshPane::OnUpdateNewMeshPoint(CCmdUI *pCmdUI){

	//Кнопка нажата: показываем, что можно выбрать точку
	if(m_cursor == 300) pCmdUI->SetCheck(1);
	else pCmdUI->SetCheck(0); 
}

void C2DMeshPane::OnImportSketch() {

	CEMMADoc *pCurDoc = m_pDoc;
	C2DMesher* pMesher = pCurDoc ? dynamic_cast<C2DMesher*>(pCurDoc) : nullptr;

	C2DSketch* parent = dynamic_cast<C2DSketch*>(pCurDoc->GetParent()->SubDoc(0));
	C2DOutline *pOutline = nullptr;
	if (parent) {
		pOutline = parent->GetOutline();
	}


	if (pOutline) {
		//TODO: проверка должна быть полнее - например, на незамкнутость контура(ов)
		if (pOutline->GetNodeCount() < 3 || pOutline->GetCurveCount() < 2) {
			pOutline = nullptr;

			//вывод сообщения об ошибке
			CDlgShowError Diag_err(ID_ERROR_2DMESHERPANE_NO_SKETCH);	//Показываем окно ошибки
			return;
		}
		pMesher->SetOutline(pOutline);
	}
	else {
		//вывод сообщения об ошибке
		CDlgShowError Diag_err(ID_ERROR_2DMESHERPANE_NO_SKETCH);	//Показываем окно ошибки
		return;
	}
	//*/
}

void C2DMeshPane::OnUpdateImportSketch(CCmdUI * pCmdUI)
{
}

void C2DMeshPane::ButtonApply()
{
	m_pDoc->UpdatePropList(&m_wndPropList);
}