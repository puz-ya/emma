﻿#pragma once
#include "emmarightpane.h"
#include "2DSketch.h"
#include "Resource.h"
#include "RegisteredMessages.h"
#include "../../Common/DlgShowError.h"


class C2DSketchPane : public CEMMARightPane
{
public:
	C2DSketchPane(size_t nID) : CEMMARightPane(nID){
		m_cursor = 0;
	}
	virtual ~C2DSketchPane(void);

	//CButton m_button_contour;
	//CButton m_button_save_to_meta;

public:

	C2DSketch* GetSketch(){return m_pDoc ? dynamic_cast<C2DSketch*> (m_pDoc) : nullptr;}

	//! Устанавливаем размеры Тулбара и Таблицы
	void AdjustLayout();

	DECLARE_MESSAGE_MAP()

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	////TOOLBAR
	afx_msg void OnClearSketch();
	afx_msg void OnUpdateClearSketch(CCmdUI *pCmdUI);

	afx_msg void OnNewNode();
	afx_msg void OnUpdateNewNode(CCmdUI *pCmdUI);

	afx_msg void OnNewCurve();
	afx_msg void OnUpdateNewCurve(CCmdUI *pCmdUI);

	afx_msg void OnNewCircleArc();
	afx_msg void OnUpdateNewCircleArc(CCmdUI *pCmdUI);

	afx_msg void OnNewFacet();
	afx_msg void OnUpdateNewFacet(CCmdUI *pCmdUI);

	//Применение изменений в таблице свойств
	afx_msg void ButtonApply();
	afx_msg void OnEnableButtonApply(CCmdUI *pCmdUI);

	//Button
	//afx_msg void ShowContours();
	//afx_msg void OnEnableButton(CCmdUI *pCmdUI);

	//Сохранение всего чертежа в meta-файл
	//afx_msg void SaveSketchToMeta();
	//afx_msg void OnEnableButtonSave(CCmdUI *pCmdUI);

};
