/*
 * Anagram MIDI Control
 * Copyright (C) 2025 Filipe Coelho <falktx@darkglass.com>
 * SPDX-License-Identifier: ISC
 */

#include "DistrhoUI.hpp"
#include "DistrhoStandaloneUtils.hpp"

#include <algorithm>

START_NAMESPACE_DISTRHO

// --------------------------------------------------------------------------------------------------------------------

class AnagramControlUI : public UI
{
    static constexpr const char* const kBankNames[] = {
        " 1", " 2", " 3", " 4", " 5", " 6", " 7", " 8", " 9", "10", "11", "12", "13", "14",
        "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25", "26", "27", "28",
        "29", "30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "40", "41", "42",
    };
    static_assert(ARRAY_SIZE(kBankNames) == 42, "wrong number of banks");
    static constexpr const char* const kPresetNames[] = {
        "  1", "  2", "  3", "  4", "  5", "  6", "  7", "  8", "  9", " 10", " 11", " 12", " 13", " 14",
        " 15", " 16", " 17", " 18", " 19", " 20", " 21", " 22", " 23", " 24", " 25", " 26", " 27", " 28",
        " 29", " 30", " 31", " 32", " 33", " 34", " 35", " 36", " 37", " 38", " 39", " 40", " 41", " 42",
        " 43", " 44", " 45", " 46", " 47", " 48", " 49", " 50", " 51", " 52", " 53", " 54", " 55", " 56",
        " 57", " 58", " 59", " 60", " 61", " 62", " 63", " 64", " 65", " 66", " 67", " 68", " 69", " 70",
        " 71", " 72", " 73", " 74", " 75", " 76", " 77", " 78", " 79", " 80", " 81", " 82", " 83", " 84",
        " 85", " 86", " 87", " 88", " 89", " 90", " 91", " 92", " 93", " 94", " 95", " 96", " 97", " 98",
        " 99", "100", "101", "102", "103", "104", "105", "106", "107", "108", "109", "110", "111", "112",
        "113", "114", "115", "116", "117", "118", "119", "120", "121", "122", "123", "124", "125", "126",
    };
    static_assert(ARRAY_SIZE(kPresetNames) == 126, "wrong number of presets");
    int params[kParamCount] = {};
    int bank = 0;
    int preset = 0;

    // ----------------------------------------------------------------------------------------------------------------

public:
   /**
      UI class constructor.
      The UI should be initialized to a default state that matches the plugin side.
    */
    AnagramControlUI()
    {
        // match DSP default state
        for (int i = kParamPot1; i <= kParamPot6; ++i)
            params[i] = 63;

        // set minimum size constraint
        const double scaleFactor = getScaleFactor();
        setGeometryConstraints(DISTRHO_UI_DEFAULT_WIDTH * scaleFactor, DISTRHO_UI_DEFAULT_HEIGHT * scaleFactor, false);

        // always enable MIDI support (special permissions needed on some cases)
        if (isUsingNativeAudio() && supportsMIDI() && !isMIDIEnabled())
            requestMIDI();
    }

protected:
    // ----------------------------------------------------------------------------------------------------------------
    // DSP/Plugin Callbacks

