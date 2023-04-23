#include <JuceHeader.h>
#include "PlaylistComponent.h"
#include <string>
#include <fstream>
#include <algorithm>

//==============================================================================
PlaylistComponent::PlaylistComponent(DeckGUI* _deckGUI1, DeckGUI* _deckGUI2, DJAudioPlayer* _playerForParsingMetaData) : deckGUI1(_deckGUI1), deckGUI2(_deckGUI2), playerForParsingMetaData(_playerForParsingMetaData)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

    // create a new directory to store the tracks
    musicFolder.createDirectory();

    // adding different columns into the table component
    tableComponent.getHeader().addColumn("Track Title", 1, 50);
    tableComponent.getHeader().addColumn("Duration", 2, 50);
    tableComponent.getHeader().addColumn("Load to Deck 1", 3, 50);
    tableComponent.getHeader().addColumn("Load to Deck 2", 4, 50);
    tableComponent.getHeader().addColumn("Delete", 5, 50);

    tableComponent.setModel(this);

    // add components
    addAndMakeVisible(tableComponent);
    addAndMakeVisible(libLoadBtn);
    addAndMakeVisible(libSaveBtn);
    addAndMakeVisible(libRestoreBtn);
    addAndMakeVisible(trackFinder);

    // adding listeners
    libLoadBtn.addListener(this);
    libSaveBtn.addListener(this);
    libRestoreBtn.addListener(this);

    // track finder configs
    trackFinder.setTextToShowWhenEmpty("Find track", Colours::white);
    trackFinder.setJustification(Justification::centred);

    // calls a lambda function whenever the return key is pressed. lambda function takes on user search input to find track in library
    trackFinder.onReturnKey = [this] {findTrack(trackFinder.getText());};

    formatManager.registerBasicFormats();
}

PlaylistComponent::~PlaylistComponent()
{
}

void PlaylistComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (juce::Colours::white);
    g.setFont (14.0f);
    g.drawText ("PlaylistComponent", getLocalBounds(),
                juce::Justification::centred, true);   // draw some placeholder text
}

void PlaylistComponent::resized()
{
    double rowH = getHeight() / 8;
    double rowW = getWidth() / 5;

    // setting column bounds
    tableComponent.setBounds(0, rowH, getWidth(), getHeight());
    tableComponent.getHeader().setColumnWidth(1, rowW);
    tableComponent.getHeader().setColumnWidth(2, rowW);
    tableComponent.getHeader().setColumnWidth(3, rowW);
    tableComponent.getHeader().setColumnWidth(4, rowW);
    tableComponent.getHeader().setColumnWidth(5, rowW);

    // setting button bounds
    libLoadBtn.setBounds(getWidth() / 4, 0, getWidth() / 4, rowH);
    libSaveBtn.setBounds((getWidth() / 4) * 3, 0, getWidth() / 4, rowH);
    libRestoreBtn.setBounds(getWidth() / 2, 0, getWidth() / 4, rowH);

    // setting track finder bounds
    trackFinder.setBounds(0, 0, getWidth() / 4, rowH);
}

int PlaylistComponent::getNumRows()
{
    return trackTitles.size();
}

// logic to show selected row. when row is selected, its colour is changed to show highlight
void PlaylistComponent::paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
    if(rowIsSelected)
    {
        g.fillAll(Colours::orange);
    }
    else {
        g.fillAll(Colours::darkgrey);
    }
}

// generates different columns
void PlaylistComponent::paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    // track title column
    if (columnId == 1)
    {
        g.drawText(trackTitles[rowNumber], 5, 0, width, height, Justification::centredLeft, true);
    }

    // track duration column
    if (columnId == 2)
    {
        g.drawText(trackDurations[rowNumber], 5, 0, width, height, Justification::centredLeft, true);
    }
}

Component* PlaylistComponent::refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate)
{
    // each column generates its own respective buttons. buttons are then asigned an ID to differentiate them from each other.
    // buttons contains lamdba function which loads track onto the corresponding decks or to delete track from library
    // load to deck1 column
    if (columnId == 3)
    {
        if (existingComponentToUpdate == nullptr)
        {
            TextButton* loadBtn1 = new TextButton{ "Load" };
            String id{ std::to_string(rowNumber) };
            loadBtn1->setComponentID(id);
            loadBtn1->addListener(this);
            existingComponentToUpdate = loadBtn1;

            loadBtn1->onClick = [this] {loadIntoDeck1();};
        }
    }

    // load to deck 2 colum
    if (columnId == 4)
    {
        if (existingComponentToUpdate == nullptr)
        {
            TextButton* loadBtn2 = new TextButton{ "Load" };
            String id{ std::to_string(rowNumber) };
            loadBtn2->setComponentID(id);
            loadBtn2->addListener(this);
            existingComponentToUpdate = loadBtn2;

            loadBtn2->onClick = [this] {loadIntoDeck2();};
        }
    }

    // delete track
    if (columnId == 5)
    {
        if (existingComponentToUpdate == nullptr)
        {
            TextButton* delBtn = new TextButton { "Delete" };
            String id{ std::to_string(rowNumber) };
            delBtn->setComponentID(id);
            delBtn->addListener(this);
            existingComponentToUpdate = delBtn;

            delBtn->onClick = [this] {deleteTrack();};
        }
    }

    return existingComponentToUpdate;
}

