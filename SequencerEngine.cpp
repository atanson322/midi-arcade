#include "SequencerEngine.h"

SequencerEngine::SequencerEngine()
{
    // Default initialization
    initialize(16, 16);
}

SequencerEngine::~SequencerEngine()
{
    stop();
}

void SequencerEngine::initialize(int steps, int rows)
{
    numSteps = steps;
    numRows = rows;
    
    // Initialize the grid with all steps off
    sequencerGrid.resize(numSteps);
    for (auto& column : sequencerGrid)
    {
        column.resize(numRows, false);
    }
    
    // Initialize key signature manager
    keySignatureManager.setRootNote(0); // C
    keySignatureManager.setScaleType(0); // Major
    keySignatureManager.setFilterMode(0); // Highlight
    
    // Reset playback state
    currentStep = 0;
    isPlaying = false;
}

void SequencerEngine::prepareToPlay(double newSampleRate, int samplesPerBlock)
{
    sampleRate = newSampleRate;
    updateStepLength();
}

void SequencerEngine::processBlock(juce::MidiBuffer& midiBuffer, int numSamples)
{
    if (!isPlaying)
        return;
    
    int samplePosition = 0;
    
    while (samplePosition < numSamples)
    {
        int samplesToNextStep = juce::jmin(numSamples - samplePosition, 
                                          static_cast<int>(samplesPerStep - sampleCounter));
        
        // If we've reached the next step
        if (sampleCounter + samplesToNextStep >= samplesPerStep)
        {
            // Calculate the exact sample offset for the step change
            int offsetToNextStep = static_cast<int>(samplesPerStep - sampleCounter);
            
            // Send note-off events for the current step
            sendNoteOffEvents(midiBuffer, samplePosition + offsetToNextStep);
            
            // Advance to the next step
            advanceStep();
            
            // Send note-on events for the new step
            sendNoteOnEvents(midiBuffer, samplePosition + offsetToNextStep);
            
            // Reset sample counter
            sampleCounter = 0.0;
        }
        else
        {
            // Update sample counter
            sampleCounter += samplesToNextStep;
        }
        
        // Move to next block of samples
        samplePosition += samplesToNextStep;
    }
}

void SequencerEngine::releaseResources()
{
    // Nothing to release
}

void SequencerEngine::start()
{
    isPlaying = true;
}

void SequencerEngine::stop()
{
    isPlaying = false;
    reset();
}

void SequencerEngine::reset()
{
    currentStep = 0;
    sampleCounter = 0.0;
}

bool SequencerEngine::getStep(int step, int row) const
{
    if (step >= 0 && step < numSteps && row >= 0 && row < numRows)
    {
        return sequencerGrid[step][row];
    }
    return false;
}

void SequencerEngine::setStep(int step, int row, bool state)
{
    if (step >= 0 && step < numSteps && row >= 0 && row < numRows)
    {
        // Check if the note is allowed based on key signature
        int midiNote = rowToMidiNote(row);
        
        if (keySignatureManager.getFilterMode() == 1) // Lock mode
        {
            // Only allow setting steps for notes in the current key
            if (!keySignatureManager.isNoteInKey(midiNote) && state)
                return;
        }
        
        sequencerGrid[step][row] = state;
    }
}

void SequencerEngine::clearAllSteps()
{
    for (auto& column : sequencerGrid)
    {
        std::fill(column.begin(), column.end(), false);
    }
}

void SequencerEngine::updatePlayheadPosition(const juce::AudioPlayHead::CurrentPositionInfo& posInfo)
{
    // Update tempo
    if (posInfo.bpm > 0.0)
    {
        bpm = posInfo.bpm;
        updateStepLength();
    }
    
    // Update current step based on PPQ position
    if (posInfo.isPlaying && posInfo.ppqPositionOfLastBarStart >= 0.0)
    {
        double ppqPerStep = 4.0 / numSteps; // Assuming 4/4 time signature
        double currentPpq = posInfo.ppqPosition - posInfo.ppqPositionOfLastBarStart;
        
        // Calculate which step we should be on
        int newStep = static_cast<int>(currentPpq / ppqPerStep) % numSteps;
        
        // If the step has changed, update and trigger events
        if (newStep != currentStep)
        {
            currentStep = newStep;
        }
    }
}

void SequencerEngine::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "rootNote")
    {
        keySignatureManager.setRootNote(static_cast<int>(newValue));
    }
    else if (parameterID == "scaleType")
    {
        keySignatureManager.setScaleType(static_cast<int>(newValue));
    }
    else if (parameterID == "keyFilterMode")
    {
        keySignatureManager.setFilterMode(static_cast<int>(newValue));
    }
    else if (parameterID == "numSteps")
    {
        // Convert from parameter index to actual step count (4, 8, 12, 16, etc.)
        int newStepCount = (static_cast<int>(newValue) + 1) * 4;
        
        // Resize the grid if needed
        if (newStepCount != numSteps)
        {
            // Save existing steps
            auto oldGrid = sequencerGrid;
            int oldSteps = numSteps;
            
            // Update step count
            numSteps = newStepCount;
            
            // Resize the grid
            sequencerGrid.resize(numSteps);
            for (auto& column : sequencerGrid)
            {
                column.resize(numRows, false);
            }
            
            // Copy old data (up to the minimum of old and new sizes)
            for (int step = 0; step < juce::jmin(oldSteps, numSteps); ++step)
            {
                for (int row = 0; row < numRows; ++row)
                {
                    if (step < oldSteps)
                        sequencerGrid[step][row] = oldGrid[step][row];
                }
            }
            
            // Update step length
            updateStepLength();
        }
    }
}

