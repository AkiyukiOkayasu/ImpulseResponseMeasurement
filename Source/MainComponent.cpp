#include "MainComponent.h"

MainContentComponent::MainContentComponent()
{
    if(JUCE_MAC) setMacMainMenu(this);
    formatManager.registerBasicFormats();
    setSize (400, 255);
    
    //保存したパラメータをXMLファイルから呼び出し
    PropertiesFile::Options options;
    options.applicationName     = ProjectInfo::projectName;
    options.filenameSuffix      = "settings";
    options.osxLibrarySubFolder = "Preferences";
    appProperties = new ApplicationProperties();
    appProperties->setStorageParameters (options);
    auto userSettings = appProperties->getUserSettings();
    ScopedPointer<XmlElement> savedAudioState (userSettings->getXmlValue ("audioDeviceState"));//オーディオインターフェースの設定
    ScopedPointer<XmlElement> savedParameter (userSettings->getXmlValue("parameterSettings"));//パラメータの設定
    const double minFreq = savedParameter->hasAttribute("minFreqRange") ? savedParameter->getDoubleAttribute("minFreqRange") : 20.0;
    const double maxFreq = savedParameter->hasAttribute("maxFreqRange") ? savedParameter->getDoubleAttribute("maxFreqRange") : 20000.0;
    const double duration = savedParameter->hasAttribute("duration") ? savedParameter->getDoubleAttribute("duration") : 5.0;
    const double preSilence = savedParameter->hasAttribute("preSilence") ? savedParameter->getDoubleAttribute("preSilence") : 2.0;
    const double postSilence = savedParameter->hasAttribute("postSilence") ? savedParameter->getDoubleAttribute("postSilence") : 3.0;
    const double sampleRate = savedAudioState->hasAttribute("audioDeviceRate") ? savedAudioState->getDoubleAttribute("audioDeviceRate") : 20000.0;
    const int nyquistRate = sampleRate / 2.0;
    
    addAndMakeVisible(sl_freqRange.range);
    sl_freqRange.range.setSliderStyle (Slider::TwoValueHorizontal);
    sl_freqRange.range.setTextBoxStyle (Slider::NoTextBox, true, 80, 20);
    sl_freqRange.range.setColour (Slider::thumbColourId, Colours::aqua);
    sl_freqRange.range.setColour (Slider::trackColourId, Colour (0xff0093ff));
    sl_freqRange.range.setRange (1, nyquistRate, 1);
    sl_freqRange.range.setSkewFactorFromMidPoint(2500.0);
    sl_freqRange.range.addListener (this);
    addAndMakeVisible(sl_freqRange.minNumberBox);
    sl_freqRange.minNumberBox.setSliderStyle (Slider::LinearBarVertical);
    sl_freqRange.minNumberBox.setTextBoxStyle (Slider::TextBoxLeft, false, 80, 20);
    sl_freqRange.minNumberBox.setColour (Slider::trackColourId, Colour (0x00181f22));
    sl_freqRange.minNumberBox.setTextValueSuffix("Hz");
    sl_freqRange.minNumberBox.setVelocityBasedMode(true);
    sl_freqRange.minNumberBox.setRange (1, nyquistRate, 1);
    sl_freqRange.minNumberBox.addListener(this);
    addAndMakeVisible(sl_freqRange.maxNumberBox);
    sl_freqRange.maxNumberBox.setSliderStyle (Slider::LinearBarVertical);
    sl_freqRange.maxNumberBox.setTextBoxStyle (Slider::TextBoxLeft, false, 80, 20);
    sl_freqRange.maxNumberBox.setColour (Slider::trackColourId, Colour (0x00181f22));
    sl_freqRange.maxNumberBox.setTextValueSuffix("Hz");
    sl_freqRange.maxNumberBox.setVelocityBasedMode(true);
    sl_freqRange.maxNumberBox.setRange (1, nyquistRate, 1);
    sl_freqRange.maxNumberBox.addListener(this);
    sl_freqRange.minNumberBox.getValueObject().referTo(sl_freqRange.minSharedValue);
    sl_freqRange.maxNumberBox.getValueObject().referTo(sl_freqRange.maxSharedValue);
    sl_freqRange.range.getMinValueObject().referTo (sl_freqRange.minSharedValue);
    sl_freqRange.range.getMaxValueObject().referTo (sl_freqRange.maxSharedValue);
    sl_freqRange.minNumberBox.setValue(minFreq, dontSendNotification);
    sl_freqRange.maxNumberBox.setValue(maxFreq, dontSendNotification);
    
    addAndMakeVisible(lbl_appName);
    lbl_appName.setText("IR measure", dontSendNotification);
    lbl_appName.setFont (Font (Font::getDefaultMonospacedFontName(), 26.00f, Font::plain).withTypefaceStyle ("Regular"));
    lbl_appName.setJustificationType (Justification::centredLeft);
    lbl_appName.setEditable (false, false, false);
    lbl_appName.setColour (TextEditor::textColourId, Colours::black);
    lbl_appName.setColour (TextEditor::backgroundColourId, Colour (0x00000000));
    addAndMakeVisible (lbl_version);
    std::string version = "ver" + std::string(ProjectInfo::versionString);
    lbl_version.setText(version, dontSendNotification);
    lbl_version.setFont (Font (Font::getDefaultMonospacedFontName(), 15.00f, Font::plain).withTypefaceStyle ("Regular"));
    lbl_version.setJustificationType (Justification::centredLeft);
    lbl_version.setEditable (false, false, false);
    lbl_version.setColour (TextEditor::textColourId, Colours::black);
    lbl_version.setColour (TextEditor::backgroundColourId, Colour (0x00000000));
    
    addAndMakeVisible (sl_preSilence);
    sl_preSilence.setRange (0, 30, 1);
    sl_preSilence.setSliderStyle (Slider::LinearBar);
    sl_preSilence.setVelocityBasedMode(true);
    sl_preSilence.setTextValueSuffix("s");
    sl_preSilence.setTextBoxStyle (Slider::TextBoxLeft, false, 60, 20);
    sl_preSilence.setValue(preSilence, dontSendNotification);
    sl_preSilence.addListener (this);
    addAndMakeVisible (lbl_preSilence);
    lbl_preSilence.setText("Pre Silence", dontSendNotification);
    lbl_preSilence.setFont (Font (Font::getDefaultMonospacedFontName(), 15.00f, Font::plain).withTypefaceStyle ("Regular"));
    lbl_preSilence.setJustificationType (Justification::centredLeft);
    lbl_preSilence.setEditable (false, false, false);
    lbl_preSilence.setColour (TextEditor::textColourId, Colours::black);
    lbl_preSilence.setColour (TextEditor::backgroundColourId, Colour (0x00000000));
    lbl_preSilence.attachToComponent(&sl_preSilence, true);
    
    addAndMakeVisible (sl_postSilence);
    sl_postSilence.setRange (0, 30, 1);
    sl_postSilence.setSliderStyle(Slider::LinearBar);
    sl_postSilence.setVelocityBasedMode(true);
    sl_postSilence.setTextValueSuffix("s");
    sl_postSilence.setTextBoxStyle (Slider::TextBoxLeft, false, 60, 20);
    sl_postSilence.setValue(postSilence, dontSendNotification);
    sl_postSilence.addListener (this);
    addAndMakeVisible (lbl_postSilence);
    lbl_postSilence.setText("Post Silence", dontSendNotification);
    lbl_postSilence.setFont (Font (Font::getDefaultMonospacedFontName(), 15.00f, Font::plain).withTypefaceStyle ("Regular"));
    lbl_postSilence.setJustificationType (Justification::centredLeft);
    lbl_postSilence.setEditable (false, false, false);
    lbl_postSilence.setColour (TextEditor::textColourId, Colours::black);
    lbl_postSilence.setColour (TextEditor::backgroundColourId, Colour (0x00000000));
    lbl_postSilence.attachToComponent(&sl_postSilence, true);
    
    addAndMakeVisible (sl_duration);
    sl_duration.setRange (5, 180, 5);
    sl_duration.setSliderStyle (Slider::LinearBar);
    sl_duration.setVelocityBasedMode(true);
    sl_duration.setTextValueSuffix("s");
    sl_duration.setTextBoxStyle (Slider::TextBoxLeft, false, 60, 20);
    sl_duration.setValue(duration, dontSendNotification);
    sl_duration.addListener (this);
    addAndMakeVisible (lbl_duration);
    lbl_duration.setText("Duration", dontSendNotification);
    lbl_duration.setFont (Font (Font::getDefaultMonospacedFontName(), 15.00f, Font::plain).withTypefaceStyle ("Regular"));
    lbl_duration.setJustificationType (Justification::centredLeft);
    lbl_duration.setEditable (false, false, false);
    lbl_duration.setColour (TextEditor::textColourId, Colours::black);
    lbl_duration.setColour (TextEditor::backgroundColourId, Colour (0x00000000));
    lbl_duration.attachToComponent(&sl_duration, true);
    
    addAndMakeVisible (btn_measure);
    btn_measure.setButtonText ("Start Measurement");
    btn_measure.addListener (this);
    
    setAudioChannels (128, 128, savedAudioState);
    deviceManager.addAudioCallback(&sweptSinePlayer);
}

