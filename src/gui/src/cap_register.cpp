#include "cap_register.h"
#include <iomanip>
#include <sstream>
#include <string>

namespace wGui
{

CRegister::CRegister(const CRect& WindowRect, CWindow* pParent, const std::string& name, CFontEngine* pFontEngine) :
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
	m_pHexValue = new CEditBox(CRect(CPoint(20, 0), 30, 20), this);
	m_pDecValue = new CEditBox(CRect(CPoint(55, 0), 40, 20), this);
	m_pCharValue = new CEditBox(CRect(CPoint(100, 0), 25, 20), this);

	// By default, registers are read-only. We only set as read-write the ones we
	// support modifying.
	m_pHexValue->SetReadOnly(true);
	m_pDecValue->SetReadOnly(true);
	m_pCharValue->SetReadOnly(true);

	Draw();
}


CRegister::~CRegister()  // virtual
{
	delete m_pLabel;
	delete m_pHexValue;
	delete m_pDecValue;
	delete m_pCharValue;
}

void CRegister::SetReadOnly(bool bReadOnly) {
	// Only allow editing the hex value. It would be nice to allow editing the
	// decimal value too but this would require a bit more logic, listening to
	// CTRL_VALUECHANGE events and updating all fields accordingly when one
	// changes.
	m_pHexValue->SetReadOnly(bReadOnly);
}

void CRegister::SetValue(const unsigned int c)
{
	m_Value = c;
	std::ostringstream oss;
	oss << std::hex << c;
	m_pHexValue->SetWindowText(oss.str());
	m_pDecValue->SetWindowText(std::to_string(c));
	if ( c < 256 ) {
		m_pCharValue->SetWindowText(std::string(1, static_cast<char>(c)));
	} else {
		m_pCharValue->SetWindowText(
				std::string(1, static_cast<char>((c & 0xFF00) >> 8)) +
				std::string(1, static_cast<char>(c & 0xFF)));
	}
}

unsigned int CRegister::GetValue() const {
	std::string x = m_pHexValue->GetWindowText();
	return stoi(x, nullptr, 16);
}

}
