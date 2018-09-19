import os
import time
from PIL import Image, ImageChops, ImageStat


def _timestr():
    return time.strftime("%Y%m%d_%H_%M_%S", time.gmtime()) + "_" + str(round(time.time() % 1000))


def get_tmp_dir():
    return os.path.join(os.getcwd(), 'tmp')


def get_tmp_path(name: str):
    return os.path.join(get_tmp_dir(), '{}_{}'.format(_timestr(), name))


def image_compare(test_img: str, ref_img: str):
    out = Image.open(test_img)
    ref = Image.open(ref_img)

    if out.mode != ref.mode or out.size != ref.size:
        return False

    # Require an exact binary match
    if sum(ImageStat.Stat(ImageChops.difference(out, ref)).sum) > 0:
        return False

    return True

