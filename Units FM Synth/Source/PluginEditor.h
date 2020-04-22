/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

#include "DSP/Synth.h"
#include "DSP/SmoothedValueBuffer.h"

#include "GUI/Knobs.h"
#include "GUI/LookAndFeel.h"
#include "GUI/TabbedContainers.h"

#include "../../Examples/AudioParameterUnit.h"

//==============================================================================
class UnitsFmSynthAudioProcessorEditor  : public AudioProcessorEditor, public KeyListener, public FMSynthLookAndFeel, public ChangeListener, public Timer
{
public:
    UnitsFmSynthAudioProcessorEditor (UnitsFmSynthAudioProcessor&, std::reference_wrapper<std::atomic<ControlDisplayMode>>);
    ~UnitsFmSynthAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    bool keyPressed(const KeyPress& press, Component*) override;

    bool keyStateChanged(bool isKeyDown, Component*) override;
        
    
    // sync the envelope widget state with the processor's synth state
    void changeListenerCallback (ChangeBroadcaster*) override;
    
    // update the knobs shown in the auto tab based on which synth is active.
    // In addition, make sure the editor receives key presses in standalone mode
    void timerCallback() override;
    
    //resize the piano keyboard at the bottom of the window (if this is a standalone)
    void resizeKeyboardComponent(Rectangle<int>& localBounds) {
        #if JUCE_STANDALONE_APPLICATION // || JucePlugin_Build_Unity || JucePlugin_Build_IAA
            if(parent.wrapperType == AudioProcessor::WrapperType::wrapperType_Standalone) {
                const auto numKeys = 12.0f * /*std::ceil*/((static_cast<float>(localBounds.getWidth()) / static_cast<float>(midiKeyboard.getKeyWidth())) / 7.0f);
                const std::pair keyRange{ static_cast<int>(std::max(0.0f, 60 - numKeys * .5f)), static_cast<int>(std::min(127.0f, 63 + numKeys * .5f)) };
                //midiKeyboard.setScrollButtonsVisible(keyRange.first > 11 || keyRange.second < 120);
                midiKeyboard.setAvailableRange(keyRange.first, keyRange.second);

                midiKeyboard.setBounds(localBounds.removeFromBottom(70));
            }
        #endif
    }

private:
	UnitsFmSynthAudioProcessor& parent;
    
    //Holder for the processor's midi mode.
    std::reference_wrapper<std::atomic<ControlDisplayMode>> synthMode;
    //Reference to the value tree's midi mode. Used to update the stored midi mode value when it is changed
    CachedValue<int> cachedMode;
    //Previous mode
    ControlDisplayMode previousMode{ControlDisplayMode::AutoMIDI};
    
    //Tabbed Display for the bottom set of knobs
    CenteredTabComponent envelopeWidgetContainer{TabbedButtonBar::Orientation::TabsAtTop};
    
    //Containers for the MIDI and MPE Synth Controls
    TabbedContainerComponent
        midiControls {std::unique_ptr<Component>(new LabeledKnob<FMSynthKnob>("Attack Time", " s")),
                      std::unique_ptr<Component>(new LabeledKnob<FMSynthKnob>("Decay Time", " s")),
                      std::unique_ptr<Component>(new LabeledKnob<FMSynthKnob>("Sustain Level", "dB")),
                      std::unique_ptr<Component>(new LabeledKnob<FMSynthKnob>("Release Time", " s"))},
        mpeControls  {std::unique_ptr<Component>(new LabeledKnob<FMSynthKnob>("Pressure Amount", "%")),
                      std::unique_ptr<Component>(new LabeledKnob<FMSynthKnob>("Slide Amount", "%")),
                      std::unique_ptr<Component>(new LabeledKnob<FMSynthKnob>("Sustain Level", "dB")),
                      std::unique_ptr<Component>(new LabeledKnob<FMSynthKnob>("Release Time", " s"))};

    //Filter and FM knobs
    LabeledKnob<FMSynthKnob> cutoffSlider{"Filter Cutoff", "Hz"},
                             resonanceSlider{ "Filter Resonance", "%" },
                             modulationDepthSlider{ "FM Mod Depth", "%" },
                             fmRatioSlider{ "FM Ratio", "%" };
    //Slider<->State Attatchments
    AudioProcessorValueTreeState::SliderAttachment
                                cutoffAttachment, resonanceAttachment,
                                modulationDepthAttatchment,
                                fmRatioAttachment,
                                attackAttachment, decayAttachment,
                                sustainAttachment, releaseAttachment,
                                pressureAttachment,   slideAttachment,
                                gainAttachment,    mpeReleaseAttachment;
    
    const Colour backgroundColour = (Colours::white).darker(.3f);
    Rectangle<int> topBackingPanel{};


#if JUCE_STANDALONE_APPLICATION // || JucePlugin_Build_Unity || JucePlugin_Build_IAA
    MidiKeyboardComponent midiKeyboard{parent.getMIDIKeyboardState(), MidiKeyboardComponent::horizontalKeyboard};
	int currentOctave = 6;
	float velocity = 0.5f;
#endif


    // these are used to persist the UI's size - the values are stored along with the
    // filter's other parameters, and the UI component will update them when it gets
    // resized.
    Value lastUIWidth, lastUIHeight;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UnitsFmSynthAudioProcessorEditor)
};
