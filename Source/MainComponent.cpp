#include "MainComponent.h"

MainContentComponent::MainContentComponent()
{
    if(JUCE_MAC) setMacMainMenu(this);
    formatManager.registerBasicFormats();
    
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
    
    addAndMakeVisible (sl_order);
    sl_order.setRange (7, 24, 1);
    sl_order.setSliderStyle (Slider::IncDecButtons);
    sl_order.setTextBoxStyle (Slider::TextBoxLeft, false, 60, 20);
    sl_order.addListener (this);
    addAndMakeVisible (lbl_order);
    lbl_order.setText("Swept-Sine Order", dontSendNotification);
    lbl_order.setFont (Font (Font::getDefaultMonospacedFontName(), 15.00f, Font::plain).withTypefaceStyle ("Regular"));
    lbl_order.setJustificationType (Justification::centredLeft);
    lbl_order.setEditable (false, false, false);
    lbl_order.setColour (TextEditor::textColourId, Colours::black);
    lbl_order.setColour (TextEditor::backgroundColourId, Colour (0x00000000));
    lbl_order.attachToComponent(&sl_order, true);
    
    addAndMakeVisible (sl_repeat);
    sl_repeat.setRange (1, 8, 1);
    sl_repeat.setSliderStyle (Slider::IncDecButtons);
    sl_repeat.setTextBoxStyle (Slider::TextBoxLeft, false, 60, 20);
    sl_repeat.addListener (this);
    addAndMakeVisible (lbl_repeat);
    lbl_repeat.setText("Repeat", dontSendNotification);
    lbl_repeat.setFont (Font (Font::getDefaultMonospacedFontName(), 15.00f, Font::plain).withTypefaceStyle ("Regular"));
    lbl_repeat.setJustificationType (Justification::centredLeft);
    lbl_repeat.setEditable (false, false, false);
    lbl_repeat.setColour (TextEditor::textColourId, Colours::black);
    lbl_repeat.setColour (TextEditor::backgroundColourId, Colour (0x00000000));
    lbl_repeat.attachToComponent(&sl_repeat, true);
    
    addAndMakeVisible (sl_preSilence);
    sl_preSilence.setRange (0, 20, 0.5);
    sl_preSilence.setSliderStyle (Slider::IncDecButtons);
    sl_preSilence.setTextBoxStyle (Slider::TextBoxLeft, false, 60, 20);
    sl_preSilence.addListener (this);
    addAndMakeVisible (lbl_preSilence);
    lbl_preSilence.setText("Pre Silence(sec)", dontSendNotification);
    lbl_preSilence.setFont (Font (Font::getDefaultMonospacedFontName(), 15.00f, Font::plain).withTypefaceStyle ("Regular"));
    lbl_preSilence.setJustificationType (Justification::centredLeft);
    lbl_preSilence.setEditable (false, false, false);
    lbl_preSilence.setColour (TextEditor::textColourId, Colours::black);
    lbl_preSilence.setColour (TextEditor::backgroundColourId, Colour (0x00000000));
    lbl_preSilence.attachToComponent(&sl_preSilence, true);
    
    addAndMakeVisible (btn_calib);
    btn_calib.setButtonText (TRANS("Calibrate Latency"));
    btn_calib.addListener (this);
    addAndMakeVisible (lbl_latency);
    lbl_latency.setText("Latency is xxx samples", dontSendNotification);
    lbl_latency.setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
    lbl_latency.setJustificationType (Justification::centredLeft);
    lbl_latency.setEditable (false, false, false);
    lbl_latency.setColour (TextEditor::textColourId, Colours::black);
    lbl_latency.setColour (TextEditor::backgroundColourId, Colour (0x00000000));
    
    addAndMakeVisible (btn_measure);
    btn_measure.setButtonText (TRANS("Start Measurement"));
    btn_measure.addListener (this);
    
    setSize (400, 260);
    
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
    const int odr = savedParameter && savedParameter->hasAttribute("order") ? savedParameter->getIntAttribute("order") : 16;
    const int rept = savedParameter && savedParameter->hasAttribute("repeat") ? savedParameter->getIntAttribute("repeat") : 5;
    const double silnce = savedParameter && savedParameter->hasAttribute("preSilence") ? savedParameter->getDoubleAttribute("preSilence") : 1.5;
    sl_order.setValue(odr, dontSendNotification);
    sl_repeat.setValue(rept, dontSendNotification);
    sl_preSilence.setValue(silnce, dontSendNotification);
    setAudioChannels (1, 1, savedAudioState);
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
    generateSweptSine(sl_order.getValue());
}

void MainContentComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    switch (measureState)
    {
        case measurementState::stopped:
            bufferToFill.clearActiveBufferRegion();
            break;
        case measurementState::starting:
            recordIndex = 0;
            measureState = measurementState::started;
            break;
        case measurementState::started:
            for (int sample = 0; sample < bufferToFill.buffer->getNumSamples(); ++sample)
            {
                if (recordIndex >= buf_recordedSweptSine.getNumSamples())
                {
                    measureState = measurementState::computingIR;
                    computeIR(sl_order.getValue());
                    break;
                }
                
                buf_recordedSweptSine.setSample(0, recordIndex, bufferToFill.buffer->getSample(0, sample));
                recordIndex++;
            }
            break;
        case measurementState::computingIR:
            break;
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
    lbl_appName.setBounds (10, 11, 170, 24);
    lbl_version.setBounds (153, 14, 90, 24);
    sl_order.setBounds (160, 54, 150, 24);
    sl_repeat.setBounds (160, 86, 150, 24);
    sl_preSilence.setBounds (160, 118, 150, 24);
    btn_calib.setBounds (10, 161, 140, 28);
    lbl_latency.setBounds (151, 163, 150, 24);
    btn_measure.setBounds (10, 212, 140, 28);
}