juce::ValueTree SequencerEngine::getState() const
{
    juce::ValueTree state("SequencerState");
    
    // Store grid dimensions
    state.setProperty("numSteps", numSteps, nullptr);
    state.setProperty("numRows", numRows, nullptr);
    state.setProperty("lowestNote", lowestNote, nullptr);
    
    // Store grid data
    for (int step = 0; step < numSteps; ++step)
    {
        for (int row = 0; row < numRows; ++row)
        {
            if (sequencerGrid[step][row])
            {
                juce::ValueTree noteState("Note");
                noteState.setProperty("step", step, nullptr);
                noteState.setProperty("row", row, nullptr);
                state.addChild(noteState, -1, nullptr);
            }
        }
    }
    
    return state;
}

void SequencerEngine::setState(const juce::ValueTree& state)
{
    if (!state.hasType("SequencerState"))
        return;
    
    // Clear existing grid
    clearAllSteps();
    
    // Get grid dimensions
    numSteps = state.getProperty("numSteps", numSteps);
    numRows = state.getProperty("numRows", numRows);
    lowestNote = state.getProperty("lowestNote", lowestNote);
    
    // Resize grid if needed
    if (sequencerGrid.size() != numSteps)
    {
        sequencerGrid.resize(numSteps);
        for (auto& column : sequencerGrid)
        {
            column.resize(numRows, false);
        }
    }
    
    // Load note data
    for (int i = 0; i < state.getNumChildren(); ++i)
    {
        juce::ValueTree noteState = state.getChild(i);
        if (noteState.hasType("Note"))
        {
            int step = noteState.getProperty("step");
            int row = noteState.getProperty("row");
            
            if (step >= 0 && step < numSteps && row >= 0 && row < numRows)
            {
                sequencerGrid[step][row] = true;
            }
        }
    }
}

void SequencerEngine::advanceStep()
{
    currentStep = (currentStep + 1) % numSteps;
}

void SequencerEngine::updateStepLength()
{
    // Calculate samples per step based on BPM
    // Assuming 4/4 time signature and quarter notes
    double stepsPerBeat = numSteps / 4.0;
    double beatsPerSecond = bpm / 60.0;
    double secondsPerStep = 1.0 / (beatsPerSecond * stepsPerBeat);
    samplesPerStep = secondsPerStep * sampleRate;
}

int SequencerEngine::rowToMidiNote(int row) const
{
    // Convert row index to MIDI note number
    // Row 0 = highest note, numRows-1 = lowest note
    return lowestNote + (numRows - 1 - row);
}

juce::String SequencerEngine::midiNoteToName(int noteNumber) const
{
    static const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    
    int octave = noteNumber / 12 - 1;
    int note = noteNumber % 12;
    
    return juce::String(noteNames[note]) + juce::String(octave);
}

void SequencerEngine::sendNoteOnEvents(juce::MidiBuffer& midiBuffer, int offset)
{
    // Send note-on messages for all active notes in the current step
    for (int row = 0; row < numRows; ++row)
    {
        if (sequencerGrid[currentStep][row])
        {
            int midiNote = rowToMidiNote(row);
            int velocity = 100; // Default velocity
            int channel = 1;    // Default channel (MIDI channels are 1-based in UI, 0-based in MIDI messages)
            
            // Create MIDI message
            juce::MidiMessage message = juce::MidiMessage::noteOn(channel, midiNote, static_cast<juce::uint8>(velocity));
            midiBuffer.addEvent(message, offset);
            
            // Update MIDI info for display
            currentMidiInfo.stepPosition = currentStep;
            currentMidiInfo.noteNumber = midiNote;
            currentMidiInfo.noteName = midiNoteToName(midiNote);
            currentMidiInfo.velocity = velocity;
            currentMidiInfo.channel = channel + 1; // Display 1-based channel
        }
    }
}

void SequencerEngine::sendNoteOffEvents(juce::MidiBuffer& midiBuffer, int offset)
{
    // Send note-off messages for all active notes in the current step
    for (int row = 0; row < numRows; ++row)
    {
        if (sequencerGrid[currentStep][row])
        {
            int midiNote = rowToMidiNote(row);
            int channel = 1;    // Default channel (MIDI channels are 1-based in UI, 0-based in MIDI messages)
            
            // Create MIDI message
            juce::MidiMessage message = juce::MidiMessage::noteOff(channel, midiNote);
            midiBuffer.addEvent(message, offset);
        }
    }
}
