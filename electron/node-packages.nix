# This file has been generated by node2nix 1.6.0. Do not edit!

{nodeEnv, fetchurl, fetchgit, globalBuildInputs ? []}:

let
  sources = {
    "node-zstandard-1.2.4" = {
      name = "node-zstandard";
      packageName = "node-zstandard";
      version = "1.2.4";
      src = fetchurl {
        url = "https://registry.npmjs.org/node-zstandard/-/node-zstandard-1.2.4.tgz";
        sha1 = "44f62eb7cb626b948ec79dd2327d3e594ada2f06";
      };
    };
  };
in
{
  "fluorine-git+https://github.com/fohristiwhirl/fluorine" = nodeEnv.buildNodePackage {
    name = "Fluorine";
    packageName = "Fluorine";
    version = "1.2.6";
    src = fetchgit {
      url = "https://github.com/fohristiwhirl/fluorine";
      rev = "d58c343801110eff40a75673dbd7d5a9cf172458";
      sha256 = "dfc87c4a03210f6f08a68edfc0ee695adf61712a571243d3cc9b97062c6f8a9e";
    };
    dependencies = [
      sources."node-zstandard-1.2.4"
    ];
    buildInputs = globalBuildInputs;
    meta = {
      description = "Replay viewer for Halite 3";
    };
    production = true;
    bypassCache = true;
  };
  "iodine-git+https://github.com/fohristiwhirl/iodine" = nodeEnv.buildNodePackage {
    name = "Iodine";
    packageName = "Iodine";
    version = "0.2.0";
    src = fetchgit {
      url = "https://github.com/fohristiwhirl/iodine";
      rev = "d9761d84e1e89fc9a55446b6cd71ab9ead76473b";
      sha256 = "938055c078165d3a8d9e87aebe46a818eec12ecf5d5221a46405b92b0bc4061f";
    };
    buildInputs = globalBuildInputs;
    meta = {
      description = "Realtime viewer for Halite 3";
    };
    production = true;
    bypassCache = true;
  };
}