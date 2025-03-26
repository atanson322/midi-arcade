#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "MidiDeviceManager.cpp" // Include implementation directly to avoid linker errors

MidiArcadeAudioProcessor::MidiArcadeAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    // Initialize sequencer engine
    sequencerEngine.initialize(16, 16); // Default to 16 steps, 16 notes
    
    // Set up parameter listeners
    parameters.addParameterListener("rootNote", &sequencerEngine);
    parameters.addParameterListener("scaleType", &sequencerEngine);
    parameters.addParameterListener("keyFilterMode", &sequencerEngine);
    parameters.addParameterListener("midiChannel", &midiDeviceManager);
    parameters.addParameterListener("numSteps", &sequencerEngine);
}

MidiArcadeAudioProcessor::~MidiArcadeAudioProcessor()
{
    stopSequencer();
}

juce::AudioProcessorValueTreeState::ParameterLayout MidiArcadeAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    // Root note selection (C, C#, D, etc.)
    juce::StringArray rootNotes = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    layout.add(std::make_unique<juce::AudioParameterChoice>("rootNote", "Root Note", rootNotes, 0));
    
    // Scale type selection (Major, Minor)
    juce::StringArray scaleTypes = {"Major", "Minor"};
    layout.add(std::make_unique<juce::AudioParameterChoice>("scaleType", "Scale Type", scaleTypes, 0));
    
    // Key filter mode (Highlight, Lock)
    juce::StringArray keyFilterModes = {"Highlight", "Lock"};
    layout.add(std::make_unique<juce::AudioParameterChoice>("keyFilterMode", "Key Filter Mode", keyFilterModes, 0));
    
    // MIDI channel selection (1-16)
    juce::StringArray midiChannels;
    for (int i = 1; i <= 16; ++i)
        midiChannels.add(juce::String(i));
    layout.add(std::make_unique<juce::AudioParameterChoice>("midiChannel", "MIDI Channel", midiChannels, 0));
    
    // Number of steps (4-64)
    juce::StringArray stepCounts;
    for (int i = 4; i <= 64; i += 4)
        stepCounts.add(juce::String(i));
    layout.add(std::make_unique<juce::AudioParameterChoice>("numSteps", "Number of Steps", stepCounts, 3)); // Default to 16 steps
    
    return layout;
}

void MidiArcadeAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Prepare sequencer with sample rate
    sequencerEngine.prepareToPlay(sampleRate, samplesPerBlock);
}

void MidiArcadeAudioProcessor::releaseResources()
{
    // Release any resources
    sequencerEngine.releaseResources();
}

bool MidiArcadeAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // We only support stereo or mono
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void MidiArcadeAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // Clear the output audio buffer
    buffer.clear();
    
    // Get current playhead info
    juce::AudioPlayHead* playHead = getPlayHead();
    juce::AudioPlayHead::CurrentPositionInfo posInfo;
    
    if (playHead != nullptr && playHead->getCurrentPosition(posInfo))
    {
        // Update sequencer with current playhead position
        sequencerEngine.updatePlayheadPosition(posInfo);
        
        // Update playing state based on host transport
        if (posInfo.isPlaying && !isPlaying)
            startSequencer();
        else if (!posInfo.isPlaying && isPlaying)
            stopSequencer();
    }
    
    // Process sequencer to generate MIDI events
    if (isPlaying)
    {
        sequencerEngine.processBlock(midiMessages, buffer.getNumSamples());
    }
    
    // In standalone mode, route MIDI to selected output device
    if (wrapperType == wrapperType_Standalone)
    {
        midiDeviceManager.sendBlockOfMessages(midiMessages);
    }
}

void MidiArcadeAudioProcessor::startSequencer()
{
    sequencerEngine.start();
    isPlaying = true;
}

void MidiArcadeAudioProcessor::stopSequencer()
{
    sequencerEngine.stop();
    isPlaying = false;
}

bool MidiArcadeAudioProcessor::isSequencerPlaying() const
{
    return isPlaying;
}

juce::AudioProcessorEditor* MidiArcadeAudioProcessor::createEditor()
{
    return new MidiArcadeAudioProcessorEditor(*this);
}

bool MidiArcadeAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String MidiArcadeAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MidiArcadeAudioProcessor::acceptsMidi() const
{
    return true;
}

bool MidiArcadeAudioProcessor::producesMidi() const
{
    return true;
}

bool MidiArcadeAudioProcessor::isMidiEffect() const
{
    return true;
}

double MidiArcadeAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MidiArcadeAudioProcessor::getNumPrograms()
{
    return 1;
}

int MidiArcadeAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MidiArcadeAudioProcessor::setCurrentProgram(int index)
{
    // No programs to set
}

const juce::String MidiArcadeAudioProcessor::getProgramName(int index)
{
    return {};
}

void MidiArcadeAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    // No programs to rename
}

void MidiArcadeAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Save parameters and sequencer state
    auto state = parameters.copyState();
    
    // Add sequencer grid state
    juce::ValueTree sequencerState = sequencerEngine.getState();
    state.addChild(sequencerState, -1, nullptr);
    
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void MidiArcadeAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Restore parameters and sequencer state
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
    {
        auto state = juce::ValueTree::fromXml(*xmlState);
        parameters.replaceState(state);
        
        // Find and restore sequencer grid state
        for (int i = 0; i < state.getNumChildren(); ++i)
        {
            juce::ValueTree child = state.getChild(i);
            if (child.hasType("SequencerState"))
            {
                sequencerEngine.setState(child);
                break;
            }
        }
    }
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MidiArcadeAudioProcessor();
}
