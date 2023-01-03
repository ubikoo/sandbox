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
# Execute
#
execute() {
    pushd "${1}"
    popd
}

run make -f Makefile clean
run make -f Makefile -j32 all

plo=0.55;
phi=0.65;
nps=10;
dp=$(echo "($phi - $plo)/$nps" | bc -l)
for (( c=0; c<nps; c++ )); do
    p=$(echo "$plo + $c * $dp" | bc -l)
    run ./percolation.out $p
done

run make -f Makefile clean
