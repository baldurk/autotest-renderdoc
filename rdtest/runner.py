import os
import shutil
import sys
import re
from . import util
from . import testcase


def get_tests():
    testcases = []

    for m in sys.modules.values():
        for name in m.__dict__:
            obj = m.__dict__[name]
            if isinstance(obj, type) and issubclass(obj, testcase.TestCase) and obj != testcase.TestCase:
                testcases.append(obj)

    testcases.sort(key=lambda t: t.__name__)

    return testcases


def run_tests(test_filter=".*"):
    print("Cleaning up")

    if os.path.exists(util.get_tmp_dir()):
        shutil.rmtree(util.get_tmp_dir())

    print("Running tests matching '{}'".format(test_filter))

    testcases = get_tests()

    regexp = re.compile(test_filter)

    for testclass in testcases:
        name = testclass.__name__

        if not regexp.match(name):
            print("Skipping {}".format(name))
            continue

        print("Running {}".format(name))
        instance = testclass()
        instance.run()

    print("Tests complete")