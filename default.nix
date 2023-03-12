let
    pkgs = (import <nixpkgs>) {};
    gcc = pkgs.gcc;
    stdenv = pkgs.stdenv;
in stdenv.mkDerivation {
  name = "dosu";
  system = builtins.currentSystem;
  src = ./.;

  phases = [ "unpackPhase" "buildPhase" "installPhase" ];

  buildInputs = [ pkgs.libxcrypt ];

  buildPhase = ''
      ${gcc}/bin/gcc ./dosu.c -o dosu -lxcrypt
  '';

  installPhase = ''
      mkdir -p $out/bin
      cp dosu $out/bin
  '';
}
