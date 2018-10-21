{ stdenv, fetchurl
, cmake, p7zip }:

{ src, version }:

with stdenv;

stdenv.mkDerivation {
  inherit src version;
  name = "Lingjian-${version}";

  buildInputs = [
    cmake
    p7zip
  ];

  installPhase = ''
    mkdir -p $out
    find -maxdepth 1 -type f -executable -exec cp "{}" $out \;
    7z a $out/submission.zip ${src}/*
  '';
}
