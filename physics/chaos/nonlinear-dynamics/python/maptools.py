#!/usr/bin/env ipython

# breadth_first_search.py
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
def trajectory(ax, xt, yt, scale):
    """ Function doc """
    tics_maj = 5
    xmin, xmax = min(xt) / scale, scale * max(xt)
    ymin, ymax = min(yt) / scale, scale * max(yt)
    ax.set_xlim(xmin, xmax)
    ax.set_ylim(ymin, ymax)
    ax.set_xticks(numpy.linspace(xmin, xmax, tics_maj+1))
    # ax.set_xticks(numpy.linspace(xmin, xmax, tics_min+1), minor=True)
    ax.set_yticks(numpy.linspace(ymin, ymax, tics_maj+1))
    # ax.set_yticks(numpy.linspace(ymin, ymax, tics_min+1), minor=True)

    # ax.grid(which='both')
    ax.grid(which='minor', alpha=0.05)
    ax.grid(which='major', alpha=0.05)
    # ax.legend(frameon=False, fontsize="small")
    # ax.set_xlabel('n')
    # ax.set_ylabel('x')
    ax.plot(xt, yt, '.', markersize=1.0, linewidth=0.1)

# -----------------------------------------------------------------------------
def cobweb(ax, x, y, yt, scale):
    """ Function doc """
    tics_maj = 5
    xmin, xmax = min(yt) / scale, scale * max(yt)
    ymin, ymax = min(yt) / scale, scale * max(yt)
    ax.set_xlim(xmin, xmax)
    ax.set_ylim(ymin, ymax)
    ax.set_xticks(numpy.linspace(xmin, xmax, tics_maj+1))
    # ax.set_xticks(numpy.linspace(xmin, xmax, tics_min+1), minor=True)
    ax.set_yticks(numpy.linspace(ymin, ymax, tics_maj+1))
    # ax.set_yticks(numpy.linspace(ymin, ymax, tics_min+1), minor=True)

    # ax.grid(which='both')
    ax.grid(which='minor', alpha=0.05)
    ax.grid(which='major', alpha=0.05)
    # ax.legend(frameon=False, fontsize="small")
    # ax.set_xlabel('x')
    # ax.set_ylabel('x')

    ax.plot(x, x, 'k-', markersize=0.2, linewidth=0.5)
    ax.plot(x, y, 'r-', markersize=0.2, linewidth=0.5)
    for k in range(len(yt)-1):
        ax.plot([yt[k], yt[k]], [yt[k], yt[k+1]], 'g-', markersize=0.2, linewidth=0.25)
        ax.plot([yt[k], yt[k+1]], [yt[k+1], yt[k+1]], 'b-', markersize=0.2, linewidth=0.25)

# -----------------------------------------------------------------------------
def display(x, y, xt, yt, scale = 1.0):
    fig = pyplot.figure(figsize=(12,8), dpi= 100, facecolor='w', edgecolor='k')

    ax1 = fig.add_subplot(121)
    trajectory(ax1, xt, yt, scale)

    ax2 = fig.add_subplot(122)
    cobweb(ax2, x, y, yt, scale)

    pyplot.show()
    # pyplot.savefig('fig.pdf', bbox_inches='tight')
