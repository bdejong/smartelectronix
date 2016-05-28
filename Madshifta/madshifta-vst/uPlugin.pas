// This is the effect part of the MadShifta source code

// All administrative stuff like parameter management is handled here,
// but of course also the implementation of the actual algorithm.

// Written by Tobias Fleischer/Tobybear (www.tobybear.de),
// original algorithm source code & additional ideas by Bram from
// SmartElectronix (www.smartelectronix.com)

// Even if it is source code, this is meant to be a learning project,
// so don't even try to change just the graphics and sell it :-)

unit uPlugin;

interface
uses Windows, DVSTUtils, uEditor, DAEffect, DAEffectX, DAudioEffect, DAudioEffectX, math;

// some very important VST variables are defined here:
const
 p4=1.0e-24; // the threshold for denormalisation on Pentium 4 CPUs
 kID = 'TBMS'; // unique plugin identifier, has to be different for every plugin
 kChannelID='MadS'; // string displayed in the VSTi channel mixer
 kEffectName = 'MadShifta'; // effect name
 kProduct = 'MadShifta'; // product name
 kVendor = 'Tobybear'; // vendor name
 kIsSynth = false; //false=audio effect, true=synth
 kNumInputs = 2; // number of inputs
 kNumOutputs = 2; // number of outputs
 kCanMono = true; // can be fed with mono signals?
 kCanReplacing = true; //processreplacing() is called instead of process()

 // then we have the settings for the MIDI CC controllers, ie
 // what dial is controlled by what CC:
 kCC_Tune=68;
 kCC_Fine=69;
 kCC_Length=70;
 kCC_Reso=71;
 kCC_Feedback=72;
 kCC_DryWet=73;
 kCC_Cutoff=74;
 kCC_OutVol=75;

 // and finally the set of constants defining the effect's parameter set:
 kTune=0;
 kFine=1;
 kDelayLen=2;
 kDelayFB=3;
 kCutoff=4;
 kResonance=5;
 kFType=6;
 kOutVol=7;
 kDryWet=8;
 kRoot= 9;
 kMMode=10;
 kpar12=11; // reserved for future use
 kpar13=12;
 kpar14=13;
 kpar15=14;
 kpar16=15;

 kNumParams = 16;   // 0-15, can be adjusted to your wishes
 kNumPrograms = 16; // 16 programs per fxb bank
 kNumChannels=2;    // 2 channel mode, might be expanded in future

 type APluginEditor=class;
      dsingle=array[1..kNumChannels] of single;
 APluginProgram=class
 private
  // the parameter set for one program:
  fDryWet,fTune,fDelayLen,fDelayFB,fFine,fRoot,fMMode,
  fCutoff,fResonance,fFType,fOutVol:single;

  // program name:
  name:array[0..50] of char;
 public
  constructor Create;
 end;

 APlugin=class(AudioEffectX)
 private
  // the current parameter set in memory:
  fDryWet,fTune,fDelayLen,fDelayFB,fFine,fRoot,fMMode,
  fCutoff,fResonance,fFType,fOutVol:single;

  // the buffers:
  fade:array[0..100000] of single;
  delay, //the delay buffer
  buffer:array[1..knumchannels,0..65536] of single; // the shift buffer

  // some variables for the algorithm:
  last,old1,old2:array[1..knumchannels] of single; // filter variables
  semitones:integer; // semitone shift (can be negative)
  oldtune, // last valid MIDI tuning
  Cut,Reso:single; // current cutoff and resonance values
  p1,p2, // output pointers
  delay_in,delay_out, // delay buffer pointers
  inp, // input pointer for the shift-buffer
  dp, // pointer increment
  nBuffer, nSize, // determine size of fade buffer
  displace,ndelay:uint;

  notecount:integer; // number of notes currently held down
  // safety flag for destruction of plugin, useful for some older Logic versions
  done:boolean;
 public
  programs:array[0..kNumPrograms-1] of APluginProgram;
  vu:single;
  function GetPluginEditor:APluginEditor;
  constructor CreateAPlugin(audioMaster:TAudioMasterCallbackFunc);virtual;
  destructor Destroy;override;
  procedure DoProcess(var i:dsingle);
  procedure process(inputs,outputs:PPSingle;sampleframes:Longint);override;
  procedure processReplacing(inputs,outputs:PPSingle;sampleframes:Longint);override;
  procedure setProgram(aProgram:Longint);override;
  function canDo(text:pchar):longint;override;
  procedure setProgramName(name:PChar);override;
  procedure getProgramName(name:PChar);override;
  procedure setParameter(index:Longint;value:Single);override;
  function getParameter(index:Longint):Single;override;
  function getVu:Single;override;
  function DoFilter(i:single;cutoff,res:single;ch:byte):Single;
  function processEvents(ev:PVSTEvents):longint;override;
  procedure resume;override;
  procedure suspend;override;
  procedure getParameterLabel(index:longint;text:pchar);override;
  procedure getParameterDisplay(index:longint;text:pchar);override;
  procedure getParameterName(index:longint;text:pchar);override;
  function getOutputProperties(index:longint;properties:PVstPinProperties):boolean;override;
  function getProgramNameIndexed(category,index:longint;text:pchar):boolean;override;
  function getEffectName(name:pchar):boolean;override;
  function getVendorString(text:pchar):boolean;override;
  function getProductString(text:pchar):boolean;override;
  function getVendorVersion:longint;override;
  property PluginEditor:APluginEditor read GetPluginEditor;
 end;

 APluginEditor=class(AEffEditor)
 private
  r:ERect;
  useCount:Longint;
  Editor:TPluginEditorWindow;
  systemWindow:HWnd;
  function GetPlugin:APlugin;
 public
  constructor Create(effect:AudioEffect);override;
  destructor Destroy;override;
  function getRect(rect:PPERect):Longint;override;
  function open(ptr:Pointer):Longint;override;
  procedure close;override;
  procedure idle;override;
  procedure update;override;
  property Plugin:APlugin read GetPlugin;
 end;

