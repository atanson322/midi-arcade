#pragma once

#include <JuceHeader.h>
#include "SequencerEngine.h"

class MidiInfoPanel : public juce::Component
{
public:
    MidiInfoPanel();
    ~MidiInfoPanel() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Update with current MIDI event info
    void update(const MidiEventInfo& info);
    
private:
    // Current MIDI event info
    MidiEventInfo currentInfo;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiInfoPanel)
};
