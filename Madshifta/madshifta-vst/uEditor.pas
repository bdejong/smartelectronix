// This is the editor part of the MadShifta source code

// All graphical input is handled here.
// To view the corresponding graphical interface, press F12 in Delphi.
// Note: You need to have the TobyGauges.pas component installed
// before opening this form, else you get an error message!

// All of this part is written by Tobias Fleischer/Tobybear (tobybear@web.de)
// and even if it is source code, this is meant to be a learning project,
// so don't even try to change just the graphics and sell it :-)

unit uEditor;
interface
uses Forms, DAudioEffectX, TobyGauges, DVSTUtils, Dialogs, SysUtils,
     StdCtrls, ExtCtrls, Graphics, Controls, Classes;

// below is an array of string corresponding to the note names of MIDI notes
// used in the root key selector
const na:array[0..96] of string[3]=(
'C 0','C#0','D 0','D#0','E 0','F 0','F#0','G 0','G#0','A 0','A#0','B 0',
'C 1','C#1','D 1','D#1','E 1','F 1','F#1','G 1','G#1','A 1','A#1','B 1',
'C 2','C#2','D 2','D#2','E 2','F 2','F#2','G 2','G#2','A 2','A#2','B 2',
'C 3','C#3','D 3','D#3','E 3','F 3','F#3','G 3','G#3','A 3','A#3','B 3',
'C 4','C#4','D 4','D#4','E 4','F 4','F#4','G 4','G#4','A 4','A#4','B 4',
'C 5','C#5','D 5','D#5','E 5','F 5','F#5','G 5','G#5','A 5','A#5','B 5',
'C 6','C#6','D 6','D#6','E 6','F 6','F#6','G 6','G#6','A 6','A#6','B 6',
'C 7','C#7','D 7','D#7','E 7','F 7','F#7','G 7','G#7','A 7','A#7','B 7',
'C 8');

type
  TPluginEditorWindow = class(TForm)
    bg: TImage;
    Label03: TLabel;
    Label04: TLabel;
    Label05: TLabel;
    Label06: TLabel;
    Label08: TLabel;
    DelayFB: TTobyGauge;
    Fine: TTobyGauge;
    Cutoff: TTobyGauge;
    DelayLen: TTobyGauge;
    Resonance: TTobyGauge;
    OutVol: TTobyGauge;
    Label1: TLabel;
    DryWet: TTobyGauge;
    Label2: TLabel;
    Tune: TTobyGauge;
    Label3: TLabel;
    TextSwitch: TImage;
    rand1: TImage;
    rand2: TImage;
    root: TLabel;
    MMode: TLabel;
    updater: TTimer;
    FType: TLabel;
    doabout: TImage;
    AboutScreen: TPanel;
    Label6: TLabel;
    Label7: TLabel;
    Label8: TLabel;
    Label9: TLabel;
    Label10: TLabel;
    Label11: TLabel;
    Label12: TLabel;
    Label13: TLabel;
    procedure DelayFBMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure DelayFBMouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure FineMouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure FineMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure TuneMouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure TuneMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure OutVolMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure OutVolMouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure ResonanceMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure ResonanceMouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure CutoffMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure CutoffMouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure DelayLenMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure DelayLenMouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure DryWetMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure DryWetMouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure TextSwitchClick(Sender: TObject);
    procedure rand1Click(Sender: TObject);
    procedure rootMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure rootMouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure MModeMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure updaterTimer(Sender: TObject);
    procedure FTypeMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure FormCreate(Sender: TObject);
    procedure doaboutClick(Sender: TObject);
  private
    FEffect:AudioEffectX;
  public
    my,mo:longint; // these are used for mouse movement translations

    // the parameter set, corresponding to the set in uPlugin.pas
    fDryWet,fTune,fDelayLen,fDelayFB,fRoot,fMMode,
    fCutoff,fResonance,fFType,fOutVol,fFine:single;

    property Effect: AudioEffectX read FEffect write FEffect;
  end;

implementation
{$R *.DFM}

uses uPlugin;

