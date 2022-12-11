#!/usr/bin/env ipython

# ising-mean-field.py
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
# solve
def solve(betaJ, betaE):
    maxiter = 10000
    maxerr = 1.0E-6

    err = numpy.finfo(float).max
    iter = 0
    s = 1.0
    while (err > maxerr and iter < maxiter):
        s_new = numpy.tanh(betaJ + 2.0 * betaE * s)
        err = abs(s_new - s)
        s = s_new
        iter += 1
    print("iter ", iter, " s ", s, " err ", err, file=sys.stdout, end='\n')
    return s

# -----------------------------------------------------------------------------
def main (argv):
    """ Function doc """
    n_points = 10000
    J = 0.0
    E = 1.0
    kT = numpy.linspace(0.01, 2.5, n_points)
    beta = 1.0 / kT
    betaJ = beta * J
    betaE = beta * E

    s = []
    for i in range(n_points):
        s.append(solve(betaJ[i], betaE[i]))

    # plot 2d
    fig = pyplot.figure(figsize=(7,6))
    ax = fig.add_subplot(111)

    # set axes limits
    xmin, xmax = min(kT), max(kT)
    ymin, ymax = min(s), max(s)

    tics_maj = 5
    tics_min = 5
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
    ax.legend(frameon=False, fontsize="small")
    ax.set_xlabel('x')
    ax.set_ylabel('y')

    # plot data
    ax.plot(kT, s, 'r-', linewidth=2.0, label="(x,y)")

    pyplot.show()
    # pyplot.savefig('fig.pdf', bbox_inches='tight')

    # Done
    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv))