MainContentComponent::~MainContentComponent()
{
    if (JUCE_MAC) setMacMainMenu(nullptr);
    shutdownAudio();
}

//==============================================================================
void MainContentComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    auto freqRange = sl_freqRange.range.getRange();
    double nyquistFreq = sampleRate / 2.0;
    freqRange.setEnd(nyquistFreq);
    sl_freqRange.minNumberBox.setRange(freqRange, sl_freqRange.minNumberBox.getInterval());
    sl_freqRange.maxNumberBox.setRange(freqRange, sl_freqRange.maxNumberBox.getInterval());
    sl_freqRange.range.setRange(freqRange, sl_freqRange.range.getInterval());
    generateSweptSine(sl_freqRange.minSharedValue.getValue(), sl_freqRange.maxSharedValue.getValue(), sl_duration.getValue(), sl_postSilence.getValue());
}

void MainContentComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    switch (measureState)
    {
        case measurementState::stopped:
        {
            break;
        }
        case measurementState::starting:
        {
            recordIndex = 0;
            measureState = measurementState::started;
            break;
        }
        case measurementState::started:
        {
            const int bufferSize = bufferToFill.buffer->getNumSamples();
            const unsigned long ESSLength = buf_recordedSweptSine.getNumSamples();
            if(recordIndex + bufferSize < ESSLength)
            {
                for(int channel = 0; channel < buf_recordedSweptSine.getNumChannels(); ++channel)
                {
                    buf_recordedSweptSine.copyFrom(channel, recordIndex, *bufferToFill.buffer, channel, 0, bufferSize);
                }
            }
            else
            {
                const int remains = ESSLength - recordIndex;
                for(int channel = 0; channel < buf_recordedSweptSine.getNumChannels(); ++channel)
                {
                    buf_recordedSweptSine.copyFrom(channel, recordIndex, *bufferToFill.buffer, channel, 0, remains);
                }
                measureState = measurementState::computingIR;
                std::thread th(&MainContentComponent::computeIR, this);
                th.detach();
            }
            
            recordIndex += bufferSize;
            break;
        }
        case measurementState::computingIR:
        {
            break;
        }
    }
    bufferToFill.clearActiveBufferRegion();
}

