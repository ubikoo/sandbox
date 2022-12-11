#!/usr/bin/env ipython

# untitled.py
#
# Copyright (c) 2020 Carlos Braga
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the MIT License.
#
# See accompanying LICENSE.md or https://opensource.org/licenses/MIT.

import sys
import logging
import numpy
import matplotlib
from matplotlib import pyplot
from matplotlib import cm
from mpl_toolkits.mplot3d import axes3d
from scipy.integrate import odeint, ode

# -----------------------------------------------------------------------------
##
# next_power_two
def next_power_two(number):
    """ Function doc """
    n = 1
    while n < number:
        n <<= 1
    print(number, n, file=sys.stdout, end='\n')
    return n

##
# compute_index
def red_black_index(n_sites, offset):
    """ Function doc """
    result=[]
    for i in range(n_sites):
        result.append(['.' for _ in range(n_sites)])

    for i in range(n_sites):
        for j in range((i+offset)%2, n_sites, 2):
            result[i][j] = '*'
    return result

# -----------------------------------------------------------------------------
def main (argv):
    """ Function doc """
    n_sites = next_power_two(10)

    offset = 0
    while True:
        result = red_black_index(n_sites, offset)
        offset = 1 if offset == 0 else 0

        for i in range(n_sites):
            for j in range(n_sites):
                print(result[i][j], file=sys.stdout, end=' ')
            print(file=sys.stdout, end='\n')
        input("press return to continue")

    # Done
    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv))
