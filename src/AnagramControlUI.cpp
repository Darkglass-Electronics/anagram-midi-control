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
    int params[kParamCount] = {};

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

        for (int i = kParamFoot1; i < kParamCount; ++i)
            params[i] = 0;

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
        const float width = getWidth();
        const float height = getHeight();
        const float margin = 0;

        ImGui::SetNextWindowPos(ImVec2(margin, margin));
        ImGui::SetNextWindowSize(ImVec2(width - 2 * margin, height - 2 * margin));

        if (ImGui::Begin("Anagram MIDI Control", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration))
        {
            String name;

            ImGui::SeparatorText("Bank Preloading");
            if (ImGui::Button("Previous##bank"))
                setState("bank", "-");
            ImGui::SameLine();
            if (ImGui::Button("Next##bank"))
                setState("bank", "+");

            ImGui::SeparatorText("Presets");
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
                name = "Pot " + String(i + 1);

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
                name = "Foot " + String(i - kParamFoot1 + 1);

                if (ImGui::SliderInt(name, params + i, 0, 1))
                {
                    if (ImGui::IsItemActivated())
                        editParameter(i, true);

                    setParameterValue(i, params[i]);
                }

                if (ImGui::IsItemDeactivated())
                    editParameter(i, false);
            }

            {
                if (ImGui::SliderInt("Exp.Pedal", params + kParamExpPedal, 0, 127))
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