void MainContentComponent::releaseResources()
{
}

//==============================================================================
void MainContentComponent::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void MainContentComponent::resized()
{
    lbl_appName.setBounds (5, 7, 170, 24);
    lbl_version.setBounds (153, 11, 90, 24);
    sl_freqRange.minNumberBox.setBounds (10, 40, 70, 24);
    sl_freqRange.maxNumberBox.setBounds (310, 40, 70, 24);
    sl_freqRange.range.setBounds (10, 66, 380, 30);
    sl_duration.setBounds(255, 104, 130, 24);
    sl_preSilence.setBounds (255, 133, 130, 24);
    sl_postSilence.setBounds(255, 162, 130, 24);
    btn_measure.setBounds (10, 195, 380, 50);
}

//==============================================================================
void MainContentComponent::sliderValueChanged (Slider* slider)
{
    if(slider == &sl_freqRange.range || slider == &sl_freqRange.minNumberBox || slider == &sl_freqRange.maxNumberBox || slider == &sl_duration || slider == &sl_postSilence)
    {
        generateSweptSine(sl_freqRange.minSharedValue.getValue(), sl_freqRange.maxSharedValue.getValue(), sl_duration.getValue(), sl_postSilence.getValue());
    }
    else if (slider == &sl_preSilence)
    {
    }
    
    //スライダーの値をXMLで保存
    String xmltag =  "parameter";
    ScopedPointer<XmlElement> parameterSettings = new XmlElement(xmltag);
    parameterSettings->setAttribute("minFreqRange", sl_freqRange.minNumberBox.getValue());
    parameterSettings->setAttribute("maxFreqRange", sl_freqRange.maxNumberBox.getValue());
    parameterSettings->setAttribute("preSilence", sl_preSilence.getValue());
    parameterSettings->setAttribute("postSilence", sl_postSilence.getValue());
    parameterSettings->setAttribute("duration", sl_duration.getValue());
    appProperties->getUserSettings()->setValue ("parameterSettings", parameterSettings);
    appProperties->getUserSettings()->saveIfNeeded();
}