implementation
uses SysUtils;

// this function gives you the smallest power of 2
// that is larger than/equal to "number"
function n_larger(number:uint):uint;
var numb:uint;
begin
 numb:=1;
 while (numb<number) do numb:=numb shl 1;
 result:=numb;
end;

// simple approximation
function powerof2(a:double):double;
begin
 result:=exp(a*0.69314718056);
end;

// this is the filter section, currently a simple LP/HP filter
// is included, but this could be extended
function APlugin.DoFilter(i:single;cutoff,res:single;ch:byte):Single;
var fb:single;
begin
 fb:=res+res/(1-cutoff); // calculate feedback for resonance setting
 old1[ch]:=old1[ch]+cutoff*(i-old1[ch]+fb*(old1[ch]-old2[ch]));
 old2[ch]:=old2[ch]+cutoff*(old1[ch]-old2[ch]);
 if fFType<0.5 then DoFilter:=old2[ch]  // return lowpass filtered signal
 else DoFilter:=i-old2[ch]; // return highpass filtered signal
end;

// This is the processing routine that is called for every sample
// from process() or processReplacing()
// In its current implementation, i[0] contains the left channel,
// i[1] the right channel, but this could be extended to more channels.
// NOTE: arrays are generally not the fastest way to access memory
// locations, but they show the structured approach better than direct
// pointer access. But feel free to change that... :-)

// The algorithm goes like this:
// out = filter(pitchshift(input + delay(out)*feedback))*volume

procedure APlugin.DoProcess(var i:dsingle);
var a,b:single;
    ul1,ul2:uint;
    ch:byte;
