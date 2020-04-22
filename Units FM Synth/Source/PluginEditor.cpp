/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
UnitsFmSynthAudioProcessorEditor::UnitsFmSynthAudioProcessorEditor (UnitsFmSynthAudioProcessor& p,
                                                                    std::reference_wrapper<std::atomic<ControlDisplayMode>> mode)
    : AudioProcessorEditor (p),
                  parent(p),
                  synthMode(mode),
                  cutoffAttachment(parent.getParameterTree(), "cutoff", *static_cast<Slider*>(cutoffSlider.getChildComponent(0))),
                  resonanceAttachment(parent.getParameterTree(), "resonance", *static_cast<Slider*>(resonanceSlider.getChildComponent(0))),
                  modulationDepthAttatchment(parent.getParameterTree(), "modDepth", *static_cast<Slider*>(modulationDepthSlider.getChildComponent(0))),
                  fmRatioAttachment(parent.getParameterTree(), "modRatio", *static_cast<Slider*>(fmRatioSlider.getChildComponent(0))),

                  attackAttachment(parent.getParameterTree(), "attack", *static_cast<Slider*>(midiControls.getChildComponent(0)->getChildComponent(0))),
                  decayAttachment(parent.getParameterTree(), "decay", *static_cast<Slider*>(midiControls.getChildComponent(1)->getChildComponent(0))),
                  sustainAttachment(parent.getParameterTree(), "sustain", *static_cast<Slider*>(midiControls.getChildComponent(2)->getChildComponent(0))),
                  releaseAttachment(parent.getParameterTree(), "release", *static_cast<Slider*>(midiControls.getChildComponent(3)->getChildComponent(0))),
                
                  pressureAttachment(parent.getParameterTree(), "pressureCF", *static_cast<Slider*>(mpeControls.getChildComponent(0)->getChildComponent(0))),
                  slideAttachment(parent.getParameterTree(), "slideCF", *static_cast<Slider*>(mpeControls.getChildComponent(1)->getChildComponent(0))),
                  gainAttachment(parent.getParameterTree(), "voiceGain", *static_cast<Slider*>(mpeControls.getChildComponent(2)->getChildComponent(0))),
                  mpeReleaseAttachment(parent.getParameterTree(), "mpeRelease", *static_cast<Slider*>(mpeControls.getChildComponent(3)->getChildComponent(0))),
#if JUCE_STANDALONE_APPLICATION
                  midiKeyboard (parent.getMIDIKeyboardState(), MidiKeyboardComponent::horizontalKeyboard)
#endif
            {
                setLookAndFeel(this);
                LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypefaceName ("Helvitica Neue");

                // add some sliders..
                addAndMakeVisible(&cutoffSlider);
                addAndMakeVisible(&resonanceSlider);
                addAndMakeVisible(&modulationDepthSlider);
                addAndMakeVisible(&fmRatioSlider);
                
                addAndMakeVisible(&envelopeWidgetContainer);
                
                envelopeWidgetContainer.addTab("Auto", backgroundColour, &midiControls, false);
                envelopeWidgetContainer.addTab("MIDI", backgroundColour, &midiControls, false);
                envelopeWidgetContainer.addTab("MPE" , backgroundColour, &mpeControls , false);
                
                envelopeWidgetContainer.setOutline(0);
                
                // add the midi keyboard component..
                addAndMakeVisible (&midiKeyboard);
                midiKeyboard.setScrollButtonsVisible(false);
                midiKeyboard.setKeyPressBaseOctave(currentOctave);

                //if the property tree is valid, initialize the ui dimensions and selected envelope
                // component to their previous values
                auto propertyTree = parent.getParameterTree().state.getChildWithName ("Properties");
                if(propertyTree.isValid()) {
                    lastUIWidth .referTo (propertyTree.getPropertyAsValue ("width",  nullptr));
                    lastUIHeight.referTo (propertyTree.getPropertyAsValue ("height", nullptr));
                    cachedMode  .referTo (propertyTree, "MIDIMode",  nullptr);
                }

                // set our component's initial size to be the last one that was stored in the filter's settings
                setSize (lastUIWidth.getValue(), lastUIHeight.getValue());
                
                // set resize limits for this plug-in
                setResizeLimits (900, 460, 1195, 700);

                envelopeWidgetContainer.getTabbedButtonBar().setCurrentTabIndex(cachedMode.get() - (cachedMode.get() > 0));
                envelopeWidgetContainer.getTabbedButtonBar().addChangeListener(this);
                
                //grab keyboard focus (usually doesn't work, but w/e)
                setWantsKeyboardFocus(true);
                setFocusContainer(true);
                
                addKeyListener(this);
                
                //start the timer that we use to sync the selected envelope mode to the synth state
                startTimer(50);
                changeListenerCallback(&envelopeWidgetContainer.getTabbedButtonBar());
                envelopeWidgetContainer.setCurrentTabIndex(0, false);
            }

