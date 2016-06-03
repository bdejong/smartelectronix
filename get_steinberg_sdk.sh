#!/bin/sh
wget http://www.steinberg.net/sdk_downloads/vstsdk365_12_11_2015_build_67.zip
unzip -q -o vstsdk365_12_11_2015_build_67.zip
mv -rf VST3\ SDK/* Steinberg/
rm -rf VST3\ SDK vstsdk365_12_11_2015_build_67*
