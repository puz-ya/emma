﻿#include "StdAfx.h"
#include "2DTool.h"


IOIMPL (C2DTool, C2DTOOL)

//! Конструктор и Создание подменю "Траектория"
C2DTool::C2DTool(CString strName) : C2DSketch(strName){
	m_StickPoint() = Math::C2DPoint(0,0);
	m_FrictionCoeff = 1.0;

	RegisterMember(&m_StickPoint);
	RegisterMember(&m_FrictionCoeff);

	C2DTrajectory *pTrajectory = new C2DTrajectory;
	pTrajectory->SetName(IDS_TRAJECTORY);
	InsertSubDoc(pTrajectory);
	SetIconName(IDS_ICON_TOOL);
}

C2DTool::~C2DTool(void){
}


//! Перемещение чертежа Инструмента
void C2DTool::ChangeNodes(Math::C2DPoint *new_point){

	double d_x, d_y;
	Math::C2DPoint point;

	// Находим изменения координат
	d_x = new_point->x - m_StickPoint().x;
	d_y = new_point->y - m_StickPoint().y;

	// Меняем положение Инструмента
	for (size_t i = 0; i < m_Outline.GetNodeCount(); i++){
		
		C2DNode *pNode = m_Outline.GetNode(i);
		point.x = pNode->GetPoint().x + d_x;
		point.y = pNode->GetPoint().y + d_y;

		pNode->SetPoint(point);
	}
	
	m_StickPoint().x += d_x;
	m_StickPoint().y += d_y;
}

//! Возвращает ссылку на Траекторию
C2DTrajectory* C2DTool::GetTrajectory(){

	C2DTrajectory* pTraj = dynamic_cast<C2DTrajectory*>(SubDoc(0));
	if (pTraj){
		return pTraj;
	}

	return nullptr;
}

//! Копирует чертёж Инструмента
bool C2DTool::CopyOutline(C2DTool* tool){
	
	m_StickPoint() = tool->m_StickPoint();
	bool flag = m_Outline.Copy(tool->GetOutline());
	return flag;
	//return C2DSketch::Copy(tool);
}

void C2DTool::AddStickPoint(Math::C2DPoint ppoint, GLParam &parameter){

	m_StickPoint() = ppoint;
	DrawGL(parameter);
	
	InitialUpdateView();
	InitialUpdatePane();
}

//! Добавление свойства в таблицу справа
CMFCPropertyGridProperty* C2DTool::AddProp(CStringW name, double eval, size_t pos){

	CMFCPropertyGridProperty *pProp = new CMFCPropertyGridProperty(name, eval, name);
	pProp->SetData(pos);
	if (pos == 3) pProp->AllowEdit(0);	//точка внутри или снаружи
	if (pos == 8) pProp->AllowEdit(0);	//длина кривой
	return pProp;
}

//! Заполнение параметров (правая панель)
void C2DTool::FillPropList(CMFCPropertyGridCtrl *pGrid){
	
	if(pGrid == 0) return;
	pGrid->RemoveAll();

	pGrid->AddProperty(AddProp(_T("X_point"), m_StickPoint().x,1));
	pGrid->AddProperty(AddProp(_T("Y_point"), m_StickPoint().y,2));
	pGrid->AddProperty(AddProp(_T("Is_Inside_Instr"), GetOutline()->IsInside(m_StickPoint()),3));
	pGrid->AddProperty(AddProp(_T("Friction_Coeff"), m_FrictionCoeff,12));	//number 12 !!!

	if ((m_CurvesActive.size() == 1) && (m_NodesActive.size() == 0)){

		C2DCurve * t_curve;
		t_curve = m_Outline.GetCurve(m_CurvesActive[0]);
			
		pGrid->AddProperty(AddProp(_T("X1"), t_curve->GetStartNode()->GetPoint().x,4));
		pGrid->AddProperty(AddProp(_T("Y1"), t_curve->GetStartNode()->GetPoint().y,5));
		pGrid->AddProperty(AddProp(_T("X2"), t_curve->GetEndNode()->GetPoint().x,6));
		pGrid->AddProperty(AddProp(_T("Y2"), t_curve->GetEndNode()->GetPoint().y,7));
		pGrid->AddProperty(AddProp(_T("L"), t_curve->CalcLength(),8));

		if (m_Outline.GetCurve(m_CurvesActive[0])->GetType() == C2DCIRCARC) {
			C2DCircleArc *pCurve;
			pCurve = dynamic_cast<C2DCircleArc*>(m_Outline.GetCurve(m_CurvesActive[0]));
			pGrid->AddProperty(AddProp(_T("R"),pCurve->GetRadius(),9));
			
		}

		return;
	}
	
	//Если кривая не выделена, значит нужен узел
	if (m_NodesActive.size() == 1){
		
		C2DNode * t_node;
		t_node = m_Outline.GetNode(m_NodesActive[0]);

		pGrid->AddProperty(AddProp(_T("X"), t_node->GetPoint().x,10));
		pGrid->AddProperty(AddProp(_T("Y"), t_node->GetPoint().y,11));

		return;
		}
		
	pGrid->Invalidate();
}

