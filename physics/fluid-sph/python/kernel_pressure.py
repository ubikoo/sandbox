#!/usr/bin/env ipython

# kernel_val.py
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

import kernel_tools

# -----------------------------------------------------------------------------
# kernel_val
def kernel_val(r, h):
    if r <= 0.0 or r >= h:
        return 0.0

    h3 = h * h * h
    h6 = h3 * h3
    C = 15.0 / (numpy.pi * h6)
    z = h - r

    return C * z * z * z

# kernel_grad
def kernel_grad(r, h):
    if r <= 0.0 or r >= h:
        return 0.0

    h3 = h * h * h
    h6 = h3 * h3
    C = -15.0 / (numpy.pi * h6)

    z = h - r
    return C * z * z / r

# -----------------------------------------------------------------------------
# main function
def main (argv):
    """ Function doc """
    n_points = 1000
    kernel_h = 1.0

    # Compute the kernel value
    radius = numpy.linspace(0, 2*kernel_h, n_points, dtype=numpy.float64)
    spline = [kernel_val(r, kernel_h) for r in radius]
    spline_grad = [kernel_grad(r, kernel_h) for r in radius]

    # Compute the kernel integral
    # for it in spline:
    #     print(it, file=sys.stdout, end='\n')
    spline_sphere = 4 * numpy.pi * radius * radius * spline
    print(numpy.trapz(spline_sphere, radius), file=sys.stdout, end='\n')

    kernel_tools.plot(radius, spline, [0,2], [0,5])
    kernel_tools.plot(radius, spline_grad, [0,2], [-50,5])
    return 0
#end main

if __name__ == '__main__':
    sys.exit(main(sys.argv))
