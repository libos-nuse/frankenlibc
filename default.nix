{ pkgs ? import <nixpkgs> {} }:

with pkgs;

let
  callPackage = lib.callPackageWith (pkgs // self);

  keepDebugInfo = stdenv: stdenv // {
    mkDerivation = args: stdenv.mkDerivation (args // {
      dontStrip = true;
    });
  };

  self = {
    frankenlibc-gcc = callPackage ./nix/frankenlibc.nix {
      stdenv = overrideCC stdenv gcc6;
    };

    frankenlibc-clang = callPackage ./nix/frankenlibc.nix {
      inherit (llvmPackages_38) stdenv;
    };

    frankenlibc-lkl = callPackage ./nix/frankenlibc.nix {
      rumpKernel = "linux";
    };

    frankenlibc-arm = callPackage ./nix/frankenlibc.nix {
      stdenv = pkgsCross.arm-embedded.buildPackages.stdenv;
      rumpKernel = "linux";
    };

    qemu-circle = callPackage ./nix/qemu-circle.nix { };

    # XXX: We can't use enableDebugging from nixpkgs, since the linux kernel
    # refuses to compile with -O0 flags.
    enableDebugging = pkg: pkg.override { stdenv = keepDebugInfo pkg.stdenv; };
  };

in self
