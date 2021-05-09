#ifndef CAPRICEGUI_H
#define CAPRICEGUI_H

#include "wgui.h"

class CapriceGui : public wGui::CApplication
{
	public:
		CapriceGui() : wGui::CApplication() {};
		void Init() override;
};

#endif
