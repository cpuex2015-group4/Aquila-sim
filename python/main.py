#!/usr/bin/env python
# coding: utf-8

"""
Simple simulator script

Usage
---------
$ python /path/to/this_script [exec-file-name]

"""
import sys
from argparse import ArgumentParser
from simulator import Simulator
import utils

if __name__ == "__main__":
	desc = u'{0} [Args] [Options]\nDetailed options -h or --help'.format(__file__)
	parser = ArgumentParser(description=desc)

	parser.add_argument(
		dest = 'filename',
		action = 'store',
		type = str,
		help = 'target testbench name')

	parser.add_argument(
		'-v', '--verbose',
		action = 'store_true',
		dest = 'verbose',
		help = "use verbose output")

	parser.add_argument(
		'-f', '--float',
		action = 'store_true',
		dest = 'get_float',
		help = "get return value as float")

	args = parser.parse_args()

	sim = Simulator(args.filename)
	v0 = sim.simulate(verbose = args.verbose)
	if args.get_float:
		v0 = v0[1]
	else:
		v0 = v0[0]
	sys.stderr.write("\n===== RESULT =====\nbin  : {}\nint  : {}\nfloat: {}".format(v0, int(v0, 2), utils.reg2float(v0)))
