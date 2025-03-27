#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class TransportController : public juce::Component
{
public:
    TransportController(MidiArcadeAudioProcessor& processor);
    ~TransportController() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Update the MIDI info display
    void update();
    
private:
    // Reference to the processor
    MidiArcadeAudioProcessor& audioProcessor;
    
    // UI Components
    juce::Label midiInfoLabel;
    juce::Label lastNoteLabel;
    juce::Label channelLabel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportController)
};
