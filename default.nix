{ pkgs ? import <nixpkgs> { } }:

with pkgs;

let
  mkHaliteBot = callPackage ./common.nix { };

  haliteVersion = "1.1.4";
  haliteSource = fetchgit {
    url = "https://github.com/HaliteChallenge/Halite-III";
    rev = "v${haliteVersion}";
    sha256 = "1g581bzyyajilvc3p8akvpg0jfdpmaxb6sn188qxspg5j2f0kzhv";
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
    version = "20181114";
    goPackagePath = "github.com/fohristiwhirl/dubnium";

    src = fetchFromGitHub {
      owner = "fohristiwhirl";
      repo = "dubnium";
      rev = "4de9ff73098718ddbc1bc060910dfa27ec74f095";
      sha256 = "0lmxvl1r50l7f07z47kvhinhlpfqsng68bg0pnm2gj1jaxlz8afi";
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
    name = "Halite-III-Benchmark-Bots-${version}";
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
    version = "2.0.3";
    name = "Lingjian-${version}";
  };

  lingjian_legacy2 = mkHaliteBot rec {
    src = "${fetchgit {
      url = ./.;
      rev = version;
      sha256 = "1n93mdrp2k88ihvjfzjzkk28fqli0kxmfzn3mzmv7rgq7hz2fwvp";
    }}/bot";
    version = "2.0.2";
    name = "Lingjian-${version}";
  };

  mycppbot = mkHaliteBot rec {
    src = "${haliteSource}/starter_kits/C++";
    version = "1.0.0";
    name = "MyCppBot-${version}";
  };
}
