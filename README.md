# Bram @ Smartelectronix
Open source versions of all bram @ smartelectronix plugins. 20 Years after woking on my first plugin I've decided to open source all these plugins. Consider a lot of this source code was written by a very inexperienced version of myself: it's kind of a mess!

You can find the original smartelectronix pages here: http://bram.smartelectronix.com

## Donations

http://paypal.me/BramdeJong

## Downloads

- Download the latest builds [here](https://dl.bintray.com/bdejong/Plugins/), hosted by [BinTray](http://bintray.com)

## Commerial non-GPL licensing

All these plugins are available for licensing under a dual-license scheme. GPL for open source and non-GPL for commercial usage. For those people who want to collaborate on the project there is a [CLA](https://github.com/bdejong/smartelectronix/wiki/CLA).

# Building
## macOS
Prerequisites:
- Install XCode
- Install brew

After the checkout, run these commands:
```bash
brew update
brew upgrade cmake
git submodule update --init --recursive
python get_steinberg_sdk.py
cmake .
make
make test
```

## Windows
Prerequisites:
- Install Visual Studio 14 2015
- Install python

After the checkout, run these commands in powershell:
```posh
git submodule update --init --recursive
python get_steinberg_sdk.py
.\build.ps1 -Verbose x86 Release
ctest
.\build.ps1 -Verbose x64 Release
ctest
```

# Plugin list
- **AnechoicRoomSimulator**: Silly 1st of April plugin
- **Bitmurderer**: Unrelease plugin which can x-or and mess up bytes in the incoming audio. Even has a nice GUI, but was never finished...
- **Bouncy**: Bouncing ball delay.
- **CrazyIvan**: Insane feedback with distortion effect.
- **Cyanide2**: Spline wave-shaper with oversampling.
- **H2O**: Heavy pumping compressor.
- **Madshifta**: Strange pitch-shifting and delay effect. A collaboration between me (original algorithm), TobyBear.de (translation to Delphi & UI), Sophia Poirier (translation to C++ and AU).
- **OnePingOnly**: Simple ping-generating synth.
- **S(m)exoscope**: Oscilloscope plugin that lets you retrigger the oscilloscope in a few different ways.
- **SupaPhaser2**: Deep phaser.
- **SupaTrigga**: Tempo-locked stuttering effect.
