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
			auto background = Colours::black;
			auto highlight = juce::Colours::orange;
			auto componentBase = juce::Colours::snow;
			auto textBase = juce::Colours::white;
			setColour(ResizableWindow::backgroundColourId, background);
			setColour(Slider::thumbColourId, highlight);//ロータリーエンコーダーのつまみ
			setColour(Slider::rotarySliderFillColourId, highlight);//ロータリーエンコーダーの外周(有効範囲)
			setColour(Slider::rotarySliderOutlineColourId, componentBase);//ロータリーエンコーダーの外周(非有効範囲)
			setColour(Slider::trackColourId, highlight);//スライダーの有効範囲
			setColour(Slider::backgroundColourId, componentBase);//スライダーの背景
			setColour(Slider::textBoxTextColourId, textBase);
			setColour(Slider::textBoxOutlineColourId, componentBase);
			setColour(Label::textColourId, highlight);
			setColour(ToggleButton::tickColourId, highlight);
      setColour(ToggleButton::tickDisabledColourId, highlight);
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
		std::unique_ptr<XmlElement> savedAudioState;//オーディオインターフェースの設定
		std::unique_ptr<XmlElement> savedParameter;//パラメータの設定
    CustomLookAndFeel lookAndFeel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()     { return new MainContentComponent(); }
