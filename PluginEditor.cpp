#include "PluginEditor.h"

MidiArcadeAudioProcessorEditor::MidiArcadeAudioProcessorEditor(MidiArcadeAudioProcessor& p)
    : AudioProcessorEditor(&p), 
      audioProcessor(p),
      sequencerGrid(p.getSequencerEngine()),
      keySignaturePanel(p.parameters),
      midiInfoPanel(),
      transportController(p)
{
    // Set up cyberpunk look and feel
    setupCyberpunkLookAndFeel();
    setLookAndFeel(&cyberpunkLookAndFeel);
    
    // Set up sequencer viewport
    sequencerViewport.setViewedComponent(&sequencerGrid, false);
    sequencerViewport.setScrollBarsShown(true, false);
    addAndMakeVisible(sequencerViewport);
    
    // Set up key signature panel
    addAndMakeVisible(keySignaturePanel);
    
    // Set up transport controller
    addAndMakeVisible(transportController);
    
    // Set up MIDI info panel (initially hidden)
    addChildComponent(midiInfoPanel);
    
    // Set up MIDI info toggle button
    midiInfoToggleButton.setButtonText("Show MIDI Info");
    midiInfoToggleButton.setToggleState(false, juce::dontSendNotification);
    midiInfoToggleButton.onClick = [this] { midiInfoPanel.setVisible(midiInfoToggleButton.getToggleState()); };
    addAndMakeVisible(midiInfoToggleButton);
    
    // Set up octave control buttons
    octaveUpButton.setButtonText("Octave +");
    octaveUpButton.onClick = [this] { 
        audioProcessor.getSequencerEngine()->shiftOctaveUp(); 
        updateOctaveLabel();
    };
    addAndMakeVisible(octaveUpButton);
    
    octaveDownButton.setButtonText("Octave -");
    octaveDownButton.onClick = [this] { 
        audioProcessor.getSequencerEngine()->shiftOctaveDown(); 
        updateOctaveLabel();
    };
    addAndMakeVisible(octaveDownButton);
    
    // Set up octave label
    octaveLabel.setText("Octave: 3", juce::dontSendNotification);
    octaveLabel.setJustificationType(juce::Justification::centred);
    octaveLabel.setFont(juce::Font("Consolas", 14.0f, juce::Font::bold));
    octaveLabel.setColour(juce::Label::textColourId, juce::Colours::cyan);
    addAndMakeVisible(octaveLabel);
    updateOctaveLabel();
    
    // Set up resolution selector
    resolutionSelector.addItem("Half Time", SequencerEngine::HALF_TIME + 1);
    resolutionSelector.addItem("Normal", SequencerEngine::NORMAL_TIME + 1);
    resolutionSelector.addItem("Double Time", SequencerEngine::DOUBLE_TIME + 1);
    resolutionSelector.setSelectedId(SequencerEngine::NORMAL_TIME + 1);
    resolutionSelector.onChange = [this] {
        int selectedId = resolutionSelector.getSelectedId();
        if (selectedId > 0) {
            audioProcessor.getSequencerEngine()->setResolutionMultiplier(
                static_cast<SequencerEngine::ResolutionMultiplier>(selectedId - 1));
        }
    };
    addAndMakeVisible(resolutionSelector);
    
    // Set up resolution label
    resolutionLabel.setText("Resolution:", juce::dontSendNotification);
    resolutionLabel.setJustificationType(juce::Justification::centredRight);
    resolutionLabel.setFont(juce::Font("Consolas", 14.0f, juce::Font::bold));
    addAndMakeVisible(resolutionLabel);
    
    // Set up random button
    randomButton.setButtonText("Random");
    randomButton.onClick = [this] { audioProcessor.getSequencerEngine()->generateRandomSequence(); };
    addAndMakeVisible(randomButton);
    
    // Set up clear button
    clearButton.setButtonText("Clear");
    clearButton.onClick = [this] { audioProcessor.getSequencerEngine()->clearAllSteps(); };
    addAndMakeVisible(clearButton);
    
    // Set up MIDI device selector (standalone mode only)
    if (audioProcessor.wrapperType == juce::AudioProcessor::wrapperType_Standalone)
    {
        midiOutputSelector = std::make_unique<juce::ComboBox>("MIDI Output");
        midiOutputSelector->addListener(this);
        addAndMakeVisible(midiOutputSelector.get());
        
        midiOutputLabel.setText("MIDI Output:", juce::dontSendNotification);
        midiOutputLabel.attachToComponent(midiOutputSelector.get(), true);
        addAndMakeVisible(midiOutputLabel);
        
        updateMidiDeviceList();
        
        // MIDI channel attachment
        midiChannelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            audioProcessor.parameters, "midiChannel", *midiOutputSelector);
    }
    
    // Set window size
    setSize(800, 600);
    
    // Start timer for UI updates
    startTimerHz(30); // 30 fps for smooth animations
}

