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
    
    // Octave shifting
    void shiftOctaveUp();
    void shiftOctaveDown();
    int getCurrentOctave() const;
    
    // Resolution control
    enum ResolutionMultiplier { HALF_TIME = 0, NORMAL_TIME = 1, DOUBLE_TIME = 2 };
    void setResolutionMultiplier(ResolutionMultiplier multiplier);
    ResolutionMultiplier getResolutionMultiplier() const { return resolutionMultiplier; }
    
    // Random sequence generation
    void generateRandomSequence();
    
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
    double lastPPQPosition = 0.0;
    float stepsPerBeat = 4.0f;
    
    // Scale and note properties
    int rootNote = 60; // Middle C
    std::vector<int> scaleIntervals = { 0, 3, 5, 7, 10 }; // Minor pentatonic scale
    
    // Key signature management
    KeySignatureManager keySignatureManager;
    
    // MIDI event info for display
    MidiEventInfo currentMidiInfo;
    
    // Resolution multiplier
    ResolutionMultiplier resolutionMultiplier = NORMAL_TIME;
    
    // Helper methods
    void advanceStep();
    void updateStepLength();
    int rowToMidiNote(int row) const;
    juce::String midiNoteToName(int noteNumber) const;
    void sendNoteOnEvents(juce::MidiBuffer& midiBuffer, int offset);
    void sendNoteOffEvents(juce::MidiBuffer& midiBuffer, int offset);
};
