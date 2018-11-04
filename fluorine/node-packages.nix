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
  args = rec {
    name = "Fluorine";
    packageName = "Fluorine";
    version = "1.2.3";
    src = fetchgit {
      url = "https://github.com/fohristiwhirl/fluorine";
      rev = "v${version}";
      sha256 = "0j4m462iwam0bb8ksgw5c8ncbf59v3m7js0cwyib7jx7ih8f2z93";
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
in
{
  tarball = nodeEnv.buildNodeSourceDist args;
  package = nodeEnv.buildNodePackage args;
  shell = nodeEnv.buildNodeShell args;
}
