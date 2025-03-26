#include "MidiDeviceManager.h"

MidiDeviceManager::MidiDeviceManager()
    : midiChannel(1)
{
    // Initialize with no output device
}

MidiDeviceManager::~MidiDeviceManager()
{
    // The unique_ptr will handle cleanup of the MIDI output
}

void MidiDeviceManager::setMidiOutput(const juce::MidiDeviceInfo& deviceInfo)
{
    // Close existing output
    midiOutput.reset();
    
    // Try to open the new device
    midiOutput = juce::MidiOutput::openDevice(deviceInfo.identifier);
    
    if (midiOutput != nullptr)
    {
        currentDeviceName = deviceInfo.name;
    }
    else
    {
        currentDeviceName = "No Output";
    }
}

void MidiDeviceManager::setMidiOutput(std::nullptr_t)
{
    // Close existing output
    midiOutput.reset();
    currentDeviceName = "No Output";
}

juce::String MidiDeviceManager::getCurrentDeviceName() const
{
    return currentDeviceName.isEmpty() ? "No Output" : currentDeviceName;
}

void MidiDeviceManager::setMidiChannel(int channel)
{
    // MIDI channels are 1-16 in the UI, but 0-15 in MIDI messages
    if (channel >= 1 && channel <= 16)
    {
        midiChannel = channel;
    }
}

void MidiDeviceManager::sendBlockOfMessages(const juce::MidiBuffer& buffer)
{
    // Only send if we have a valid output device
    if (midiOutput != nullptr)
    {
        // Convert the MIDI buffer to a MidiMessage sequence
        for (const auto metadata : buffer)
        {
            const auto message = metadata.getMessage();
            
            // If it's a channel message, ensure it's on our selected channel
            if (message.isForChannel(midiChannel))
            {
                midiOutput->sendMessageNow(message);
            }
            else if (message.getChannel() == 0) // If it's on channel 0, redirect to our channel
            {
                auto newMessage = message;
                newMessage.setChannel(midiChannel - 1); // Convert 1-based UI channel to 0-based MIDI channel
                midiOutput->sendMessageNow(newMessage);
            }
        }
    }
}

void MidiDeviceManager::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "midiChannel")
    {
        // Parameter value is 0-based index, but we want 1-based channel
        setMidiChannel(static_cast<int>(newValue) + 1);
    }
}
