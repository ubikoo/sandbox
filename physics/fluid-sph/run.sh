#! /bin/bash

#
# runtest.sh
#
# Copyright (c) 2020 Carlos Braga
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the MIT License.
#
# See accompanying LICENSE.md or https://opensource.org/licenses/MIT.
#

# -----------------------------------------------------------------------------
# die with message
#
die() {
    echo >&2 "$@"
    exit 1
}

#
# run command and check exit code
#
run() {
    echo "$@" && "$@"
    code=$?
    [[ $code -ne 0 ]] && die "[$@] failed with error code $code"
    return 0
}

#
# ask for input query
#
ask() {
    echo -n "$@ (y/n [n]): "
    local ans
    read ans
    [[ "$ans" != "y" ]] && return 1
    return 0
}


# -----------------------------------------------------------------------------
# Run sph
#
execute() {
    pushd "${1}"
    run make -f ../Makefile clean
    run make -f ../Makefile -j48 all
    run ./sph.out
    run make -f ../Makefile clean
    popd
}

execute sph-gpu-full
execute sph-gpu-full-pbc
execute sph-gpu-grid
execute sph-gpu-grid-surface
execute sph-gpu-grid-surface-raymarch
