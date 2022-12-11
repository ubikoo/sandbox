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
from mpl_toolkits.mplot3d import axes3d
from scipy.integrate import odeint, ode
from PIL import Image

#sys.path.append('/Users/carlos/usr/local/example/lib')
#import example

# -----------------------------------------------------------------------------
def map(xi, n_steps, r):
    """ Function doc """
    x = numpy.zeros(n_steps)
    x[0] = xi
    for i in range(n_steps-1):
        x[i+1] = r * x[i] * (1.0 - x[i])
        # x[i+1] = numpy.cos(r * x[i])
    return x

##
# compare
def compare(x0, x1, r_coeff, n_steps, title):
    """ Function doc """

    # Compute trajectories
    xt0 = map(x0, n_steps, r_coeff)
    xt1 = map(x1, n_steps, r_coeff)
    xt_diff = abs(xt1 - xt0)
    print(xt_diff.mean())

    # Plot difference
    fig = pyplot.figure(figsize=(8,8), dpi= 100, facecolor='w', edgecolor='k')
    ax = fig.add_subplot(111)
    # ax.grid(which='both')
    ax.grid(which='minor', alpha=0.05)
    ax.grid(which='major', alpha=0.05)
    # ax.legend(frameon=False, fontsize="small")
    # ax.set_xlabel('n')
    # ax.set_ylabel('x')
    ax.plot(xt_diff, '.', markersize=1.0, linewidth=0.1)
    pyplot.title(title)
    pyplot.show()

# -----------------------------------------------------------------------------
def main (argv):
    """ Function doc """
    # Compute single trajectory
    # x_init = 0.2
    # r_coeff = 2.6
    # n_steps = 20
    # xt = map(x0, n_steps, r_coeff)
    # for (ix, value) in enumerate(xt):
    #     print(ix, value)

    # Compute the attracting fixed point of a collection of trajectories
    # x_init = numpy.linspace(0.1, 0.8, 100)
    # r_coeff = 2.0
    # n_steps = 1000
    # for x0 in x_init:
    #     xt = map(x0, n_steps, r_coeff)
    #     print (x0, xt[-1])

    # Compare two trajectories
    x_init0 = 0.200000
    x_init1 = 0.200001
    compare(x_init0, x_init1, 2.0, 500000, "2.0")
    compare(x_init0, x_init1, 3.4, 500000, "3.4")
    compare(x_init0, x_init1, 3.72, 500000, "3.72")

    # Done
    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv))
