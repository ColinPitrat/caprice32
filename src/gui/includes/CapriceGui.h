#ifndef CAPRICEGUI_H
#define CAPRICEGUI_H

#include "wgui.h"

class CapriceGui : public wGui::CApplication
{
	public:
		CapriceGui() = default;
		void Init() override;
};

#endif
