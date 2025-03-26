#include "MidiInfoPanel.h"

MidiInfoPanel::MidiInfoPanel()
{
    // Initialize with default values
}

MidiInfoPanel::~MidiInfoPanel()
{
}

void MidiInfoPanel::paint(juce::Graphics& g)
{
    // Draw panel background
    g.setColour(juce::Colour(0xFF202830));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.0f);
    
    // Draw border
    g.setColour(juce::Colour(0xFFFF00FF)); // Magenta border for MIDI info
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1.0f), 10.0f, 2.0f);
    
    // Draw title
    g.setFont(juce::Font("Consolas", 16.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xFFCCFFFF));
    g.drawText("MIDI Info", getLocalBounds().removeFromTop(30), juce::Justification::centred, false);
    
    // Draw MIDI event info
    g.setFont(juce::Font("Consolas", 14.0f, juce::Font::plain));
    
    auto area = getLocalBounds().reduced(10, 40);
    int rowHeight = 20;
    
    // Step position
    g.drawText("Step: " + juce::String(currentInfo.stepPosition + 1), 
               area.removeFromTop(rowHeight), juce::Justification::left, false);
    
    // Note number and name
    g.drawText("Note: " + currentInfo.noteName + " (" + juce::String(currentInfo.noteNumber) + ")", 
               area.removeFromTop(rowHeight), juce::Justification::left, false);
    
    // Velocity
    g.drawText("Velocity: " + juce::String(currentInfo.velocity), 
               area.removeFromTop(rowHeight), juce::Justification::left, false);
    
    // Channel
    g.drawText("Channel: " + juce::String(currentInfo.channel), 
               area.removeFromTop(rowHeight), juce::Justification::left, false);
    
    // Gate length
    g.drawText("Gate: " + juce::String(currentInfo.gateLength, 2) + " beats", 
               area.removeFromTop(rowHeight), juce::Justification::left, false);
}

void MidiInfoPanel::resized()
{
    // Nothing to resize
}

void MidiInfoPanel::update(const MidiEventInfo& info)
{
    currentInfo = info;
    repaint();
}
