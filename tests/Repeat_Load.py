import rdtest
import os
import psutil
import renderdoc as rd


class Repeat_Load(rdtest.TestCase):
    slow_test = True

    def repeat_load(self, path):
        memory_baseline = 0

        for i in range(20):
            rdtest.log.print("Loading for iteration {}".format(i))

            try:
                controller = rdtest.open_capture(path)
            except RuntimeError as err:
                rdtest.log.print("Skipping. Can't open {}: {}".format(path, err))
                return

            # Do nothing, just ensure it's loaded

            controller.Shutdown()

            memory_usage: int = psutil.Process(os.getpid()).memory_info().rss

            # We measure the baseline memory usage after loading and shutting down the first time (since we don't want
            # to measure the change from nothing to after one capture has been loaded - some persistent caches may have
            # been initialised)
            if memory_baseline == 0:
                memory_baseline = memory_usage
            elif memory_baseline * 1.01 < memory_usage:
                raise rdtest.TestFailureException('In iteration {} memory usage was {}, greater than baseline {}'
                                                  .format(i, memory_usage, memory_baseline))

            rdtest.log.success("Succeeded iteration {}, memory usage is {}".format(i, memory_usage))

    def run(self):
        dir_path = self.get_ref_path('', extra=True)

        for file in os.scandir(dir_path):
            rdtest.log.print('Repeat loading {}'.format(file.name))

            self.repeat_load(file.path)

            rdtest.log.success("Successfully repeat loaded {}".format(file.name))

        rdtest.log.success("Repeat loaded all files")
