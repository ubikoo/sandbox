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
def main (argv):
    """ Function doc """

    n_colors = 16

    Reds = cm.get_cmap('Reds')
    print('const std::vector<std::vector<uint32_t>> Rop::Reds = {')
    for color in [[*Reds(it)] for it in numpy.linspace(0,1,n_colors)]:
        print('    {', end='')
        for c in color[0:3]:
            print(int(255*c), end=',')
        print('},', end='\n')
    print('};', end='\n')

    Blues = cm.get_cmap('Blues')
    print('const std::vector<std::vector<uint32_t>> Rop::Blues = {')
    for color in [[*Blues(it)] for it in numpy.linspace(0,1,n_colors)]:
        print('    {', end='')
        for c in color[0:3]:
            print(int(255*c), end=', ')
        print('},', end='\n')
    print('};', end='\n')


    # Done
    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv))