void MainContentComponent::buttonClicked (Button* button)
{
    if (button == &btn_measure)
    {
        std::thread th(&MainContentComponent::measurementSweptSine, this);
        th.detach();
    }
}

StringArray MainContentComponent::getMenuBarNames()
{
    const char* const names[] = { "Options", nullptr };
    return StringArray (names);
}

PopupMenu MainContentComponent::getMenuForIndex (int topLevelMenuIndex, const String& /*menuName*/)
{
    PopupMenu menu;
    if (topLevelMenuIndex == 0) menu.addItem(1, "Audio Pref.");
    return menu;
}

void MainContentComponent::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
    if (menuItemID == 1) showAudioSettings();
}

void MainContentComponent::generateSweptSine(const double freqBegin, const double freqEnd, const double sweptSineDuration, const double postSilenceDuration)
{
    const double sampleRate = deviceManager.getCurrentAudioDevice()->getCurrentSampleRate();
    const int sweptSineLengthInSamples = sweptSineDuration * sampleRate;//ESS信号長
    const int totalLengthInSamples = sweptSineLengthInSamples + (postSilenceDuration * sampleRate);//ESS信号後の無音も含めた全体のサンプル数
    buf_sweptSine.clear();
    buf_sweptSine.setSize(1, totalLengthInSamples);
    buf_inverseFilter.clear();
    buf_inverseFilter.setSize(1, totalLengthInSamples);
    
    const double alpha = (2.0 * double_Pi * freqBegin * sweptSineDuration) / log(freqEnd / freqBegin);
    const double invFilterGainCoefficient = (-6.0 * log2(freqEnd / freqBegin));
    auto* ESSArray = buf_sweptSine.getWritePointer(0);
    auto* inverseFilterArray = buf_inverseFilter.getWritePointer(0);
    for(long i = 0; i < sweptSineLengthInSamples; ++i)
    {
        //ESS生成
        double tESS = (double)i / sampleRate;
        double vESS = alpha * (exp((tESS / sweptSineDuration) * log(freqEnd / freqBegin)) - 1.0);
        vESS = fmod(vESS, 2.0 * double_Pi);
        vESS = sin(vESS);
        ESSArray[i] = vESS;
        
        //逆フィルタ生成
        double gainInvFilter = Decibels::decibelsToGain(invFilterGainCoefficient * (i / (double)sweptSineLengthInSamples));
        double tInvFilter = (double)(sweptSineLengthInSamples - (i + 1)) / sampleRate;
        double vInvFilter = alpha * (exp((tInvFilter / sweptSineDuration) * log(freqEnd / freqBegin)) - 1.0);
        vInvFilter = fmod(vInvFilter, 2.0 * double_Pi);
        vInvFilter = sin(vInvFilter) * gainInvFilter;
        inverseFilterArray[i] = vInvFilter;
    }
    
    for(long i = sweptSineLengthInSamples - 1; i > 0; --i)
    {
        //ESS終端でのクリックノイズ除去のために終端最近傍のゼロ位相点でトリミング
        double t = (double)i / sampleRate;
        double v = alpha * (exp((t / sweptSineDuration) * log(freqEnd / freqBegin)) - 1.0);
        v = fmod(v, 2.0 * double_Pi);
        if(v <= 0.001) break;
        ESSArray[i] = 0.0;
    }
}

void MainContentComponent::measurementSweptSine()
{
    if(measureState == measurementState::stopped)
    {
        const int size = buf_sweptSine.getNumSamples();
        const int numInputChannels = getNumInputChannels();
        buf_recordedSweptSine.clear();
        buf_recordedSweptSine.setSize(numInputChannels, size);
        std::this_thread::sleep_for(std::chrono::seconds(int(sl_preSilence.getValue())));
        measureState = measurementState::starting;
        sweptSinePlayer.play(&buf_sweptSine, false, true);
    }
}

