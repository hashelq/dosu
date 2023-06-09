{ lib, pkgs, ... }:
let
    gcc = pkgs.gcc;
    stdenv = pkgs.stdenv;
in stdenv.mkDerivation {
  name = "dosu";
  system = builtins.currentSystem;
  src = ./.;

  phases = [ "unpackPhase" "buildPhase" "installPhase" ];

  buildInputs = [ pkgs.pkgconfig pkgs.libxcrypt ];

  buildPhase = ''
      ${gcc}/bin/gcc ./dosu.c -o dosu `pkg-config --cflags --libs libxcrypt`
  '';

  installPhase = ''
      mkdir -p $out/bin
      cp dosu $out/bin
  '';
}
