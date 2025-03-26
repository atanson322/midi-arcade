#include "SequencerGrid.h"

SequencerGrid::SequencerGrid(SequencerEngine* engine)
    : sequencerEngine(engine)
{
    // Start the timer for animations
    startTimerHz(30); // 30 fps for smooth animations
}

SequencerGrid::~SequencerGrid()
{
    stopTimer();
}

void SequencerGrid::paint(juce::Graphics& g)
{
    // Fill background
    g.fillAll(juce::Colour(0xFF101820));
    
    // Draw the grid components
    drawGrid(g);
    drawNoteLabels(g);
    drawCells(g);
    drawStepIndicator(g);
}

void SequencerGrid::resized()
{
    // Update cell width based on available width
    int gridWidth = getWidth() - noteNameWidth;
    cellWidth = gridWidth / sequencerEngine->getNumSteps();
}

void SequencerGrid::mouseDown(const juce::MouseEvent& e)
{
    int step, row;
    if (getCellFromMousePosition(e.getPosition(), step, row))
    {
        // Toggle the cell state
        bool currentState = sequencerEngine->getStep(step, row);
        sequencerEngine->setStep(step, row, !currentState);
        
        // Repaint to show the change
        repaint();
    }
}

void SequencerGrid::mouseDrag(const juce::MouseEvent& e)
{
    int step, row;
    if (getCellFromMousePosition(e.getPosition(), step, row))
    {
        // Set the cell state based on the initial cell that was clicked
        int initialStep, initialRow;
        if (getCellFromMousePosition(e.getMouseDownPosition(), initialStep, initialRow))
        {
            bool initialState = sequencerEngine->getStep(initialStep, initialRow);
            sequencerEngine->setStep(step, row, !initialState);
            
            // Repaint to show the change
            repaint();
        }
    }
}

void SequencerGrid::timerCallback()
{
    // Update pulse animation
    if (pulseIncreasing)
    {
        pulseAlpha += 0.05f;
        if (pulseAlpha >= 1.0f)
        {
            pulseAlpha = 1.0f;
            pulseIncreasing = false;
        }
    }
    else
    {
        pulseAlpha -= 0.05f;
        if (pulseAlpha <= 0.5f)
        {
            pulseAlpha = 0.5f;
            pulseIncreasing = true;
        }
    }
    
    // Repaint to update animations
    repaint();
}

void SequencerGrid::updateCurrentStep()
{
    // This is called from the editor to update the current step indicator
    repaint();
}

void SequencerGrid::drawGrid(juce::Graphics& g)
{
    // Draw vertical grid lines (step divisions)
    g.setColour(juce::Colour(0x30FFFFFF));
    
    for (int step = 0; step <= sequencerEngine->getNumSteps(); ++step)
    {
        int x = noteNameWidth + step * cellWidth;
        g.drawLine(x, 0, x, getHeight(), 1.0f);
    }
    
    // Draw horizontal grid lines (note divisions)
    for (int row = 0; row <= sequencerEngine->getNumRows(); ++row)
    {
        int y = row * rowHeight;
        g.drawLine(0, y, getWidth(), y, 1.0f);
    }
    
    // Draw a thicker line for the note name divider
    g.setColour(juce::Colour(0x80FFFFFF));
    g.drawLine(noteNameWidth, 0, noteNameWidth, getHeight(), 2.0f);
}

void SequencerGrid::drawNoteLabels(juce::Graphics& g)
{
    g.setColour(juce::Colour(0xFFCCFFFF));
    g.setFont(juce::Font("Consolas", 12.0f, juce::Font::bold));
    
    for (int row = 0; row < sequencerEngine->getNumRows(); ++row)
    {
        // Calculate the MIDI note number for this row
        int midiNote = sequencerEngine->getLowestNote() + (sequencerEngine->getNumRows() - 1 - row);
        
        // Get the note name (e.g., "C3", "F#4")
        juce::String noteName;
        
        static const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        int octave = midiNote / 12 - 1;
        int note = midiNote % 12;
        noteName = juce::String(noteNames[note]) + juce::String(octave);
        
        // Draw the note name
        juce::Rectangle<float> labelRect(0, row * rowHeight, noteNameWidth, rowHeight);
        g.drawText(noteName, labelRect, juce::Justification::centred, false);
    }
}

void SequencerGrid::drawStepIndicator(juce::Graphics& g)
{
    if (sequencerEngine->isSequencerPlaying())
    {
        int currentStep = sequencerEngine->getCurrentStep();
        
        // Draw a highlight for the current step
        g.setColour(juce::Colour(0x80FFFFFF));
        juce::Rectangle<float> stepRect(noteNameWidth + currentStep * cellWidth, 0,
                                      cellWidth, getHeight());
        g.fillRect(stepRect);
    }
}

void SequencerGrid::drawCells(juce::Graphics& g)
{
    for (int step = 0; step < sequencerEngine->getNumSteps(); ++step)
    {
        for (int row = 0; row < sequencerEngine->getNumRows(); ++row)
        {
            // Calculate the MIDI note number for this row
            int midiNote = sequencerEngine->getLowestNote() + (sequencerEngine->getNumRows() - 1 - row);
            
            // Get the cell state
            bool isActive = sequencerEngine->getStep(step, row);
            
            // Get the color based on key signature
            juce::Colour cellColor = sequencerEngine->getKeySignatureManager()->getNoteColor(midiNote);
            
            // Draw the cell
            juce::Rectangle<float> cellRect(noteNameWidth + step * cellWidth + 1, row * rowHeight + 1,
                                          cellWidth - 2, rowHeight - 2);
            
            if (isActive)
            {
                // Active cell - filled with color and pulsing
                g.setColour(cellColor.withAlpha(pulseAlpha));
                g.fillRect(cellRect);
                
                // Draw a border
                g.setColour(juce::Colours::white);
                g.drawRect(cellRect, 1.0f);
            }
            else
            {
                // Inactive cell - just an outline with key-based color
                g.setColour(cellColor.withAlpha(0.3f));
                g.drawRect(cellRect, 1.0f);
            }
        }
    }
}

bool SequencerGrid::getCellFromMousePosition(const juce::Point<int>& position, int& step, int& row)
{
    // Check if the position is within the grid area
    if (position.x < noteNameWidth)
        return false;
    
    // Calculate the step and row
    step = (position.x - noteNameWidth) / cellWidth;
    row = position.y / rowHeight;
    
    // Check if the step and row are valid
    return (step >= 0 && step < sequencerEngine->getNumSteps() &&
            row >= 0 && row < sequencerEngine->getNumRows());
}