//==============================================================================
void MainContentComponent::sliderValueChanged (Slider* slider)
{
    if (slider == &sl_order)
    {
        generateSweptSine(sl_order.getValue());
    }
    else if (slider == &sl_repeat)
    {
    }
    else if (slider == &sl_preSilence)
    {
    }
    
    //スライダーの値をXMLで保存
    String xmltag =  "parameter";
    ScopedPointer<XmlElement> parameterSettings = new XmlElement(xmltag);
    parameterSettings->setAttribute("order", (int)sl_order.getValue());
    parameterSettings->setAttribute("repeat", (int)sl_repeat.getValue());
    parameterSettings->setAttribute("preSilence", sl_preSilence.getValue());
    appProperties->getUserSettings()->setValue ("parameterSettings", parameterSettings);
    appProperties->getUserSettings()->saveIfNeeded();
}

void MainContentComponent::buttonClicked (Button* button)
{
    if (button == &btn_measure)
    {
        const int size = buf_sweptSine.getNumSamples();
        buf_recordedSweptSine.clear();
        buf_recordedSweptSine.setSize(1, size);
        buf_IR.clear();
        buf_IR.setSize(1, size);
        measureState = measurementState::starting;
        sweptSinePlayer.play(&buf_sweptSine, false, true);
    }
    else if (button == &btn_calib)
    {
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

void MainContentComponent::generateSweptSine(const int order)
{
    dsp::FFT fft(order);
    const int N = pow(2, order);//SweptSine信号長
    const int J = N / 4;//SweptSine実行長
    const double alpha = 2.0 * double_Pi * (double)J;
    buf_sweptSine.clear();
    buf_sweptSine.setSize(1, N);
    buf_inverseFilter.clear();
    buf_inverseFilter.setSize(1, N);
    buf_recordedSweptSine.clear();
    buf_recordedSweptSine.setSize(1, N);
    buf_IR.clear();
    buf_IR.setSize(1, N);
    std::vector<std::complex<float>> H(N);//SweptSine信号
    std::vector<std::complex<float>> invH(N);//逆フィルタ
    
    for (int i = 0; i < N; ++i)
    {
        if(i <= N / 2)
        {
            //SweptSine信号
            H[i].real(0.0);
            H[i].imag(-1.0 * alpha * pow((double)i / (double)N, 2.0));
            H[i] = std::exp(H[i]);
            //逆フィルタ
            invH[i].real(0.0);
            invH[i].imag(1.0 * alpha * pow((double)i / (double)N, 2.0));
            invH[i] = std::exp(invH[i]);
        }
        else
        {
            H[i] = std::conj(H[N - i]);
            invH[i] = std::conj(invH[N - i]);
        }
    }
    //逆FFT
    fft.perform(H.data() , H.data(), true);
    fft.perform(invH.data(), invH.data(), true);
    //円状シフト
    const int roll = (N - J) / 2;
    std::rotate(H.rbegin(), H.rbegin() + roll, H.rend());
    std::rotate(invH.begin(), invH.begin() + roll, invH.end());
    
    for (int i = 0; i < N; ++i)
    {
        buf_sweptSine.setSample(0, i, H[i].real());
        buf_inverseFilter.setSample(0, i, invH[i].real());
    }
    
    //ノーマライズ
    double normalizeFactor = 0.97 / buf_sweptSine.getMagnitude(0, buf_sweptSine.getNumSamples());
    buf_sweptSine.applyGain(normalizeFactor);
    buf_inverseFilter.applyGain(normalizeFactor);
}

void MainContentComponent::computeIR(const int order)
{
    const int N = pow(2, order);//SweptSineP信号長
    const int FFTOrder = order + 1;//円状畳み込みを直線上畳み込みと同等にするために2N分のFFTサイズを確保する
    dsp::FFT fft(FFTOrder);
    std::vector<std::complex<float>> H(2*N, std::complex<float>(0.0f, 0.0f));//SweptSine信号
    std::vector<std::complex<float>> invH(2*N, std::complex<float>(0.0f, 0.0f));//逆フィルタ
    
    std::string timeStamp = getTimeStamp();
    exportWav(buf_recordedSweptSine, timeStamp + "_recordedSweptSine.wav");
    for(int i = 0; i < N; ++i)
    {
        H.at(i).real(buf_recordedSweptSine.getSample(0, i));
        invH.at(i).real(buf_inverseFilter.getSample(0, i));
    }
    fft.perform(H.data(), H.data(), false);
    fft.perform(invH.data(), invH.data(), false);
    
    for(int i = 0; i < 2*N; ++i)
    {
        H.at(i) *= invH.at(i);
    }
    
    fft.perform(H.data(), H.data(), true);
    
    const int offset = N;//円状畳み込みのオフセット(後半のみ取り出す)
    for (int i = 0; i < N; ++i)
    {
        const int index = i + offset;
        buf_IR.setSample(0, i, H.at(index).real());
    }
    
    //ノーマライズ
    double normalizeFactor = 1.0 / buf_IR.getMagnitude(0, buf_IR.getNumSamples());
    buf_IR.applyGain(normalizeFactor);
    
    exportWav(buf_IR, timeStamp + "_ImpulseResponse.wav");
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
                                                    1, 1,//InputChannels: min/max
                                                    1, 1,//OutputChannels: min/max
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
    ss << std::put_time(localtime(&now_c), "%F %T");
    return ss.str();
}
