#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include <complex>
#include <chrono>
#include <sstream>
#include <iomanip>

class MainContentComponent   :
public AudioAppComponent,
public Slider::Listener,
public Button::Listener,
public MenuBarModel
{
public:
    //==============================================================================
    MainContentComponent();
    ~MainContentComponent();
    
    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    
    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;
    
    //==============================================================================
    void sliderValueChanged (Slider* slider) override;
    void buttonClicked (Button* button) override;
    StringArray getMenuBarNames() override;
    PopupMenu getMenuForIndex (int topLevelMenuIndex, const String& /*menuName*/) override;
    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;
    void showAudioSettings();
    
private:
    void generateSweptSine(const double freqBegin, const double freqEnd, const double duration);
    void computeIR();
    void exportWav(AudioSampleBuffer &bufferToWrite, String fileName);
    std::string getTimeStamp();
    int nextpow2(int n);
    
    enum class measurementState {
        stopped,
        starting,
        started,
        computingIR
    };
    measurementState measureState = measurementState::stopped;
    AudioFormatManager formatManager;
    SoundPlayer sweptSinePlayer;
    struct RangeSlider{
        Slider range;
        Slider minNumberBox,maxNumberBox;
        Value minSharedValue{20.0};
        Value maxSharedValue{20000.0};
    };
    
    
    RangeSlider sl_freqRange;
    Label lbl_appName;
    Label lbl_version;
    Label lbl_preSilence;
    Slider sl_preSilence;
    Label lbl_postSilence;
    Slider sl_postSilence;
    Label lbl_duration;
    Slider sl_duration;
    TextButton btn_measure;
    TextButton btn_calib;
    Label lbl_latency;
    AudioSampleBuffer buf_sweptSine;
    AudioSampleBuffer buf_inverseFilter;
    AudioSampleBuffer buf_recordedSweptSine;
    AudioSampleBuffer buf_IR;
    unsigned long recordIndex = 0;
    ScopedPointer<ApplicationProperties> appProperties;//オーディオ設定,パラメータの記録用
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()     { return new MainContentComponent(); }
