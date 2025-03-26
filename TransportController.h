#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class TransportController : public juce::Component,
                           public juce::Button::Listener
{
public:
    TransportController(MidiArcadeAudioProcessor& processor);
    ~TransportController() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Button listener implementation
    void buttonClicked(juce::Button* button) override;
    
    // Update the transport display
    void update();
    
private:
    // Reference to the processor
    MidiArcadeAudioProcessor& audioProcessor;
    
    // UI Components
    juce::TextButton playButton;
    juce::TextButton stopButton;
    juce::Label tempoDisplay;
    
    // Helper methods
    void updateButtonStates();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportController)
};
