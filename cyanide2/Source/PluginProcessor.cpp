#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "DSP/Presets.h"

#include <array>
#include <cstring>
#include <cmath>
#include <cstdlib>

//==============================================================================
// Helper functions (from original CyanideEffect.cpp)

static void amp(float *input, float *output, float *noise, float amount, long nSamp)
{
    long i;
    for(i=0;i<nSamp/4;i++)
    {
        output[0] = input[0]*amount + noise[0];
        output[1] = input[1]*amount + noise[1];
        output[2] = input[2]*amount + noise[2];
        output[3] = input[3]*amount + noise[3];
        output += 4;
        input += 4;
        noise += 4;
    }
    for(i=0;i<nSamp % 4;i++)
        output[i] = (input[i] + noise[i])*amount;
}

static void mix(float *output, float *in1, float *in2, float a1, float a2, long nSamp)
{
    long i;
    for(i=0;i < nSamp/4;i++)
    {
        output[0] = in1[0]*a1 + in2[0]*a2;
        output[1] = in1[1]*a1 + in2[1]*a2;
        output[2] = in1[2]*a1 + in2[2]*a2;
        output[3] = in1[3]*a1 + in2[3]*a2;
        output += 4;
        in1 += 4;
        in2 += 4;
    }
    for(i=0;i < nSamp % 4;i++)
        output[i] = in1[i]*a1 + in2[i]*a2;
}

//==============================================================================

Cyanide2Processor::Cyanide2Processor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    // Cache parameter pointers
    preGainParam    = apvts.getRawParameterValue(PARAM_PREGAIN);
    preFilterParam  = apvts.getRawParameterValue(PARAM_PREFILTER);
    postGainParam   = apvts.getRawParameterValue(PARAM_POSTGAIN);
    postFilterParam = apvts.getRawParameterValue(PARAM_POSTFILTER);
    preTypeParam    = apvts.getRawParameterValue(PARAM_PRETYPE);
    postTypeParam   = apvts.getRawParameterValue(PARAM_POSTTYPE);
    dryWetParam     = apvts.getRawParameterValue(PARAM_DRYWET);
    overSampleParam = apvts.getRawParameterValue(PARAM_OVERSAMPLE);

    // Init DSP
    F1L.setrate(44100.0f);
    F1R.setrate(44100.0f);
    F2L.setrate(44100.0f);
    F2R.setrate(44100.0f);

    shaper.updatedata();

    // Anti-denormalization noise
    for (size_t i = 0; i < noise.size(); i++)
        noise[i] = (float)(((double)rand()) / ((double)RAND_MAX * 1048576.0));

    // Load factory presets
    loadFactoryPresets();
    loadProgramToState(0);
}

Cyanide2Processor::~Cyanide2Processor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout Cyanide2Processor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto makeParam = [](const char* id, const char* name, float def,
                        std::function<juce::String(float, int)> toString = nullptr)
    {
        return std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(id, 1), name,
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
            def, juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            toString, nullptr);
    };

    params.push_back(makeParam(PARAM_PREGAIN, "PreGain", 0.76f,
        [](float v, int) {
            float db = 20.0f * log10f(gainMapScaled(v));
            return juce::String(db, 1) + " dB";
        }));

    params.push_back(makeParam(PARAM_PREFILTER, "PreFilter", 1.0f,
        [](float v, int) {
            CButterXOver tmp;
            return juce::String((int)tmp.getFreq((long)(v * (nPre - 1.f)))) + " Hz";
        }));

    params.push_back(makeParam(PARAM_POSTGAIN, "PostGain", 0.76f,
        [](float v, int) {
            float db = 20.0f * log10f(gainMapScaled(v));
            return juce::String(db, 1) + " dB";
        }));

    params.push_back(makeParam(PARAM_POSTFILTER, "PostFilter", 1.0f,
        [](float v, int) {
            CButterXOver tmp;
            return juce::String((int)tmp.getFreq((long)(v * (nPre - 1.f)))) + " Hz";
        }));

    params.push_back(makeParam(PARAM_PRETYPE, "PreType", 0.0f,
        [](float v, int) {
            int lp = 100 - (int)(v * 100.f);
            int hp = (int)(v * 100.f);
            return "LP " + juce::String(lp) + "% | HP " + juce::String(hp) + "%";
        }));

    params.push_back(makeParam(PARAM_POSTTYPE, "PostType", 0.0f,
        [](float v, int) {
            int lp = 100 - (int)(v * 100.f);
            int hp = (int)(v * 100.f);
            return "LP " + juce::String(lp) + "% | HP " + juce::String(hp) + "%";
        }));

    params.push_back(makeParam(PARAM_DRYWET, "DryWet", 1.0f,
        [](float v, int) {
            return juce::String((int)(v * 100)) + "% wet";
        }));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID(PARAM_OVERSAMPLE, 1), "OverSample", false));

    return { params.begin(), params.end() };
}

