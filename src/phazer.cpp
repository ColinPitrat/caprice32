#include "phazer.h"

std::string PhazerType::ToString()
{
  switch(value)
  {
    case None: return "off";
    case AmstradMagnumPhaser: return "Amstrad Magnum Phaser";
    case TrojanLightPhazer: return "Trojan Light Phazer";
    default: return "Unimplemented";
  }
}

PhazerType PhazerType::Next()
{
  auto new_value = value + 1;
  if (new_value == LastPhazerType)
  {
    return PhazerType(None);
  }
  return PhazerType(Value(new_value));
}
