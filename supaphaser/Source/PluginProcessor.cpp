#include "PluginProcessor.h"
#include "PluginEditor.h"

static void phase2(float* y1, float* in_11, float* y2, float* in_12, int n,
                   float inputL, float inputR, float aLeft, float aRight)
{
    float tmpL = y1[0];
    float tmpR = y2[0];

    y1[0] = aLeft * (inputL - y1[0]) + in_11[0];
    y2[0] = aRight * (inputR - y2[0]) + in_12[0];

    in_11[0] = inputL;
    in_12[0] = inputR;

    float tmp2L;
    float tmp2R;
    int ntimes = (n - 1) / 2;
    int i = 1;

    while (ntimes--)
    {
        tmp2L = y1[i];
        tmp2R = y2[i];

        y1[i] = aLeft * (y1[i - 1] - tmp2L) + tmpL;
        y2[i] = aRight * (y2[i - 1] - tmp2R) + tmpR;

        tmpL = y1[i + 1];
        tmpR = y2[i + 1];

        y1[i + 1] = aLeft * (y1[i] - tmpL) + tmp2L;
        y2[i + 1] = aRight * (y2[i] - tmpR) + tmp2R;

        i += 2;
    }

    if ((n - 1) % 2)
    {
        y1[i] = aLeft * (y1[i - 1] - y1[i]) + tmpL;
        y2[i] = aRight * (y2[i - 1] - y2[i]) + tmpR;
    }
}

SupaPhaserProcessor::SupaPhaserProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    attackParam  = apvts.getRawParameterValue(PARAM_ATTACK);
    releaseParam = apvts.getRawParameterValue(PARAM_RELEASE);
    minEnvParam  = apvts.getRawParameterValue(PARAM_MIN_ENV);
    maxEnvParam  = apvts.getRawParameterValue(PARAM_MAX_ENV);
    mixtureParam = apvts.getRawParameterValue(PARAM_MIXTURE);
    freqParam    = apvts.getRawParameterValue(PARAM_FREQ);
    minFreqParam = apvts.getRawParameterValue(PARAM_MIN_FREQ);
    maxFreqParam = apvts.getRawParameterValue(PARAM_MAX_FREQ);
    extendParam  = apvts.getRawParameterValue(PARAM_EXTEND);
    stereoParam  = apvts.getRawParameterValue(PARAM_STEREO);
    stagesParam  = apvts.getRawParameterValue(PARAM_STAGES);
    distortParam = apvts.getRawParameterValue(PARAM_DISTORT);
    feedParam    = apvts.getRawParameterValue(PARAM_FEED);
    dryWetParam  = apvts.getRawParameterValue(PARAM_DRY_WET);
    gainParam    = apvts.getRawParameterValue(PARAM_GAIN);
    invertParam  = apvts.getRawParameterValue(PARAM_INVERT);

    for (int i = 0; i < MaxnStages; i++)
        y2[i] = in_12[i] = y1[i] = in_11[i] = 0.f;

    Osc1.setrate(44100.0f);
    Osc2.setrate(44100.0f);

    initNoise(512);
}

SupaPhaserProcessor::~SupaPhaserProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout SupaPhaserProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto makeParam = [&](const char* id, const char* name, float def)
    {
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(id, 1), name,
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
            def));
    };

    makeParam(PARAM_ATTACK,   "Attack",       0.0f);
    makeParam(PARAM_RELEASE,  "Release",      0.2f);
    makeParam(PARAM_MIN_ENV,  "Min Env",      0.0f);
    makeParam(PARAM_MAX_ENV,  "Max Env",      1.0f);
    makeParam(PARAM_MIXTURE,  "Env-LFO Mix",  0.0f);
    makeParam(PARAM_FREQ,     "Sweep Freq",   0.5f);
    makeParam(PARAM_MIN_FREQ, "Min Depth",    0.1f);
    makeParam(PARAM_MAX_FREQ, "Max Depth",    0.8f);
    makeParam(PARAM_EXTEND,   "Freq Range",   0.0f);
    makeParam(PARAM_STEREO,   "Stereo",       0.5f);
    makeParam(PARAM_STAGES,   "Stages",       0.347826f);
    makeParam(PARAM_DISTORT,  "Distortion",   0.0f);
    makeParam(PARAM_FEED,     "Feedback",     0.4f);
    makeParam(PARAM_DRY_WET,  "Dry-Wet",      0.5f);
    makeParam(PARAM_GAIN,     "Out Gain",     0.76f);
    makeParam(PARAM_INVERT,   "Invert",       0.0f);

    return { params.begin(), params.end() };
}

void SupaPhaserProcessor::initNoise(int bufSize)
{
    Noise.resize(static_cast<size_t>(bufSize));
    for (int i = 0; i < bufSize; i++)
        Noise[static_cast<size_t>(i)] = (float)(((double)rand()) / (RAND_MAX * 1048576.0));
}

void SupaPhaserProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    Osc1.setrate(static_cast<float>(sampleRate));
    Osc2.setrate(static_cast<float>(sampleRate));

    for (int i = 0; i < MaxnStages; i++)
        y2[i] = in_12[i] = y1[i] = in_11[i] = 0.f;

    prevOutL = prevOutR = 0.f;
    ENV1 = ENV2 = 0.f;

    Osc1.reset();
    Osc2.reset();

    initNoise(samplesPerBlock);
}

void SupaPhaserProcessor::releaseResources()
{
}

bool SupaPhaserProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

void SupaPhaserProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    if (buffer.getNumChannels() < 2)
        return;

    int sampleFrames = buffer.getNumSamples();

    // Re-init noise if block size changed
    if (static_cast<int>(Noise.size()) < sampleFrames)
        initNoise(sampleFrames);

    float* out1 = buffer.getWritePointer(0);
    float* out2 = buffer.getWritePointer(1);
    const float* in1 = buffer.getReadPointer(0);
    const float* in2 = buffer.getReadPointer(1);

    float* pNoise = Noise.data();

    // Read parameters
    float SAVE_kAttack   = attackParam->load();
    float SAVE_kRelease  = releaseParam->load();
    float SAVE_kMinEnv   = minEnvParam->load();
    float SAVE_kMaxEnv   = maxEnvParam->load();
    float SAVE_kMixture  = mixtureParam->load();
    float SAVE_kFreq     = freqParam->load();
    float SAVE_kMinFreq  = minFreqParam->load();
    float SAVE_kMaxFreq  = maxFreqParam->load();
    float SAVE_kExtend   = extendParam->load();
    float SAVE_kStereo   = stereoParam->load();
    float SAVE_knStages  = stagesParam->load();
    float SAVE_kDistort  = distortParam->load();
    float SAVE_kFeed     = feedParam->load();
    float SAVE_kDryWet   = dryWetParam->load();
    float SAVE_kGain     = gainParam->load();
    float SAVE_kInvert   = invertParam->load();

    float feed = SAVE_kFeed * 0.99f;
    float outtmp = gainMap(SAVE_kGain) * (float)sqrt(1 - feed * feed);
    float freqtmp1 = (1 - SAVE_kMixture) * (SAVE_kMaxFreq + SAVE_kMinFreq) * 0.5f;
    float freqtmp2 = (SAVE_kMaxFreq - SAVE_kMinFreq) * 0.5f * (1 - SAVE_kMixture);
    float freq = ScaleFreq(SAVE_kFreq, 4, SAVE_kExtend > 0.5f ? 10.f : 2.f);

    Osc1.setfreq(freq);
    Osc2.setfreq(freq);
    Osc2.setstereo(Osc1.getphase(), SAVE_kStereo);

    int nStages = Float2Int(SAVE_knStages, 0, (double)MaxnStages);
    if (nStages < 3) nStages = 3;
    if (nStages > MaxnStages) nStages = MaxnStages;

    float ga, ga2;
    if (SAVE_kAttack < 0.0001f)
        ga = (float)expf(-1.f / (static_cast<float>(getSampleRate()) * 0.0001f));
    else
        ga = (float)expf(-1.f / (static_cast<float>(getSampleRate()) * SAVE_kAttack * 0.4f));
    ga2 = 1 - ga;

    float gr, gr2;
    if (SAVE_kRelease < 0.0001f)
        gr = (float)expf(-1.f / (static_cast<float>(getSampleRate()) * 0.0001f));
    else
        gr = (float)expf(-1.f / (static_cast<float>(getSampleRate()) * SAVE_kRelease));
    gr2 = 1 - gr;

    float envtmp2 = SAVE_kMaxEnv * 2.5f - SAVE_kMinEnv * 1.5f;
    float envtmp1 = SAVE_kMinEnv * 1.5f;

    float distMap = sqrtf(SAVE_kDistort) * 0.95f;
    float dist = distMap;
    float k = 2.f * 0.99f * distMap / (1.f - 0.99f * distMap);
    float k2 = (1.f + k) * (1.f - DIST_FIX * SAVE_kDistort);

    unsigned char x = (unsigned char)(SAVE_kInvert * 3.0);
    bool invertWet = (x & 0x2) == 2;
    bool invertFeed = (x & 0x1) == 1;

    float inputL, inputR;
    float aL, aR, inL, inR;
    float LFO1, LFO2;

    for (int s = 0; s < sampleFrames; s++)
    {
        inL = in1[s] + pNoise[s];
        inR = in2[s] + pNoise[s];

        // LEFT
        LFO1 = Osc1.GetVal();

        float env1in = fabsf(inL);
        if (ENV1 < env1in)
            ENV1 = ga * (ENV1 - env1in) + env1in;
        else
            ENV1 = gr * (ENV1 - env1in) + env1in;

        float envscale = envtmp1 + envtmp2 * ENV1;
        if (envscale > 1.f)
            envscale = 1.f;

        aL = -SAVE_kMixture * envscale - freqtmp1 - (float)LFO1 * freqtmp2;

        inL *= k2 / (1.f + k * env1in);

        inputL = inL + prevOutL * feed;

        // RIGHT
        LFO2 = Osc2.GetVal();

        float env2in = fabsf(inR);
        if (ENV2 < env2in)
            ENV2 = ga * (ENV2 - env2in) + env2in;
        else
            ENV2 = gr * (ENV2 - env2in) + env2in;

        envscale = envtmp1 + envtmp2 * ENV2;
        if (envscale > 1.f)
            envscale = 1.f;

        aR = -SAVE_kMixture * envscale - freqtmp1 - (float)LFO2 * freqtmp2;

        inR *= k2 / (1.f + k * env2in);

        inputR = inR + prevOutR * feed;

        phase2(y1, in_11, y2, in_12, nStages, inputL, inputR, aL, aR);

        prevOutL = y1[nStages - 1];
        prevOutR = y2[nStages - 1];

        if (invertWet)
        {
            out1[s] = (SAVE_kDryWet * (inL + prevOutL) - prevOutL) * outtmp;
            out2[s] = (SAVE_kDryWet * (inR + prevOutR) - prevOutR) * outtmp;
        }
        else
        {
            out1[s] = (SAVE_kDryWet * (inL - prevOutL) + prevOutL) * outtmp;
            out2[s] = (SAVE_kDryWet * (inR - prevOutR) + prevOutR) * outtmp;
        }

        if (invertFeed)
        {
            prevOutL = -prevOutL;
            prevOutR = -prevOutR;
        }
    }
}

