cmake_minimum_required(VERSION 3.8...3.31)

project(anagram-midi-control C CXX)

set(CMAKE_POLICY_DEFAULT_CMP0025 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0063 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0126 NEW)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

# import dpf
add_subdirectory(DPF)

# custom definitions
add_compile_definitions(
  # custom C++ namespace, less chance of conflicts with other plugins in DEBUG
  DISTRHO_NAMESPACE=DPF_Darkglass_Anagram
  DGL_NAMESPACE=DGL_Darkglass_Anagram

  # skip standalone backends that don't have MIDI and/or might be problematic
  DPF_JACK_STANDALONE_SKIP_PULSEAUDIO_FALLBACK
  DPF_JACK_STANDALONE_SKIP_SDL2_FALLBACK

  # disable ImGui extras
  IMGUI_DISABLE_DEBUG_TOOLS
  IMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS
  IMGUI_DISABLE_DEMO_WINDOWS

  # no input needed for wasm builds
  PUGL_WASM_NO_KEYBOARD_INPUT
)

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
  set(EMSCRIPTEN TRUE)
  set(TARGETS jack)
  set(UI_TYPE gles2)
  add_link_options(-sSTACK_SIZE=4mb)
else()
  set(EMSCRIPTEN FALSE)
  set(TARGETS au clap jack lv2 vst2)
  set(UI_TYPE opengl)
endif()

# ---------------------------------------------------------------------------------------------------------------------
# anagram-midi-control target

dpf_add_plugin(anagram-midi-control
  TARGETS ${TARGETS}
  UI_TYPE ${UI_TYPE}
  FILES_DSP
      src/AnagramControlPlugin.cpp
  FILES_UI
      src/AnagramControlUI.cpp
      DPF-Widgets/opengl/DearImGui.cpp
  FORCE_NATIVE_AUDIO_FALLBACK
)

set_target_properties(anagram-midi-control
  PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    PREFIX ""
)

target_include_directories(anagram-midi-control
  PUBLIC
    DPF-Widgets/opengl
    src
)

# ---------------------------------------------------------------------------------------------------------------------
