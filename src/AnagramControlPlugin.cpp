/*
 * Anagram MIDI Control
 * Copyright (C) 2025 Filipe Coelho <falktx@darkglass.com>
 * SPDX-License-Identifier: ISC
 */

#include "DistrhoPlugin.hpp"

#include <algorithm>

START_NAMESPACE_DISTRHO

// --------------------------------------------------------------------------------------------------------------------

enum Actions {
    kActionBank,
    kActionPreset,
    kActionScene,
    kActionMode,
    kActionTuner,
    kActionCount
};

class AnagramControlPlugin : public Plugin
{
    int params[kParamCount] = {};
    bool updatedParams[kParamCount] = {};

    int8_t actions[kActionCount] = {};
    bool updatedActions[kActionCount] = {};

public:
   /**
      Plugin class constructor.@n
      You must set all parameter values to their defaults, matching ParameterRanges::def.
    */
    AnagramControlPlugin()
        : Plugin(kParamCount, 0, 0) // parameters, programs, states
    {
        for (int i = kParamPot1; i <= kParamPot6; ++i)
            params[i] = 63;
    }

protected:
    // ----------------------------------------------------------------------------------------------------------------
    // Information

   /**
      Get the plugin label.@n
      This label is a short restricted name consisting of only _, a-z, A-Z and 0-9 characters.
    */
    const char* getLabel() const noexcept override
    {
        return "AnagramMIDIControl";
    }

   /**
      Get an extensive comment/description about the plugin.@n
      Optional, returns nothing by default.
    */
    const char* getDescription() const override
    {
        return "An audio plugin dedicated to controlling Anagram's functions through MIDI";
    }

   /**
      Get the plugin author/maker.
    */
    const char* getMaker() const noexcept override
    {
        return "falkTX";
    }

   /**
      Get the plugin license (a single line of text or a URL).@n
      For commercial plugins this should return some short copyright information.
    */
    const char* getLicense() const noexcept override
    {
        return "ISC";
    }