begin
 // increasing the delay buffer pointers, including
 // some more power-of-2 automatic wrap arounds
 delay_in:=(delay_in+1) AND (nDelay-1);
 delay_out:=(delay_out+1) AND (nDelay-1);

 ul1:=p1 shr 12; // the '12' is to keep the accuracy...
 ul2:=p2 shr 12;
 
 for ch:=1 to knumchannels do // currently this is intended for 2 channels!
 begin
  // write last value at delay_in in delay buffer + denormal fix
  delay[ch,delay_in]:=last[ch]+p4;

  // store input value in buffer + feedback of delay buffer at delay_out
  buffer[ch,inp]:=i[ch]+fdelayfb*delay[ch,delay_out]+p4;

  // apply the fading to smooth things
  a:=buffer[ch,(inp-ul1) AND (nBuffer-1)]*fade[ul1 AND(nBuffer-1)];
  b:=buffer[ch,(inp-ul2) AND (nBuffer-1)]*fade[ul2 AND(nBuffer-1)];

  // apply filter & denormal fix
  last[ch]:=dofilter(a+b,cut,reso,ch)+p4;

  a:=fDryWet*last[ch];

  // do hard clipping, could be improved by saturation shaping:
  if a>1 then a:=1 else if a<-1 then a:=-1;

  // mix processed/unprocessed signal
  i[ch]:=i[ch]*(1-fDryWet)+a+p4;
 end;
 p1:=p1-dp;
 p2:=p2-dp;

 // increase/wrap input pointer
 inp:=(inp+1) AND (nBuffer-1);
end;

procedure APlugin.process(inputs,outputs:PPSingle;sampleframes:Longint);
var In1,In2,Out1,Out2:PSingle;
    inp:dSingle;
    i:Integer;
begin
 In1:=inputs^;
 In2:=PPSingle(Longint(inputs)+SizeOf(PSingle))^;
 Out1:=outputs^;
 Out2:=PPSingle(Longint(outputs)+SizeOf(PSingle))^;
 for i:=0 to sampleFrames-1 do
 begin
  inp[1]:=In1^;
  inp[2]:=In2^;
  DoProcess(inp);
  Out1^:=Out1^+inp[1]*foutvol;
  Out2^:=Out2^+inp[2]*foutvol;
  Inc(In1);
  Inc(In2);
  Inc(Out1);
  Inc(Out2);
 end;
end;

procedure APlugin.processReplacing(inputs, outputs: PPSingle; sampleframes: Longint);
var In1,In2,Out1,Out2:PSingle;
    inp:dSingle;
    i:Integer;
begin
 In1:=inputs^;
 In2:=PPSingle(Longint(inputs)+SizeOf(PSingle))^;
 Out1:=outputs^;
 Out2:=PPSingle(Longint(outputs)+SizeOf(PSingle))^;
 for i:=0 to sampleFrames-1 do
 begin
  inp[1]:=In1^;
  inp[2]:=In2^;
  DoProcess(inp);
  Out1^:=inp[1]*foutvol;
  Out2^:=inp[2]*foutvol;
  Inc(In1);
  Inc(In2);
  Inc(Out1);
  Inc(Out2);
 end;
end;
       
// In this procedure, incoming MIDI events are handled
function APlugin.processEvents(ev:PVstEvents):longint;
var root,note,k,i,status:longint;
    s,nvol:single;
    event:PVstMidiEvent;
    midiData:array[0..3] of byte;
