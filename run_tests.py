import argparse
import os
import sys

parser = argparse.ArgumentParser()
parser.add_argument('-r', '--renderdoc', help="The location of the renderdoc library to use", type=str)
parser.add_argument('-p', '--pyrenderdoc', help="The location of the renderdoc python module to use", type=str)
parser.add_argument('-l', '--list', help="Lists the tests available to run", action="store_true")
parser.add_argument('-t', '--test_filter', default=".*", help="The tests to run, as a regexp filter", type=str)
args = parser.parse_args()

if args.renderdoc is not None:
    if os.path.isfile(args.renderdoc):
        os.environ["PATH"] += os.pathsep + os.path.abspath(os.path.dirname(args.renderdoc))
    elif os.path.isdir(args.renderdoc):
        os.environ["PATH"] += os.pathsep + os.path.abspath(args.renderdoc)
    else:
        raise RuntimeError("'{}' is not a valid path to the renderdoc library".format(args.renderdoc))

if args.pyrenderdoc is not None:
    if os.path.isfile(args.pyrenderdoc):
        sys.path.insert(0, os.path.abspath(os.path.dirname(args.pyrenderdoc)))
    elif os.path.isdir(args.pyrenderdoc):
        sys.path.insert(0, os.path.abspath(args.pyrenderdoc))
    else:
        raise RuntimeError("'{}' is not a valid path to the pyrenderdoc module".format(args.pyrenderdoc))

import rdtest as rdtest

from tests import *

if args.list:
    for test in rdtest.get_tests():
        print("Test: {}".format(test.__name__))
    sys.exit(0)

rdtest.run_tests(args.test_filter)
