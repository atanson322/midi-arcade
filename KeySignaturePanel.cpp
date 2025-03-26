#include "KeySignaturePanel.h"

KeySignaturePanel::KeySignaturePanel(juce::AudioProcessorValueTreeState& params)
    : parameters(params)
{
    // Set up root note combo box
    rootNoteComboBox.addItemList({"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"}, 1);
    rootNoteComboBox.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(rootNoteComboBox);
    
    rootNoteLabel.setText("Root Note", juce::dontSendNotification);
    rootNoteLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(rootNoteLabel);
    
    // Set up scale type combo box
    scaleTypeComboBox.addItemList({"Major", "Minor"}, 1);
    scaleTypeComboBox.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(scaleTypeComboBox);
    
    scaleTypeLabel.setText("Scale", juce::dontSendNotification);
    scaleTypeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(scaleTypeLabel);
    
    // Set up key filter mode combo box
    keyFilterModeComboBox.addItemList({"Highlight", "Lock"}, 1);
    keyFilterModeComboBox.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(keyFilterModeComboBox);
    
    keyFilterModeLabel.setText("Key Mode", juce::dontSendNotification);
    keyFilterModeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(keyFilterModeLabel);
    
    // Create parameter attachments
    rootNoteAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        parameters, "rootNote", rootNoteComboBox);
    
    scaleTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        parameters, "scaleType", scaleTypeComboBox);
    
    keyFilterModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        parameters, "keyFilterMode", keyFilterModeComboBox);
}

KeySignaturePanel::~KeySignaturePanel()
{
}

void KeySignaturePanel::paint(juce::Graphics& g)
{
    // Draw panel background
    g.setColour(juce::Colour(0xFF202830));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.0f);
    
    // Draw border
    g.setColour(juce::Colour(0xFF00FFFF));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1.0f), 10.0f, 2.0f);
    
    // Draw title
    g.setFont(juce::Font("Consolas", 16.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xFFCCFFFF));
    g.drawText("Key Signature", getLocalBounds().removeFromTop(30), juce::Justification::centred, false);
}

void KeySignaturePanel::resized()
{
    auto area = getLocalBounds();
    
    // Title area
    area.removeFromTop(30);
    
    // Divide remaining area into three columns
    int columnWidth = area.getWidth() / 3;
    
    // Root note column
    auto rootNoteArea = area.removeFromLeft(columnWidth);
    rootNoteLabel.setBounds(rootNoteArea.removeFromTop(20));
    rootNoteComboBox.setBounds(rootNoteArea.reduced(5));
    
    // Scale type column
    auto scaleTypeArea = area.removeFromLeft(columnWidth);
    scaleTypeLabel.setBounds(scaleTypeArea.removeFromTop(20));
    scaleTypeComboBox.setBounds(scaleTypeArea.reduced(5));
    
    // Key filter mode column
    auto keyFilterModeArea = area;
    keyFilterModeLabel.setBounds(keyFilterModeArea.removeFromTop(20));
    keyFilterModeComboBox.setBounds(keyFilterModeArea.reduced(5));
}
