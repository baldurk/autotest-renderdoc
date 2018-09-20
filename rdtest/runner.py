import os
import shutil
import sys
import re
import renderdoc as rd
from . import util
from . import testcase
from .logging import log


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
    # clean up artifacts and temp folder
    if os.path.exists(util.get_artifact_path('')):
        shutil.rmtree(util.get_artifact_path(''))

    if os.path.exists(util.get_tmp_dir()):
        shutil.rmtree(util.get_tmp_dir())

    log.add_output(util.get_artifact_path("output.log.html"))

    log.header("Tests running for RenderDoc {} ({})".format(rd.GetVersionString(), rd.GetCommitHash()))

    log.print("Running tests matching '{}'".format(test_filter))

    testcases = get_tests()

    regexp = re.compile(test_filter)

    failedcases = []

    for testclass in testcases:
        name = testclass.__name__

        if not regexp.match(name):
            log.print("Skipping {}".format(name))
            continue

        log.begin_test(name)

        try:
            instance = testclass()
            instance.invoketest()
        except Exception as ex:
            log.failure(ex)
            failedcases.append(testclass)

        log.end_test(name)

    log.header("Tests complete: {} passed out of {} run".format(len(testcases)-len(failedcases), len(testcases)))
    log.print("Failed tests:")
    for testclass in failedcases:
        log.print("  '{}'".format(testclass.__name__))

    # Print code to style & invoke the javascript
    log.print('\n\n\n' +
              '<!-- format.js footer: This code will format the log nicely for display in a browser -->\n' +
              '<style>body{visibility:hidden;}</style><script src="format.js"></script>', with_stdout=False)

    # Copy javascript file to artifacts directory
    shutil.copyfile(os.path.join(os.path.dirname(__file__), 'format.js'), util.get_artifact_path('format.js'))

    if len(failedcases) > 0:
        sys.exit(1)

    sys.exit(0)