//==============================================================================

void Cyanide2Processor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    float sr = (float)sampleRate;
    F1L.setrate(sr);
    F1R.setrate(sr);
    F2L.setrate(sr);
    F2R.setrate(sr);

    F1L.suspend();
    F1R.suspend();
    F2L.suspend();
    F2R.suspend();

    for (int i = 0; i < 8; i++)
    {
        P1[i].suspend();
        P2[i].suspend();
    }
}

void Cyanide2Processor::releaseResources()
{
}

void Cyanide2Processor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Handle MIDI CC
    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();
        if (msg.isController())
        {
            int cc = msg.getControllerNumber();
            float value = (float)msg.getControllerValue() / 127.0f;
            switch (cc)
            {
                case 73: apvts.getParameter(PARAM_PREGAIN)->setValueNotifyingHost(value); break;
                case 74: apvts.getParameter(PARAM_PRETYPE)->setValueNotifyingHost(value); break;
                case 75: apvts.getParameter(PARAM_PREFILTER)->setValueNotifyingHost(value); break;
                case 76: apvts.getParameter(PARAM_POSTGAIN)->setValueNotifyingHost(value); break;
                case 77: apvts.getParameter(PARAM_POSTTYPE)->setValueNotifyingHost(value); break;
                case 78: apvts.getParameter(PARAM_POSTFILTER)->setValueNotifyingHost(value); break;
                case 79: apvts.getParameter(PARAM_DRYWET)->setValueNotifyingHost(value); break;
            }
        }
    }

    int totalSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();
    if (numChannels < 2 || totalSamples == 0) return;

    float* inL = buffer.getWritePointer(0);
    float* inR = buffer.getWritePointer(1);

    // Read parameters
    float fPreGain    = preGainParam->load();
    float fPreFilter  = preFilterParam->load();
    float fPostGain   = postGainParam->load();
    float fPostFilter = postFilterParam->load();
    float fPreType    = preTypeParam->load();
    float fPostType   = postTypeParam->load();
    float fDryWet     = dryWetParam->load();
    bool oversample   = overSampleParam->load() > 0.5f;

    long newf1 = (long)(fPreFilter * (nPre - 1.f));
    long newf2 = (long)(fPostFilter * (nPre - 1.f));
    float PreGain  = gainMapScaled(fPreGain);
    float PostGain = gainMapScaled(fPostGain);
    float m1 = sinf(fDryWet * 3.141592f * 0.5f);
    float m2 = cosf(fDryWet * 3.141592f * 0.5f);

    if (previousOversample != oversample)
    {
        for (int i = 0; i < 8; i++)
        {
            P1[i].suspend();
            P2[i].suspend();
        }
        previousOversample = oversample;
    }

    F1L.settype(fPreType);
    F1R.settype(fPreType);
    F2L.settype(fPostType);
    F2R.settype(fPostType);

    // Process in FRAME_SIZE chunks
    int pos = 0;
    while (pos < totalSamples)
    {
        int chunk = juce::jmin(totalSamples - pos, FRAME_SIZE);

        // Left channel
        amp(&inL[pos], buffer1.data(), noise.data(), PreGain, chunk);
        F1L.processchange(buffer1.data(), buffer1.data(), newf1, chunk);

        if (!oversample)
            shaper.process(buffer1.data(), buffer1.data(), chunk);
        else
        {
            P1[0].Upsample(buffer1.data(), buffer2.data(), chunk);
            P1[1].Upsample(buffer2.data(), buffer1.data(), chunk * 2);
            P1[2].Upsample(buffer1.data(), buffer2.data(), chunk * 4);
            P1[3].Upsample(buffer2.data(), buffer1.data(), chunk * 8);
            shaper.process(buffer1.data(), buffer1.data(), chunk * 16);
            P1[4].Downsample(buffer1.data(), buffer2.data(), chunk * 8);
            P1[5].Downsample(buffer2.data(), buffer1.data(), chunk * 4);
            P1[6].Downsample(buffer1.data(), buffer2.data(), chunk * 2);
            P1[7].Downsample(buffer2.data(), buffer1.data(), chunk);
        }

        F2L.processchange(buffer1.data(), buffer1.data(), newf2, chunk);
        mix(&inL[pos], buffer1.data(), &inL[pos], m1 * PostGain, m2, chunk);

        // Right channel
        amp(&inR[pos], buffer1.data(), noise.data(), PreGain, chunk);
        F1R.processchange(buffer1.data(), buffer1.data(), newf1, chunk);

        if (!oversample)
            shaper.process(buffer1.data(), buffer1.data(), chunk);
        else
        {
            P2[0].Upsample(buffer1.data(), buffer2.data(), chunk);
            P2[1].Upsample(buffer2.data(), buffer1.data(), chunk * 2);
            P2[2].Upsample(buffer1.data(), buffer2.data(), chunk * 4);
            P2[3].Upsample(buffer2.data(), buffer1.data(), chunk * 8);
            shaper.process(buffer1.data(), buffer1.data(), chunk * 16);
            P2[4].Downsample(buffer1.data(), buffer2.data(), chunk * 8);
            P2[5].Downsample(buffer2.data(), buffer1.data(), chunk * 4);
            P2[6].Downsample(buffer1.data(), buffer2.data(), chunk * 2);
            P2[7].Downsample(buffer2.data(), buffer1.data(), chunk);
        }

        F2R.processchange(buffer1.data(), buffer1.data(), newf2, chunk);
        mix(&inR[pos], buffer1.data(), &inR[pos], m1 * PostGain, m2, chunk);

        pos += chunk;
    }
}

