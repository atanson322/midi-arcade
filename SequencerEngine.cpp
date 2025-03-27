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
    timeSignatureNumerator = 4;
    timeSignatureDenominator = 4;
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
    // Set grid dimensions
    numSteps = steps;
    numRows = rows;
    
    // Initialize the grid with all steps off
    sequencerGrid.resize(numSteps);
    for (int i = 0; i < numSteps; ++i)
    {
        sequencerGrid[i].resize(numRows, false);
    }
    
    // Initialize with default values
    currentStep = 0;
    isPlaying = false;
    
    // Default timing values - these will be updated from the DAW
    bpm = 120.0;
    timeSignatureNumerator = 4;
    timeSignatureDenominator = 4;
    
    // Update timing calculations
    updateStepLength();
}

void SequencerEngine::prepareToPlay(double newSampleRate, int samplesPerBlock)
{
    sampleRate = newSampleRate;
    updateStepLength();
}

void SequencerEngine::updateStepLength()
{
    // Calculate samples per step based on BPM, time signature, and sample rate
    if (bpm > 0.0 && sampleRate > 0.0)
    {
        // Calculate beats per minute to beats per second
        double beatsPerSecond = bpm / 60.0;
        
        // Calculate seconds per beat
        double secondsPerBeat = 1.0 / beatsPerSecond;
        
        // Calculate beats per bar based on time signature
        double beatsPerBar = timeSignatureNumerator * (4.0 / timeSignatureDenominator);
        
        // Calculate steps per beat based on our grid size and time signature
        double stepsPerBar = numSteps;
        double stepsPerBeat = stepsPerBar / beatsPerBar;
        
        // Calculate seconds per step
        double secondsPerStep = secondsPerBeat / stepsPerBeat;
        
        // Calculate samples per step
        samplesPerStep = secondsPerStep * sampleRate;
        
        DBG("Updated timing - BPM: " + juce::String(bpm) + 
            " Time Sig: " + juce::String(timeSignatureNumerator) + "/" + juce::String(timeSignatureDenominator) + 
            " Samples per step: " + juce::String(samplesPerStep));
    }
}

void SequencerEngine::updatePlayheadPosition(const juce::AudioPlayHead::CurrentPositionInfo& posInfo)
{
    // Update timing information from the DAW
    if (posInfo.bpm > 0.0)
    {
        // Only update if the BPM has changed to avoid unnecessary calculations
        if (bpm != posInfo.bpm)
        {
            bpm = posInfo.bpm;
            DBG("BPM updated from DAW: " + juce::String(bpm));
            updateStepLength();
        }
    }
    
    // Update time signature if available
    if (posInfo.timeSigNumerator > 0 && posInfo.timeSigDenominator > 0)
    {
        if (timeSignatureNumerator != posInfo.timeSigNumerator || 
            timeSignatureDenominator != posInfo.timeSigDenominator)
        {
            timeSignatureNumerator = posInfo.timeSigNumerator;
            timeSignatureDenominator = posInfo.timeSigDenominator;
            DBG("Time signature updated from DAW: " + 
                juce::String(timeSignatureNumerator) + "/" + 
                juce::String(timeSignatureDenominator));
        }
    }
    
    // Calculate which step we should be on based on PPQ position
    if (posInfo.isPlaying && posInfo.ppqPosition >= 0.0)
    {
        // Calculate beats per bar based on time signature
        double beatsPerBar = timeSignatureNumerator * (4.0 / timeSignatureDenominator);
        
        // Calculate steps per beat based on our grid size and time signature
        double stepsPerBar = numSteps;
        double stepsPerBeat = stepsPerBar / beatsPerBar;
        double ppqPerStep = 1.0 / stepsPerBeat;
        
        // Calculate current position within a pattern cycle
        double adjustedPpq = posInfo.ppqPosition;
        
        // Get the step index within our pattern
        int newStep = static_cast<int>(std::fmod(adjustedPpq / ppqPerStep, numSteps));
        
        // If the step has changed, update
        if (newStep != currentStep)
        {
            // When jumping to a new position or starting playback
            if (std::abs(lastPPQPosition - posInfo.ppqPosition) > ppqPerStep || !isPlaying)
            {
                // Calculate how far we are into the current step (0.0 to 1.0)
                double stepPhase = std::fmod(adjustedPpq / ppqPerStep, 1.0);
                
                // Set sample counter based on phase within the step
                sampleCounter = stepPhase * samplesPerStep;
                
                // Debug output
                DBG("Transport jump detected! PPQ: " + juce::String(posInfo.ppqPosition) + 
                    " Step: " + juce::String(newStep) + 
                    " Phase: " + juce::String(stepPhase));
            }
            else
            {
                // Normal step change during playback
                sampleCounter = 0.0;
            }
            
            currentStep = newStep;
            
            // Debug output
            DBG("Step changed to: " + juce::String(currentStep) + 
                " at PPQ: " + juce::String(posInfo.ppqPosition) + 
                " BPM: " + juce::String(bpm));
        }
        
        lastPPQPosition = posInfo.ppqPosition;
    }
}

