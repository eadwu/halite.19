{ stdenv, fetchurl
, cmake, p7zip }:

{ name, src, version }:

stdenv.mkDerivation {
  inherit name src version;

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
