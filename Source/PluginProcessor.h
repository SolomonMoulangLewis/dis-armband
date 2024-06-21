/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "daisysp/Utility/delayline.h"

#include "daisysp/Filters/svf.h"

#include "daisysp/Effects/overdrive.h"
#include "daisysp/Effects/wavefolder.h"

//==============================================================================
/**
*/
class DsbrassardAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    DsbrassardAudioProcessor();
    ~DsbrassardAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    using APVTS = juce::AudioProcessorValueTreeState;
    APVTS::ParameterLayout createParameterLayout();
    APVTS apvts {*this, nullptr, "Parameter Layout", createParameterLayout()};

//    LinkwitzRileyFilter     lp1_,   ap2_,
//                            hp1_,   lp2_,
//                                    hp2_;
    
    daisysp::Svf lp1_, lp2_, hp1_, hp2_;
    
    daisysp::DelayLine<float, 1440> low_del_, mid_del_, high_del_;
    
    daisysp::Overdrive low_od_, mid_od_, high_od_;
    daisysp::Wavefolder low_wf_, mid_wf_, high_wf_;
    
    float old_low_mid_freq_ = 0.f;
    float old_mid_high_freq_ = 0.f;
    
    std::vector<float> temp_buf_;
    
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DsbrassardAudioProcessor)
};
