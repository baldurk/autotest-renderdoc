import os
import sys
import traceback
import mimetypes
import difflib
import shutil
from . import util


class TestFailureException(Exception):
    def __init__(self, message, *args):
        self.message = message
        self.files = []
        for a in args:
            self.files.append(str(a))

    def __str__(self):
        return self.message

    def __repr__(self):
        return "<TestFailureException '{}' with files: {}>".format(self.message, repr(self.files))


class TestLogger:
    def __init__(self):
        self.indentation = 0
        self.test_name = ''
        self.outputs = [sys.stdout]
        self.failed = False

    def rawprint(self, line: str, with_stdout=True):
        for o in self.outputs:
            if o == sys.stdout and not with_stdout:
                continue

            for l in line.split('\n'):
                if self.indentation > 0:
                    o.write(self.indentation*' ')
                o.write(l)
                o.write('\n')

            o.flush()

    def add_output(self, o, header='', footer=''):
        os.makedirs(os.path.dirname(o), exist_ok=True)
        self.outputs.append(open(o, "w"))

    def print(self, line: str, with_stdout=True):
        self.rawprint('.. ' + line, with_stdout)

    def header(self, text):
        self.rawprint('\n## ' + text + ' ##\n')

    def indent(self):
        self.indentation += 4

    def dedent(self):
        self.indentation -= 4

    def begin_test(self, test_name: str):
        self.test_name = test_name
        self.rawprint(">> Test {}".format(test_name))
        self.indent()

        self.failed = False

    def end_test(self, test_name: str):
        if self.failed:
            self.rawprint("$$ FAILED")
        self.dedent()
        self.rawprint("<< Test {}".format(test_name))
        self.test_name = ''

    def success(self, message):
        self.rawprint("** " + message)

    def error(self, message):
        self.failed = True

        self.rawprint("!! " + message)

    def failure(self, ex):
        self.failed = True

        self.rawprint("!+ FAILURE in {}: {}".format(self.test_name, ex))

        self.rawprint('>> Callstack')
        tb = traceback.extract_tb(sys.exc_info()[2])
        for frame in reversed(tb):
            filename = frame.filename.replace(util.get_root_dir(), '').replace('\\', '/')
            if filename[0] == '/':
                filename = filename[1:]
            self.rawprint("    File \"{}\", line {}, in {}".format(filename, frame.lineno, frame.name))
        self.rawprint('<< Callstack')

        if isinstance(ex, TestFailureException):
            file_list = []
            for f in ex.files:
                fname = '{}_{}'.format(self.test_name, os.path.basename(f))
                shutil.copyfile(f, util.get_artifact_path(fname))
                file_list.append(fname)

            diff_file = ''
            diff = ''

            # Special handling for the common case where we have two files to generate comparisons
            if len(file_list) == 2:
                mime = mimetypes.guess_type(ex.files[0])

                if 'image' in mime[0]:
                    # If we have two files and they are images, a failed image comparison should have
                    # generated a diff.png. Grab it and include it
                    diff_tmp_file = util.get_tmp_path('diff.png', include_time=False)
                    if os.path.exists(diff_tmp_file):
                        diff_artifact = '{}_diff.png'.format(self.test_name)
                        shutil.move(diff_tmp_file, util.get_artifact_path(diff_artifact))
                        diff_file = ' ({})'.format(diff_artifact)

                elif 'text' in mime[0]:
                    with open(ex.files[0]) as f:
                        fromlines = f.readlines()
                    with open(ex.files[1]) as f:
                        tolines = f.readlines()
                    diff = difflib.unified_diff(fromlines, tolines, fromfile=file_list[0], tofile=file_list[1])

            if diff != '':
                self.rawprint("=+ Compare: " + ','.join(file_list) + diff_file)
                self.indent()
                self.rawprint(''.join(diff).strip())
                self.dedent()
                self.rawprint("=- Compare")
            else:
                self.rawprint("== Compare: " + ','.join(file_list) + diff_file)

        self.rawprint("!- FAILURE")


log = TestLogger()
