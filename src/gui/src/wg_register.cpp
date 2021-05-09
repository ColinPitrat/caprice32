#include "wg_register.h"
#include <iomanip>
#include <sstream>

namespace wGui
{

CRegister::CRegister(const CRect& WindowRect, CWindow* pParent, std::string name, CFontEngine* pFontEngine) :
	CWindow(WindowRect, pParent)
{
	if (pFontEngine)
	{
		m_pFontEngine = pFontEngine;
	}
	else
	{
		m_pFontEngine = CApplication::Instance()->GetDefaultFontEngine();
	}
  m_pLabel = new CLabel(CPoint(0, 7), this, name);
  m_pHexValue = new CEditBox(CRect(CPoint(15, 0), 30, 20), this);
  m_pDecValue = new CEditBox(CRect(CPoint(50, 0), 30, 20), this);
  m_pCharValue = new CEditBox(CRect(CPoint(85, 0), 30, 20), this);

	Draw();
}


CRegister::~CRegister()  // virtual
{
	delete m_pLabel;
	delete m_pHexValue;
	delete m_pDecValue;
	delete m_pCharValue;
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

}
