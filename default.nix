{ pkgs ? import <nixpkgs> { } }:

with pkgs;

let
  mkHaliteBot = callPackage ./common.nix { };

  haliteVersion = "1.2";
  haliteSource = fetchFromGitHub {
    owner = "HaliteChallenge";
    repo = "Halite-III";
    rev = "v${haliteVersion}";
    sha256 = "1g6xjc3s454vkbn85151l3xc637280crszh0awgbaaqqn2jhzzds";
  };

  electronPackages = import ./electron { inherit pkgs; };

  fluorineWrapper = writeScript "fluorine" ''
    #!${stdenv.shell}
    ${electron}/bin/electron @out@/share/fluorine/electron "$@"
  '';
  Fluorine = electronPackages."fluorine-git+https://github.com/fohristiwhirl/fluorine".override {
    postInstall = ''
      patchelf \
        --set-interpreter "$(cat $NIX_CC/nix-support/dynamic-linker)" \
        $out/lib/node_modules/Fluorine/node_modules/node-zstandard/bin/zstd.linux64
    '';
  };

  iodineWrapper = writeScript "iodine" ''
    #!${stdenv.shell}
    ${electron}/bin/electron @out@/share/iodine/electron "$@"
  '';
  iodineSettings = writeText "settings.json" ''
    {
      "engine": "@dubnium@",
      "sleep": 20
    }
  '';
  Iodine = electronPackages."iodine-git+https://github.com/fohristiwhirl/iodine".override {
    postInstall = ''
      cp ${iodineSettings} $out/lib/node_modules/Iodine/settings.json
    '';
  };
in rec {
  dubnium = buildGoPackage rec {
    name = "Dubnium-${version}";
    version = "20190112";
    goPackagePath = "github.com/fohristiwhirl/dubnium";

    src = fetchFromGitHub {
      owner = "fohristiwhirl";
      repo = "dubnium";
      rev = "19d469db630921c105b28c9d484ace65397d3c87";
      sha256 = "0w9l5b36w85fi51jv7s8f2s96rw7c1z6r3jcb0v75hkgsp0gvz0h";
    };

    allowGoReference = true;

    buildPhase = ''
      go build ${src}/dubnium.go
    '';

    installPhase = ''
      mkdir -p $bin
      cp dubnium $bin
    '';
  };

  fluorine = stdenv.mkDerivation rec {
    name = "Fluorine-${version}";
    inherit (Fluorine) version;

    buildInputs = [
      gtk3
      makeWrapper
      hicolor-icon-theme
    ];

    buildCommand = ''
      mkdir -p $out/bin
      mkdir -p $out/share/fluorine

      cp -r ${Fluorine}/lib/node_modules/Fluorine $out/share/fluorine/electron
      substitute ${fluorineWrapper} $out/bin/fluorine --subst-var out
      chmod +x $out/bin/fluorine
      wrapProgram $out/bin/fluorine \
        --prefix XDG_DATA_DIRS : $GSETTINGS_SCHEMAS_PATH \
        --suffix XDG_DATA_DIRS : ${hicolor-icon-theme}/share
    '';
  };

  halite = stdenv.mkDerivation rec {
    name = "Halite-III-${version}";
    version = haliteVersion;

    srcs = [
      "${haliteSource}/game_engine"

      (fetchFromGitHub {
        name = "catch";
        owner = "catchorg";
        repo = "Catch2";
        rev = "v2.2.3";
        sha256 = "1v7j7rd2i79qaij0izvidjvcjximxp6drimc1ih7sinv2194j1f8";
      })

      ("${fetchFromGitHub {
        owner = "nlohmann";
        repo = "json";
        rev = "v3.1.2";
        sha256 = "1mpr781fb2dfbyscrr7nil75lkxsazg4wkm749168lcf2ksrrbfi";
      }}/include")

      (fetchgit {
        name = "tclap";
        url = "https://git.code.sf.net/p/tclap/code";
        rev = "v1.2.2";
        sha256 = "16hwjkm5q26p11w2wfzbkyxm20z8hn8wid9v8iqhz5cmayz808l7";
      })

      (fetchFromGitHub {
        name = "zstd";
        owner = "facebook";
        repo = "zstd";
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
    name = "Halite-III-Benchmark-Bots-${version}";
    version = "20181020";

    src = fetchFromGitHub {
      owner = "fohristiwhirl";
      repo = "halite3_benchmarks";
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
    version = haliteVersion;

    src = "${haliteSource}/tools/hlt_client";

    propagatedBuildInputs = with python3Packages; [
      zstd
      appdirs
      requests
      trueskill
    ];

    doCheck = false;
  };

  iodine = stdenv.mkDerivation rec {
    name = "Iodine-${version}";
    inherit (Iodine) version;

    buildCommand = ''
      mkdir -p $out/bin
      mkdir -p $out/share/iodine

      cp -r ${Iodine}/lib/node_modules/Iodine $out/share/iodine/electron
      substituteInPlace $out/share/iodine/electron/settings.json \
        --subst-var-by dubnium ${dubnium.bin}/dubnium
      substitute ${iodineWrapper} $out/bin/iodine \
        --subst-var out
      chmod +x $out/bin/iodine
    '';
  };

  lingjian = mkHaliteBot rec {
    src = ./bot;
    version = "3.0.0";
    name = "Lingjian-${version}";
  };

  lingjian_legacy2 = mkHaliteBot rec {
    src = "${fetchgit {
      url = ./.;
      rev = version;
      sha256 = "1p5vli7dcx6pr2rm47kjgq3lq17dm6n362vjqpv0jw55z3ykb71h";
    }}/bot";
    version = "2.0.3";
    name = "Lingjian-${version}";
  };

  mycppbot = mkHaliteBot rec {
    src = "${haliteSource}/starter_kits/C++";
    version = "1.0.0";
    name = "MyCppBot-${version}";
  };
}