begin
 for i:=0 to ev^.numEvents-1 do
 if (ev.events[i].vtype=kVstMidiType) then
 begin
  event:=PVstMidiEvent(ev^.events[i]); // get current event
  for k:=0 to 3 do midiData[k]:=event.midiData[k];
  status:=midiData[0] AND $f0; // channel information is removed
  if (status=$90)and(mididata[2]>0) then // "note on" ?
  begin
   note:=(midiData[1] and $7F); // midi note
   root:=24+round(fRoot*100);
   // check the boundaries:
   if root<3 then root:=3;
   if root>100 then root:=100;
   inc(notecount);
   if notecount=1 then oldtune:=ftune;
   s:=(root-note)/48+0.5;
   setparameter(ktune,s); // recalculate the tuning
  end
  else if ((status=$90)and(mididata[2]=0))or(status=$80) then // "note off" ?
  begin
   dec(notecount);
   if notecount<=0 then
   begin
    notecount:=0;
    if fMMode<=0.5 then setparameter(ktune,oldtune);
   end; 
  end
  else if (status=$B0) then // Midi CC ?
  begin
   note:=event^.mididata[1]; // Midi CC#
   nvol:=event^.mididata[2]/127; // CC data
   case note of
    kCC_Tune:setparameter(kTune,nvol);
    kCC_Fine:setparameter(kFine,nvol);
    kCC_Length:setparameter(kDelayLen,nvol);
    kCC_Reso:setparameter(kResonance,nvol);
    kCC_Feedback:setparameter(kDelayFB,nvol);
    kCC_DryWet:setparameter(kDryWet,nvol);
    kCC_Cutoff:setparameter(kCutoff,nvol);
    kCC_OutVol:setparameter(kOutVol,nvol);
   else
   end;
   if assigned(editor) then editor.update;
  end
 end;
 Result:=1;
end;

function APlugin.canDo(text:pchar):longint;
begin
 Result:=-1; // may be
 if StrComp(text,'receiveVstEvents')=0 then Result:=1; // can do
 if StrComp(text,'receiveVstMidiEvent')=0 then Result:=1; // can do
end;

constructor APluginProgram.Create;
begin
 inherited Create;
 // Here the parameters for a program are inititialized
 fDryWet:=1;
 fFine:=0.5;
 fTune:=0.5;
 fRoot:=36/100;
 fMMode:=0;
 fDelayLen:=0;
 fDelayFB:=0;
 fCutoff:=1;
 fResonance:=0;
 fFType:=0;
 fOutVol:=1;
 StrCopy(name,'Init'); // set program name
end;

constructor APlugin.CreateAPlugin(audioMaster:TAudioMasterCallbackFunc);
var i:integer;
begin
 inherited Create(audioMaster,knumprograms,kNumParams);

 // create the programs
 for i:=kNumPrograms-1 downto 0 do programs[i] := APluginProgram.Create;

 notecount:=0;
 
 nSize:=10;
 nBuffer:=1 shl nSize; // same as 2^10, but faster
 // nBuffer is the buffer size of the fade buffer

 inp:=0;
 dp:=1;
 dp:=dp-(1 shl 12);

 suspend; // empty the buffers

 // initialize the crossfading buffer with a raised cosine
 for i:=0 to nBuffer-1 do
  fade[i]:=0.5+0.5*cos(((i/(nBuffer-1))-0.5)*2*3.141592);

 editor:=APluginEditor.Create(Self);
 randomize; // important if you use the random() function!

 // set some important variables, according to the constants
 // you specified at the beginning of this file
 hasVu(false);
 setNumInputs(KNumInputs);
 setNumOutputs(KNumOutputs);
 canMono(KCanMono);
 canProcessReplacing(KCanReplacing);
 isSynth(KIsSynth);
 setUniqueID(FourCharToLong(kID[1], kID[2], kID[3], kID[4]));

 // initial program names
 StrCopy(programs[0].name, 'Mad Mother 01');
 StrCopy(programs[1].name, 'Mad Mother 02');
 StrCopy(programs[2].name, 'Mad Mother 03');
 StrCopy(programs[3].name, 'Mad Mother 04');
 StrCopy(programs[4].name, 'Mad Mother 05');
 StrCopy(programs[5].name, 'Mad Mother 06');
 StrCopy(programs[6].name, 'Mad Mother 07');
 StrCopy(programs[7].name, 'Mad Mother 08');
 StrCopy(programs[8].name, 'Mad Mother 09');
 StrCopy(programs[9].name, 'Mad Mother 10');
 StrCopy(programs[10].name, 'Mad Mother 11');
 StrCopy(programs[11].name, 'Mad Mother 12');
 StrCopy(programs[12].name, 'Mad Mother 13');
 StrCopy(programs[13].name, 'Mad Mother 14');
 StrCopy(programs[14].name, 'Mad Mother 15');
 StrCopy(programs[15].name, 'Mad Mother 16');
 curprogram:=0;
 setProgram(0);
