#include "TransportController.h"

TransportController::TransportController(MidiArcadeAudioProcessor& p)
    : audioProcessor(p)
{
    // Set up MIDI info label
    midiInfoLabel.setText("MIDI OUTPUT", juce::dontSendNotification);
    midiInfoLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    midiInfoLabel.setJustificationType(juce::Justification::centred);
    midiInfoLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(midiInfoLabel);
    
    // Set up last note label
    lastNoteLabel.setText("No MIDI data", juce::dontSendNotification);
    lastNoteLabel.setFont(juce::Font(12.0f));
    lastNoteLabel.setJustificationType(juce::Justification::centred);
    lastNoteLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
    addAndMakeVisible(lastNoteLabel);
    
    // Set up channel label
    channelLabel.setText("Channel: --", juce::dontSendNotification);
    channelLabel.setFont(juce::Font(12.0f));
    channelLabel.setJustificationType(juce::Justification::centred);
    channelLabel.setColour(juce::Label::textColourId, juce::Colours::lightblue);
    addAndMakeVisible(channelLabel);
}

TransportController::~TransportController()
{
}

void TransportController::paint(juce::Graphics& g)
{
    // Draw background
    g.fillAll(juce::Colour(0xFF202020));
    
    // Draw border
    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 1);
}

void TransportController::resized()
{
    auto area = getLocalBounds().reduced(5);
    
    // Position labels
    midiInfoLabel.setBounds(area.removeFromTop(20));
    area.removeFromTop(5); // Add some spacing
    lastNoteLabel.setBounds(area.removeFromTop(20));
    area.removeFromTop(5); // Add some spacing
    channelLabel.setBounds(area.removeFromTop(20));
}

void TransportController::update()
{
    // Get current MIDI info from the sequencer
    auto midiInfo = audioProcessor.getSequencerEngine()->getCurrentMidiInfo();
    
    // Update the last note label
    if (midiInfo.noteName.isNotEmpty())
    {
        lastNoteLabel.setText("Last Note: " + midiInfo.noteName + " (" + juce::String(midiInfo.noteNumber) + "), Velocity: " + juce::String(midiInfo.velocity),
                            juce::dontSendNotification);
        lastNoteLabel.setColour(juce::Label::textColourId, juce::Colours::lime);
    }
    else
    {
        lastNoteLabel.setText("No MIDI data", juce::dontSendNotification);
        lastNoteLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
    }
    
    // Update channel label
    channelLabel.setText("Channel: " + juce::String(midiInfo.channel), juce::dontSendNotification);
    
    // Force a repaint to ensure the display updates
    repaint();
}