//==============================================================================
// Programs / Presets

void Cyanide2Processor::loadFactoryPresets()
{
    // Parse the binary preset data from Presets.h
    // Each program = 1072 bytes: 24 bytes kookie + 1024 bytes save data + 24 bytes name
    const unsigned char* data = preset;

    for (int prog = 0; prog < 16; prog++)
    {
        const unsigned char* p = data + prog * 1072;

        // Skip kookie (24 bytes)
        const unsigned char* saveData = p + 24;

        // Read 8 float parameters (indices 1..8 in original = preGain..overSample)
        int ptr = 0;
        for (int i = 0; i < 8; i++)
        {
            float val;
            memcpy(&val, saveData + ptr, sizeof(float));
            programs[prog].params[i] = val;
            ptr += sizeof(float);
        }

        // Read point count (original used 32-bit long)
        int32_t nPoints;
        memcpy(&nPoints, saveData + ptr, sizeof(int32_t));
        ptr += sizeof(int32_t);

        // Read points
        programs[prog].points.resize((size_t)nPoints);
        for (long i = 0; i < nPoints; i++)
        {
            float x, y;
            memcpy(&x, saveData + ptr, sizeof(float));
            ptr += sizeof(float);
            memcpy(&y, saveData + ptr, sizeof(float));
            ptr += sizeof(float);
            programs[prog].points[(size_t)i] = { x, y };
        }

        // Read name (24 bytes at offset 1048)
        const char* namePtr = (const char*)(p + 1048);
        programs[prog].name = juce::String(namePtr, 24).trimEnd();
    }
}

void Cyanide2Processor::saveCurrentToProgram(int index)
{
    if (index < 0 || index >= 16) return;

    programs[index].params[0] = preGainParam->load();
    programs[index].params[1] = preFilterParam->load();
    programs[index].params[2] = postGainParam->load();
    programs[index].params[3] = postFilterParam->load();
    programs[index].params[4] = preTypeParam->load();
    programs[index].params[5] = postTypeParam->load();
    programs[index].params[6] = dryWetParam->load();
    programs[index].params[7] = overSampleParam->load();

    long n = shaper.GetNPoints();
    programs[index].points.resize((size_t)n);
    std::array<SplinePoint, maxn> pts {};
    shaper.GetData(pts.data());
    for (long i = 0; i < n; i++)
        programs[index].points[(size_t)i] = pts[i];
}