juce::String SupaPhaserProcessor::getDisplayText(const juce::String& paramId) const
{
    auto getVal = [&](const char* id) -> float {
        if (auto* p = apvts.getRawParameterValue(id))
            return p->load();
        return 0.f;
    };

    juce::String result;

    if (paramId == PARAM_ATTACK)
    {
        float v = getVal(PARAM_ATTACK);
        int ms = (v < 0.0001f) ? (int)(0.0001f * 1000.f) : (int)(v * 0.4f * 1000.f);
        result = juce::String(ms);
    }
    else if (paramId == PARAM_RELEASE)
    {
        float v = getVal(PARAM_RELEASE);
        int ms = (v < 0.0001f) ? (int)(0.0001f * 1000.f) : (int)(v * 1000.f);
        result = juce::String(ms);
    }
    else if (paramId == PARAM_MIN_ENV)
    {
        result = juce::String((int)(getVal(PARAM_MIN_ENV) * 100.f)) + "%";
    }
    else if (paramId == PARAM_MAX_ENV)
    {
        result = juce::String((int)(getVal(PARAM_MAX_ENV) * 100.f)) + "%";
    }
    else if (paramId == PARAM_MIXTURE)
    {
        result = juce::String((int)(getVal(PARAM_MIXTURE) * 100.f)) + "%";
    }
    else if (paramId == PARAM_FREQ)
    {
        float ext = getVal(PARAM_EXTEND);
        float fv = ScaleFreq(getVal(PARAM_FREQ), 4, ext > 0.5f ? 10.f : 2.f);
        result = juce::String(fv, 2);
        if (result.length() > 5)
            result = result.substring(0, 5);
    }
    else if (paramId == PARAM_STEREO)
    {
        result = juce::String((int)(getVal(PARAM_STEREO) * 360.f));
    }
    else if (paramId == PARAM_MIN_FREQ)
    {
        result = juce::String((int)(getVal(PARAM_MIN_FREQ) * 100.f)) + "%";
    }
    else if (paramId == PARAM_MAX_FREQ)
    {
        result = juce::String((int)(getVal(PARAM_MAX_FREQ) * 100.f)) + "%";
    }
    else if (paramId == PARAM_STAGES)
    {
        int n = Float2Int(getVal(PARAM_STAGES), 0, (double)MaxnStages);
        if (n < 3) n = 3;
        if (n > MaxnStages) n = MaxnStages;
        result = juce::String(n);
    }
    else if (paramId == PARAM_DISTORT)
    {
        result = juce::String((int)(getVal(PARAM_DISTORT) * 100.f));
    }
    else if (paramId == PARAM_FEED)
    {
        result = juce::String((int)(getVal(PARAM_FEED) * 100.f)) + "%";
    }
    else if (paramId == PARAM_DRY_WET)
    {
        result = juce::String(100 - (int)(getVal(PARAM_DRY_WET) * 100)) + "%";
    }
    else if (paramId == PARAM_GAIN)
    {
        float g = gainMap(getVal(PARAM_GAIN));
        if (g < 0.00001f)
            result = "-INF";
        else
        {
            float db = 20.f * log10f(g);
            result = juce::String(db, 1);
            if (result.length() > 4)
                result = result.substring(0, 4);
        }
    }

    return result.toUpperCase();
}

juce::AudioProcessorEditor* SupaPhaserProcessor::createEditor()
{
    return new SupaPhaserEditor(*this);
}

void SupaPhaserProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SupaPhaserProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SupaPhaserProcessor();
}
