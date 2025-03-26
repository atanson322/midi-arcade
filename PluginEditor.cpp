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
    midiInfoToggleButton.setButtonText("MIDI Info");
    midiInfoToggleButton.onClick = [this] { toggleMidiInfoPanel(); };
    addAndMakeVisible(midiInfoToggleButton);
    
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
    auto area = getLocalBounds();
    
    // Title area
    area.removeFromTop(40);
    
    // Top panel for controls
    auto topPanel = area.removeFromTop(80);
    
    // Key signature panel on the left
    keySignaturePanel.setBounds(topPanel.removeFromLeft(300));
    
    // Transport controls on the right
    transportController.setBounds(topPanel.removeFromRight(200));
    
    // MIDI info toggle button
    midiInfoToggleButton.setBounds(topPanel.removeFromLeft(100).reduced(5));
    
    // MIDI device selector (standalone mode only)
    if (audioProcessor.wrapperType == juce::AudioProcessor::wrapperType_Standalone)
    {
        midiOutputSelector->setBounds(topPanel.reduced(5));
    }
    
    // MIDI info panel at the bottom (when visible)
    if (midiInfoPanel.isVisible())
    {
        midiInfoPanel.setBounds(area.removeFromBottom(100));
    }
    
    // Sequencer grid in the viewport
    sequencerViewport.setBounds(area);
    
    // Set the size of the sequencer grid (larger than viewport to allow scrolling)
    sequencerGrid.setSize(sequencerViewport.getWidth() - sequencerViewport.getScrollBarThickness(),
                          sequencerGrid.getRowHeight() * sequencerGrid.getNumRows());
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
    resized();
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
