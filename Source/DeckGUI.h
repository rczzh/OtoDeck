/*
  ==============================================================================

    DeckGUI.h
    Created: 10 Mar 2023 7:19:31pm
    Author:  ryanc

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DJAudioPlayer.h"
#include "WaveformDisplay.h"

//==============================================================================
/*
*/
class DeckGUI  : public juce::Component, public Button::Listener, public Slider::Listener, public FileDragAndDropTarget, public Timer
{
public:
    DeckGUI(DJAudioPlayer* player, AudioFormatManager& formatManagerToUse, AudioThumbnailCache& cacheToUse);
    ~DeckGUI() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    /** implement Button::Listener */
    void buttonClicked(Button*) override;

    /** implement Slider::Listener */
    void sliderValueChanged(Slider* slider) override;

    bool isInterestedInFileDrag(const StringArray &files) override;
    void filesDropped(const StringArray& files, int x, int y) override;

    void timerCallback() override;
    
    void loadFile(URL audioURL);

private:
    TextButton playButton{ "PLAY" };
    TextButton stopButton{ "STOP" };
    TextButton loadButton{ "LOAD" };
    ToggleButton replayButton{ "Replay" };

    Slider volSlider;
    Slider speedSlider;
    Slider posSlider;

    Label currentTrackTitle;
    Label currentTrackDur;

    Label volSliderLabel;
    Label speedSliderLabel;
    Label posSliderLabel;

    FileChooser fChooser{"Select a file..."};
    DJAudioPlayer* player;

    WaveformDisplay waveformDisplay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeckGUI)
};
