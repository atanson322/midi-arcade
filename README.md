# midi.arcade - Interactive MIDI Sequencer

A futuristic, cyberpunk-inspired MIDI sequencer VST plugin and standalone application built with JUCE.

## Features

- Step-based sequencer with adjustable step length (4-64 steps)
- Key signature system with root note and scale selection
- Visual key filtering modes (highlight or lock)
- Scrollable piano roll view with note labels
- Real-time MIDI parameter readout
- MIDI output device selection for standalone mode
- DAW transport synchronization
- Cyberpunk-inspired visual design

## Building the Project

### Prerequisites

- Visual Studio 2022
- JUCE 7+

### Build Instructions

1. Open a command prompt in the project directory
2. Run the `rebuild.bat` script to clean, generate project files, and build the plugin
3. The build outputs will be located at:
   - Standalone application: `Builds\VisualStudio2022\x64\Debug\Standalone Plugin\MidiArcade.exe`
   - VST3 plugin: `Builds\VisualStudio2022\x64\Debug\VST3\MidiArcade.vst3`

Alternatively, you can:
1. Open the `MidiArcade.jucer` file with Projucer
2. Click "Save Project and Open in IDE"
3. Build the solution in Visual Studio

## Usage

### Standalone Application

1. Launch the standalone application
2. Select your MIDI output device from the dropdown
3. Use the grid to create patterns by clicking on cells
4. Press Play to start playback

### VST3 Plugin

1. Load the VST3 plugin in your DAW
2. Create patterns by clicking on the grid
3. The plugin will sync to your DAW's transport

## Project Structure

- **PluginProcessor**: Core audio processing and MIDI generation
- **PluginEditor**: Main UI component and layout
- **SequencerEngine**: Step sequencer logic and MIDI event generation
- **KeySignatureManager**: Musical scale and key filtering logic
- **MidiDeviceManager**: MIDI output device handling
- **SequencerGrid**: Visual grid representation and interaction
- **KeySignaturePanel**: UI for selecting musical key and scale
- **MidiInfoPanel**: Display for real-time MIDI event data
- **TransportController**: Playback control and transport sync

## License

Copyright (c) 2025 midi.arcade
