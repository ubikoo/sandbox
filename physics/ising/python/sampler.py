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
import numpy
import collections

##
# Sampler
class Sampler:
    """ Class doc """

    # ---- Sampler special methods --------------------------------------------
    def __init__(self, name):
        self.name = name
        self.items = collections.deque()
        self.n_items = 0

    def __del__(self):
        pass

    def __repr__(self):
        (avg, sdv) = self.stats()
        s = []
        s.append(" %s " % self.name)
        s.append(" %d " % self.n_items)
        s.append(" %f " % avg)
        s.append(" %f " % sdv)
        return ''.join(s)

    def __iter__(self):
        ix = 0
        while ix < self.n_items:
            item = self.items[ix]
            yield item
            ix += 1

    # ---- Sampler API --------------------------------------------------------
    def add(self, item):
        """ Add a new sample """
        self.items.append(item)
        self.n_items += 1

    def clear(self, arg):
        """ Clear the samples """
        self.items.clear()
        self.n_items = 0

    def size(self):
        """ Return the sample size """
        return self.n_items

    def stats(self):
        """ Compute sample statistics """
        if self.n_items < 2:
            return (0, 0)

        norm = float(self.n_items)
        avg = 0
        for item in self.items:
            avg += item
        avg /= norm

        var = 0
        for item in self.items:
            var += (item - avg) * (item - avg)
        var /= (norm * (norm - 1.0))
        sdv = numpy.sqrt(var)

        return (avg, sdv)

# -----------------------------------------------------------------------------
def main (argv):
    """ Function doc """

    n_items = 1000000
    sigma = 1.0
    mu = 1.0
    smp_1 = Sampler("smp_1")
    smp_2 = Sampler("smp_2")
    [smp_1.add(sigma*x + mu) for x in numpy.random.randn(n_items)]
    [smp_2.add(sigma*x - mu) for x in numpy.random.randn(n_items)]
    print(smp_1)
    print(smp_2)

    # Done
    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv))