UnitsFmSynthAudioProcessorEditor::~UnitsFmSynthAudioProcessorEditor() {setLookAndFeel(nullptr);}

//==============================================================================
void UnitsFmSynthAudioProcessorEditor::paint (Graphics& g) {
      g.fillAll(backgroundColour.brighter(.65f));
      g.setColour(backgroundColour);
      g.fillRect(topBackingPanel);
}

void UnitsFmSynthAudioProcessorEditor::resized() {
    // This lays out our child components...
    auto localBounds = getLocalBounds();

    resizeKeyboardComponent(localBounds);

    auto r = localBounds.reduced(5);

    const auto topSliderRowBounds = r.removeFromTop(roundToInt(
                                                      (r.getHeight()-envelopeWidgetContainer
                                                                     .getTabBarDepth())/2.0));

    topBackingPanel = topSliderRowBounds.reduced(5);

    const auto row1WidgetWidth= std::max(topSliderRowBounds.getWidth() / 4.0f, 150.0f);
    const auto row1WidgetHeight = static_cast<float>(std::min(topSliderRowBounds.getHeight(), 150));
    
    //make a flex box for the top row of sliders
    const auto makeFlexBox = [](const auto& width, const auto& height, auto&& argTuple){
        FlexBox box{ FlexBox::Direction::row, FlexBox::Wrap::noWrap,       FlexBox::AlignContent::center,
                                              FlexBox::AlignItems::center, FlexBox::JustifyContent::spaceBetween };
        std::apply([&](auto&&... args) {(box.items.add(FlexItem(args).withMinWidth(width).withMinHeight(height)), ...);}, argTuple);
        return box;
    };
    
    auto timbreControlsContainer = makeFlexBox(row1WidgetWidth, row1WidgetHeight, std::forward_as_tuple(modulationDepthSlider, fmRatioSlider, cutoffSlider, resonanceSlider));

    //actually position the components
    timbreControlsContainer.performLayout(topSliderRowBounds);
    
    const auto bottomSliderRowBounds = r.reduced(5);
    envelopeWidgetContainer.setBounds(bottomSliderRowBounds.toNearestInt());
    
    //set saved ui state
    lastUIWidth  = getWidth();
    lastUIHeight = getHeight();
}

//handle un/redo and keyboard component key presses
bool UnitsFmSynthAudioProcessorEditor::keyPressed(const KeyPress& press, Component*) {
    if (press == KeyPress('z', ModifierKeys::commandModifier, '\n')) {
        if (parent.getParameterTree().undoManager->canUndo())
            parent.getParameterTree().undoManager->undo();
        return true;
    }
    else if (press ==
             KeyPress('z', (ModifierKeys::commandModifier + ModifierKeys::shiftModifier), '\n')) {
        if (parent.getParameterTree().undoManager->canRedo())
            parent.getParameterTree().undoManager->redo();
        return true;
    }
    if(parent.wrapperType == AudioProcessor::WrapperType::wrapperType_Standalone) {
        if (press == KeyPress('z')) {
            midiKeyboard.focusLost(FocusChangeType::focusChangedDirectly);
            midiKeyboard.setKeyPressBaseOctave(jlimit(0, 10, --currentOctave));
        }
        else if (press == KeyPress('x')) {
            midiKeyboard.focusLost(FocusChangeType::focusChangedDirectly);
            midiKeyboard.setKeyPressBaseOctave(jlimit(0, 10, ++currentOctave));
        }
        else if (press == KeyPress('c')) {
            midiKeyboard.focusLost(FocusChangeType::focusChangedDirectly);
            midiKeyboard.setVelocity(velocity = std::max(0.0f, velocity -= 5.0f/127.0f), true);
        }
        else if (press == KeyPress('v')) {
            midiKeyboard.focusLost(FocusChangeType::focusChangedDirectly);
            midiKeyboard.setVelocity(velocity = std::min(1.0f, velocity += 5.0f / 127.0f), true);
        }
        else if (press == KeyPress('c', ModifierKeys::shiftModifier, '\n')) {
            midiKeyboard.focusLost(FocusChangeType::focusChangedDirectly);
            midiKeyboard.setVelocity(velocity = std::max(0.0f, velocity -= 1.0f/127.0f), true);
        }
        else if (press == KeyPress('v', ModifierKeys::shiftModifier, '\n')) {
            midiKeyboard.focusLost(FocusChangeType::focusChangedDirectly);
            midiKeyboard.setVelocity(velocity = std::min(1.0f, velocity += 1.0f / 127.0f), true);
        }
        else
            return false;
        return true;
    }

    return false;
}