void MainContentComponent::computeIR()
{
    std::string timeStamp = getTimeStamp();
    exportWav(buf_recordedSweptSine, timeStamp + "_recordedESS.wav");
    const int numChannels = buf_recordedSweptSine.getNumChannels();
    const int N = nextpow2(buf_recordedSweptSine.getNumSamples());//ゼロ埋め後のサンプル数
    const int FFTOrder = log2(2*N);//周波数領域の畳み込みを直線上畳み込みと同等にするためにFFTサイズは2Nにする
    const int IROffset = sl_duration.getValue() * deviceManager.getCurrentAudioDevice()->getCurrentSampleRate();
    dsp::FFT fft(FFTOrder);
    buf_IR.clear();
    buf_IR.setSize(numChannels, buf_recordedSweptSine.getNumSamples());
    auto** recordESSArray = buf_recordedSweptSine.getArrayOfWritePointers();
    auto* inverseFilterArray = buf_inverseFilter.getWritePointer(0);
    auto** IRArray = buf_IR.getArrayOfWritePointers();
    
    for(int channel = 0; channel < numChannels; ++channel)
    {
        std::vector<std::complex<float>> H(2*N, std::complex<float>(0.0f, 0.0f));//SweptSine信号
        std::vector<std::complex<float>> invH(2*N, std::complex<float>(0.0f, 0.0f));//逆フィルタ
        for(long i = 0; i < buf_recordedSweptSine.getNumSamples(); ++i)
        {
            H.at(i).real(recordESSArray[channel][i]);
            invH.at(i).real(inverseFilterArray[i]);
        }
        
        fft.perform(H.data(), H.data(), false);
        fft.perform(invH.data(), invH.data(), false);
        
        for(long i = 0; i < 2*N; ++i) {
            H.at(i) *= invH.at(i);
        }
        
        fft.perform(H.data(), H.data(), true);
        
        for (long i = 0; i < buf_IR.getNumSamples(); ++i) {
            IRArray[channel][i] = H.at(i + IROffset).real();
        }
    }
    
    double normalizeFactor = 1.0 / buf_IR.getMagnitude(0, buf_IR.getNumSamples());//IRのノーマライズ
    buf_IR.applyGain(normalizeFactor);
    exportWav(buf_IR, timeStamp + "_IR.wav");
    measureState = measurementState::stopped;
}

void MainContentComponent::exportWav(AudioSampleBuffer &bufferToWrite, String fileName)
{
    const double sampleRate = deviceManager.getCurrentAudioDevice()->getCurrentSampleRate();
    const int numChannel = bufferToWrite.getNumChannels();
    const int bitDepth = 32;
    File file(File::getSpecialLocation(File::SpecialLocationType::userDesktopDirectory).getChildFile(fileName));
    file.deleteFile();
    ScopedPointer<FileOutputStream>  fos(file.createOutputStream());
    WavAudioFormat wavFormat;
    ScopedPointer<AudioFormatWriter> writer(wavFormat.createWriterFor(fos, sampleRate, numChannel, bitDepth, StringPairArray(), 0));
    fos.release();
    writer->writeFromAudioSampleBuffer(bufferToWrite, 0, bufferToWrite.getNumSamples());
}

void MainContentComponent::showAudioSettings()
{
    AudioDeviceSelectorComponent audioSettingsComp (deviceManager,
                                                    0, 128,//InputChannels: min/max
                                                    0, 128,//OutputChannels: min/max
                                                    false,//Show MIDI input options
                                                    false,//Show MIDI output selector
                                                    false,//Stereo pair
                                                    false//Hide advanced option with button
                                                    );
    
    audioSettingsComp.setSize (450, 250);
    DialogWindow::LaunchOptions o;
    o.content.setNonOwned (&audioSettingsComp);
    o.dialogTitle                   = "Audio Settings";
    o.componentToCentreAround       = this;
    o.dialogBackgroundColour        = Colours::grey;
    o.escapeKeyTriggersCloseButton  = false;
    o.useNativeTitleBar             = true;
    o.resizable                     = false;
    o.runModal();
    
    //設定をXMLファイルで保存
    ScopedPointer<XmlElement> audioState (deviceManager.createStateXml());
    appProperties->getUserSettings()->setValue ("audioDeviceState", audioState);
    appProperties->getUserSettings()->saveIfNeeded();
}

std::string MainContentComponent::getTimeStamp()
{
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(localtime(&now_c), "%Y%m%d_%H%M%S");
    return ss.str();
}

int MainContentComponent::nextpow2(int n)
{
    //nより大きい最小の2のべき乗の数を求める
    return (int)pow(2.0, (floor(log2(n - 1) + 1.0)));
}

int MainContentComponent::getNumInputChannels()
{
    AudioDeviceManager::AudioDeviceSetup deviceSetup;
    deviceManager.getAudioDeviceSetup(deviceSetup);
    auto inputChannelInfo = deviceSetup.inputChannels.toString(2);
    int numInputChannels = 0;
    for(int i = 0; i < inputChannelInfo.length(); ++i) {
        if(inputChannelInfo[i] == '1') ++numInputChannels;
    }
    return numInputChannels;
}
