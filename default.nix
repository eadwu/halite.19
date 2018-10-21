{ pkgs ? import <nixpkgs> {} }:

with pkgs;

let
  mkLingjian = callPackage ./common.nix { };
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

  lingjian = mkLingjian {
    src = ./bot;
    version = "beta";
  };

  lingjian_legacy1 = mkLingjian {
    src = "${fetchgit {
      url = "https://github.com/HaliteChallenge/Halite-III";
      rev = "v1.0.2";
      sha256 = "1ls0gvzsf2vraa73bahkm3vxycglbbdr1h8rkwn8rb1942gk0axz";
    }}/starter_kits/C++";
    version = "1.0.0";
  };
}
