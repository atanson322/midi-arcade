#include "TransportController.h"

TransportController::TransportController(MidiArcadeAudioProcessor& processor)
    : audioProcessor(processor)
{
    // Set up play button
    playButton.setButtonText("Play");
    playButton.addListener(this);
    addAndMakeVisible(playButton);
    
    // Set up stop button
    stopButton.setButtonText("Stop");
    stopButton.addListener(this);
    addAndMakeVisible(stopButton);
    
    // Set up tempo display
    tempoDisplay.setText("120 BPM", juce::dontSendNotification);
    tempoDisplay.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(tempoDisplay);
    
    // Update button states
    updateButtonStates();
}

TransportController::~TransportController()
{
    playButton.removeListener(this);
    stopButton.removeListener(this);
}

void TransportController::paint(juce::Graphics& g)
{
    // Draw panel background
    g.setColour(juce::Colour(0xFF202830));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.0f);
    
    // Draw border
    g.setColour(juce::Colour(0xFF00FF00)); // Green border for transport
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1.0f), 10.0f, 2.0f);
    
    // Draw title
    g.setFont(juce::Font("Consolas", 16.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xFFCCFFFF));
    g.drawText("Transport", getLocalBounds().removeFromTop(30), juce::Justification::centred, false);
}

void TransportController::resized()
{
    auto area = getLocalBounds();
    
    // Title area
    area.removeFromTop(30);
    
    // Divide remaining area into three sections
    int buttonWidth = area.getWidth() / 3;
    
    // Play button
    playButton.setBounds(area.removeFromLeft(buttonWidth).reduced(5));
    
    // Stop button
    stopButton.setBounds(area.removeFromLeft(buttonWidth).reduced(5));
    
    // Tempo display
    tempoDisplay.setBounds(area.reduced(5));
}

void TransportController::buttonClicked(juce::Button* button)
{
    if (button == &playButton)
    {
        // Start playback
        audioProcessor.startSequencer();
    }
    else if (button == &stopButton)
    {
        // Stop playback
        audioProcessor.stopSequencer();
    }
    
    // Update button states
    updateButtonStates();
}

void TransportController::update()
{
    // Update button states
    updateButtonStates();
    
    // Update tempo display (if we had access to the current tempo)
    // For now, just use a fixed value
    tempoDisplay.setText("120 BPM", juce::dontSendNotification);
}

void TransportController::updateButtonStates()
{
    bool isPlaying = audioProcessor.isSequencerPlaying();
    
    playButton.setEnabled(!isPlaying);
    stopButton.setEnabled(isPlaying);
}