end;

destructor APlugin.Destroy;
var i:integer;
begin
 inherited Destroy;
 // destroy the created programs
 for i:=0 to kNumPrograms-1 do programs[i].Free;
end;

procedure APlugin.setProgram(aProgram: Longint);
begin
 if (aprogram<0)or(aprogram>knumprograms-1) then exit;

 curProgram := aProgram;
 // all parameters have to be set here for a program change
 SetParameter(kDryWet,programs[curprogram].fDryWet);
 SetParameter(kFine,programs[curprogram].fFine);
 SetParameter(kTune,programs[curprogram].fTune);
 SetParameter(kRoot,programs[curprogram].fRoot);
 SetParameter(kMMode,programs[curprogram].fMMode);
 SetParameter(kDelayLen,programs[curprogram].fDelayLen);
 SetParameter(kDelayFB,programs[curprogram].fDelayFB);
 SetParameter(kCutoff,programs[curprogram].fCutoff);
 SetParameter(kResonance,programs[curprogram].fResonance);
 SetParameter(kFType,programs[curprogram].fFType);
 SetParameter(kOutVol,programs[curprogram].fOutVol);

 if done then exit;

 if assigned(editor) then editor.update;
end;

procedure APlugin.setProgramName(name:PChar);
begin
 StrCopy(programs[curProgram].name, name);
 if done then exit;
 if assigned(editor) then editor.update;
end;

procedure APlugin.getProgramName(name: PChar);
begin
 StrCopy(name, programs[curProgram].name);
end;

procedure APlugin.suspend;
begin
 // empty those buffers!
 fillchar(buffer,sizeof(buffer),0);
 fillchar(delay,sizeof(delay),0);
end;

function APlugin.getVu:Single;
var cvu:Single;
begin
 cvu:=vu;
 vu:=0;
 Result:=cvu;
end;

procedure APlugin.setParameter(index:Longint;value:Single);
var f:double;
begin
 // value HAS to be between 0 and 1 !!!
 if (value>1) then value:=1 else if (value<0) then value:=0;

 case index of
  kFine:begin
   fFine:=value;programs[curprogram].fFine:=value;
   // recalculate tune and finetune values for the algorithm:
   semitones:=round(-(fTune-0.5)*48);
   f:=powerof2(semitones/12);
   dp:=round(((0.5-fFine)*(f*0.25)+f)*(1 shl 12));
   dp:=dp-(1 shl 12);
  end;
  kTune:begin
   // recalculate tune and finetune values for the algorithm:
   fTune:=value;programs[curprogram].fTune:=value;
   semitones:=round(-(fTune-0.5)*48);
   f:=powerof2(semitones/12);
   dp:=round(((0.5-fFine)*(f*0.25)+f)*(1 shl 12));
   dp:=dp-(1 shl 12);
  end;
  kDelayLen:begin
   fDelayLen:=value;programs[curprogram].fDelayLen:=value;
   // logarithmic scale for this control:
   value:=round(-500*log10((1-value)+0.01))/2000;
   // lower boundary:
   if value<0.0001 then value:=0.0001;
   displace:=round(44100*value)+1;
   ndelay:=n_larger(displace);
   delay_in:=0;
   // nifty wrap-around:
   delay_out:=(delay_in-displace) AND (nDelay-1);
   inp:=0;
   p1:=0;
   p2:=(nBuffer shr 1) shl 12; // p2 starts at center of fade
  end;
  kCutoff:begin
   Cut:=value;
   // simple boundary check for filter cutoff (to keep it stable):
   if value>0.99 then Cut:=0.99 else if value<0.01 then Cut:=0.01;
   fCutoff:=value;programs[curprogram].fCutoff:=value;
  end;
  kResonance:begin
   // simple boundary check for filter resonance (to keep it stable):
   Reso:=value;if value>0.98 then Reso:=0.98;
   fResonance:=value;programs[curprogram].fResonance:=value;
  end;
  // these go regularly:
  kDryWet:begin fDryWet:=value;programs[curprogram].fDryWet:=value end;
  kDelayFB:begin fDelayFB:=value;programs[curprogram].fDelayFB:=value end;
  kFType:begin fFType:=value;programs[curprogram].fFType:=value end;
  kRoot:begin fRoot:=value;programs[curprogram].fRoot:=value end;
  kMMode:begin fMMode:=value;programs[curprogram].fMMode:=value end;
  kOutVol:begin fOutVol:=value;programs[curprogram].fOutVol:=value end;
 end;
 if assigned(editor) then editor.update;
