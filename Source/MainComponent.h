#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include <complex>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <thread>

struct CustomLookAndFeel    : public LookAndFeel_V4
{
    CustomLookAndFeel()
    {
        setColour(ResizableWindow::backgroundColourId, Colour::Colour(0xFFF9F9F4));
        setColour(Slider::thumbColourId, Colour::Colour(0xFFAED1E6));//ロータリーエンコーダーのつまみ
        setColour(Slider::rotarySliderFillColourId, Colour::Colour(0xFFC6DBF0));//ロータリーエンコーダーの外周(有効範囲)
        setColour(Slider::rotarySliderOutlineColourId, Colour::Colour(0xFF2B2B2A));//ロータリーエンコーダーの外周(非有効範囲)
        setColour(Slider::trackColourId, Colour::Colour(0xFFAED1E6));//スライダーの有効範囲
        setColour(Slider::backgroundColourId, Colour::Colour(0xFF2B2B2A));//スライダーの背景
        setColour(Slider::textBoxTextColourId, Colour::Colour(0xFF2B2B2A));
        setColour(Slider::textBoxOutlineColourId, Colour::Colour(0xFF2B2B2A));
        setColour(Label::textColourId, Colour::Colour(0xFF2B2B2A));
        setColour(ToggleButton::tickColourId, Colour::Colour(0xFF2B2B2A));
        setColour(ToggleButton::tickDisabledColourId, Colour::Colour(0xFF2B2B2A));
    }
};

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
    enum class measurementState {
        stopped,
        starting,
        started,
        computingIR
    };
    void changeMeasurementState(measurementState newState);
    void generateSweptSine(const double freqBegin, const double freqEnd, const double sweptSineDuration, const double postSilenceDuration);
    void measurementSweptSine();
    void computeIR();
    void exportWav(AudioSampleBuffer &bufferToWrite, String fileName);
    std::string getTimeStamp();
    int nextpow2(int n);
    int getNumInputChannels();
    
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
    AudioSampleBuffer buf_sweptSine;
    AudioSampleBuffer buf_inverseFilter;
    AudioSampleBuffer buf_recordedSweptSine;
    AudioSampleBuffer buf_IR;
    unsigned long recordIndex = 0;
    ScopedPointer<ApplicationProperties> appProperties;//オーディオ設定,パラメータの記録用
    CustomLookAndFeel lookAndFeel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()     { return new MainContentComponent(); }