// logic to handle button events
void PlaylistComponent::buttonClicked(Button* button)
{

    if (button == &libLoadBtn)
    {
        // loading track to lib
        loadToLib();

        tableComponent.updateContent();
    }

    if (button == &libSaveBtn)
    {
        // saving tracks inside library to music folder
        saveLib();

        tableComponent.updateContent();
    }

    if (button == &libRestoreBtn)
    {
        // loading tracks from music folder to library
        loadLib();

        tableComponent.updateContent();
    }

    rowSelected = button->getComponentID().getDoubleValue();
}

void PlaylistComponent::loadIntoDeck1()
{
    // load track file to deck1
    deckGUI1->loadFile(addedTracks[rowSelected]);
}

void PlaylistComponent::loadIntoDeck2()
{
    // load track file to deck2
    deckGUI2->loadFile(addedTracks[rowSelected]);
}

void PlaylistComponent::deleteTrack()
{
    // Delete selected track from music lib. track meta data is also removed from other respective arrays
    trackDurations.erase(trackDurations.begin() + rowSelected);
    trackTitles.erase(trackTitles.begin() + rowSelected);
    addedTracks.remove(rowSelected);
    addedFiles.remove(rowSelected);

    tableComponent.updateContent();
}

// logic to handle search result
void PlaylistComponent::findTrack(String searchText)
{
    // if there is a match, the row is highlighted by change of colour
    if (searchText != "")
    {
        getTrackIndex(searchText);

        int rowNumber = getTrackIndex(searchText);
        tableComponent.selectRow(rowNumber);
    }
    else
    {
        tableComponent.deselectAllRows();
    }
}

// function to search library
int PlaylistComponent::getTrackIndex(String searchText)
{
    // searches the track title array for a match, then returns the index
    auto index = find_if(trackTitles.begin(), trackTitles.end(), [&searchText](const String& string) {return string.contains(searchText); });
    int i = -1;

    if (index != trackTitles.end())
    {
        i = std::distance(trackTitles.begin(), index);
    }

    return i;
}

// function to load track to library
void PlaylistComponent::loadToLib()
{
    // user selects file
    std::string trackTitle;
    FileChooser chooser{ "Add files to library" };

    if (chooser.browseForMultipleFilesToOpen())
    {
        for (File& selectedFiles : chooser.getResults())
        {
            // Retrieve track title and converts it into a string
            // string is then pushed into trackTitles array
            trackTitle = selectedFiles.getFileName().toStdString();
            trackTitles.push_back(trackTitle);

            // Track file is added to the addedFiles array
            addedFiles.add(selectedFiles);
            
            // Track file is added to addedTrack array
            addedTracks.add(URL{ selectedFiles });

            // Retrieve track duration and is processesed before pushing to array
            std::string duration = getTrackDur(selectedFiles).toStdString();
            trackDurations.push_back(duration);
        }
    }
}

// function to save track to library
void PlaylistComponent::saveLib()
{
    for (int i = 0; i < addedFiles.size(); i++)
    {
        // files are added to folder
        File folder(musicFolder.getFullPathName() + "/" + addedFiles[i].getFileName());

        if (addedFiles[i].copyFileTo(folder))
        {
            DBG("Track copied");
        }
        else {
            DBG("Track not copied");
        }
    }
}

// function to load from file to library
void PlaylistComponent::loadLib()
{
    int numOfTracks;

    if (musicFolder.isDirectory())
    {
        // Find all mp3 files from music folder and pushes the files to array
        musicFolder.findChildFiles(addedFiles, File::findFiles, false, "*.mp3");
        DBG(addedFiles.size());

        // Find the number of audio files inside the music folder
        numOfTracks = musicFolder.getNumberOfChildFiles(File::findFiles, "*.mp3");

        // Iterate through each file in music folder
        for (int i = 0; i < numOfTracks; i++)
        {
            // Retrieve the track name and push it into respective vector
            trackTitles.push_back(addedFiles[i].getFileName().toStdString());
            addedTracks.add(URL{ addedFiles[i] });

            // Retrieve track duration and is processesed before pushing to array
            std::string duration = getTrackDur(addedFiles[i]).toStdString();
            trackDurations.push_back(duration);
        }
    }
    else
    {
        std::cout << "No music folder found!" << std::endl;
    }
}

// function to get track duration
String PlaylistComponent::getTrackDur(File file)
{
    auto reader = formatManager.createReaderFor(file);
    auto lengthInSeconds = reader->lengthInSamples / reader->sampleRate;
    int roundedSecs = std::round(lengthInSeconds);

    // seconds are converted to mins and remaining secs
    std::string mins = std::to_string(roundedSecs / 60);
    std::string secs = std::to_string(roundedSecs % 60);

    // Formatted time
    std::string duration = mins + ":" + secs;

    delete reader;
    return duration;
}