end;

function APlugin.getParameter(index: Longint): Single;
var j:single;
begin
 case index of
  kDryWet:j:=fDryWet;
  kFine:j:=fFine;
  kTune:j:=fTune;
  kRoot:j:=fRoot;
  kMMode:j:=fMMode;
  kDelayLen:j:=fDelayLen;
  kDelayFB:j:=fDelayFB;
  kCutoff:j:=fCutoff;
  kResonance:j:=fResonance;
  kFType:j:=fFType;
  kOutVol:j:=fOutVol;
 else
  j:=0;
 end;
 Result:=j;
end;

function APlugin.GetPluginEditor:APluginEditor;
begin
 Result:=(editor as APluginEditor);
end;

constructor APluginEditor.Create(effect:AudioEffect);
begin
 inherited Create(effect);
 useCount:=0;
end;

destructor APluginEditor.Destroy;
begin
 if assigned(editor) then
 begin
  Plugin.done:=true;
  Editor.Free;
  systemWindow:=0;
 end;
 inherited Destroy;
end;

function APluginEditor.getRect(rect:PPERect):Longint;
begin
 r.top:=0;
 r.left:=0;
 // some window handling stuff, since every hosts calls this at
 // other times or gets the data from different sources:
 if assigned(editor) then
 begin
  r.bottom:=editor.ClientHeight;
  r.right:=editor.ClientWidth;
 end else
 begin
  r.bottom:=360;
  r.right:=432;
 end;
 rect^:=@r;
 Result := 1;
end;

function APluginEditor.open(ptr:Pointer):Longint;
begin
 systemWindow := HWnd(ptr);
 Inc(useCount);
 if (useCount=1)or(not assigned(editor)) then
 begin
  Editor:=TPluginEditorWindow.CreateParented(systemWindow);
  Editor.SetBounds(0,0,Editor.Width,Editor.Height);
  // initialize editor's parameter set
  Editor.fDryWet:=Plugin.fDryWet;
  Editor.fFine:=Plugin.fFine;
  Editor.fTune:=Plugin.fTune;
  Editor.fRoot:=Plugin.fRoot;
  Editor.fMMode:=Plugin.fMMode;
  Editor.fDelayLen:=Plugin.fDelayLen;
  Editor.fDelayFB:=Plugin.fDelayFB;
  Editor.fCutoff:=Plugin.fCutoff;
  Editor.fResonance:=Plugin.fResonance;
  Editor.fFType:=Plugin.fFType;
  Editor.fOutVol:=Plugin.fOutVol;
  Editor.Effect:=Self.Plugin;
 end;
 Plugin.done:=false;
 Editor.Update;
 Editor.Show;
 editor.UpdaterTimer(nil);
 Result:=1;
end;

procedure APluginEditor.close;
begin
 if assigned(editor) then editor.visible:=false;
 Dec(useCount);

 // compensate Logic bug, where usecount can even get negative:
 if usecount<0 then usecount:=0;

 if useCount=0 then
 begin
  Plugin.done:=true;
  Editor.Free;
  Editor:=nil;
  systemWindow:=0;
 end;