bool UnitsFmSynthAudioProcessorEditor::keyStateChanged(bool isKeyDown, Component*) {
    midiKeyboard.keyStateChanged(isKeyDown);
    return isKeyDown;
}

// if the tab bar is in auto mode, make sure the correct set of controls is displayed
//  if the most recent synth played was the mpe synth, display the mpe controls
//  if the most recent is the midi, display the midi controls.
// if this is a standalone, grab keyboard focus
void UnitsFmSynthAudioProcessorEditor::timerCallback() {
    auto propertyTree = parent.getParameterTree().state.getChildWithName("Properties");
    if(synthMode.get().load() == ControlDisplayMode::AutoMIDI && previousMode != ControlDisplayMode::AutoMIDI) {
        envelopeWidgetContainer.removeTab(0);
        envelopeWidgetContainer.addTab ("Auto", backgroundColour, &midiControls, false, 0);
        previousMode = ControlDisplayMode::AutoMIDI;
        if(propertyTree.isValid())
            cachedMode.setValue(0, nullptr);
    }
    else if(synthMode.get().load() == ControlDisplayMode::AutoMPE && previousMode != ControlDisplayMode::AutoMPE) {
        envelopeWidgetContainer.removeTab(0);
        envelopeWidgetContainer.addTab ("Auto", backgroundColour, &mpeControls, false, 0);
        previousMode = ControlDisplayMode::AutoMPE;
        if (propertyTree.isValid())
            cachedMode.setValue(1, nullptr);
    }
//            else if(envelopeWidgetContainer.getCurrentTabIndex() != cachedMode.get()-1)
//                envelopeWidgetContainer.setCurrentTabIndex(std::max(cachedMode.get()-1, 0));

    if (parent.wrapperType == AudioProcessor::WrapperType::wrapperType_Standalone
        && getTopLevelComponent() == DialogWindow::getActiveTopLevelWindow()
        && !hasKeyboardFocus(true))
            grabKeyboardFocus();
}

//update the synth mode to
void UnitsFmSynthAudioProcessorEditor::changeListenerCallback (ChangeBroadcaster*) {
    auto propertyTree = parent.getParameterTree().state.getChildWithName("Properties");
    if(envelopeWidgetContainer.getCurrentTabIndex() == 0) {
        synthMode.get().store(ControlDisplayMode::AutoMIDI);
        if (propertyTree.isValid())
            cachedMode.setValue(0, parent.getParameterTree().undoManager);
    }
    else if(envelopeWidgetContainer.getCurrentTabIndex() == 1) {
        synthMode.get().store(ControlDisplayMode::MIDI);
        if (propertyTree.isValid())
            cachedMode.setValue(2, parent.getParameterTree().undoManager);
    }
    else if(envelopeWidgetContainer.getCurrentTabIndex() == 2) {
        synthMode.get().store(ControlDisplayMode::MPE);
        if (propertyTree.isValid())
            cachedMode.setValue(3, parent.getParameterTree().undoManager);
    }
    
    previousMode = synthMode.get();
    if(propertyTree.isValid())
        propertyTree.setProperty("MIDIMode", cachedMode.get(), nullptr);
}

