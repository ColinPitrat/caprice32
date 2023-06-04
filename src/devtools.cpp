#include "devtools.h"

#include <string>
#include "log.h"
#include "video.h"
#include "wg_error.h"

bool DevTools::Activate(int scale) {
  ShowCursor(true);
  try {
    // TODO: This position only makes sense for me. Ideally we would probably want to find where current window is, find display size and place
    // the window where there's the most space available. On the other hand, getting display size is not very reliable on multi-screen setups under linux ...
    window = SDL_CreateWindow("Caprice32 - Developers' tools", 100, 100, DEVTOOLS_WIDTH*scale, DEVTOOLS_HEIGHT*scale, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!window || !renderer) { Deactivate(); return false; }
    surface = SDL_CreateRGBSurface(0, DEVTOOLS_WIDTH, DEVTOOLS_HEIGHT, renderer_bpp(renderer), 0, 0, 0, 0);
    if (!surface) { Deactivate(); return false; }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) { Deactivate(); return false; }
    SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 0, 0, 0));
    capriceGui = std::make_unique<CapriceGui>(window, /*bInMainView=*/false, scale);
    capriceGui->Init();
    devToolsView = std::make_unique<CapriceDevToolsView>(*capriceGui, surface, renderer, texture, wGui::CRect(0, 0, DEVTOOLS_WIDTH, DEVTOOLS_HEIGHT), this);
  } catch(wGui::Wg_Ex_App& e) {
      // TODO: improve: this is pretty silent if people don't look at the console
      LOG_ERROR("Failed displaying developer's tools: " << e.what());
  }
  active = true;
  return true;
}

void DevTools::Deactivate() {
  ShowCursor(false);
  devToolsView = nullptr;
  capriceGui = nullptr;
  if (texture) SDL_DestroyTexture(texture);
  if (surface) SDL_FreeSurface(surface);
  if (renderer) SDL_DestroyRenderer(renderer);
  if (window) SDL_DestroyWindow(window);
  texture = nullptr;
  surface = nullptr;
  renderer = nullptr;
  window = nullptr;
  active = false;
}

void DevTools::LoadSymbols(const std::string& filename) {
  devToolsView->LoadSymbols(filename);
}

void DevTools::PreUpdate() {
  devToolsView->PreUpdate();
}

void DevTools::PostUpdate() {
  devToolsView->PostUpdate();
  capriceGui->Update();
}

bool DevTools::PassEvent(SDL_Event& event) {
  return capriceGui->ProcessEvent(event);
}
