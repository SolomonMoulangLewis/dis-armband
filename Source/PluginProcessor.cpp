/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "ParamHelpers.h"

//==============================================================================
DsbrassardAudioProcessor::DsbrassardAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

DsbrassardAudioProcessor::~DsbrassardAudioProcessor()
{
}

//==============================================================================
const juce::String DsbrassardAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DsbrassardAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DsbrassardAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DsbrassardAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DsbrassardAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DsbrassardAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DsbrassardAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DsbrassardAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DsbrassardAudioProcessor::getProgramName (int index)
{
    return {};
}

void DsbrassardAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DsbrassardAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
//    lp1_.init (sampleRate);
//    lp1_.set_mode (LinkwitzRileyFilter::Mode::LOW);
//    ap2_.init (sampleRate);
//    ap2_.set_mode (LinkwitzRileyFilter::Mode::ALL);
//    hp1_.init (sampleRate);
//    hp1_.set_mode (LinkwitzRileyFilter::Mode::HIGH);
//    lp2_.init (sampleRate);
//    lp2_.set_mode (LinkwitzRileyFilter::Mode::LOW);
//    hp2_.init (sampleRate);
//    hp2_.set_mode (LinkwitzRileyFilter::Mode::HIGH);
    
    lp1_.Init (sampleRate);
    lp2_.Init (sampleRate);
    hp1_.Init (sampleRate);
    hp2_.Init (sampleRate);
    
    low_od_.Init();
    mid_od_.Init();
    high_od_.Init();
    
    low_wf_.Init();
    mid_wf_.Init();
    high_wf_.Init();
    
    low_del_.Init();
    mid_del_.Init();
    high_del_.Init();
    
    temp_buf_.resize (samplesPerBlock);
}

void DsbrassardAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DsbrassardAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

float SetDelayMs (float ms, float sample_rate)
{
    ms     = fmax (.1, ms);
    return ms * .001f * sample_rate;
}

void DsbrassardAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    float ns = buffer.getNumSamples();
    
    //------------------------------------------------------------------------
    // cache params
    float low_mid_freq = apvts.getRawParameterValue ("low-mid-freq")->load();
    float mid_high_freq = apvts.getRawParameterValue ("mid-high-freq")->load();
    
    float res = apvts.getRawParameterValue ("res")->load(); // actually gets rid of phasing idk why...
    float drive = 1.f;
    
    lp1_.SetFreq (low_mid_freq);
    lp1_.SetRes (res); // must also be called whenever setting frequency
    hp1_.SetFreq (low_mid_freq);
    hp1_.SetRes (res);
    
    lp2_.SetFreq (mid_high_freq);
    lp2_.SetRes (res);
    hp2_.SetFreq (mid_high_freq);
    hp2_.SetRes (res);
    
    lp1_.SetDrive (drive);
    hp1_.SetDrive (drive);
    lp2_.SetDrive (drive);
    hp2_.SetDrive (drive);
    
    // temp mute on bands
    bool mute_low = apvts.getRawParameterValue ("mute-low")->load();
    bool mute_mid = apvts.getRawParameterValue ("mute-mid")->load();
    bool mute_high = apvts.getRawParameterValue ("mute-high")->load();
    
    // to boost bands
    float boost_low = apvts.getRawParameterValue ("low-band")->load();
    float boost_mid = apvts.getRawParameterValue ("mid-band")->load();
    float boost_high = apvts.getRawParameterValue ("high-band")->load();
    
    low_od_.SetDrive (boost_low);
    mid_od_.SetDrive (boost_mid);
    high_od_.SetDrive (boost_high);
    
    low_wf_.SetGain (boost_low);
    mid_wf_.SetGain (boost_mid);
    high_wf_.SetGain (boost_high);
    low_wf_.SetOffset (apvts.getRawParameterValue ("low-wf-offset")->load());
    mid_wf_.SetOffset (apvts.getRawParameterValue ("mid-wf-offset")->load());
    high_wf_.SetOffset (apvts.getRawParameterValue ("high-wf-offset")->load());
    
    //delay
    float low_del_time_p = apvts.getRawParameterValue ("low-del-time")->load();
    float mid_del_time_p = apvts.getRawParameterValue ("mid-del-time")->load();
    float high_del_time_p = apvts.getRawParameterValue ("high-del-time")->load();
    low_del_time_p = .1f + low_del_time_p * 29.9f;
    mid_del_time_p = .1f + mid_del_time_p * 29.9f;
    high_del_time_p = .1f + high_del_time_p * 29.9f;
    
    low_del_.SetDelay (SetDelayMs (low_del_time_p, getSampleRate()));
    mid_del_.SetDelay (SetDelayMs (mid_del_time_p, getSampleRate()));
    high_del_.SetDelay (SetDelayMs (high_del_time_p, getSampleRate()));
    
    //------------------------------------------------------------------------
    // per sample loop
    for (int s = 0; s < ns; s++)
    {
        //--------------------------------------------------------------------
        // store bands in separate variables
        float in_s = buffer.getSample (0, s);
        
        //---------------------------------------------------
        // low band split
        lp1_.Process (in_s);
        float low_band = lp1_.Low();
        
        // low band process
        low_del_.Write (low_band); // delay
        low_band = low_del_.Read();
        
        low_band = low_od_.Process (low_band);
//        low_band = low_wf_.Process (low_band);
        
        //---------------------------------------------------
        // mid band split
        hp1_.Process (in_s);
        float mid_band = hp1_.High();
        float high_band = mid_band;
        lp2_.Process (mid_band);
        mid_band = lp2_.Low();
        
        // mid band process
        mid_del_.Write (mid_band); // delay
        mid_band = mid_del_.Read();
        
        mid_band = mid_od_.Process (mid_band); // overdrive
//        mid_band = mid_wf_.Process (mid_band); // wavefold

        //---------------------------------------------------
        // high band split
        hp2_.Process (high_band);
        high_band = hp2_.High();
        
        // high band process
        high_del_.Write (high_band); // delay
        high_band = high_del_.Read();
        high_band = high_od_.Process (high_band);
//        high_band = high_wf_.Process (high_band);
        
        //--------------------------------------------------------------------
        // store variables to temp buffer
        temp_buf_[s] = 0.0f;
        
        temp_buf_[s] += !mute_low ? low_band : 0.f;
        temp_buf_[s] += !mute_mid ? mid_band : 0.f;
        temp_buf_[s] += !mute_high ? high_band : 0.f;
        
        buffer.setSample (0, s, temp_buf_[s]);
        buffer.setSample (1, s, temp_buf_[s]);
    }
}

