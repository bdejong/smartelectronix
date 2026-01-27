#include "PluginProcessor.h"
#include "PluginEditor.h"

AnechoicRoomSimProcessor::AnechoicRoomSimProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

AnechoicRoomSimProcessor::~AnechoicRoomSimProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout AnechoicRoomSimProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Room Size parameter - displays as meters (but does nothing, it's a joke!)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(SIZE_ID, 1),
        "Room Size",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f),
        0.31415f,  // Default value (pi/10, a joke value)
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) {
            return juce::String(static_cast<int>(value * 10000.0f)) + " m";
        },
        nullptr));

    return { params.begin(), params.end() };
}

const juce::String AnechoicRoomSimProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AnechoicRoomSimProcessor::acceptsMidi() const
{
    return false;
}

bool AnechoicRoomSimProcessor::producesMidi() const
{
    return false;
}

bool AnechoicRoomSimProcessor::isMidiEffect() const
{
    return false;
}

double AnechoicRoomSimProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AnechoicRoomSimProcessor::getNumPrograms()
{
    return 1;
}

int AnechoicRoomSimProcessor::getCurrentProgram()
{
    return 0;
}

void AnechoicRoomSimProcessor::setCurrentProgram(int /*index*/)
{
}

const juce::String AnechoicRoomSimProcessor::getProgramName(int /*index*/)
{
    return {};
}

void AnechoicRoomSimProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/)
{
}

void AnechoicRoomSimProcessor::prepareToPlay(double /*sampleRate*/, int /*samplesPerBlock*/)
{
}

void AnechoicRoomSimProcessor::releaseResources()
{
}

bool AnechoicRoomSimProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void AnechoicRoomSimProcessor::processBlock(juce::AudioBuffer<float>& /*buffer*/,
                                             juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

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
    //
    // An anechoic room has NO reflections, so we just pass audio through unchanged!
    // (This is an April Fools plugin)

    // The "Room Size" parameter does absolutely nothing :)
    (void)apvts.getRawParameterValue(SIZE_ID)->load();

    // Just pass audio through - that's what an anechoic room does!
    // (No processing needed - input already equals output in JUCE's processBlock)
}

bool AnechoicRoomSimProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* AnechoicRoomSimProcessor::createEditor()
{
    return new AnechoicRoomSimEditor(*this);
}

void AnechoicRoomSimProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AnechoicRoomSimProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
    {
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AnechoicRoomSimProcessor();
}
