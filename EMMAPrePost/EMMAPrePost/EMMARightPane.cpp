#include "stdafx.h"
#include "EMMARightPane.h"

CEMMARightPane::~CEMMARightPane(void)
{
}

BEGIN_MESSAGE_MAP(CEMMARightPane, CEMMADockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()

	ON_UPDATE_COMMAND_UI(ID_RIGHTPANE_BUTTON_APPLY, CEMMARightPane::OnEnableButtonApply)

END_MESSAGE_MAP()

//! Устанавливаем заголовки для колонок таблицы
void CEMMARightPane::SetColumnNames(void) {

	CString sParam, sValue;
	int nNumPar = sParam.LoadString(IDS_PARAM);
	int nNumVal = sValue.LoadString(IDS_PARAM_VALUE);
	m_wndPropList.EnableHeaderCtrl( 1 , sParam, sValue);
}

int CEMMARightPane::CreateApplyButton(CRect rectDummy)
{
	CString str4button_apply;
	str4button_apply.LoadStringW(ID_RIGHTPANE_BUTTON_APPLY);
	if (!m_buttonApply.Create(str4button_apply, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rectDummy, this, ID_RIGHTPANE_BUTTON_APPLY)) {
		CDlgShowError cError(ID_ERROR_2DSKETCHPANE_FAIL_BUTTON); //_T("Failed to create Button \n"));
		return -1;      // fail to create
	}
	//устанавливаем такой же шрифт, как в таблице свойств
	m_buttonApply.SetFont(m_wndPropList.GetFont());
	return 0;
}

int CEMMARightPane::OnEraseBkgnd(CDC* pDC) {
	return CWnd::OnEraseBkgnd(pDC);
}

void CEMMARightPane::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rect;
	GetClientRect(&rect);	//Получаем размеры
							//Перекраска фона в белый цвет при изменении размера
	CBrush bg;
	bg.CreateStockObject(WHITE_BRUSH);
	dc.FillRect(&rect, &bg);

}

void CEMMARightPane::UpdatePane() {

	//Заполняем таблицу свойств из документа
	m_pDoc->FillPropList(&m_wndPropList);

}

void CEMMARightPane::OnSize(UNINT nType, int cx, int cy)
{
	CEMMADockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CEMMARightPane::OnEnableButtonApply(CCmdUI * pCmdUI)
{
	//Приходится активировать кнопку для нажатий (MFC style)
	pCmdUI->Enable();
}
