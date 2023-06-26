import urllib
from zipfile import ZipFile
import shutil
import os

try:
    from urllib.request import urlretrieve
except ImportError:
    from urllib import urlretrieve

class ProgressReporter:
    def __init__(self):
        self.progress = None

    def __call__(self, block_count, block_size, total):
        progress = int((block_count * block_size * 100) / total)
        progress = int(progress / 20) * 20
        if progress != self.progress:
            print("{}%".format(progress))
            self.progress = progress


url = "https://www.steinberg.net/vst3sdk"  # noqa
urlretrieve(url, "Steinberg.zip", ProgressReporter())

zip = ZipFile('Steinberg.zip')
zip.extractall()

shutil.rmtree("Steinberg", ignore_errors=True)
os.rename("VST_SDK", "Steinberg")
shutil.rmtree("Steinberg.zip", ignore_errors=True)
