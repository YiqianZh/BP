/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#if JUCE_GCC
 #pragma GCC diagnostic push
 #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif JUCE_CLANG
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif JUCE_MSVC
 #pragma warning (push, 0)
 #pragma warning (disable: 4996)
#endif

class LegacyAudioParameter :   public AudioProcessorParameter
{
public:
    LegacyAudioParameter (AudioProcessor& audioProcessorToUse, int audioParameterIndex)
        : audioProcessor (audioProcessorToUse), idx (audioParameterIndex)
    {
        jassert (idx < audioProcessor.getNumParameters());
    }

    //==============================================================================
    float getValue() const override                    { return audioProcessor.getParameter (idx); }
    void setValue (float newValue) override            { audioProcessor.setParameter (idx, newValue); }
    float getDefaultValue() const override             { return audioProcessor.getParameterDefaultValue (idx); }
    String getName (int maxLen) const override         { return audioProcessor.getParameterName (idx, maxLen); }
    String getLabel() const override                   { return audioProcessor.getParameterLabel (idx); }
    int getNumSteps() const override                   { return audioProcessor.getParameterNumSteps (idx); }
    bool isDiscrete() const override                   { return audioProcessor.isParameterDiscrete (idx); }
    bool isBoolean() const override                    { return false; }
    bool isOrientationInverted() const override        { return audioProcessor.isParameterOrientationInverted (idx); }
    bool isAutomatable() const override                { return audioProcessor.isParameterAutomatable (idx); }
    bool isMetaParameter() const override              { return audioProcessor.isMetaParameter (idx); }
    Category getCategory() const override              { return audioProcessor.getParameterCategory (idx); }
    String getCurrentValueAsText() const override      { return audioProcessor.getParameterText (idx); }
    String getParamID() const                          { return audioProcessor.getParameterID (idx); }

    //==============================================================================
    float getValueForText (const String&) const override
    {
        // legacy parameters do not support this method
        jassertfalse;
        return 0.0f;
    }

    String getText (float, int) const override
    {
        // legacy parameters do not support this method
        jassertfalse;
        return {};
    }

    //==============================================================================
    static bool isLegacy (AudioProcessorParameter* param) noexcept
    {
        return (dynamic_cast<LegacyAudioParameter*> (param) != nullptr);
    }

    static int getParamIndex (AudioProcessor& processor, AudioProcessorParameter* param) noexcept
    {
        if (auto* legacy = dynamic_cast<LegacyAudioParameter*> (param))
        {
            return legacy->idx;
        }
        else
        {
            auto n = processor.getNumParameters();
            jassert (n == processor.getParameters().size());

            for (int i = 0; i < n; ++i)
            {
                if (processor.getParameters()[i] == param)
                    return i;
            }
        }

        return -1;
    }

    static String getParamID (AudioProcessorParameter* param, bool forceLegacyParamIDs) noexcept
    {
        if (auto* legacy = dynamic_cast<LegacyAudioParameter*> (param))
        {
            return legacy->getParamID();
        }
        else if (auto* paramWithID = dynamic_cast<AudioProcessorParameterWithID*> (param))
        {
            if (! forceLegacyParamIDs)
                return paramWithID->paramID;
        }

        return String (param->getParameterIndex());
    }
private:
    AudioProcessor& audioProcessor;
    int idx;
};

//==============================================================================
class LegacyAudioParametersWrapper
{
public:
    void update (AudioProcessor& audioProcessor, bool forceLegacyParamIDs)
    {
        clear();

        legacyParamIDs = forceLegacyParamIDs;

        auto numParameters = audioProcessor.getNumParameters();
        usingManagedParameters = (audioProcessor.getParameters().size() == numParameters) && (! legacyParamIDs);

        for (int i = 0; i < numParameters; ++i)
        {
            AudioProcessorParameter* param = usingManagedParameters ? audioProcessor.getParameters()[i]
                                                                    : (legacy.add (new LegacyAudioParameter (audioProcessor, i)));
            params.add (param);
        }
    }

    void clear()
    {
        legacy.clear();
        params.clear();
    }

    AudioProcessorParameter* getParamForIndex (int index) const
    {
        if (isPositiveAndBelow (index, params.size()))
            return params[index];

        return nullptr;
    }

    String getParamID (AudioProcessor& processor, int idx) const noexcept
    {
        return usingManagedParameters ? processor.getParameterID (idx) : String (idx);
    }

    bool isUsingManagedParameters() const noexcept    { return usingManagedParameters; }
    int getNumParameters() const noexcept             { return params.size(); }

    Array<AudioProcessorParameter*> params;

private:
    OwnedArray<LegacyAudioParameter> legacy;
    bool legacyParamIDs = false, usingManagedParameters = false;
};

#if JUCE_GCC
 #pragma GCC diagnostic pop
#elif JUCE_CLANG
 #pragma clang diagnostic pop
#elif JUCE_MSVC
 #pragma warning (pop)
#endif

} // namespace juce
