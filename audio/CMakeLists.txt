cmake_minimum_required(VERSION 3.22)

# USAM Audio Engine — M0 scaffold
# Builds: USAMEngine (VST3 + Standalone synth plugin), usam_dsp + usam_core libs,
#         USAMUnitTests (console unit-test runner).
project(USAMEngine VERSION 0.1.0 LANGUAGES C CXX)

# JUCE 9 removed the juce_set_cxx_standard helper; set the standard globally.
# JUCE modules themselves require at least cxx_std_17.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# JUCE (pinned release tag)
# ---------------------------------------------------------------------------
include(FetchContent)
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(JUCE_BUILD_EXTRAS  OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        9.0.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# ---------------------------------------------------------------------------
# usam_dsp — self-contained, allocation-free DSP primitives (testable)
# ---------------------------------------------------------------------------
add_library(usam_dsp STATIC
    src/dsp/osc/WavetableOscillator.cpp
    src/dsp/filter/SVFFilter.cpp
    src/dsp/env/AdsrEnvelope.cpp
)
target_include_directories(usam_dsp PUBLIC src)
target_link_libraries(usam_dsp PUBLIC
    juce::juce_dsp
    juce::juce_recommended_config_flags
)

# ---------------------------------------------------------------------------
# usam_core — synth voice + voice manager (depends only on usam_dsp)
# ---------------------------------------------------------------------------
add_library(usam_core STATIC
    src/core/SynthVoice.cpp
    src/core/VoiceManager.cpp
)
target_include_directories(usam_core PUBLIC src)
target_link_libraries(usam_core PUBLIC
    usam_dsp
    juce::juce_recommended_config_flags
)

# ---------------------------------------------------------------------------
# USAMEngine — the plugin itself (VST3 + Standalone)
# ---------------------------------------------------------------------------
juce_add_plugin(USAMEngine
    VERSION                      0.1.0
    PRODUCT_NAME                 "USAM Engine"
    BUNDLE_IDENTIFIER            com.usam.engine
    COMPANY_NAME                 "USAM"
    PLUGIN_MANUFACTURER_CODE     Usam
    PLUGIN_CODE                  Usam
    FORMATS                      VST3 Standalone
    IS_SYNTH                     TRUE
    NEEDS_MIDI_INPUT             TRUE
    EDITOR_NAME                  USAMEngineEditor
)

target_include_directories(USAMEngine PRIVATE src)
target_link_libraries(USAMEngine PRIVATE
    usam_core
    usam_dsp
    juce::juce_audio_utils
)
target_sources(USAMEngine PRIVATE
    src/core/PluginProcessor.cpp
    src/gui/PluginEditor.cpp
)

# VST3-only plugin (no legacy VST2 release): this silences JUCE's warning about
# VST2/VST3 automation ID mismatches, which only matters when both formats ship.
target_compile_definitions(USAMEngine PRIVATE
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_IGNORE_VST3_MISMATCHED_PARAMETER_ID_WARNING=1
)

# ---------------------------------------------------------------------------
# USAMUnitTests — console runner for DSP / core unit tests
# ---------------------------------------------------------------------------
juce_add_console_app(USAMUnitTests
    VERSION      0.1.0
    PRODUCT_NAME "USAM Unit Tests"
    COMPANY_NAME "USAM"
)

target_include_directories(USAMUnitTests PRIVATE src)
target_link_libraries(USAMUnitTests PRIVATE
    usam_core
    usam_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)
target_sources(USAMUnitTests PRIVATE
    tests/UnitTestMain.cpp
    tests/WavetableOscillatorTests.cpp
    tests/SVFFilterTests.cpp
    tests/AdsrEnvelopeTests.cpp
    tests/VoiceManagerTests.cpp
    tests/SynthVoiceTests.cpp
)
