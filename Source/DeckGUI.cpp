#include <JuceHeader.h>
#include "DeckGUI.h"
#include "PlaylistComponent.h"
#include <fstream>

//==============================================================================
DeckGUI::DeckGUI(DJAudioPlayer* _player, AudioFormatManager& formatManagerToUse, AudioThumbnailCache& cacheToUse) : player(_player), waveformDisplay(formatManagerToUse, cacheToUse)
{

    // track labels
    currentTrackTitle.setText("Playing: ...", dontSendNotification);
    currentTrackDur.setText("Track Duration: ...", dontSendNotification);
    addAndMakeVisible(currentTrackTitle);
    addAndMakeVisible(currentTrackDur);

    // buttons
    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(loadButton);
    addAndMakeVisible(replayButton);

    //sliders
    addAndMakeVisible(volSlider);
    addAndMakeVisible(speedSlider);
    addAndMakeVisible(posSlider);

    addAndMakeVisible(volSliderLabel);
    addAndMakeVisible(speedSliderLabel);
    addAndMakeVisible(posSliderLabel);

    addAndMakeVisible(waveformDisplay);

    // pointer to listeners
    playButton.addListener(this);
    stopButton.addListener(this);
    loadButton.addListener(this);

    volSlider.addListener(this);
    speedSlider.addListener(this);
    posSlider.addListener(this);

    // setting ranges
    volSlider.setRange(0.0, 1.0);
    speedSlider.setRange(0.0, 10.0);
    posSlider.setRange(0.0, 1.0);

    // presetting values
    volSlider.setValue(1.0);
    speedSlider.setValue(1.0);
    posSlider.setValue(0.0);

    // slider precision
    volSlider.setNumDecimalPlacesToDisplay(2);
    speedSlider.setNumDecimalPlacesToDisplay(2);
    posSlider.setNumDecimalPlacesToDisplay(2);

    // slide label text
    volSliderLabel.setText("VOL", dontSendNotification);
    speedSliderLabel.setText("SPEED", dontSendNotification);
    posSliderLabel.setText("POS", dontSendNotification);

    volSliderLabel.attachToComponent(&volSlider, true);
    speedSliderLabel.attachToComponent(&speedSlider, true);
    posSliderLabel.attachToComponent(&posSlider, true);

    startTimer(500);
}

DeckGUI::~DeckGUI()
{
    stopTimer();
}

void DeckGUI::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component
}

void DeckGUI::resized()
{
    double rowH = getHeight() / 8;
    double rowW = getWidth() / 3;

    int labelW = 50;

    // setting bounds for each component
    currentTrackTitle.setBounds(0, 0, getWidth() / 2, rowH);
    currentTrackDur.setBounds(getWidth() / 2, 0, getWidth() / 2, rowH);

    waveformDisplay.setBounds(0, rowH, getWidth(), rowH * 2);
    playButton.setBounds(0, rowH * 3, rowW, rowH);
    stopButton.setBounds(rowW, rowH * 3, rowW, rowH);
    replayButton.setBounds((2 * rowW) + (rowW / 5), rowH * 3, rowW, rowH);

    volSlider.setBounds(labelW, rowH * 4, getWidth() - labelW, rowH);
    speedSlider.setBounds(labelW, rowH * 5, getWidth() - labelW, rowH);
    posSlider.setBounds(labelW, rowH * 6, getWidth() - labelW, rowH);
    
    loadButton.setBounds(0, rowH * 7, getWidth(), rowH);
}

void DeckGUI::buttonClicked(Button* button)
{
    // logic to handle different buttons
    if (button == &playButton)
    {
        // starts track
        std::cout << "Play button was clicked " << std::endl;
        player->start();
    }
     if (button == &stopButton)
    {
        // stops track
        std::cout << "Stop button was clicked " << std::endl;
        player->stop();

    }

    if (button == &loadButton)
    {
        // prompts user to select a file. file is then processed and used in other functions to retrieve meta data such as waveform / track title
        auto fileChooserFlags = FileBrowserComponent::canSelectFiles;

        fChooser.launchAsync(fileChooserFlags, [this](const FileChooser& chooser)
        {
            auto chosenFile = chooser.getResult();
            player->loadURL(URL{chosenFile});
            currentTrackTitle.setText("Playing: " + URL{ chosenFile }.getFileName().toStdString(), dontSendNotification);
            currentTrackDur.setText("Track Duration: " + player->getTrackDuration(), dontSendNotification);
            waveformDisplay.loadURL(URL{ chosenFile });
        });
    }
}

void DeckGUI::sliderValueChanged(Slider* slider)
{
    // logic to handle different sliders
    // sliders return values which are then used to alter the tracks
    if (slider == &volSlider)
    {
        player->setGain(slider->getValue());
    }

    if (slider == &speedSlider)
    {
        player->setSpeed(slider->getValue());
    }

    if (slider == &posSlider)
    {
        player->setPositionRelative(slider->getValue());
    }
}

bool DeckGUI::isInterestedInFileDrag(const StringArray& files)
{
    std::cout << "DeckGUI::isInterestedInFileDrag" << std::endl;
    return true;
}
void DeckGUI::filesDropped(const StringArray& files, int x, int y)
{
    std::cout << "DeckGUI::filesDropped" << std::endl;
    if (files.size() == 1)
    {
        player->loadURL(URL{ File{files[0]} });
        waveformDisplay.loadURL(URL{ File{files[0]} });
    }
}

// function is called periodically to retrieve information
void DeckGUI::timerCallback()
{
    std::cout << "DeckGUI::timerCallBack" << std::endl;
    waveformDisplay.setPositionRelative(player->getPositionRelative());

    // when the song ends, either of two outcomes are selected. if repeat is toggled, then track is left on replay
    if (player->getPositionRelative() > 1)
    {
        if (replayButton.getToggleState())
        {
            posSlider.setValue(0);
            player->repeat();
        }
        else
        {
            posSlider.setValue(0);
            player->stop();
        }
    }
}

// function to handle the incoming file url
void DeckGUI::loadFile(URL audioURL)
{
    // url is converted back to file
    File file = audioURL.getLocalFile();

    player->loadURL(audioURL);
    waveformDisplay.loadURL(audioURL);

    // file if passed on to other functions to get back meta data
    currentTrackTitle.setText("Playing: " + file.getFileNameWithoutExtension(), dontSendNotification);
    currentTrackDur.setText("Track Duration: " + player->getTrackDuration(), dontSendNotification);
}