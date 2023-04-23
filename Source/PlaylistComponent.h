#pragma once

#include <JuceHeader.h>
#include "DJAudioPlayer.h"
#include "WaveformDisplay.h"
#include "DeckGUI.h"

#include <vector>
#include <string>

//==============================================================================
/*
*/
class PlaylistComponent : public juce::Component, public juce::TableListBoxModel, public juce::Button::Listener
{
public:
    PlaylistComponent(DeckGUI* _deckGUI1, DeckGUI* _deckGUI2, DJAudioPlayer* _playerForParsingMetaData);
    ~PlaylistComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    int getNumRows() override;
    void paintRowBackground(Graphics&, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(Graphics&, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate);
    void buttonClicked(Button* button) override;

private:
    AudioFormatManager formatManager;
    FileChooser fChooser{ "Select a file..." };
    TableListBox tableComponent;

    double duration;

    Array<URL> addedTracks;
    Array<File> addedFiles;
    std::vector<std::string> trackTitles; // to store track titles

    std::vector<std::string> testArray; // to store track titles

    DeckGUI* deckGUI1;
    DeckGUI* deckGUI2;
    DJAudioPlayer* playerForParsingMetaData;

    TextButton libLoadBtn{ "Load into library" };
    TextButton libSaveBtn{ "Save Tracks" };
    TextButton libRestoreBtn{ "Load Tracks" };

    String getTrackDur(File file);

    std::vector<std::string> trackDurations;

    void loadIntoDeck1();
    void loadIntoDeck2();
    void deleteTrack();

    TextEditor trackFinder;
    void findTrack(String searchText);
    int getTrackIndex(String searchText);

    void loadToLib();
    void saveLib();
    void loadLib();
    File musicFolder = File::getSpecialLocation(File::userDesktopDirectory).getFullPathName() + "/music-folder";

    double rowSelected;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaylistComponent)
};
