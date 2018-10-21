{ pkgs ? import (<nixpkgs>) {} }:

with pkgs;

stdenv.mkDerivation {
  name = "Halite-III";

  buildInputs = [
    cmake
  ];
}
