#ifndef MEMUTILS_H
#define MEMUTILS_H

namespace memutils
{
  template<typename cap32_cb>
  class scope_exit {
    cap32_cb cb;
    bool upd {true};
  public:
    scope_exit(cap32_cb _cb) : cb(std::move(_cb)) {}
    scope_exit(const scope_exit &) = delete;
    scope_exit& operator=(const scope_exit &) = delete;
    scope_exit(scope_exit &&d) : cb(d.cb), upd(d.upd)
    {
      d.upd = false;
    }
    ~scope_exit()
    {
      if (upd)
        cb();
    }
  };
}

#endif

