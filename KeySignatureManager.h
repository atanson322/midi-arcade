#pragma once

#include <JuceHeader.h>

class KeySignatureManager
{
public:
    KeySignatureManager();
    ~KeySignatureManager();
    
    // Set the root note (0 = C, 1 = C#, etc.)
    void setRootNote(int rootNote);
    
    // Set the scale type (0 = Major, 1 = Minor)
    void setScaleType(int scaleType);
    
    // Set the filter mode (0 = Highlight, 1 = Lock)
    void setFilterMode(int mode);
    
    // Get current settings
    int getRootNote() const { return rootNote; }
    int getScaleType() const { return scaleType; }
    int getFilterMode() const { return filterMode; }
    
    // Check if a MIDI note is in the current key
    bool isNoteInKey(int midiNote) const;
    
    // Get the color for a note based on whether it's in key
    juce::Colour getNoteColor(int midiNote) const;
    
private:
    int rootNote = 0;  // C
    int scaleType = 0; // Major
    int filterMode = 0; // Highlight
    
    // Scale patterns (semitone intervals)
    // Major: W-W-H-W-W-W-H (2-2-1-2-2-2-1)
    // Minor: W-H-W-W-H-W-W (2-1-2-2-1-2-2)
    std::vector<int> majorScale = {0, 2, 4, 5, 7, 9, 11};
    std::vector<int> minorScale = {0, 2, 3, 5, 7, 8, 10};
    
    // Update the current scale based on root note and scale type
    void updateCurrentScale();
    
    // The current scale (MIDI note numbers in the scale)
    std::vector<int> currentScale;
};