// This is the updating routine that makes sure the display is always
// showing the right values. I used a pretty "lazy" standard timer
// (not very precise), but precision is not necessary for the screen update.
// You can adjust how often the screen is updated by changing the
// updater.interval value
procedure TPluginEditorWindow.updaterTimer(Sender: TObject);
var js:string;
begin
 js:=na[round(froot*100)];
 if root.caption<>js then root.caption:=js;
 if (fMMode>0.5) and (MMode.caption[1]='B') then MMode.caption:='Hold' else
 if (fMMode<=0.5) and (MMode.caption[1]='H') then MMode.caption:='Back';
 if (fFtype>0.5) and (FType.caption[1]='L') then FType.caption:='HP' else
 if (fFtype<=0.5) and (FType.caption[1]='H') then FType.caption:='LP';
 Fine.progress:=round(fFine*100);
 Tune.progress:=round(fTune*48)-24;
 DelayLen.progress:=round(fDelayLen*100);
 DelayFB.progress:=round(fDelayFB*100);
 Cutoff.progress:=round(fCutoff*100);
 Resonance.progress:=round(fResonance*100);
 OutVol.progress:=round(fOutVol*126);
 DryWet.progress:=round(fDryWet*126);
end;

procedure TPluginEditorWindow.DelayFBMouseDown(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin
 if button=mbleft then // if left button is pressed
 begin
  // get current position/value of gauge
  mo:=DelayFB.progress;
  // get current mouse y-position:
  my:=y;
  // set editor's own parameter:
  fDelayFB:=DelayFB.progress/100;
  // tell effect in uPlugin.pas that parameter has changed:
  Effect.setParameterAutomated(kDelayFB,fDelayFB);
 end;
end;

procedure TPluginEditorWindow.DelayFBMouseMove(Sender: TObject;
  Shift: TShiftState; X, Y: Integer);
begin
 if shift=[ssleft] then // if left button is held down
 begin
  DelayFB.progress:=mo+(y-my); // calculate new value
  fDelayFB:=DelayFB.progress/100; // set it internally
  Effect.setParameterAutomated(kDelayFB,fDelayFB); // tell effect new value
 end;
end;

procedure TPluginEditorWindow.FineMouseDown(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin
 if button=mbleft then
 begin
  mo:=Fine.progress;
  my:=y;
  Fine.progress:=y;
  fFine:=Fine.progress/100;
  Effect.setParameterAutomated(kFine,fFine);
 end;
end;

procedure TPluginEditorWindow.FineMouseMove(Sender: TObject;
  Shift: TShiftState; X, Y: Integer);
begin
 if shift=[ssleft] then
 begin
  Fine.progress:=mo+(y-my);
  fFine:=Fine.progress/100;
  Effect.setParameterAutomated(kFine,fFine);
 end;
end;

procedure TPluginEditorWindow.TuneMouseDown(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin
 if button=mbleft then
 begin
  my:=y;
  Tune.progress:=round(((y-50)/100)*48);
  fTune:=(Tune.progress+24)/48;
  Effect.setParameterAutomated(kTune,fTune);
 end;
end;

procedure TPluginEditorWindow.TuneMouseMove(Sender: TObject;
  Shift: TShiftState; X, Y: Integer);
begin
 if shift=[ssleft] then
 begin
  Tune.progress:=round(((y-50)/100)*48);
  fTune:=(Tune.progress+24)/48;
  Effect.setParameterAutomated(kTune,fTune);
 end;
end;

procedure TPluginEditorWindow.OutVolMouseDown(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin
 if button=mbleft then
 begin
  mo:=OutVol.progress;
  my:=y;
  OutVol.progress:=126-y;
  fOutVol:=OutVol.progress/126;
  Effect.setParameterAutomated(kOutVol,fOutVol);
 end;
end;

procedure TPluginEditorWindow.OutVolMouseMove(Sender: TObject;
  Shift: TShiftState; X, Y: Integer);
begin
 if shift=[ssleft] then
 begin
  OutVol.progress:=mo-(y-my);
  fOutVol:=OutVol.progress/126;
  Effect.setParameterAutomated(kOutVol,fOutVol);
 end;
end;

procedure TPluginEditorWindow.ResonanceMouseDown(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin
 if button=mbleft then
 begin
  mo:=Resonance.progress;
  my:=y;
  fResonance:=Resonance.progress/100;
  Effect.setParameterAutomated(kResonance,fResonance);
 end;
end;

procedure TPluginEditorWindow.ResonanceMouseMove(Sender: TObject;
  Shift: TShiftState; X, Y: Integer);
begin
 if shift=[ssleft] then
 begin
  Resonance.progress:=mo+(y-my);
  fResonance:=Resonance.progress/100;
  Effect.setParameterAutomated(kResonance,fResonance);
 end;
end;

procedure TPluginEditorWindow.CutoffMouseDown(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin
 if button=mbleft then
 begin
  mo:=Cutoff.progress;
  my:=y;
  fCutoff:=Cutoff.progress/100;
  Effect.setParameterAutomated(kCutoff,fCutoff);
 end;
end;

procedure TPluginEditorWindow.CutoffMouseMove(Sender: TObject;
  Shift: TShiftState; X, Y: Integer);
begin
 if shift=[ssleft] then
 begin
  Cutoff.progress:=mo+(y-my);
  fCutoff:=Cutoff.progress/100;
  Effect.setParameterAutomated(kCutoff,fCutoff);
 end;
end;

procedure TPluginEditorWindow.DelayLenMouseDown(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin
 if button=mbleft then
 begin
  mo:=DelayLen.progress;
  my:=y;
  fDelayLen:=DelayLen.progress/100;
  Effect.setParameterAutomated(kDelayLen,fDelayLen);
 end;
end;

procedure TPluginEditorWindow.DelayLenMouseMove(Sender: TObject;
  Shift: TShiftState; X, Y: Integer);
begin
 if shift=[ssleft] then
 begin
  DelayLen.progress:=mo+(y-my);
  fDelayLen:=DelayLen.progress/100;
  Effect.setParameterAutomated(kDelayLen,fDelayLen);
 end;
end;

procedure TPluginEditorWindow.DryWetMouseDown(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin
 if button=mbleft then
 begin
  mo:=DryWet.progress;
  my:=y;
  DryWet.progress:=126-y;
  fDryWet:=DryWet.progress/126;
  Effect.setParameterAutomated(kDryWet,fDryWet);
 end;
end;

procedure TPluginEditorWindow.DryWetMouseMove(Sender: TObject;
  Shift: TShiftState; X, Y: Integer);
begin
 if shift=[ssleft] then
 begin
  DryWet.progress:=mo-(y-my);
  fDryWet:=DryWet.progress/126;
  Effect.setParameterAutomated(kDryWet,fDryWet);
 end;
end;

procedure TPluginEditorWindow.TextSwitchClick(Sender: TObject);
begin
 DelayFB.ShowText:=not DelayFB.ShowText;
 DelayLen.ShowText:=not DelayLen.ShowText;
 Cutoff.ShowText:=not Cutoff.ShowText;
 Resonance.ShowText:=not Resonance.ShowText;
 OutVol.ShowText:=not OutVol.ShowText;
 DryWet.ShowText:=not DryWet.ShowText;
 Tune.ShowText:=not Tune.ShowText;
 Fine.ShowText:=not Fine.ShowText;
end;

procedure TPluginEditorWindow.rand1Click(Sender: TObject);
var i:integer;
    s:single;
begin
 // randomize preset (bear: all, note: only pitch)
 for i:=0 to (sender as TImage).tag do
 begin
  s:=0.9*random;
  effect.setparameter(kTune+i,s);
 end;
end;

procedure TPluginEditorWindow.rootMouseDown(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin
 if shift=[ssleft] then
 begin
  mo:=round(fRoot*100);
  my:=y;
 end;
end;

procedure TPluginEditorWindow.rootMouseMove(Sender: TObject;
  Shift: TShiftState; X, Y: Integer);
var i:integer;
begin
 if shift=[ssleft] then
 begin
  i:=mo+(my-y div 2);
  if i<0 then i:=0;
  if i>96 then i:=96;
  root.caption:=na[i];
  fRoot:=i/100;
  effect.setparameterautomated(kRoot,fRoot);
 end;
end;

procedure TPluginEditorWindow.MModeMouseDown(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin
 // set the MIDI mode:
 fMMode:=1-fMMode;
 if fMMode>0.5 then MMode.caption:='Hold' else
 MMode.caption:='Back';
 effect.setparameter(kMMode,fMMode);
end;

procedure TPluginEditorWindow.FTypeMouseDown(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
begin
 fFtype:=1-fFtype;
 if fFtype<0.5 then FType.caption:='LP' else FType.caption:='HP';
 effect.setparameterautomated(kftype,fftype);
end;

procedure TPluginEditorWindow.FormCreate(Sender: TObject);
begin
 AboutScreen.Left:=120;
 AboutScreen.Top:=80;
end;

procedure TPluginEditorWindow.doaboutClick(Sender: TObject);
begin
 AboutScreen.visible:=not AboutScreen.visible;
end;

end.

