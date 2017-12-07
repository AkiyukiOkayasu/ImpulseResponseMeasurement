#include "MainComponent.h"

MainContentComponent::MainContentComponent()
{
    setSize (800, 450);
    if(JUCE_MAC) setMacMainMenu(this);
    formatManager.registerBasicFormats();
    
    addAndMakeVisible(btn_measurement);
    btn_measurement.setButtonText("Measurement Start");
    btn_measurement.addListener(this);
    
    addAndMakeVisible(btn_tspGenerate);
    btn_tspGenerate.setButtonText("TSP Generate");
    btn_tspGenerate.addListener(this);
    
    addAndMakeVisible(btn_playIR);
    btn_playIR.setButtonText("IR Play");
    btn_playIR.addListener(this);
    
    //保存したパラメータをXMLファイルから呼び出し
    PropertiesFile::Options options;
    options.applicationName     = ProjectInfo::projectName;
    options.filenameSuffix      = "settings";
    options.osxLibrarySubFolder = "Preferences";
    appProperties = new ApplicationProperties();
    appProperties->setStorageParameters (options);
    ScopedPointer<XmlElement> savedAudioState (appProperties->getUserSettings()->getXmlValue ("audioDeviceState"));//オーディオインターフェースの設定
    setAudioChannels (1, 1, savedAudioState);
    deviceManager.addAudioCallback(&tspPlayer);
    deviceManager.addAudioCallback(&irPlayer);
}

MainContentComponent::~MainContentComponent()
{
    if (JUCE_MAC) setMacMainMenu(nullptr);
    shutdownAudio();
}

//==============================================================================
void MainContentComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
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
                if (recordIndex >= buf_recordedTSP.getNumSamples())
                {
                    measureState = measurementState::computingIR;
                    computeIR();
                    break;
                }
                
                buf_recordedTSP.setSample(0, recordIndex, bufferToFill.buffer->getSample(0, sample));
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
    auto r = getLocalBounds();
    btn_tspGenerate.setBounds(r.removeFromTop(100));
    btn_measurement.setBounds(r.removeFromTop(100));
    btn_playIR.setBounds(r.removeFromTop(100));
}

//==============================================================================
void MainContentComponent::sliderValueChanged (Slider* slider)
{
}

void MainContentComponent::buttonClicked (Button* button)
{
    if(button == &btn_measurement)
    {
        const int size = buf_upTSP.getNumSamples();
        buf_recordedTSP.clear();
        buf_recordedTSP.setSize(1, size);
        buf_impulseResponse.clear();
        buf_impulseResponse.setSize(1, size);
        measureState = measurementState::starting;
        tspPlayer.play(&buf_upTSP, false, true);
    }
    else if(button == &btn_tspGenerate)
    {
        generateTSP(16);
    }
    else if(button == &btn_playIR)
    {
        irPlayer.play(&buf_impulseResponse, false, true);
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

void MainContentComponent::generateTSP(const int FFTOrder)
{
    dsp::FFT fft(FFTOrder);
    const int N = pow(2, FFTOrder);
    const int J = N / 4;
    const double alpha = 2.0 * double_Pi * (double)J;
    constexpr float amp = 60.0;
    buf_upTSP.setSize(1, N);
    buf_downTSP.setSize(1, N);
    buf_recordedTSP.setSize(1, N);
    buf_impulseResponse.clear();
    buf_impulseResponse.setSize(1, N);
    std::vector<std::complex<float>> H(N);
    std::vector<std::complex<float>> invH(N);
    
    for (int i = 0; i < N; ++i)
    {
        if(i <= N / 2)
        {
            H[i].real(0.0);
            H[i].imag(-1.0 * alpha * pow((double)i / (double)N, 2.0));
            H[i] = std::exp(H[i]) * std::complex<float>(amp, 0.0);
            
            invH[i].real(0.0);
            invH[i].imag(1.0 * alpha * pow((double)i / (double)N, 2.0));
            invH[i] = std::exp(invH[i]) * std::complex<float>(amp, 0.0);
        }
        else
        {
            H[i] = std::conj(H[N - i]);
            invH[i] = std::conj(invH[N - i]);
        }
    }
    fft.perform(H.data() , H.data(), true);//逆FFT
    fft.perform(invH.data(), invH.data(), true);//逆FFT
    
    const int roll = (N - J) / 2;
    std::rotate(H.rbegin(), H.rbegin() + roll, H.rend());
    std::rotate(invH.begin(), invH.begin() + roll, invH.end());
    
    for (int i = 0; i < N; ++i)
    {
        buf_upTSP.setSample(0, i, H[i].real());
        buf_downTSP.setSample(0, i, invH[i].real());
    }
}

void MainContentComponent::computeIR()
{
    exportWav(buf_recordedTSP, "preRecord.wav");
    const int N = buf_downTSP.getNumSamples();
    const auto numChannel = static_cast<uint32>(1);//monoral
    const auto IRLength = static_cast<size_t> (N);
    const double sampleRate = deviceManager.getCurrentAudioDevice()->getCurrentSampleRate();
    dsp::ProcessSpec spec {sampleRate, static_cast<uint32>(N), numChannel};
    convolution.prepare(spec);
    convolution.copyAndLoadImpulseResponseFromBuffer(buf_downTSP, sampleRate, false, false, IRLength);
    dsp::AudioBlock<float> block(buf_recordedTSP);
    dsp::ProcessContextReplacing<float> context(block);
    convolution.process(context);
    buf_impulseResponse.makeCopyOf(buf_recordedTSP);
    
    exportWav(buf_impulseResponse, "ImpulseResponse.wav");
    exportWav(buf_upTSP, "UpTSP.wav");
    exportWav(buf_downTSP, "DownTSP.wav");
    exportWav(buf_recordedTSP, "postRecord.wav");
    measureState = measurementState::stopped;
}

void MainContentComponent::exportWav(AudioSampleBuffer &bufferToWrite, String fileName)
{
    const double sampleRate = deviceManager.getCurrentAudioDevice()->getCurrentSampleRate();
    const int numChannel = bufferToWrite.getNumChannels();//Channel
    const int bit = 32;//BitDepth
    File file(File::getSpecialLocation(File::SpecialLocationType::userDesktopDirectory).getChildFile(fileName));
    file.deleteFile();
    ScopedPointer<FileOutputStream>  fos(file.createOutputStream());
    WavAudioFormat wavFormat;
    ScopedPointer<AudioFormatWriter> writer(wavFormat.createWriterFor(fos, sampleRate, numChannel, bit, StringPairArray(), 0));
    
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
