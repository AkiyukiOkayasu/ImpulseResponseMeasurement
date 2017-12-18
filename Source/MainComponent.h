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
    void computeIR(const int order);
    void generateSweptSine(const double freqBegin, const double freqEnd, const double duration);
    void exportWav(AudioSampleBuffer &bufferToWrite, String fileName);
    std::string getTimeStamp();
    
    enum class measurementState {
        stopped,
        starting,
        started,
        computingIR
    };
    measurementState measureState = measurementState::stopped;
    AudioFormatManager formatManager;
    SoundPlayer sweptSinePlayer;
    Slider sl_order;
    Label lbl_order;
    Label lbl_appName;
    Label lbl_version;
    Label lbl_repeat;
    Slider sl_repeat;
    Label lbl_preSilence;
    Slider sl_preSilence;
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
