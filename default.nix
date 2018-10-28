{ pkgs ? import <nixpkgs> {} }:

with pkgs;

let
  mkHaliteBot = callPackage ./common.nix { };
in {
  halite = stdenv.mkDerivation rec {
    name = "Halite-${version}";
    version = "1.0.2";

    srcs = [
      "${fetchgit {
        url = "https://github.com/HaliteChallenge/Halite-III";
        rev = "v${version}";
        sha256 = "1ls0gvzsf2vraa73bahkm3vxycglbbdr1h8rkwn8rb1942gk0axz";
      }}/game_engine"

      (fetchgit {
        name = "catch";
        url = "https://github.com/catchorg/Catch2";
        rev = "v2.2.3";
        sha256 = "1v7j7rd2i79qaij0izvidjvcjximxp6drimc1ih7sinv2194j1f8";
      })

      ("${fetchgit {
        url = "https://github.com/nlohmann/json";
        rev = "v3.1.2";
        sha256 = "1mpr781fb2dfbyscrr7nil75lkxsazg4wkm749168lcf2ksrrbfi";
      }}/include")

      (fetchgit {
        name = "tclap";
        url = "https://git.code.sf.net/p/tclap/code";
        rev = "v1.2.2";
        sha256 = "16hwjkm5q26p11w2wfzbkyxm20z8hn8wid9v8iqhz5cmayz808l7";
      })

      (fetchgit {
        name = "zstd";
        url = "https://github.com/facebook/zstd";
        rev = "v1.3.4";
        sha256 = "090ba7dnv5z2v4vlb8b275b0n7cqsdzjqvr3b6a0w65z13mgy2nw";
      })
    ];

    postUnpack = ''
      mkdir -p game_engine/build/external

      cp -r --no-preserve=all catch game_engine/build/external
      cp -r --no-preserve=all tclap game_engine/build/external
      cp -r --no-preserve=all zstd game_engine/build/external
      cp -r --no-preserve=all include game_engine/build/external/nlohmann
    '';

    sourceRoot = "game_engine";

    prePatch = ''
      sed -i 's/URL/#\0/' CMakeLists.txt.in
    '';

    buildInputs = [
      cmake
      unzip
      glibc.static
    ];

    installPhase = ''
      mkdir -p $out/bin
      cp halite $out/bin
    '';
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

  hlt-client = python3Packages.buildPythonApplication rec {
    name = "hlt-client-${version}";
    version = "1.0.2";

    src = "${fetchgit {
      url = "https://github.com/HaliteChallenge/Halite-III";
      rev = "v${version}";
      sha256 = "1ls0gvzsf2vraa73bahkm3vxycglbbdr1h8rkwn8rb1942gk0axz";
    }}/tools/hlt_client";

    propagatedBuildInputs = with python3Packages; [
      zstd
      appdirs
      requests
      trueskill
    ];

    doCheck = false;
  };

  lingjian = mkHaliteBot rec {
    src = ./bot;
    version = "2.0.1";
    name = "Lingjian-${version}";
  };

  lingjian_legacy2 = mkHaliteBot rec {
    src = "${fetchgit {
      url = ./.;
      rev = version;
      sha256 = "0g7rx8pvmjg38rcxhfjwa4cpg1d70q8kw6ry5hzhz0yn8di623a9";
    }}/bot";
    version = "2.0.1";
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
}
