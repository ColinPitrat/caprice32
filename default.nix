with import <nixpkgs> {};
caprice32.overrideDerivation (old: {
  # overrideDerivation allows it to specify additional dependencies
  buildInputs = [ gettext ] ++ old.buildInputs;
})
