Hello and welcome to Tobybear MadShifta!

This is a VST plugin (PC only) by Tobybear (www.tobybear.de) and 
Bram (www.smartelectronix.com) with full Delphi source included!
It is available on Bram's truly excellent DSP source code page 
www.musicdsp.org!

* This is a Mac/C++ distribution put together by Sophia Poirier of 
Destroy FX / Smart Electronix.  
The plugins included in this distribution are for Mac OS X and 
classic Mac OS (8 and 9).  
The source code has been translated from Delphi to C++ by 
Sophia Poirier and CodeWarrior 8 project files are included.  
It's actually not really a straight translation:  the editor 
code was completely rewritten to use Steinberg's VSTGUI library 
and Sophia also changed a few little things here and there.

1. What does it do?
Well, as Bram puts it:  it is a "ultrafast and lofi pitchshifter",
where the pitch of the audio is changed in realtime while the
tempo is preserved.
You can adjust the pitch in semitones (+/- 24 semitones) and
with finetuning.  Furthermore a delay line with feedback is
included and the obligatory LP/HP filter with resonance!
Extra special feature:  you can use MIDI note messages to pitch
the audio in realtime up or down, relatively to a root key.
Furthermore a randomizer, dry/wet and output adjustments enhance
the fun :-)

Note:  Our main intention here was to make clear how such an algorithm
could be implemented, but the way it is presented here may not
necessarily be the most performance-oriented way, so there is always
room for improvement.

2. The controls:
tune:  pitch audio stream up/or down with up to 24 semitones
fine:  fine tuning of the pitching
root:  root note, if MIDI "Note on" messages are received the
audio is pitched relatively to that note in semitones
hold/back:  describes MIDI behaviour: "hold" means the relative
distance between root and current note stays the current pitch,
if "back" is set, it falls back to the original pitch once a "note off"
is received
delay:  delay amount, logarithmic scale
feedback:  feedback amount of the delay
cutoff:  filter cutoff
resonance:  filter resonance
LP/HP:  sets filter type (lowpass or highpass)
dry/wet:  controls amount of processed (pitched) signal in relation
to the original signal
outvol:  the output volume

Click on the bear to the left to randomize all parameters
Click on the note to the right to randomize only pitch

3. Why is the source code in Delphi?
Well, Delphi is still a very underestimated and underrated DSP language,
but it is in my opinion equally powerful as C++.  Check out my other plugins
at www.tobybear.de, they are all coded in Delphi too.
So, first of all, to show people that it is also possible to compile
VST plugins without a C++ compiler, secondly because there aren't that 
much Delphi open source DSP codes floating around.

4. What can I do with the source code?
Look through it, learn from it, enhance it, add new features, but always
give proper credits to both of us!  I say it again:  this plugin and its
source code are mainly meant as a learning resource!
Some possible extensions:
- stereo mode/delay
- variable buffer sizes
- other filter modes
- sync to host
- filter/delay routing
- longer delay times
- LFOs

5. What do I need to compile this plugin?
CodeWarrior 8 is what Sophia used to compile MadShifta.  Much older versions 
can be used to compile classic Mac OS builds, and versions 7 or 6 should 
also be okay for carbon builds.  
You can also use Apple's Project Builder (the old classic Mac OS version) 
to make classic Mac OS builds, but you won't be able to use the VSTGUI 
library if you do that because that library unfortunately only works with 
CodeWarrior.  
The Mac OS X version of Project Builder cannot be used to compile VST 
plugins because the current VST carbon spec is for CFM code-resource 
binaries, and PB for OS X does not do CFM (and it never will).
You can get Project Builder for free at apple.com
CodeWarrior is not free, but you can find out more at metroworks.com

6. Final words
None of us is responsible if anything goes wrong with this plugin, you 
use it at your own risk. Of course we tried to prevent any possible harm, 
after all it is just a simple pitchshifter :-)

Contact:
tobybear@web.de
bram@smartelectronix.com

www.tobybear.de
www.smartelectronix.com
www.musicdsp.org
