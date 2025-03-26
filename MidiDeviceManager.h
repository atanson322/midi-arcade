#pragma once

#include <JuceHeader.h>

class MidiDeviceManager : public juce::AudioProcessorValueTreeState::Listener
{
public:
    MidiDeviceManager();
    ~MidiDeviceManager();
    
    // Set the MIDI output device
    void setMidiOutput(const juce::MidiDeviceInfo& deviceInfo);
    
    // Set MIDI output to nullptr (no output)
    void setMidiOutput(std::nullptr_t);
    
    // Get the current MIDI output device name
    juce::String getCurrentDeviceName() const;
    
    // Set the MIDI channel (1-16)
    void setMidiChannel(int channel);
    
    // Get the current MIDI channel (1-16)
    int getMidiChannel() const { return midiChannel; }
    
    // Send a block of MIDI messages to the current output device
    void sendBlockOfMessages(const juce::MidiBuffer& buffer);
    
    // Parameter listener implementation
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    
private:
    std::unique_ptr<juce::MidiOutput> midiOutput;
    juce::String currentDeviceName;
    int midiChannel = 1; // Default to channel 1 (1-based for UI, 0-based for MIDI messages)
};
