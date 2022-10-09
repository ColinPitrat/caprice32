#ifndef DEVTOOLS_H
#define DEVTOOLS_H

#include <string>
#include "SDL.h"
#include "CapriceGui.h"
#include "CapriceDevToolsView.h"

class DevTools {
  public:
    bool Activate(int scale);
    void Deactivate();

    bool IsActive() const { return active; };

    void LoadSymbols(const std::string& filename);

    void PreUpdate();
    void PostUpdate();

    // Return true if the event was processed
    // (i.e destined to this window)
    bool PassEvent(SDL_Event& e);

  private:
    std::unique_ptr<CapriceGui> capriceGui;
    std::unique_ptr<CapriceDevToolsView> devToolsView;
    bool active = false;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;
    SDL_Surface* surface = nullptr;
    int scale = 0;
};

#endif
