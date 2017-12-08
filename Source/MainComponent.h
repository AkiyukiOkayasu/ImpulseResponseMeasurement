#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include <complex>

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
    void generateTSP(const int order);
    void computeIR(const int order);
    void exportWav(AudioSampleBuffer &bufferToWrite, String fileName);
    
    enum class measurementState {
        stopped,
        starting,
        started,
        computingIR
    };
    measurementState measureState = measurementState::stopped;
    AudioFormatManager formatManager;
    SoundPlayer tspPlayer;
    SoundPlayer irPlayer;
    TextButton btn_measurement;
    TextButton btn_tspGenerate;
    TextButton btn_playIR;
    AudioSampleBuffer buf_TSP;
    AudioSampleBuffer buf_InverseFilter;
    AudioSampleBuffer buf_recordedTSP;
    AudioSampleBuffer buf_IR;
    unsigned int recordIndex = 0;
    ScopedPointer<ApplicationProperties> appProperties;//オーディオインターフェース,ノイズゲート設;定の記録、呼び出し用
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()     { return new MainContentComponent(); }
