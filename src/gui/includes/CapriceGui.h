#ifndef CAPRICEGUI_H
#define CAPRICEGUI_H

#include "wg_application.h"
#include "wg_view.h"
#include "SDL.h"

class CapriceGui : public wGui::CApplication
{
	public:
		CapriceGui(SDL_Window* pWindow) : wGui::CApplication(pWindow) {};
		void Init() override;
};

#endif
