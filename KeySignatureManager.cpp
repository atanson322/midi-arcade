#include "KeySignatureManager.h"

KeySignatureManager::KeySignatureManager()
{
    updateCurrentScale();
}

KeySignatureManager::~KeySignatureManager()
{
}

void KeySignatureManager::setRootNote(int newRootNote)
{
    if (newRootNote != rootNote && newRootNote >= 0 && newRootNote < 12)
    {
        rootNote = newRootNote;
        updateCurrentScale();
    }
}

void KeySignatureManager::setScaleType(int newScaleType)
{
    if (newScaleType != scaleType && newScaleType >= 0 && newScaleType <= 1)
    {
        scaleType = newScaleType;
        updateCurrentScale();
    }
}

void KeySignatureManager::setFilterMode(int mode)
{
    if (mode != filterMode && mode >= 0 && mode <= 1)
    {
        filterMode = mode;
    }
}

bool KeySignatureManager::isNoteInKey(int midiNote) const
{
    // Check if the note (modulo octave) is in our current scale
    int noteInOctave = midiNote % 12;
    
    for (int scaleNote : currentScale)
    {
        if (scaleNote == noteInOctave)
            return true;
    }
    
    return false;
}

juce::Colour KeySignatureManager::getNoteColor(int midiNote) const
{
    // Return different colors based on whether the note is in key
    if (isNoteInKey(midiNote))
    {
        // In-key note: bright cyan
        return juce::Colour(0xFF00FFFF);
    }
    else
    {
        // Out-of-key note: dimmed magenta or gray depending on filter mode
        return filterMode == 0 ? juce::Colour(0x80FF00FF) : juce::Colour(0x40808080);
    }
}

void KeySignatureManager::updateCurrentScale()
{
    currentScale.clear();
    
    // Choose the scale pattern based on scale type
    const std::vector<int>& scalePattern = (scaleType == 0) ? majorScale : minorScale;
    
    // Generate the scale based on the root note
    for (int interval : scalePattern)
    {
        currentScale.push_back((rootNote + interval) % 12);
    }
}