end;

procedure APluginEditor.idle;
begin
 if (Plugin.done) or (not Assigned(Editor)) then exit;
end;

procedure APluginEditor.update;
begin
 // whenever this function is called, the current (effect's) parameter
 // set is copied to the editor's parameter set
 if (Plugin.done) or (not Assigned(Editor)) then exit;
 Editor.fDryWet:=Plugin.fDryWet;
 Editor.fFine:=Plugin.fFine;
 Editor.fTune:=Plugin.fTune;
 Editor.fRoot:=Plugin.fRoot;
 Editor.fMMode:=Plugin.fMMode;
 Editor.fDelayLen:=Plugin.fDelayLen;
 Editor.fDelayFB:=Plugin.fDelayFB;
 Editor.fCutoff:=Plugin.fCutoff;
 Editor.fResonance:=Plugin.fResonance;
 Editor.fFType:=Plugin.fFType;
 Editor.fOutVol:=Plugin.fOutVol;
end;

function APluginEditor.GetPlugin: APlugin;
begin
 Result:=(effect as APlugin);
end;

function APlugin.getEffectName(name:pchar): boolean;
begin
 StrCopy(name,kEffectName);
 Result:=TRUE;
end;

function APlugin.getVendorString(text:pchar):boolean;
begin
 StrCopy(text, kVendor);
 Result:=TRUE;
end;

function APlugin.getProductString(text:pchar):boolean;
begin
 StrCopy(text,kProduct);
 Result:=TRUE;
end;

function APlugin.getVendorVersion:longint;
begin
 Result:=1; // return version number
end;

function APlugin.getOutputProperties(index:longint;properties:PVstPinProperties):boolean;
begin
 Result:=false;
 if (index<kNumOutputs) then
 begin
  StrCopy(properties^.vLabel, pchar(Format(kChannelID+' %d', [index+1])));
  properties^.flags:=kVstPinIsActive;
  if (index<2) then
   properties^.flags:=properties^.flags or kVstPinIsStereo;
  Result:=true;
 end;
end;

function APlugin.getProgramNameIndexed(category,index:longint;text:pchar):boolean;
begin
 Result:=false;
 if (index<kNumPrograms) then
 begin
  StrCopy(text,programs[index].name);
  Result:=true;
 end;
end;

procedure APlugin.getParameterName(index:longint;text:pchar);
begin
 case index of
 kFine:StrCopy(text, 'FineTune');
 kDryWet:StrCopy(text, 'DryWet');
 kTune:StrCopy(text, 'Tune');
 kRoot:StrCopy(text, 'Root');
 kMMode:StrCopy(text, 'MidiMode');
 kDelayLen:StrCopy(text, 'DelayLen');
 kDelayFB:StrCopy(text, 'DelayFB');
 kCutoff:StrCopy(text, 'Cutoff');
 kResonance:StrCopy(text, 'Reso');
 kFType:StrCopy(text, 'FltType');
 kOutVol:StrCopy(text, 'OutVol');
 else StrCopy(text, 'reserved');
 end;
end;

procedure APlugin.getParameterDisplay(index:longint;text:pchar);
begin
 case index of
  kFine:float2string(fFine, text);
  kDryWet:float2string(fDryWet, text);
  kTune:float2string(fTune, text);
  kRoot:float2string(fRoot, text);
  kMMode:float2string(fMMode, text);
  kDelayLen:float2string(fDelayLen, text);
  kDelayFB:float2string(fDelayFB, text);
  kCutoff:float2string(fCutoff, text);
  kResonance:float2string(fResonance, text);
  kFType:float2string(fFType, text);
  kOutVol:float2string(fOutVol, text);
  else float2string(0, text);
 end;
end;

procedure APlugin.getParameterLabel(index:longint;text:pchar);
begin
 StrCopy(text, '%');
end;

procedure APlugin.resume;
begin
 wantEvents(1); // important for all plugins that receive MIDI!
end;

end.

