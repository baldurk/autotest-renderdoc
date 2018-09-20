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

    def add_output(self, o, header='', footer=''):
        os.makedirs(os.path.dirname(o), exist_ok=True)
        self.outputs.append(open(o, "w"))

    def print(self, line: str, with_stdout=True):
        for o in self.outputs:
            if o == sys.stdout and not with_stdout:
                continue

            for l in line.split('\n'):
                if self.indentation > 0:
                    o.write(self.indentation*' ')
                o.write(l)
                o.write('\n')

            o.flush()

    def header(self, text):
        self.print('\n### ' + text + ' ###\n')

    def indent(self):
        self.indentation += 4

    def dedent(self):
        self.indentation -= 4

    def begin_test(self, test_name: str):
        self.test_name = test_name
        self.print(">> Begin test '{}'".format(test_name))
        self.indent()

    def end_test(self, test_name: str):
        self.dedent()
        self.print("<< End test '{}'".format(test_name))
        self.test_name = ''

    def success(self, message):
        self.print("== " + message)

    def failure(self, ex):
        self.print("!! FAILURE in {}: {}".format(self.test_name, ex))

        tb = traceback.extract_tb(sys.exc_info()[2])
        for frame in tb:
            self.print("!!   File \"{}\", line {}, in {}".format(frame.filename, frame.lineno, frame.name))

        if isinstance(ex, TestFailureException):
            file_list = []
            for f in ex.files:
                fname = '{}_{}'.format(self.test_name, os.path.basename(f))
                shutil.copyfile(f, util.get_artifact_path(fname))
                file_list.append(fname)

            diff_file = ''

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
                    self.print('>> Text diff')
                    self.indent()
                    self.print(''.join(diff).strip())
                    self.dedent()
                    self.print('<< Text diff')

            self.print("!! Compare: " + ','.join(file_list) + diff_file)


log = TestLogger()