void C2DTool::ResetCountourCache(){
	
	for (size_t i=0; i < GetOutline()->GetContourCount(); i++){
		GetOutline()->GetContour(i)->SetCache(false);
	}

}

//! Обновляем все поля таблицы свойств
void C2DTool::UpdatePropList(CMFCPropertyGridCtrl * pGrid) {
	
	if (pGrid == nullptr) {
		CDlgShowError err(_T("CMFCPropertyGridCtrl is null"));
		return;
	}

	//кол-во строк в таблице свойств (для проверки)
	int nCount = pGrid->GetPropertyCount();
	if (nCount == 0) {
		CDlgShowError err(_T("CMFCPropertyGridCtrl is empty"));
		return;
	}

	Math::C2DPoint tPoint;
	C2DCurve *pCur = nullptr;
	C2DCircleArc *pArc = nullptr;
	bool isCurveActive = false;

	if (m_CurvesActive.size() > 0) {
		pCur = m_Outline.GetCurve(m_CurvesActive[0]);	//берём только первую из выделенных
		isCurveActive = true;
	}

	if (m_NodesActive.size() > 0) {	//захватили узел, но выделение кривой не снялось (близко)
		isCurveActive = false;
	}

	//первые 2 поля для точки привязки - постоянные
	CMFCPropertyGridProperty* pProperty1 = pGrid->FindItemByData(1);	//X point
	if (pProperty1) {
		m_StickPoint().x = pProperty1->GetValue().dblVal;
	}

	CMFCPropertyGridProperty* pProperty2 = pGrid->FindItemByData(2);	//Y point
	if (pProperty2) {
		m_StickPoint().y = pProperty2->GetValue().dblVal;
	}

	//3 - внутри или снаружи оказалась точка привязки
	CMFCPropertyGridProperty* pProperty3 = pGrid->FindItemByData(3);	//Is inside
	if (pProperty3) {
		int isIn = GetOutline()->IsInside(m_StickPoint());
		COleVariant varIsIn((double)isIn);
		pProperty3->SetValue(&varIsIn);
	}

	if (isCurveActive) {
		CMFCPropertyGridProperty* pProperty4 = pGrid->FindItemByData(4);	//X1
		if (pProperty4) {
			tPoint = pCur->GetStartNode()->GetPoint();
			tPoint.x = pProperty4->GetValue().dblVal;
			pCur->GetStartNode()->SetPoint(tPoint);
		}

		CMFCPropertyGridProperty* pProperty5 = pGrid->FindItemByData(5);	//Y1
		if (pProperty5) {
			tPoint = pCur->GetStartNode()->GetPoint();
			tPoint.y = pProperty5->GetValue().dblVal;
			pCur->GetStartNode()->SetPoint(tPoint);
		}

		CMFCPropertyGridProperty* pProperty6 = pGrid->FindItemByData(6);	//X2
		if (pProperty6) {
			tPoint = pCur->GetEndNode()->GetPoint();
			tPoint.x = pProperty6->GetValue().dblVal;
			pCur->GetEndNode()->SetPoint(tPoint);
		}

		CMFCPropertyGridProperty* pProperty7 = pGrid->FindItemByData(7);	//Y2
		if (pProperty7) {
			tPoint = pCur->GetEndNode()->GetPoint();
			tPoint.y = pProperty7->GetValue().dblVal;
			pCur->GetEndNode()->SetPoint(tPoint);
		}

		//8 - длина кривой
		CMFCPropertyGridProperty* pProperty8 = pGrid->FindItemByData(8);	//Length
		if (pProperty8) {
			COleVariant varLength(pCur->CalcLength());
			pProperty8->SetValue(&varLength);
		}

		//9 - радиус, если дуга
		CMFCPropertyGridProperty* pProperty9 = pGrid->FindItemByData(9);	//Radius
		if (pProperty9) {
			pArc = dynamic_cast<C2DCircleArc*>(pCur);
			pArc->SetRadius(pProperty9->GetValue().dblVal);
		}
	}
	else {
		//Координаты выделенного узла
		CMFCPropertyGridProperty* pProperty10 = pGrid->FindItemByData(10);	//X
		if (pProperty10) {
			tPoint = m_Outline.GetNode(m_NodesActive[0])->GetPoint();
			tPoint.x = pProperty10->GetValue().dblVal;
			m_Outline.GetNode(m_NodesActive[0])->SetPoint(tPoint);
		}

		CMFCPropertyGridProperty* pProperty11 = pGrid->FindItemByData(11);	//Y
		if (pProperty11) {
			tPoint = m_Outline.GetNode(m_NodesActive[0])->GetPoint();
			tPoint.y = pProperty11->GetValue().dblVal;
			m_Outline.GetNode(m_NodesActive[0])->SetPoint(tPoint);
		}
	}

	//12 - коэффициент трения
	CMFCPropertyGridProperty* pProperty12 = pGrid->FindItemByData(12);	//Friction coeff
	if (pProperty12) {
		double tFric = pProperty12->GetValue().dblVal;
		// трение не может быть меньше 0 или больше 1
		if (tFric > 1.0) {
			tFric = 1.0;
		}
		if (tFric < 0.0) {
			tFric = 0.0;
		}
		//update value in property
		COleVariant varFrict(tFric);
		pProperty12->SetValue(&varFrict);
		m_FrictionCoeff = tFric;
	}

	ResetCountourCache();
	InitialUpdateView();
}