MidiArcadeAudioProcessorEditor::~MidiArcadeAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void MidiArcadeAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Fill background with dark color
    g.fillAll(juce::Colour(0xFF101820));
    
    // Draw cyberpunk grid overlay
    g.setColour(juce::Colour(0x20FFFFFF));
    for (int x = 0; x < getWidth(); x += 20)
    {
        g.drawLine(x, 0, x, getHeight(), 0.5f);
    }
    for (int y = 0; y < getHeight(); y += 20)
    {
        g.drawLine(0, y, getWidth(), y, 0.5f);
    }
    
    // Draw plugin title
    g.setFont(juce::Font("Consolas", 24.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xFF00FFFF));
    g.drawText("MIDI.ARCADE", getLocalBounds().removeFromTop(40), juce::Justification::centred, false);
}

void MidiArcadeAudioProcessorEditor::resized()
{
    // Main layout
    auto area = getLocalBounds();
    
    // Title area
    area.removeFromTop(40);
    
    // Transport controls at the bottom
    auto transportArea = area.removeFromBottom(60);
    transportController.setBounds(transportArea.reduced(10));
    
    // Control panel on the right
    auto controlPanelArea = area.removeFromRight(220);
    
    // Key signature panel at the top of the control panel
    keySignaturePanel.setBounds(controlPanelArea.removeFromTop(150).reduced(10));
    
    // New controls in the middle of the control panel
    auto controlsArea = controlPanelArea.removeFromTop(200);
    
    // Octave controls
    auto octaveControlsArea = controlsArea.removeFromTop(40).reduced(5);
    octaveDownButton.setBounds(octaveControlsArea.removeFromLeft(65));
    octaveLabel.setBounds(octaveControlsArea.removeFromLeft(80));
    octaveUpButton.setBounds(octaveControlsArea);
    
    // Resolution controls
    auto resolutionArea = controlsArea.removeFromTop(40).reduced(5);
    resolutionLabel.setBounds(resolutionArea.removeFromLeft(90));
    resolutionSelector.setBounds(resolutionArea);
    
    // Sequence manipulation buttons
    auto buttonsArea = controlsArea.removeFromTop(40).reduced(5);
    randomButton.setBounds(buttonsArea.removeFromLeft(100));
    clearButton.setBounds(buttonsArea);
    
    // MIDI info toggle
    midiInfoToggleButton.setBounds(controlsArea.removeFromTop(40).reduced(5));
    
    // MIDI output selector (standalone mode only)
    if (midiOutputSelector != nullptr)
    {
        auto midiSelectorArea = controlPanelArea.removeFromTop(80);
        midiOutputLabel.setBounds(midiSelectorArea.removeFromTop(20).reduced(5));
        midiOutputSelector->setBounds(midiSelectorArea.reduced(5));
    }
    
    // MIDI info panel at the bottom of the control panel
    midiInfoPanel.setBounds(controlPanelArea.reduced(10));
    
    // Sequencer grid in the remaining area
    sequencerViewport.setBounds(area.reduced(10));
    
    // Make sure the sequencer grid is the right size for the viewport
    int gridWidth = sequencerGrid.getNumSteps() * sequencerGrid.getCellWidth() + sequencerGrid.getNoteNameWidth();
    int gridHeight = sequencerGrid.getNumRows() * sequencerGrid.getRowHeight();
    sequencerGrid.setBounds(0, 0, gridWidth, gridHeight);
}

