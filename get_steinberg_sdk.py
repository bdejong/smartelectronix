import urllib
from zipfile import ZipFile
import shutil
import os


class ProgressReporter:
    def __init__(self):
        self.progress = None

    def __call__(self, block_count, block_size, total):
        progress = (block_count * block_size * 100) / total
        progress = (progress / 20) * 20
        if progress != self.progress:
            print "{}%".format(progress)
            self.progress = progress


url = "http://www.steinberg.net/sdk_downloads/vstsdk365_12_11_2015_build_67.zip"  # noqa
urllib.urlretrieve(url, "Steinberg.zip", ProgressReporter())

zip = ZipFile('Steinberg.zip')
zip.extractall()

shutil.rmtree("Steinberg", ignore_errors=True)
os.rename("VST3 SDK", "Steinberg")
shutil.rmtree("Steinberg.zip", ignore_errors=True)
