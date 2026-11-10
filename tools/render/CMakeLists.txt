# Outil hors plugin : compile le processeur + la fenetre de DOORN sous Linux et exporte des captures PNG (apercu, notice)
cmake_minimum_required(VERSION 3.22)
project(DoornRender VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
add_subdirectory(${JUCE_DIR} JUCE)
set(W ${CMAKE_CURRENT_SOURCE_DIR}/../..)
juce_add_binary_data(DoornAssets SOURCES
  ${W}/gui/Doorn_Background.png ${W}/gui/knob_center_fixed_190px.png ${W}/gui/knob_rotating_crown_and_arrow_190px.png ${W}/gui/knob_reflect_static_190px.png ${W}/gui/boost_center_off_100px.png ${W}/gui/boost_rotating_off_100px.png ${W}/gui/boost_center_on_100px.png ${W}/gui/boost_rotating_on_100px.png ${W}/gui/boost_reflect_static_100px.png ${W}/gui/boost_glow_on.png ${W}/gui/led_off.png ${W}/gui/led_on.png ${W}/gui/switch_left_up.png ${W}/gui/switch_left_mid.png ${W}/gui/switch_left_down.png ${W}/gui/switch_right_up.png ${W}/gui/switch_right_mid.png ${W}/gui/switch_right_down.png ${W}/Poppins-Regular.ttf ${W}/Poppins-Medium.ttf ${W}/Poppins-Bold.ttf)
juce_add_console_app(DoornRender PRODUCT_NAME "DoornRender")
target_sources(DoornRender PRIVATE main.cpp ${W}/PluginProcessor.cpp ${W}/PluginEditor.cpp)
target_include_directories(DoornRender PRIVATE ${W})
target_compile_definitions(DoornRender PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_ALSA=0 JUCE_JACK=0
  JUCE_USE_XRANDR=0 JUCE_USE_XINERAMA=0 JUCE_USE_XCURSOR=0 JucePlugin_Name="Doorn" JucePlugin_Build_VST3=0
  JucePlugin_VersionString="3.2.0" JUCE_STANDALONE_APPLICATION=1 JUCE_MODAL_LOOPS_PERMITTED=1)
target_link_libraries(DoornRender PRIVATE DoornAssets juce::juce_audio_utils juce::juce_recommended_config_flags)
