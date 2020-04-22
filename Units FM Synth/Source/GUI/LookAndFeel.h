/*
  ==============================================================================

    LookAndFeel.h
    Created: 26 Oct 2019 10:04:54am
    Author:  Me

  ==============================================================================
*/

#pragma once

//Look and feel for editor window. Makes resizer invisible.
class FMSynthLookAndFeel : public juce::LookAndFeel_V4
{
	void drawResizableFrame(Graphics&, int, int, const BorderSize<int>&) override{}

	void drawCornerResizer(Graphics&, int, int, bool, bool) override {}
};
