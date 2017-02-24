#include "AnechoicRoomSim.hpp"
#include "ASupaEditor.hpp"
#include <math.h>

void vstint2string(const VstInt32 number, char* str)
{
    std::stringstream ss;
    ss << number;
    strcpy(str, ss.str().c_str());
}

AnechoicRoomSim::AnechoicRoomSim(audioMasterCallback audioMaster)
    : AudioEffectX(audioMaster, kNumPrograms, kNumParams)
{
    setNumInputs(kNumInputChannels);   // stereo in
    setNumOutputs(kNumOutputChannels); // stereo out
    setUniqueID('AneC');               
    canProcessReplacing();             // supports both accumulating and replacing output

    setParameter(kSize, 0.31415f);

    editor = new ASupaEditor(this);
}

AnechoicRoomSim::~AnechoicRoomSim()
{
    if (editor)
    {
        delete editor;
    }

    editor = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////
// the parameters of the plugin
///////////////////////////////////////////////////////////////////////////////////////////

void AnechoicRoomSim::setParameter(VstInt32 index, float value)
{
    // save the parameter for the host...
    savedParameters[index] = value;

    switch (index)
    {
    case kSize: break;
    default: break;
    }

    if (editor)
    {
        (static_cast<ASupaEditor*>(editor))->setParameter(index, value);
    }
}

float AnechoicRoomSim::getParameter(VstInt32 index)
{
    if (index >= 0 && index < kNumParams)
    {
        return savedParameters[index];
    }
    else
    {
        return 0.f;
    }
}

void AnechoicRoomSim::getParameterName(VstInt32 index, char* label)
{
    switch (index)
    {
    case kSize: strcpy(label, "Room Size"); break;
    default:    strcpy(label, "");          break;
    }
}

void AnechoicRoomSim::getParameterDisplay(VstInt32 index, char* text)
{
    switch (index)
    {
    case kSize: vstint2string(static_cast<VstInt32>(savedParameters[index] * 10000.f), text); break;
    default: strcpy(text, ""); break;
    }
}

void AnechoicRoomSim::getParameterLabel(VstInt32 index, char* label)
{
    switch (index)
    {
    case kSize: strcpy(label, " m"); break;
    default:    strcpy(label, "");   break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
// programs (i.e. presets)
///////////////////////////////////////////////////////////////////////////////////////////

//void AnechoicRoomSim::setProgramName(char* name)
//{
//    // nothing to do here
//}

void AnechoicRoomSim::getProgramName(char* name)
{
    strcpy(name, "");
}

//void AnechoicRoomSim::setProgram(VstInt32 index)
//{
//    // nothing to do here
//};

///////////////////////////////////////////////////////////////////////////////////////////
// the actual processing
///////////////////////////////////////////////////////////////////////////////////////////

void AnechoicRoomSim::processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames)
{
    //                            .. .  ....,,....   . ..
    //                     MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMN+
    //                  .MMM,            ...$MMMMMMMMDI..  . . ....IMMMMMMMMMMM=
    //                 MMM .         ..$M,.               ..  .+ODMMMM8~ . .:MNMMM7
    //               .MMN         ..M?.    8MMD+.. .=DMM:.               . ..: ..MMMN.
    //               MM:..      .N:...8M:. ...          .   .. ......    .  .  .. . MMM.
    //              MM+      . 7. .8D..NMM=         ..          .....:+?~.  .        .MMM .
    //            .MM$     .I.  .M. .M              ,.          .D..        :N~..      IMM.
    //            MMM      .   M:.MO                 Z            M.         . M.       MM.
    //           .MM.        .$..M       .. ...      O            ..          . .       MM.
    //          .MM..             . .MMMMMMMMMMMMM=.              .         .           MMM.
    //        .MMM.               :MMMMMMMMMMM  ~MMMM.            .. .NMMMMMMM.         .MMM.
    //       8MMMMMMMD~    .. D8.NMM.ZMMMMMMM ..   ~MMM.          OMMMMMM  ,MMMM ...     .~MMD..
    //     .MMMMZ.......   .+M8. MMMMMMMMMMMMMMMMI. =MM=     IMM.MMMMMMMMMMMMMM.. ... ..$8. NMM,
    //    IMM.N.  ..NMMMMMM~....        .OM.    8MMMMMO       8MMMMM?..                I..=7..MM.
    //   NMM M.. ~MMM~   ,MMMMN       .MMMM        :~           .M=              ........=M. ,.M,
    //  +MM +   MMM .   M  .,MMMMMMMMMMM8.                       M=        .  ..MMMMMMMNM I  M,M:
    //  MM  ? .7MM.    MM7   . ....,:,...                        MN       ZMMMMMM? .. .7M.,  O MI
    // .MM. I .MM.    ,MMMMM ..                ,...:? ..         =MMM, .  ..=DMMM .M:.     . N.M8
    // .MM  ?. MM. IMMMM?.,MMMMI..     ....... M8MMMMM~.            MMM$          ,M+     N. MIM$
    //  MM  ...7MM:MMI$MM   ..MMMMM...  ..... ..MM.. ...  .         :MMMM.        ?MMM. 8?. .NMM
    //  MMM  ,..MM     MMM     ..MMMMMM~.       ZMM..MMMMMM~       ,MMMM~ .M.     MMMM.   $N MMM
    //  .MMD.D...M     MMMM~.    MM..+MMMMMM7 .. NMM ,    .  .8 . MMM       ....DMMMMM, ,.. NMM.
    //    MMN..8M..    .~MMMMMD..MMD.    ..MMMMMMM~. .        MMMMM,         .MMMMMMMMN    ZMM
    //     7MMM. ..     . MM.MMMMMMMO.      . ?MMMMMMMMMN=    .....      .:MMMMMM= MMMM    MM
    //      .MMM..        ,MM...+MMMMMM~.     MM    ..+MMMMMMMMMMMMMMMMMMMMM .  MM NMMM. .=M,
    //       .OMM..        .MN.  DMMMMMMMMMM+.MM          MO.  ...MM...   MM7  .MM.MMMM.  MM.
    //        ..MM.         .MM...MM..:MMMMMMMMM~...      MO.     MM      .MM. OMMMMMMM.  MM.
    //          +MM           MMMMN..  .  7MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM. .MM.
    //           MMM.          ,MMM        . MNNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMD   MM.
    //           .MM8            8MM8.     .,MM    .7MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM   .MM.
    //            .MMN.            IMMN .  .MM          MM.NMMMMMMMMMMMMMMMMMMMMMMM.MM.  .DM.
    //             .~MM              =MMN=.MM           MM      .MM?...IMM..:MM MM MMD.   +M=
    //                MMM  ,:         ..MMMMM.         .MM      .MI.   MM.  MM. OMMMM.   ..M8
    //                 =MMO..?N..  MM.    .NMMMMMI . .  MM     .DM..  .MM..MMDMMMMM,       MM
    //                   MMMM. .$M....N?..    .=MMMMMMMMMMMM8MMMMMM8MMMMMMMMMMM$           MM
    //                    .=MMMM.. :M:.  =M: .   ..  .. ,?ZDNMMMMMMMD$+..... .             MM.
    //                      ..,MMM7.  .DM.. .OM:         .                       ~8   ..   MM.
    //                           MMMMM  .. IM..  DM~.... .                     =~      .   MM.
    //                              DMMMMM.   ..=M?........,:?IZNMMMMNNDDNMM+..    . MZ    MM.
    //                                . ?MMMM,.     ...~MM+  .                    .M..     NM
    //                                   ..=MMMM         . . .... .:I8DMMMMMMMMMM..        MM
    //                                      ..+MMMM                                      .MM7
    //                                          .,MMMMMMMN...                          ..MM..
    //                                             .. .IMMMMMMM?                      MMMM
    //                                                    ...OMMMMMMM7 .         =MMMMM$...
    //                                                            .ZMMMMMMMMMMMMMMM?.
    //
    //
    //
    //                                      MMM:       MMM.                             MMMMM8.
    //                                      MMM:       MMM.                           ,MMMMMMMM
    //          MMMDMMM+  MMM:MMM..MMMMMM   MMM:MMMM.  MMM.  ?MMMMMO.  MMM8MMM8 MMMM .,?N:.7MMM.
    //          MMMMNMMMZ MMMMMM IMMMINMMM..MMMMMMMMN  MMM..MMMI ,MMM  MMMMMMMMMMMMMM     MMMM
    //          MMM  .MMM MMMM   MMM.  MMMM.MMM:  MMM  MMM. MMMMMMMMM .MMM  MMM:  MMM   .MMM
    //          MMM .IMMM.MMMM   MMM:  MMMO.MMM8..MMM. MMM. MMM.  ..  .MMM  MMM   MMM   .....
    //          MMMMMMMM. MMMM   .MMMMMMMM. MMMMMMMMM. MMM..+MMMMMMMM  MMM  MMM   MMM   .MMM.
    //          MMM.$M?   ?==?.   ..$MN~..  ===,:MN    ===   .,NMM:    ===  ===   ===   .===.
    //          MMM
    //         .III

    float* in1 = inputs[0];
    float* in2 = inputs[1];
    float* out1 = outputs[0];
    float* out2 = outputs[1];

    for (long i = 0; i < sampleFrames; ++i)
    {
        out1[i] = in1[i];
        out2[i] = in2[i];
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
// some more advanced VST features you should really use!
///////////////////////////////////////////////////////////////////////////////////////////

bool AnechoicRoomSim::getEffectName(char* name)
{
    strcpy(name, "Anechoic Room Simulator");
    return true;
}

bool AnechoicRoomSim::getVendorString(char* text)
{
    strcpy(text, "Bram @ Smartelectronix");
    return true;
}

bool AnechoicRoomSim::getProductString(char* text)
{
    strcpy(text, "Anechoic Room Simulator");
    return true;
}

VstInt32 AnechoicRoomSim::canDo(char* text)
{
    if (!strcmp(text, "receiveVstEvents"))    return 0;
    if (!strcmp(text, "receiveVstMidiEvent")) return 0;
    if (!strcmp(text, "receiveVstTimeInfo"))  return 0;
    if (!strcmp(text, "plugAsChannelInsert")) return 1;
    if (!strcmp(text, "plugAsSend"))          return 1;
    if (!strcmp(text, "mixDryWet"))           return 1;
    if (!strcmp(text, "1in2out"))             return 1;
    if (!strcmp(text, "2in2out"))             return 1;

    return -1;	// explicitly can't do; 0 => don't know
}

///////////////////////////////////////////////////////////////////////////////////////////
// some features you might find usefull
///////////////////////////////////////////////////////////////////////////////////////////

//void AnechoicRoomSim::suspend()
//{
//    // nothing
//}

//void AnechoicRoomSim::resume()
//{
//    // nothing
//}

//void AnechoicRoomSim::setSampleRate(float sampleRate)
//{
//    // allways call this
//    AudioEffect::setSampleRate(sampleRate);
//}

//void AnechoicRoomSim::setBlockSize(VstInt32 blockSize)
//{
//    // allways call this
//    AudioEffect::setBlockSize(blockSize);
//}
