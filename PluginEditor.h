#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SequencerGrid.h"
#include "KeySignaturePanel.h"
#include "MidiInfoPanel.h"
#include "TransportController.h"

class MidiArcadeAudioProcessorEditor : public juce::AudioProcessorEditor,
                                        public juce::Timer,
                                        public juce::ComboBox::Listener
{
public:
    MidiArcadeAudioProcessorEditor(MidiArcadeAudioProcessor&);
    ~MidiArcadeAudioProcessorEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    
    // ComboBox listener implementation
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

private:
    // Reference to the processor
    MidiArcadeAudioProcessor& audioProcessor;

    // UI Components
    SequencerGrid sequencerGrid;
    KeySignaturePanel keySignaturePanel;
    MidiInfoPanel midiInfoPanel;
    TransportController transportController;
    
    // Device selector for standalone mode
    std::unique_ptr<juce::ComboBox> midiOutputSelector;
    juce::Label midiOutputLabel;
    
    // Toggle button for MIDI info panel
    juce::TextButton midiInfoToggleButton;
    
    // Viewport for scrolling the sequencer grid
    juce::Viewport sequencerViewport;
    
    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> midiChannelAttachment;
    
    // UI update methods
    void updateMidiDeviceList();
    void midiDeviceChanged();
    void toggleMidiInfoPanel();
    
    // Cyberpunk UI styling
    juce::LookAndFeel_V4 cyberpunkLookAndFeel;
    void setupCyberpunkLookAndFeel();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiArcadeAudioProcessorEditor)
};
