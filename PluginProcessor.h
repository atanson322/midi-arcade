#pragma once

#include <JuceHeader.h>
#include "SequencerEngine.h"
#include "MidiDeviceManager.h"

class MidiArcadeAudioProcessor : public juce::AudioProcessor
{
public:
    MidiArcadeAudioProcessor();
    ~MidiArcadeAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Sequencer control methods
    void startSequencer();
    void stopSequencer();
    bool isSequencerPlaying() const;
    
    // Access to the sequencer engine
    SequencerEngine* getSequencerEngine() { return &sequencerEngine; }
    MidiDeviceManager* getMidiDeviceManager() { return &midiDeviceManager; }

    // Parameters
    juce::AudioProcessorValueTreeState parameters;

private:
    // Sequencer engine
    SequencerEngine sequencerEngine;
    
    // MIDI device manager
    MidiDeviceManager midiDeviceManager;
    
    // Transport state
    bool isPlaying = false;
    
    // Parameters
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiArcadeAudioProcessor)
};