   /**
      Get the plugin version, in hexadecimal.
      @see d_version()
    */
    uint32_t getVersion() const noexcept override
    {
        return d_version(1, 0, 0);
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Init

   /**
      Initialize the parameter @a index.@n
      This function will be called once, shortly after the plugin is created.
    */
    void initParameter(uint32_t index, Parameter& parameter) override
    {
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 127.0f;
        parameter.hints = kParameterIsAutomatable | kParameterIsInteger;

        switch (index)
        {
        case kParamPot1 ... kParamPot6:
            parameter.ranges.def = 63.0f;
            parameter.name = "Pot " + String(index + 1);
            parameter.symbol = "pot" + String(index + 1);
            break;
        case kParamFoot1 ... kParamFoot3:
            parameter.hints |= kParameterIsBoolean;
            parameter.ranges.def = 0.0f;
            parameter.name = "Foot " + String(index - kParamFoot1 + 1);
            parameter.symbol = "foot" + String(index - kParamFoot1 + 1);
            break;
        case kParamExpPedal:
            parameter.ranges.def = 0.0f;
            parameter.name = "Exp.Pedal";
            parameter.symbol = "exp_pedal";
            break;
        case kParamCCs ... kParamCount - 1:
            parameter.ranges.def = 0.0f;
            parameter.name = "CC " + String(kAllowedCCs[index - kParamCCs]);
            parameter.symbol = "cc" + String(kAllowedCCs[index - kParamCCs]);
            break;
        }
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Internal data

   /**
      Get the current value of a parameter.@n
      The host may call this function from any context, including realtime processing.
    */
    float getParameterValue(uint32_t index) const override
    {
        DISTRHO_SAFE_ASSERT_RETURN(index < kParamCount, 0.0f);
        return params[index];
    }

   /**
      Change a parameter value.@n
      The host may call this function from any context, including realtime processing.@n
      When a parameter is marked as automatable, you must ensure no non-realtime operations are performed.
      @note This function will only be called for parameter inputs.
    */
    void setParameterValue(uint32_t index, float value) override
    {
        DISTRHO_SAFE_ASSERT_RETURN(index < kParamCount,);

        params[index] = std::clamp<int>(d_roundToIntPositive(value), 0, 127);
        updatedParams[index] = true;
    }

   /**
      Change an internal state @a key to @a value.
    */
    void setState(const char* key, const char* value)
    {
        /**/ if (std::strcmp(key, "bank") == 0)
        {
            switch (value[0])
            {
            case '-':
            case '+':
                actions[kActionBank] = value[0];
                break;
            default:
                actions[kActionBank] = std::atoi(value);
                break;
            }
            updatedActions[kActionBank] = true;
        }
        else if (std::strcmp(key, "preset") == 0)
        {
            switch (value[0])
            {
            case '-':
            case '+':
                actions[kActionPreset] = value[0];
                break;
            default:
                actions[kActionPreset] = std::atoi(value);
                break;
            }
            updatedActions[kActionPreset] = true;
        }
        else if (std::strcmp(key, "scene") == 0)
        {
            actions[kActionScene] = value[0];
            updatedActions[kActionScene] = true;
        }
        else if (std::strcmp(key, "mode") == 0)
        {
            actions[kActionMode] = value[0];
            updatedActions[kActionMode] = true;
        }
        else if (std::strcmp(key, "tuner") == 0)
        {
            updatedActions[kActionTuner] = true;
        }
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Audio/MIDI Processing

   /**
      Activate this plugin.
    */
    void activate() override
    {
        std::memset(updatedParams, 0, sizeof(updatedParams));
        std::memset(updatedActions, 0, sizeof(updatedActions));
        std::memset(actions, 0, sizeof(actions));
    }

   /**
      Run/process function for plugins with MIDI input.
      @note Some parameters might be null if there are no audio inputs/outputs or MIDI events.
    */
    void run(const float**, float**, uint32_t, const MidiEvent* midiEvents, uint32_t midiEventCount) override
    {
        MidiEvent outEvent;
        outEvent.frame = 0;

        // actions
        for (int i = 0; i < kActionCount; ++i)
        {
            if (! updatedActions[i])
                continue;

            outEvent.size = 3;
            outEvent.data[0] = 0xB0;

            switch (static_cast<Actions>(i))
            {
            case kActionBank:
                switch (actions[i])
                {
                default:
                    outEvent.data[1] = 102;
                    outEvent.data[2] = actions[i];
                    break;
                case '+':
                    outEvent.data[1] = 103;
                    outEvent.data[2] = 0;
                    break;
                case '-':
                    outEvent.data[1] = 104;
                    outEvent.data[2] = 0;
                    break;
                }
                break;
            case kActionPreset:
                switch (actions[i])
                {
                default:
                    outEvent.size = 2;
                    outEvent.data[0] = 0xC0;
                    outEvent.data[1] = actions[i];
                    break;
                case '+':
                    outEvent.data[1] = 105;
                    outEvent.data[2] = 0;
                    break;
                case '-':
                    outEvent.data[1] = 106;
                    outEvent.data[2] = 0;
                    break;
                }
                break;
            case kActionScene:
                switch (actions[i])
                {
                case '0' ... '3':
                    outEvent.data[1] = 107;
                    outEvent.data[2] = std::clamp<uint8_t>(actions[i] - '0', 0, 3);
                    break;
                case '+':
                    outEvent.data[1] = 108;
                    outEvent.data[2] = 0;
                    break;
                case '-':
                    outEvent.data[1] = 109;
                    outEvent.data[2] = 0;
                    break;
                default:
                    continue;
                }
                break;
            case kActionMode:
                outEvent.data[1] = 85;
                outEvent.data[2] = std::clamp<uint8_t>(actions[i] - '1', 0, 2);
                break;
            case kActionTuner:
                outEvent.data[1] = 86;
                outEvent.data[2] = 0;
                break;
            default:
                continue;
            }

            if (! writeMidiEvent(outEvent))
                break;

            updatedActions[i] = false;
        }

        // bindings
        outEvent.size = 3;
        outEvent.data[0] = 0xB0;

        for (int i = 0; i < kParamCount; ++i)
        {
            if (! updatedParams[i])
                continue;

            outEvent.data[2] = params[i];

            switch (static_cast<Parameters>(i))
            {
            case kParamPot1 ... kParamPot6:
                outEvent.data[1] = 20 + i;
                break;
            case kParamFoot1 ... kParamFoot3:
                outEvent.data[1] = 17 + i - kParamFoot1;
                break;
            case kParamExpPedal:
                outEvent.data[1] = 89;
                break;
            case kParamCCs ... kParamCount - 1:
                outEvent.data[1] = kAllowedCCs[i - kParamCCs];
                break;
            default:
                continue;
            }

            if (! writeMidiEvent(outEvent))
                break;

            updatedParams[i] = false;
        }
    }

    // ----------------------------------------------------------------------------------------------------------------

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnagramControlPlugin)
};

// --------------------------------------------------------------------------------------------------------------------

Plugin* createPlugin()
{
    return new AnagramControlPlugin();
}

// --------------------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