void Cyanide2Processor::loadProgramToState(int index)
{
    if (index < 0 || index >= 16) return;

    auto& prog = programs[index];

    apvts.getParameter(PARAM_PREGAIN)->setValueNotifyingHost(prog.params[0]);
    apvts.getParameter(PARAM_PREFILTER)->setValueNotifyingHost(prog.params[1]);
    apvts.getParameter(PARAM_POSTGAIN)->setValueNotifyingHost(prog.params[2]);
    apvts.getParameter(PARAM_POSTFILTER)->setValueNotifyingHost(prog.params[3]);
    apvts.getParameter(PARAM_PRETYPE)->setValueNotifyingHost(prog.params[4]);
    apvts.getParameter(PARAM_POSTTYPE)->setValueNotifyingHost(prog.params[5]);
    apvts.getParameter(PARAM_DRYWET)->setValueNotifyingHost(prog.params[6]);

    // OverSample is a bool param: value > 0.5 = true
    apvts.getParameter(PARAM_OVERSAMPLE)->setValueNotifyingHost(prog.params[7]);

    if (!prog.points.empty())
    {
        shaper.SetData(prog.points.data(), (long)prog.points.size());
        shaper.updatedata();
    }
}

void Cyanide2Processor::setCurrentProgram(int index)
{
    if (index < 0 || index >= 16) return;
    saveCurrentToProgram(currentProgram);
    currentProgram = index;
    loadProgramToState(currentProgram);
}

const juce::String Cyanide2Processor::getProgramName(int index)
{
    if (index >= 0 && index < 16)
        return programs[index].name;
    return {};
}

void Cyanide2Processor::changeProgramName(int index, const juce::String& newName)
{
    if (index >= 0 && index < 16)
        programs[index].name = newName;
}

//==============================================================================
// State save/load

void Cyanide2Processor::getStateInformation(juce::MemoryBlock& destData)
{
    // Magic string
    const char magic[] = "Cyanide2-JUCE-v1";
    destData.append(magic, 16);

    // APVTS state as size-prefixed XML
    auto state = apvts.copyState();
    auto xml = state.createXml();
    auto xmlStr = xml->toString();
    auto xmlData = xmlStr.toRawUTF8();
    int32_t xmlSize = (int32_t)xmlStr.getNumBytesAsUTF8();
    destData.append(&xmlSize, sizeof(int32_t));
    destData.append(xmlData, (size_t)xmlSize);

    // Shaper points
    int32_t nPoints = (int32_t)shaper.GetNPoints();
    destData.append(&nPoints, sizeof(int32_t));

    std::array<SplinePoint, maxn> pts {};
    shaper.GetData(pts.data());
    for (int32_t i = 0; i < nPoints; i++)
    {
        destData.append(&pts[i].x, sizeof(float));
        destData.append(&pts[i].y, sizeof(float));
    }
}

void Cyanide2Processor::setStateInformation(const void* data, int sizeInBytes)
{
    const char* d = (const char*)data;
    int pos = 0;

    // Check magic
    if (sizeInBytes < 16) return;
    if (memcmp(d, "Cyanide2-JUCE-v1", 16) != 0) return;
    pos += 16;

    // Read XML size
    if (pos + (int)sizeof(int32_t) > sizeInBytes) return;
    int32_t xmlSize;
    memcpy(&xmlSize, d + pos, sizeof(int32_t));
    pos += sizeof(int32_t);

    if (pos + xmlSize > sizeInBytes) return;
    auto xmlStr = juce::String::fromUTF8(d + pos, xmlSize);
    pos += xmlSize;

    auto xml = juce::XmlDocument::parse(xmlStr);
    if (xml)
        apvts.replaceState(juce::ValueTree::fromXml(*xml));

    // Read shaper points
    if (pos + (int)sizeof(int32_t) > sizeInBytes) return;
    int32_t nPoints;
    memcpy(&nPoints, d + pos, sizeof(int32_t));
    pos += sizeof(int32_t);

    if (nPoints > 0 && nPoints <= maxn)
    {
        std::array<SplinePoint, maxn> pts {};
        for (int32_t i = 0; i < nPoints; i++)
        {
            if (pos + 2 * (int)sizeof(float) > sizeInBytes) return;
            memcpy(&pts[i].x, d + pos, sizeof(float));
            pos += sizeof(float);
            memcpy(&pts[i].y, d + pos, sizeof(float));
            pos += sizeof(float);
        }
        shaper.SetData(pts.data(), (long)nPoints);
        shaper.updatedata();
    }
}

//==============================================================================
// Display text

juce::String Cyanide2Processor::getDisplayText(const juce::String& paramId) const
{
    if (auto* param = apvts.getParameter(paramId))
    {
        return param->getCurrentValueAsText();
    }
    return {};
}

//==============================================================================

juce::AudioProcessorEditor* Cyanide2Processor::createEditor()
{
    return new Cyanide2Editor(*this);
}

//==============================================================================

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Cyanide2Processor();
}