   /**
      A parameter has changed on the plugin side.@n
      This is called by the host to inform the UI about parameter changes.
    */
    void parameterChanged(uint32_t index, float value) override
    {
        DISTRHO_SAFE_ASSERT_RETURN(index < kParamCount,);

        params[index] = std::clamp<int>(d_roundToIntPositive(value), 0, 127);
        repaint();
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Widget Callbacks

   /**
      ImGui specific onDisplay function.
    */
    void onImGuiDisplay() override
    {
        const double scaleFactor = getScaleFactor();
        const uint width1 = 330 * scaleFactor;
        const uint width2 = getWidth() - width1;
        const uint height = getHeight();
        String name;

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(width1, height));
        if (ImGui::Begin("Hardcoded", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration))
        {
            ImGui::SeparatorText("Bank Preloading");
            ImGui::SetNextItemWidth(64 * scaleFactor);
            ImGui::Combo("##bank", &bank, kBankNames, ARRAY_SIZE(kBankNames));
            ImGui::SameLine();
            if (ImGui::Button("Go##bank"))
                setState("bank", String(bank + 1));
            ImGui::SameLine();
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
            ImGui::SameLine();
            if (ImGui::Button("Previous##bank"))
                setState("bank", "-");
            ImGui::SameLine();
            if (ImGui::Button("Next##bank"))
                setState("bank", "+");

            ImGui::SeparatorText("Presets");
            ImGui::SetNextItemWidth(64 * scaleFactor);
            ImGui::Combo("##preset", &preset, kPresetNames, ARRAY_SIZE(kPresetNames));
            ImGui::SameLine();
            if (ImGui::Button("Go##preset"))
                setState("preset", String(preset + 1));
            ImGui::SameLine();
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
            ImGui::SameLine();
            if (ImGui::Button("Previous##preset"))
                setState("preset", "-");
            ImGui::SameLine();
            if (ImGui::Button("Next##preset"))
                setState("preset", "+");

            ImGui::SeparatorText("Scenes (only works in scene mode)");
            if (ImGui::Button("Default##scene"))
                setState("scene", "0");
            ImGui::SameLine();
            if (ImGui::Button("A##scene"))
                setState("scene", "1");
            ImGui::SameLine();
            if (ImGui::Button("B##scene"))
                setState("scene", "2");
            ImGui::SameLine();
            if (ImGui::Button("C##scene"))
                setState("scene", "3");
            ImGui::SameLine();
            if (ImGui::Button("Previous##scene"))
                setState("scene", "-");
            ImGui::SameLine();
            if (ImGui::Button("Next##scene"))
                setState("scene", "+");

            ImGui::SeparatorText("Mode");
            if (ImGui::Button("Preset##mode"))
                setState("mode", "1");
            ImGui::SameLine();
            if (ImGui::Button("Stomp##mode"))
                setState("mode", "2");
            ImGui::SameLine();
            if (ImGui::Button("Scene##mode"))
                setState("mode", "3");

            ImGui::SeparatorText("Tools");
            if (ImGui::Button("Tuner##tools"))
            {
                setState("tuner", "");
            }

            ImGui::SeparatorText("CC Bindings");

            for (int i = kParamPot1; i <= kParamPot6; ++i)
            {
                name = "Pot " + String(i + 1) + " (CC " + String(20 + i) + ")";

                if (ImGui::SliderInt(name, params + i, 0, 127))
                {
                    if (ImGui::IsItemActivated())
                        editParameter(i, true);

                    setParameterValue(i, params[i]);
                }

                if (ImGui::IsItemDeactivated())
                    editParameter(i, false);
            }

            for (int i = kParamFoot1; i <= kParamFoot3; ++i)
            {
                name = "Foot " + String(i - kParamFoot1 + 1) + " (CC " + String(17 + i - kParamFoot1) + ")";

                if (ImGui::SliderInt(name, params + i, 0, 1))
                {
                    if (ImGui::IsItemActivated())
                        editParameter(i, true);

                    setParameterValue(i, params[i] ? 127 : 0);
                }

                if (ImGui::IsItemDeactivated())
                    editParameter(i, false);
            }

            {
                if (ImGui::SliderInt("Exp.Pedal (CC 89)", params + kParamExpPedal, 0, 127))
                {
                    if (ImGui::IsItemActivated())
                        editParameter(kParamExpPedal, true);

                    setParameterValue(kParamExpPedal, params[kParamExpPedal]);
                }

                if (ImGui::IsItemDeactivated())
                    editParameter(kParamExpPedal, false);
            }
        }
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(width1, 0));
        ImGui::SetNextWindowSize(ImVec2(width2, height));
        if (ImGui::Begin("Flexible", nullptr, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NoDecoration))
        {
            ImGui::SeparatorText("Generic CCs");

            for (uint8_t i = 0; i < std::size(kAllowedCCs); ++i)
            {
                name = "CC " + String(kAllowedCCs[i]);

                if (ImGui::SliderInt(name, params + (kParamCCs + i), 0, 127))
                {
                    if (ImGui::IsItemActivated())
                        editParameter(kParamCCs + i, true);

                    setParameterValue(kParamCCs + i, params[kParamCCs + i]);
                }

                if (ImGui::IsItemDeactivated())
                    editParameter(kParamCCs + i, false);
            }
        }
        ImGui::End();
    }

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnagramControlUI)
};

// --------------------------------------------------------------------------------------------------------------------

UI* createUI()
{
    return new AnagramControlUI();
}

// --------------------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
