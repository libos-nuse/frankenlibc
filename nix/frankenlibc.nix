{ stdenv, lib, buildPackages, bc, e2fsprogs, nettools, python, which, zlib
, gcc-arm-embedded, qemu-circle
, rumpKernel ? "netbsd"
, doCheck ? true }:

assert rumpKernel == "netbsd" || rumpKernel == "linux";

let
  inherit (stdenv) buildPlatform hostPlatform targetPlatform;

  rumpCC =
    # XXX: We can't build with pkgsCross.arm-embedded.cc because newlib-nano
    # is required instead of newlib. See NixOS/nixpkgs#51907 for details.
    if targetPlatform.isAarch32 then gcc-arm-embedded
    else assert hostPlatform == targetPlatform; stdenv.cc;

in
stdenv.mkDerivation {
  name = "frankenlibc";

  src = lib.cleanSource ../.;

  depsBuildBuild = [ zlib ]
    ++ lib.optional (buildPlatform != targetPlatform) buildPackages.stdenv.cc;
  depsBuildTarget = [ rumpCC ];
  nativeBuildInputs = [ bc e2fsprogs nettools python which ];
  buildInputs = [ rumpCC ]
    ++ lib.optional targetPlatform.isAarch32 qemu-circle;

  preConfigure = lib.optionalString targetPlatform.isAarch32 ''
    export CC="${gcc-arm-embedded}/bin/arm-none-eabi-gcc"
    export NM="${gcc-arm-embedded}/bin/arm-none-eabi-nm"
    export AR="${gcc-arm-embedded}/bin/arm-none-eabi-ar"
    export OBJCOPY="${gcc-arm-embedded}/bin/arm-none-eabi-objcopy"
  '';

  buildFlags = [
    "-q" # XXX: Travis terminates builds with excessive log ouput
    "-k " rumpKernel
  ] ++ lib.optional targetPlatform.isAarch32 "qemu-arm"
    # XXX: We perform the check phase here because it is tricky to setup outside
    # build.sh.
    ++ lib.optional (!(doCheck && buildPlatform == hostPlatform)) "notest";

  buildPhase = ''
    mkdir -p $out
    ./build.sh -d $out $buildFlags
  '';

  installPhase = ''
    mkdir -p $out/libexec/tools
    mv rumpobj/tests $out/libexec
    mv $out/bin/rump.* $out/libexec/tools || true
  '';

  enableParallelBuilding = true;
  inherit doCheck;
}
