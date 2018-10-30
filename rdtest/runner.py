import os
import shutil
import ctypes
import sys
import re
import platform
import subprocess
import time
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


def _run_test(testclass, failedcases: list):
    name = testclass.__name__

    # Fork the interpreter to run the test, in case it crashes we can catch it.
    # We can re-run with the same parameters
    args = sys.argv.copy()
    args.insert(0, sys.executable)

    # Add parameter to run the test itself
    args.append('--internal_run_test')
    args.append(name)

    test_run = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)

    try:
        out,err = test_run.communicate(timeout=120)
    except subprocess.TimeoutExpired as timeout:
        log.failure('Timed out, 120s elapsed')
        test_run.kill()
        test_run.communicate()
        raise timeout

    # If we couldn't get the return code, something went wrong in the timeout above
    # and the program never exited. Try once more to kill it then bail
    if test_run.returncode is None:
        test_run.kill()
        test_run.communicate()
        raise RuntimeError('INTERNAL ERROR: Couldn\'t get test return code')
    # Return code of 0 means we exited cleanly, nothing to do
    elif test_run.returncode == 0:
        pass
    # Return code of 1 means the test failed, but we have already logged the exception
    # so we just need to mark this test as failed
    elif test_run.returncode == 1:
        failedcases.append(testclass)
    else:
        raise RuntimeError('Test did not exit cleanly while running, possible crash. Exit code {}'
                           .format(test_run.returncode))


def run_tests(test_include: str, test_exclude: str, in_process: bool, slow_tests: bool):
    start_time = time.time()

    rd.InitGlobalEnv(rd.GlobalEnvironment(), [])

    # On windows, disable error reporting
    if 'windll' in dir(ctypes):
        ctypes.windll.kernel32.SetErrorMode(1 | 2)  # SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX

    # clean up artifacts and temp folder
    if os.path.exists(util.get_artifact_dir()):
        shutil.rmtree(util.get_artifact_dir(), ignore_errors=True)

    if os.path.exists(util.get_tmp_dir()):
        shutil.rmtree(util.get_tmp_dir(), ignore_errors=True)

    log.add_output(util.get_artifact_path("output.log.html"))

    for file in ['testresults.css', 'testresults.js']:
        shutil.copyfile(os.path.join(os.path.dirname(__file__), file), util.get_artifact_path(file))

    log.rawprint('<meta charset="utf-8"><!-- header to prevent output from being processed as html -->' +
                 '<body><link rel="stylesheet" type="text/css" media="all" href="testresults.css">' +
                 '<script src="testresults.js"></script>' +
                 '<script id="logoutput" type="preformatted">\n\n\n', with_stdout=False)

    log.header("Tests running for RenderDoc Version {} ({})".format(rd.GetVersionString(), rd.GetCommitHash()))

    testcases = get_tests()

    include_regexp = re.compile(test_include, re.IGNORECASE)
    exclude_regexp = None
    if test_exclude != '':
        exclude_regexp = re.compile(test_exclude, re.IGNORECASE)
        log.print("Running tests matching '{}' and not matching '{}'".format(test_include, test_exclude))
    else:
        log.print("Running tests matching '{}'".format(test_include))

    failedcases = []
    skippedcases = []

    plat = os.name
    if plat == 'nt' or 'Windows' in platform.platform():
        plat = 'win32'

    ver = 0

    if plat == 'win32':
        try:
            ver = sys.getwindowsversion().major
            if ver == 6:
                ver = 7  # Windows 7 is 6.1
        except AttributeError:
            pass

    for testclass in testcases:
        name = testclass.__name__

        if ((testclass.platform != '' and testclass.platform != plat) or
                (testclass.platform_version != 0 and testclass.platform_version > ver)):
            log.print("Skipping {} as it's not supported on this platform '{} version {}'".format(name, plat, ver))
            skippedcases.append(testclass)
            continue

        if not include_regexp.search(name):
            log.print("Skipping {} as it doesn't match '{}'".format(name, test_include))
            skippedcases.append(testclass)
            continue

        if exclude_regexp is not None and exclude_regexp.search(name):
            log.print("Skipping {} as it matches '{}'".format(name, test_exclude))
            skippedcases.append(testclass)
            continue

        if not slow_tests and testclass.slow_test:
            log.print("Skipping {} as it is a slow test, which are not enabled".format(name))
            skippedcases.append(testclass)
            continue

        # Print header (and footer) outside the exec so we know they will always be printed successfully
        log.begin_test(name)

        util.set_current_test(name)

        try:
            if in_process:
                instance = testclass()
                instance.invoketest()
            else:
                _run_test(testclass, failedcases)
        except Exception as ex:
            log.failure(ex)
            failedcases.append(testclass)

        log.end_test(name)

    duration = time.time() - start_time

    hours = round(duration / 3600)
    minutes = round(duration / 60) % 60
    seconds = round(duration % 60)

    log.header("Tests complete: {} passed out of {} run from {} total in {}:{:02}:{:02}"
               .format(len(testcases)-len(skippedcases)-len(failedcases), len(testcases)-len(skippedcases), len(testcases), hours, minutes, seconds))
    if len(failedcases) > 0:
        log.print("Failed tests:")
    for testclass in failedcases:
        log.print("  - {}".format(testclass.__name__))

    # Print a proper footer if we got here
    log.rawprint('\n\n\n</script>', with_stdout=False)

    if len(failedcases) > 0:
        sys.exit(1)

    sys.exit(0)


def internal_run_test(test_name):
    testcases = get_tests()

    rd.InitGlobalEnv(rd.GlobalEnvironment(), [])

    log.add_output(util.get_artifact_path("output.log.html"))

    for testclass in testcases:
        if testclass.__name__ == test_name:
            log.begin_test(test_name, print_header=False)

            util.set_current_test(test_name)

            try:
                instance = testclass()
                instance.invoketest()
                suceeded = True
            except Exception as ex:
                log.failure(ex)
                suceeded = False

            log.end_test(test_name, print_footer=False)

            if suceeded:
                sys.exit(0)
            else:
                sys.exit(1)

    log.error("INTERNAL ERROR: Couldn't find '{}' test to run".format(test_name))
