#ifndef PHAZER_H
#define PHAZER_H

#include <string>

class PhazerType {
  public: 
    enum Value
    {
      None = 0,
      AmstradMagnumPhaser = 1,
      TrojanLightPhazer = 2,
      LastPhazerType,
    };
    PhazerType() = default;
    constexpr PhazerType(Value val) : value(val) { }

    std::string ToString();
    PhazerType Next();
    operator Value() const { return value; };

    // if(phazer_type) 
    operator bool() const { return value != None; };

  private:
    Value value;
};


#endif