//==============================================================================
bool DsbrassardAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DsbrassardAudioProcessor::createEditor()
{
//    return new DsbrassardAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void DsbrassardAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void DsbrassardAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

juce::AudioProcessorValueTreeState::ParameterLayout DsbrassardAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout layout;
    
    using namespace param_helpers;
    
    layout.add (ap_bool (id ("mute-low"), false));
    layout.add (ap_bool (id ("mute-mid"), false));
    layout.add (ap_bool (id ("mute-high"), false));
    
    layout.add (ap_float (id ("low-mid-freq"),
                          {16.f, 799.99, 0.01, 1},
                          320.0));
//
    layout.add (ap_float (id ("mid-high-freq"),
                          {800, 16000, 1, 1},
                          1600));
    
    layout.add (ap_float (id ("res"),
                          {0.f, 1.f, 0.00001f, 1.f},
                          .5f));
    
    layout.add (ap_float (id ("low-band"),
                          {0.f, .999f, 0.00001f, 1.f},
                          .5f));
    layout.add (ap_float (id ("mid-band"),
                          {0.f, .999f, 0.00001f, 1.f},
                          .5f));
    layout.add (ap_float (id ("high-band"),
                          {0.f, .999f, 0.00001f, 1.f},
                          .5f));
    
    layout.add (ap_float (id ("low-wf-offset"),
                          {0.f, .999f, 0.00001f, 1.f},
                          .5f));
    layout.add (ap_float (id ("mid-wf-offset"),
                          {0.f, .999f, 0.00001f, 1.f},
                          .5f));
    layout.add (ap_float (id ("high-wf-offset"),
                          {0.f, .999f, 0.00001f, 1.f},
                          .5f));
    
    layout.add (ap_float (id ("low-del-time"),
                          {0.f, .999f, 0.00001f, 1.f},
                          .5f));
    layout.add (ap_float (id ("mid-del-time"),
                          {0.f, .999f, 0.00001f, 1.f},
                          .5f));
    layout.add (ap_float (id ("high-del-time"),
                          {0.f, .999f, 0.00001f, 1.f},
                          .5f));
    
    layout.add (ap_float (id ("low-del-fbk"),
                          {0.f, .999f, 0.00001f, 1.f},
                          .5f));
    layout.add (ap_float (id ("mid-del-fbk"),
                          {0.f, .999f, 0.00001f, 1.f},
                          .5f));
    layout.add (ap_float (id ("high-del-fbk"),
                          {0.f, .999f, 0.00001f, 1.f},
                          .5f));
    
//    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {
//                                                               params.at (Names::Low_Mid_Crossover_Freq), 1},
//                                                               params.at (Names::Low_Mid_Crossover_Freq),
//                                                               juce::NormalisableRange<float>(20, 999, 1, 1),
//                                                               400));
//        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {
//                                                               params.at (Names::Mid_High_Crossover_Freq), 1},
//                                                               params.at (Names::Mid_High_Crossover_Freq),
//                                                               juce::NormalisableRange<float>(1000, 20000, 1, 1),
//                                                               2000));
    
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DsbrassardAudioProcessor();
}
