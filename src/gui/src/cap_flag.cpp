#include "cap_flag.h"

namespace wGui
{

CFlag::CFlag(const CRect& WindowRect, CWindow* pParent, const std::string& name, const std::string& description, CFontEngine* pFontEngine) :
	CWindow(WindowRect, pParent)
{
	if (pFontEngine)
	{
		m_pFontEngine = pFontEngine;
	}
	else
	{
		m_pFontEngine = Application().GetDefaultFontEngine();
	}
	m_pLabel = new CLabel(CPoint(0, 7), this, name);
	m_pValue = new CEditBox(CRect(CPoint(20, 0), 20, 20), this);
	m_pTooltip = new CToolTip(this, description, COLOR_BLACK);

	Draw();
}


CFlag::~CFlag()  // virtual
{
	delete m_pLabel;
	delete m_pValue;
	delete m_pTooltip;
}

void CFlag::SetValue(const std::string& c)
{
	m_pValue->SetWindowText(c);
}

bool CFlag::GetValue() const {
	return m_pValue->GetWindowText() == "1";
}

void CFlag::SetReadOnly(bool bReadOnly) {
	m_pValue->SetReadOnly(bReadOnly);
}

}
