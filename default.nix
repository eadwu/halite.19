{ pkgs ? import <nixpkgs> {} }:

with pkgs;

let
  mkHaliteBot = callPackage ./common.nix { };
in {
  halite = stdenv.mkDerivation rec {
    name = "Halite-${version}";
    version = "1.0.2";

    src = fetchurl {
      url = "https://halite.io/assets/downloads/Halite3_Linux-AMD64.zip";
      sha256 = "0wfm2xdif32i5khg8sd7r9kd33ibrix8b49gxxagv975m7wh1q1n";
    };

    buildInputs = [
      unzip
    ];

    buildCommand = ''
      mkdir -p $out/bin
      unzip ${src} -d $out/bin
    '';
  };

  lingjian = mkHaliteBot rec {
    src = ./bot;
    version = "2.0.0";
    name = "Lingjian-${version}";
  };

  mycppbot = mkHaliteBot rec {
    src = "${fetchgit {
      url = "https://github.com/HaliteChallenge/Halite-III";
      rev = "v1.0.2";
      sha256 = "1ls0gvzsf2vraa73bahkm3vxycglbbdr1h8rkwn8rb1942gk0axz";
    }}/starter_kits/C++";
    version = "1.0.0";
    name = "MyCppBot-${version}";
  };

  halite-benchmark-bots = stdenv.mkDerivation rec {
    name = "Halite-Benchmark-Bots-${version}";
    version = "20181020";

    src = fetchgit {
      url = "https://github.com/fohristiwhirl/halite3_benchmarks";
      rev = "da9614c9464b06b9d515e709f3659a7148b4c708";
      sha256 = "0pxc8yl66j7a92r46zqm0hchzmbxamhapihpyxf577zhwiirf1r4";
    };

    dontBuild = true;

    installPhase = ''
      mkdir -p $out

      rm README.md
      cp -r * $out
    '';
  };
}
