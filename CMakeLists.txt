cmake_minimum_required(VERSION 3.22)

project(Wavoria VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(WAVORIA_BUILD_PLUGIN "Build the JUCE plug-in and standalone application" ON)
option(WAVORIA_BUILD_TESTS "Build the dependency-free DSP tests" ON)

if(WAVORIA_BUILD_PLUGIN)
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
    )
    FetchContent_MakeAvailable(JUCE)

    juce_add_plugin(Wavoria
        COMPANY_NAME "Latham Audio"
        BUNDLE_ID com.lathamaudio.wavoria
        PLUGIN_MANUFACTURER_CODE Latm
        PLUGIN_CODE Wvra
        FORMATS VST3 AU Standalone
        PRODUCT_NAME "Wavoria"
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD FALSE
    )

    target_sources(Wavoria
        PRIVATE
            Source/PluginProcessor.cpp
            Source/PluginProcessor.h
            Source/PluginEditor.cpp
            Source/PluginEditor.h
            Source/dsp/WaveTerrainEngine.h
    )

    target_compile_definitions(Wavoria
        PUBLIC
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            JUCE_VST3_CAN_REPLACE_VST2=0
    )

    target_link_libraries(Wavoria
        PRIVATE
            juce::juce_audio_utils
            juce::juce_dsp
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags
    )
endif()

if(WAVORIA_BUILD_TESTS)
    enable_testing()
    add_executable(WavoriaDSPTests tests/WaveTerrainEngineTests.cpp)
    target_include_directories(WavoriaDSPTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source)
    if(MSVC)
        target_compile_options(WavoriaDSPTests PRIVATE /W4 /permissive-)
    else()
        target_compile_options(WavoriaDSPTests PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME WavoriaDSPTests COMMAND WavoriaDSPTests)
endif()

