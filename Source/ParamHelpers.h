/*
  ==============================================================================

    ParamHelpers.h
    Created: 26 Apr 2024 11:24:49am
    Author:  Solomon Moulang Lewis

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

using namespace juce;

namespace param_helpers
{
// param id lambda helper func
inline auto id = [](const String& name) -> ParameterID {
    return {name, 1};
};

// lambda helper for NormalisableRange<float>
inline auto ap_range = [](const float& min,
                          const float& max,
                          const float& interval,
                          const float& skew) -> NormalisableRange<float> {
    return {min, max, interval, skew};
};

// lambda helper for AudioParameterBool
inline auto ap_bool = [](const ParameterID& p_id,
                         const bool& init_val) {
    return std::make_unique<AudioParameterBool>(p_id,
                                                p_id.getParamID(),
                                                init_val);
};

// lambda helper for AudioParameterFloat
inline auto ap_float = [](const ParameterID& p_id,
                          const NormalisableRange<float>& range,
                          const float& init_val) {
    return std::make_unique<AudioParameterFloat>(p_id,
                                                 p_id.getParamID(),
                                                 range,
                                                 init_val);
};

// lambda helper for AudioParameterInt
inline auto ap_int = [](const ParameterID& p_id,
                        const int& min,
                        const int& max,
                        const int& init_val) {
    return std::make_unique<AudioParameterInt>(p_id,
                                               p_id.getParamID(),
                                               min, max,
                                               init_val);
};

// lambda helper for AudioParameterChoice
inline auto ap_choice = [](const ParameterID &p_id,
                           const juce::StringArray &s_arr,
                           const int &init_val) {
    return std::make_unique<AudioParameterChoice>(p_id,
                                                  p_id.getParamID(),
                                                  s_arr,
                                                  init_val);
};

} // namespace param_helpers