void SequencerEngine::processBlock(juce::MidiBuffer& midiBuffer, int numSamples)
{
    if (!isPlaying || bpm <= 0.0)
        return;
    
    // Debug output
    DBG("Processing block: " + juce::String(numSamples) + " samples, currentStep: " + 
        juce::String(currentStep) + ", sampleCounter: " + juce::String(sampleCounter));
    
    // Calculate how many samples we need to process
    int samplePosition = 0;
    
    while (samplePosition < numSamples)
    {
        // Calculate samples to next step, avoiding division by zero
        double effectiveSamplesPerStep = samplesPerStep > 0.0 ? samplesPerStep : sampleRate / 4.0;
        
        // Calculate remaining samples in current step
        int samplesInCurrentStep = static_cast<int>(effectiveSamplesPerStep - sampleCounter);
        
        // Ensure we don't go negative
        if (samplesInCurrentStep < 0)
        {
            // We're past the step boundary, reset counter
            sampleCounter = 0.0;
            samplesInCurrentStep = static_cast<int>(effectiveSamplesPerStep);
            
            // Debug output
            DBG("Sample counter reset - was negative");
        }
        
        int samplesToNextStep = juce::jmin(
            numSamples - samplePosition,
            samplesInCurrentStep
        );
        
        // If we've reached the next step or we're exactly at a step boundary (sampleCounter == 0)
        if ((sampleCounter + samplesToNextStep >= effectiveSamplesPerStep) || 
            (sampleCounter == 0.0 && samplesToNextStep > 0))
        {
            int offsetToNextStep;
            
            // Handle case where we're exactly at a step boundary
            if (sampleCounter == 0.0)
            {
                // We're exactly at the step boundary, trigger immediately
                offsetToNextStep = 0;
                
                // Debug output
                DBG("Exactly at step boundary - triggering immediately");
                
                // Send note-on events for the current step
                sendNoteOnEvents(midiBuffer, samplePosition);
            }
            else
            {
                // Calculate the exact sample offset for the step change
                offsetToNextStep = static_cast<int>(effectiveSamplesPerStep - sampleCounter);
                
                // Send note-off events for the current step
                sendNoteOffEvents(midiBuffer, samplePosition + offsetToNextStep);
                
                // Advance to the next step
                advanceStep();
                
                // Send note-on events for the new step
                sendNoteOnEvents(midiBuffer, samplePosition + offsetToNextStep);
                
                // Debug output
                DBG("Step advanced to: " + juce::String(currentStep) + 
                    " at offset: " + juce::String(offsetToNextStep));
            }
            
            // Reset sample counter
            sampleCounter = 0.0;
            
            // Move forward by the offset amount
            samplePosition += offsetToNextStep > 0 ? offsetToNextStep : samplesToNextStep;
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
    state.setProperty("timeSignatureNumerator", timeSignatureNumerator, nullptr);
    state.setProperty("timeSignatureDenominator", timeSignatureDenominator, nullptr);
    
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
    timeSignatureNumerator = state.getProperty("timeSignatureNumerator", 4);
    timeSignatureDenominator = state.getProperty("timeSignatureDenominator", 4);
    
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