/*
//! Обновление параметров (правая панель)
void C2DTool::UpdateProp(double *pEval, UNLONG *pId){

	if( (pEval == 0) || (pId == 0)) return;
	Math::C2DPoint tPoint;
	C2DCurve *pCur = nullptr;
	C2DCircleArc *pArc = nullptr;
	bool isCurveActive = false;
	////////////// ПЕРЕПИСАТЬ ВСЕ В ЦИКЛАХ!!!

	if (m_CurvesActive.size() > 0){
		pCur = m_Outline.GetCurve(m_CurvesActive[0]);
		isCurveActive = true;
	}
		
	switch(*pId){
	case 1:
		m_StickPoint().x = *pEval;
		break;

	case 2:
		m_StickPoint().y = *pEval;
		break;

	//3 - внутри ли точка привязки, не изменяем

	case 4:
		if (isCurveActive) {
			tPoint = pCur->GetStartNode()->GetPoint();
			tPoint.x = *pEval;
			pCur->GetStartNode()->SetPoint(tPoint);
			ResetCountourCache();
		}
		break;

	case 5:
		if (isCurveActive) {
			tPoint = pCur->GetStartNode()->GetPoint();
			tPoint.y = *pEval;
			pCur->GetStartNode()->SetPoint(tPoint);
			ResetCountourCache();
		}
		break;

	case 6:
		if (isCurveActive) {
			tPoint = pCur->GetEndNode()->GetPoint();
			tPoint.x = *pEval;
			pCur->GetEndNode()->SetPoint(tPoint);
			ResetCountourCache();
		}
		break;

	case 7:
		if (isCurveActive) {
			tPoint = pCur->GetEndNode()->GetPoint();
			tPoint.y = *pEval;
			pCur->GetEndNode()->SetPoint(tPoint);
			ResetCountourCache();
		}
		break;
		
	//8 - длина кривой, не изменяем

	case 9:
		if (isCurveActive) {
			pArc = dynamic_cast<C2DCircleArc*>(pCur);
			pArc->SetRadius(*pEval);
			ResetCountourCache();
		}
		break;

	// узлы
	case 10:
		tPoint = m_Outline.GetNode(m_NodesActive[0])->GetPoint();
		tPoint.x = *pEval;
		m_Outline.GetNode(m_NodesActive[0])->SetPoint(tPoint);
		ResetCountourCache();
		break;

	case 11:
		tPoint = m_Outline.GetNode(m_NodesActive[0])->GetPoint();
		tPoint.y = *pEval;
		m_Outline.GetNode(m_NodesActive[0])->SetPoint(tPoint);
		ResetCountourCache();
		break;

	case 12:	//коэффициент трения
		m_FrictionCoeff = *pEval;
		break;
	}

	InitialUpdateView();
	InitialUpdatePane();
}
//*/

void C2DTool::DrawGL(GLParam &parameter){

	C2DSketch::DrawGL(parameter);
	
	//Отрисовка точки привязки траектории
	glPointSize(5);
	glBegin(GL_POINTS);
	glColor3d(0.5,0.5,0);
	glVertex2dv(m_StickPoint());
	glEnd();
	//*/

}

bool C2DTool::GetBoundingBox(CRect2D &rect){
	if (!GetOutline()->GetBoundingBox(rect)){
		return false;
	}
	rect.AddPoint(m_StickPoint);
	return true;
}