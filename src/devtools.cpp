#include "devtools.h"
#include "video.h"

bool DevTools::Activate() {
  std::cout << "Activating dev tools" << std::endl;
  SDL_ShowCursor(SDL_ENABLE);
  try {
    // TODO: This position only makes sense for me. Ideally we would probably want to find where current window is, find display size and place
    // the window where there's the most space available. On the other hand, getting display size is not very reliable on multi-screen setups under linux ...
    window = SDL_CreateWindow("Caprice32 - Developer's tools", 100, SDL_WINDOWPOS_CENTERED, DEVTOOLS_WIDTH, DEVTOOLS_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);
    //SDL_CreateWindowAndRenderer(DEVTOOLS_WIDTH, DEVTOOLS_HEIGHT, SDL_WINDOW_SHOWN, &window, &renderer);
    // TODO: Better handling of error (free stuff, surface error ...)
    SDL_SetWindowTitle(window, "Caprice32 - Developer's tools");
    if (!window || !renderer) return false;
    surface = SDL_CreateRGBSurface(0, DEVTOOLS_WIDTH, DEVTOOLS_HEIGHT, renderer_bpp(renderer), 0, 0, 0, 0);
    if (!surface) return false;
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) return false;
    SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 0, 0, 0));
    capriceGui = std::make_unique<CapriceGui>();
    capriceGui->Init();
    devToolsView = std::make_unique<CapriceDevToolsView>(surface, renderer, texture, wGui::CRect(0, 0, DEVTOOLS_WIDTH, DEVTOOLS_HEIGHT), this);
    capriceGui->SetMouseVisibility(true);
  } catch(wGui::Wg_Ex_App& e) {
      // TODO: improve: this is pretty silent if people don't look at the console
      std::cout << "Failed displaying developer's tools: " << e.what() << std::endl;
  }
  active = true;
  return true;
}

void DevTools::Deactivate() {
  SDL_ShowCursor(SDL_DISABLE);
  capriceGui->ApplicationExit(0);
  devToolsView = nullptr;
  capriceGui = nullptr;
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_DestroyTexture(texture);
  SDL_FreeSurface(surface);
  active = false;
}

void DevTools::Update() {
  devToolsView->Update();
  capriceGui->Update();
}

bool DevTools::PassEvent(SDL_Event& event) {
  auto windowId = SDL_GetWindowID(window);
  bool forMe = false;
  // TODO: For events that don't have a windowId (e.g joystick, ...), it would
  // make sense to check if the window has focus.
  // This can be done with SDL_GetWindowFlags, checking for
  // SDL_WINDOW_INPUT_FOCUS and/or SDL_WINDOW_MOUSE_FOCUS
  switch (event.type) {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      if (event.key.windowID == windowId) forMe = true; 
      break;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
      if (event.button.windowID == windowId) forMe = true;
      break;
    case SDL_TEXTEDITING:
      if (event.edit.windowID == windowId) forMe = true;
      break;
    case SDL_MOUSEMOTION:
      if (event.motion.windowID == windowId) forMe = true;
      break;
    case SDL_TEXTINPUT:
      if (event.text.windowID == windowId) forMe = true;
      break;
    case SDL_MOUSEWHEEL:
      if (event.wheel.windowID == windowId) forMe = true;
      break;
    case SDL_WINDOWEVENT:
      if (event.window.windowID == windowId) forMe = true;
      break;
  }
  if (!forMe) return false;
  capriceGui->ProcessEvent(event);
  return true;
}
