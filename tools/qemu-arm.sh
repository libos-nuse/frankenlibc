#!/bin/bash

LKL_TAP="${LKL_TAP:-none}"
LKL_DEBUG="${LKL_DEBUG:-1}"
LKL_QEMU_CMDLINE=
BIN_PATH="$1"

main() {
  if [[ "$LKL_TAP" != "none" ]]; then
    sudo env PATH="$PATH" \
      qemu-system-arm \
        $LKL_QEMU_ARGS \
        -M raspi2 \
        -nographic \
        -device usb-net,vlan=0 \
        -net tap,vlan=0,ifname="$LKL_TAP",script=no,downscript=no \
        -append "$(build_qemu_cmdline "$@")" \
        -semihosting -kernel "$BIN_PATH"
  else
    qemu-system-arm \
      $LKL_QEMU_ARGS \
      -M raspi2 \
      -nographic \
      -device usb-net,vlan=0 \
      -net user,vlan=0 \
      -append "$(build_qemu_cmdline "$@")" \
      -semihosting -kernel "$BIN_PATH"
  fi
}

build_qemu_cmdline() {
  while [[ "$1" != "--" ]] && (( $# > 0 )); do
    shift
  done
  shift

  local cmdline=( \
    "logdev=ttyS1" \
    "env=LKL_HIJACK_NET_IFTYPE=${LKL_HIJACK_NET_IFTYPE:-raspi}" \
    "env=LKL_HIJACK_NET_IFPARAMS=${LKL_HIJACK_NET_IFPARAMS:-1}" \
    "env=LKL_HIJACK_NET_IP=${LKL_HIJACK_NET_IP:-1.1.1.1}" \
    "env=LKL_HIJACK_NET_NETMASK_LEN=${LKL_HIJACK_NET_NETMASK_LEN:-24}" \
  )

  [[ -n "$RUMP_VERBOSE" ]] && cmdline+=("env=RUMP_VERBOSE=$RUMP_VERBOSE")
  [[ -n "$LKL_DEBUG" ]] && cmdline+=("env=LKL_HIJACK_DEBUG=$LKL_DEBUG")

  cmdline+=("--" "$(basename "$BIN_PATH")" "$@" )

  echo "${cmdline[@]}"
}

main "$@"