void MidiArcadeAudioProcessorEditor::timerCallback()
{
    // Update sequencer grid to reflect current playback position
    sequencerGrid.updateCurrentStep();
    
    // Update MIDI info panel if visible
    if (midiInfoPanel.isVisible())
    {
        midiInfoPanel.update(audioProcessor.getSequencerEngine()->getCurrentMidiInfo());
    }
    
    // Update transport controller to reflect DAW transport state
    transportController.update();
}

void MidiArcadeAudioProcessorEditor::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == midiOutputSelector.get())
    {
        midiDeviceChanged();
    }
}

void MidiArcadeAudioProcessorEditor::updateMidiDeviceList()
{
    if (midiOutputSelector != nullptr)
    {
        midiOutputSelector->clear();
        
        auto devices = juce::MidiOutput::getAvailableDevices();
        int index = 1;
        
        midiOutputSelector->addItem("No Output", 1);
        
        for (auto& device : devices)
        {
            midiOutputSelector->addItem(device.name, ++index);
        }
        
        midiOutputSelector->setSelectedId(1);
    }
}

void MidiArcadeAudioProcessorEditor::midiDeviceChanged()
{
    if (midiOutputSelector != nullptr)
    {
        int selectedId = midiOutputSelector->getSelectedId();
        
        if (selectedId == 1)
        {
            // No output selected
            audioProcessor.getMidiDeviceManager()->setMidiOutput(nullptr);
        }
        else
        {
            // Get device index (adjusted for the "No Output" item)
            int deviceIndex = selectedId - 2;
            auto devices = juce::MidiOutput::getAvailableDevices();
            
            if (deviceIndex >= 0 && deviceIndex < devices.size())
            {
                audioProcessor.getMidiDeviceManager()->setMidiOutput(devices[deviceIndex]);
            }
        }
    }
}

void MidiArcadeAudioProcessorEditor::toggleMidiInfoPanel()
{
    midiInfoPanel.setVisible(!midiInfoPanel.isVisible());
    midiInfoToggleButton.setToggleState(midiInfoPanel.isVisible(), juce::dontSendNotification);
    resized();
}

void MidiArcadeAudioProcessorEditor::updateOctaveLabel()
{
    int currentOctave = audioProcessor.getSequencerEngine()->getCurrentOctave();
    octaveLabel.setText("Octave: " + juce::String(currentOctave), juce::dontSendNotification);
}

void MidiArcadeAudioProcessorEditor::setupCyberpunkLookAndFeel()
{
    // Set up colors
    cyberpunkLookAndFeel.setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xFF101820));
    cyberpunkLookAndFeel.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF303840));
    cyberpunkLookAndFeel.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFF00FFFF));
    cyberpunkLookAndFeel.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFFCCFFFF));
    cyberpunkLookAndFeel.setColour(juce::TextButton::textColourOnId, juce::Colour(0xFF000000));
    cyberpunkLookAndFeel.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF303840));
    cyberpunkLookAndFeel.setColour(juce::ComboBox::textColourId, juce::Colour(0xFFCCFFFF));
    cyberpunkLookAndFeel.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xFF00FFFF));
    cyberpunkLookAndFeel.setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xFF202830));
    cyberpunkLookAndFeel.setColour(juce::PopupMenu::textColourId, juce::Colour(0xFFCCFFFF));
    cyberpunkLookAndFeel.setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xFF00FFFF));
    cyberpunkLookAndFeel.setColour(juce::PopupMenu::highlightedTextColourId, juce::Colour(0xFF000000));
    cyberpunkLookAndFeel.setColour(juce::Label::textColourId, juce::Colour(0xFFCCFFFF));
}
