#pragma once

#include <JuceHeader.h>
#include "SequencerEngine.h"

class SequencerGrid : public juce::Component,
                      public juce::Timer
{
public:
    SequencerGrid(SequencerEngine* engine);
    ~SequencerGrid() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    
    void timerCallback() override;
    
    // Update the current step indicator (called from editor)
    void updateCurrentStep();
    
    // Get the height of a single row
    int getRowHeight() const { return rowHeight; }
    
    // Get the number of rows
    int getNumRows() const { return sequencerEngine->getNumRows(); }
    
private:
    SequencerEngine* sequencerEngine;
    
    // Grid appearance
    int cellWidth = 30;
    int rowHeight = 30;
    int noteNameWidth = 50;
    
    // Helper methods
    void drawGrid(juce::Graphics& g);
    void drawNoteLabels(juce::Graphics& g);
    void drawStepIndicator(juce::Graphics& g);
    void drawCells(juce::Graphics& g);
    
    // Convert mouse position to grid coordinates
    bool getCellFromMousePosition(const juce::Point<int>& position, int& step, int& row);
    
    // Animation properties
    float pulseAlpha = 0.0f;
    bool pulseIncreasing = true;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SequencerGrid)
};
