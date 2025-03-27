#include "SequencerEngine.h"

SequencerEngine::SequencerEngine()
{
    // Initialize with default values
    numSteps = 16;
    numRows = 16;
    lowestNote = 48; // C3
    currentStep = 0;
    isPlaying = false;
    sampleRate = 44100.0;
    samplesPerStep = 0.0;
    sampleCounter = 0.0;
    bpm = 120.0;
    resolutionMultiplier = NORMAL_TIME;
    lastPPQPosition = 0.0;
    initialize(numSteps, numRows);
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
    rootNote = 0; // C
    scaleIntervals = {0, 3, 5, 7, 10}; // Minor pentatonic
    
    // Reset playback state
    currentStep = 0;
    isPlaying = false;
    sampleCounter = 0.0;
    lastPPQPosition = 0.0;
    bpm = 120.0;
    stepsPerBeat = 4;
    sampleRate = 44100.0;
    samplesPerStep = 0.0;
    resolutionMultiplier = NORMAL_TIME;
}

void SequencerEngine::prepareToPlay(double newSampleRate, int samplesPerBlock)
{
    sampleRate = newSampleRate;
    updateStepLength();
}

void SequencerEngine::updateStepLength()
{
    // Calculate samples per step based on BPM and time signature
    // (numSteps normally represents a whole bar in 4/4)
    double beatsPerBar = 4.0; // 4/4 time signature
    double stepsPerBeat = numSteps / beatsPerBar;
    double secondsPerBeat = 60.0 / bpm;
    double secondsPerStep = secondsPerBeat / stepsPerBeat;
    
    // Apply resolution multiplier
    switch (resolutionMultiplier)
    {
        case HALF_TIME:
            secondsPerStep *= 2.0;
            break;
        case DOUBLE_TIME:
            secondsPerStep *= 0.5;
            break;
        case NORMAL_TIME:
        default:
            break;
    }
    
    samplesPerStep = secondsPerStep * sampleRate;
}

void SequencerEngine::updatePlayheadPosition(const juce::AudioPlayHead::CurrentPositionInfo& posInfo)
{
    // Update BPM from host
    if (posInfo.bpm > 0.0)
    {
        bpm = posInfo.bpm;
        updateStepLength();
    }
    
    // Calculate which step we should be on based on PPQ position
    if (posInfo.isPlaying && posInfo.ppqPosition >= 0.0)
    {
        // Calculate beats per step in 4/4 time
        double beatsPerBar = 4.0;
        double stepsPerBeat = numSteps / beatsPerBar;
        double ppqPerStep = 1.0 / stepsPerBeat;
        
        // Calculate current position within a pattern cycle
        double adjustedPpq = posInfo.ppqPosition;
        
        // Get the step index within our pattern
        int newStep = static_cast<int>(std::fmod(adjustedPpq / ppqPerStep, numSteps));
        
        // If the step has changed, update
        if (newStep != currentStep)
        {
            currentStep = newStep;
            sampleCounter = 0.0; // Reset counter for tighter timing
        }
    }
}

void SequencerEngine::processBlock(juce::MidiBuffer& midiBuffer, int numSamples)
{
    if (!isPlaying || bpm <= 0.0)
        return;
    
    // Calculate how many samples we need to process
    int samplePosition = 0;
    
    while (samplePosition < numSamples)
    {
        // Calculate samples to next step, avoiding division by zero
        double effectiveSamplesPerStep = samplesPerStep > 0.0 ? samplesPerStep : sampleRate / 4.0;
        
        int samplesToNextStep = juce::jmin(
            numSamples - samplePosition,
            static_cast<int>(effectiveSamplesPerStep - sampleCounter)
        );
        
        // If we've reached the next step
        if (sampleCounter + samplesToNextStep >= effectiveSamplesPerStep)
        {
            // Calculate the exact sample offset for the step change
            int offsetToNextStep = static_cast<int>(effectiveSamplesPerStep - sampleCounter);
            
            // Send note-off events for the current step
            sendNoteOffEvents(midiBuffer, samplePosition + offsetToNextStep);
            
            // Advance to the next step
            advanceStep();
            
            // Send note-on events for the new step
            sendNoteOnEvents(midiBuffer, samplePosition + offsetToNextStep);
            
            // Reset sample counter
            sampleCounter = 0.0;
            
            // Move forward by the offset amount
            samplePosition += offsetToNextStep;
        }
        else
        {
            // Update sample counter
            sampleCounter += samplesToNextStep;
            samplePosition += samplesToNextStep;
        }
    }
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
            
            // JUCE uses 0-based MIDI channels (0-15)
            int channel = 0; // MIDI channel 1
            
            // Create and add the MIDI message
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
            int channel = 0; // MIDI channel 1
            
            // Create and add the MIDI message
            juce::MidiMessage message = juce::MidiMessage::noteOff(channel, midiNote);
            midiBuffer.addEvent(message, offset);
        }
    }
}

void SequencerEngine::advanceStep()
{
    // Move to the next step
    currentStep = (currentStep + 1) % numSteps;
}

int SequencerEngine::rowToMidiNote(int row) const
{
    // Convert row index to MIDI note (bottom row = lowest note)
    return lowestNote + (numRows - 1 - row);
}

