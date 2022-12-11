#!/usr/bin/env ipython

# kernel_tools.py
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

# -----------------------------------------------------------------------------
# plot the kernel function
def plot(radius, spline, xrange = [0,2], yrange = [0,2]):
    """ Function doc """
    fig = pyplot.figure(figsize=(7,6))
    ax = fig.add_subplot(111)

    # set axes limits
    xmin, xmax = xrange[0], xrange[1]
    ymin, ymax = yrange[0], yrange[1]

    tics_maj = 10
    tics_min = 20

    ax.set_xlim(xmin, xmax)
    ax.set_ylim(ymin, ymax)

    ax.set_xticks(numpy.linspace(xmin, xmax, tics_maj+1))
    ax.set_xticks(numpy.linspace(xmin, xmax, tics_min+1), minor=True)

    ax.set_yticks(numpy.linspace(ymin, ymax, tics_maj+1))
    ax.set_yticks(numpy.linspace(ymin, ymax, tics_min+1), minor=True)

    # ax.grid(which='both')
    ax.grid(which='minor', alpha=0.2)
    ax.grid(which='major', alpha=0.8)

    # set axes labels
    # ax.legend(frameon=False, fontsize="small")
    ax.set_xlabel('x')
    ax.set_ylabel('y')

    # plot data
    ax.plot(radius, spline, 'r-', linewidth=2.0)
    pyplot.show()
    # plt.savefig('fig.pdf', bbox_inches='tight')

    return 0
