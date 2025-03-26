#pragma once

#include <JuceHeader.h>

class KeySignaturePanel : public juce::Component
{
public:
    KeySignaturePanel(juce::AudioProcessorValueTreeState& parameters);
    ~KeySignaturePanel() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
private:
    // Reference to parameters
    juce::AudioProcessorValueTreeState& parameters;
    
    // UI Components
    juce::ComboBox rootNoteComboBox;
    juce::ComboBox scaleTypeComboBox;
    juce::ComboBox keyFilterModeComboBox;
    
    juce::Label rootNoteLabel;
    juce::Label scaleTypeLabel;
    juce::Label keyFilterModeLabel;
    
    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> rootNoteAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> scaleTypeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> keyFilterModeAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KeySignaturePanel)
};