juce::String SequencerEngine::midiNoteToName(int noteNumber) const
{
    // Convert MIDI note number to name (e.g., C3, D#4)
    static const char* notes[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    int note = noteNumber % 12;
    int octave = noteNumber / 12 - 1;
    return juce::String(notes[note]) + juce::String(octave);
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
    lastPPQPosition = 0.0;
}

// Grid manipulation
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
        sequencerGrid[step][row] = state;
    }
}

void SequencerEngine::clearAllSteps()
{
    for (int step = 0; step < numSteps; ++step)
    {
        for (int row = 0; row < numRows; ++row)
        {
            sequencerGrid[step][row] = false;
        }
    }
}

// Octave shifting methods
void SequencerEngine::shiftOctaveUp()
{
    lowestNote += 12;
    // Make sure we don't go beyond MIDI note range
    if (lowestNote > 108)
        lowestNote = 108;
}

void SequencerEngine::shiftOctaveDown()
{
    lowestNote -= 12;
    // Make sure we don't go below MIDI note range
    if (lowestNote < 0)
        lowestNote = 0;
}

int SequencerEngine::getCurrentOctave() const
{
    // Calculate current octave based on lowest note
    return lowestNote / 12 - 1;
}

// Resolution control
void SequencerEngine::setResolutionMultiplier(ResolutionMultiplier multiplier)
{
    resolutionMultiplier = multiplier;
    updateStepLength();
}

// Random sequence generation
void SequencerEngine::generateRandomSequence()
{
    // Clear existing sequence
    clearAllSteps();
    
    // Use true randomness
    juce::Random random;
    random.setSeedRandomly();
    
    // Set random notes
    for (int step = 0; step < numSteps; ++step)
    {
        // About 1/4 of the grid cells will be active
        int numActiveRows = random.nextInt(numRows / 4 + 1);
        
        for (int i = 0; i < numActiveRows; ++i)
        {
            int row = random.nextInt(numRows);
            setStep(step, row, true);
        }
    }
}

void SequencerEngine::releaseResources()
{
    // Stop playback when releasing resources
    isPlaying = false;
}

void SequencerEngine::parameterChanged(const juce::String& parameterID, float newValue)
{
    // Handle parameter changes from the processor
    if (parameterID == "bpm")
    {
        bpm = newValue;
        updateStepLength();
    }
    else if (parameterID == "resolution")
    {
        int resolutionIndex = static_cast<int>(newValue);
        setResolutionMultiplier(static_cast<ResolutionMultiplier>(resolutionIndex));
    }
}

juce::ValueTree SequencerEngine::getState() const
{
    // Create a ValueTree to store the sequencer state
    juce::ValueTree state("SEQUENCER_STATE");
    
    // Store grid dimensions
    state.setProperty("numSteps", numSteps, nullptr);
    state.setProperty("numRows", numRows, nullptr);
    state.setProperty("lowestNote", lowestNote, nullptr);
    state.setProperty("rootNote", rootNote, nullptr);
    state.setProperty("resolutionMultiplier", static_cast<int>(resolutionMultiplier), nullptr);
    
    // Store grid data
    juce::ValueTree gridData("GRID_DATA");
    
    for (int step = 0; step < numSteps; ++step)
    {
        juce::ValueTree stepData("STEP");
        stepData.setProperty("index", step, nullptr);
        
        juce::String activeRows;
        for (int row = 0; row < numRows; ++row)
        {
            if (sequencerGrid[step][row])
            {
                if (activeRows.isNotEmpty())
                    activeRows += ",";
                activeRows += juce::String(row);
            }
        }
        
        stepData.setProperty("activeRows", activeRows, nullptr);
        gridData.addChild(stepData, -1, nullptr);
    }
    
    state.addChild(gridData, -1, nullptr);
    
    return state;
}

void SequencerEngine::setState(const juce::ValueTree& state)
{
    // Only proceed if this is a valid sequencer state
    if (!state.hasType("SEQUENCER_STATE"))
        return;
    
    // Get grid dimensions
    int savedNumSteps = state.getProperty("numSteps", numSteps);
    int savedNumRows = state.getProperty("numRows", numRows);
    
    // Re-initialize if dimensions have changed
    if (savedNumSteps != numSteps || savedNumRows != numRows)
    {
        initialize(savedNumSteps, savedNumRows);
    }
    
    // Set properties
    lowestNote = state.getProperty("lowestNote", 48);
    rootNote = state.getProperty("rootNote", 60);
    resolutionMultiplier = static_cast<ResolutionMultiplier>(
        static_cast<int>(state.getProperty("resolutionMultiplier", static_cast<int>(NORMAL_TIME))));
    
    // Clear the grid
    clearAllSteps();
    
    // Load grid data
    juce::ValueTree gridData = state.getChildWithName("GRID_DATA");
    if (gridData.isValid())
    {
        for (int i = 0; i < gridData.getNumChildren(); ++i)
        {
            juce::ValueTree stepData = gridData.getChild(i);
            int step = stepData.getProperty("index", -1);
            
            if (step >= 0 && step < numSteps)
            {
                juce::String activeRows = stepData.getProperty("activeRows", "");
                if (activeRows.isNotEmpty())
                {
                    juce::StringArray rowsArray;
                    rowsArray.addTokens(activeRows, ",", "");
                    
                    for (int j = 0; j < rowsArray.size(); ++j)
                    {
                        int row = rowsArray[j].getIntValue();
                        if (row >= 0 && row < numRows)
                        {
                            setStep(step, row, true);
                        }
                    }
                }
            }
        }
    }
    
    // Update timing based on loaded settings
    updateStepLength();
}
