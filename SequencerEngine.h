#pragma once

#include <JuceHeader.h>
#include "KeySignatureManager.h"

// Structure to hold MIDI event information for display
struct MidiEventInfo {
    int stepPosition = 0;
    int noteNumber = 60;
    juce::String noteName = "C3";
    int velocity = 100;
    int channel = 1;
    double gateLength = 0.5;
};

class SequencerEngine : public juce::AudioProcessorValueTreeState::Listener
{
public:
    SequencerEngine();
    ~SequencerEngine();
    
    // Initialize with default grid size
    void initialize(int numSteps, int numRows);
    
    // Audio processing methods
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void processBlock(juce::MidiBuffer& midiBuffer, int numSamples);
    void releaseResources();
    
    // Transport control
    void start();
    void stop();
    void reset();
    bool isSequencerPlaying() const { return isPlaying; }
    
    // Grid manipulation
    bool getStep(int step, int row) const;
    void setStep(int step, int row, bool state);
    void clearAllSteps();
    
    // Update from host playhead
    void updatePlayheadPosition(const juce::AudioPlayHead::CurrentPositionInfo& posInfo);
    
    // Parameter listener implementation
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    
    // Getters for UI
    int getCurrentStep() const { return currentStep; }
    int getNumSteps() const { return numSteps; }
    int getNumRows() const { return numRows; }
    int getLowestNote() const { return lowestNote; }
    KeySignatureManager* getKeySignatureManager() { return &keySignatureManager; }
    const MidiEventInfo& getCurrentMidiInfo() const { return currentMidiInfo; }
    
    // State saving/loading
    juce::ValueTree getState() const;
    void setState(const juce::ValueTree& state);
    
private:
    // Grid properties
    int numSteps = 16;
    int numRows = 16;
    int lowestNote = 48; // C3
    std::vector<std::vector<bool>> sequencerGrid;
    
    // Playback state
    int currentStep = 0;
    bool isPlaying = false;
    double sampleRate = 44100.0;
    double samplesPerStep = 0.0;
    double sampleCounter = 0.0;
    double bpm = 120.0;
    
    // Key signature management
    KeySignatureManager keySignatureManager;
    
    // MIDI event info for display
    MidiEventInfo currentMidiInfo;
    
    // Helper methods
    void advanceStep();
    void updateStepLength();
    int rowToMidiNote(int row) const;
    juce::String midiNoteToName(int noteNumber) const;
    void sendNoteOnEvents(juce::MidiBuffer& midiBuffer, int offset);
    void sendNoteOffEvents(juce::MidiBuffer& midiBuffer, int offset);
};
