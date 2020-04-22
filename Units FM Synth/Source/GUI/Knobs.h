#pragma once

#include "Indicator.h"

//a class that styles a juce slider as a rotary knob with an indicator on the thumb dial
class FMSynthKnob : public Slider, public juce::LookAndFeel_V4
{
public:
	FMSynthKnob() : Slider(Slider::RotaryHorizontalVerticalDrag, Slider::NoTextBox) {
		setLookAndFeel(this);
		addAndMakeVisible(&valueIndicator);

		Slider::setColour(Slider::textBoxTextColourId, Colours::black);
		Slider::setColour(Slider::thumbColourId, Colours::darkgrey);
        Slider::setTextBoxIsEditable(true);
        Slider::setVelocityModeParameters(1.0, 1, 0.0, true, ModifierKeys::shiftModifier);
	}

	~FMSynthKnob() override { setLookAndFeel(nullptr); }

	void resized() override {
		const auto minSide = std::min(getHeight(), getWidth());
		setTextBoxStyle(Slider::TextBoxBelow,
                        false,
                        std::max(25, minSide*2/3),
                        std::max(minSide/8, 25));
		Slider::resized();
	}

	void drawRotarySlider(Graphics& g, int x, int y, int width, int height, float sliderPos,
		const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider) override {
		auto outline = slider.findColour(Slider::rotarySliderOutlineColourId);
//		auto fill = slider.findColour(Slider::rotarySliderFillColourId);

		auto bounds = Rectangle<int>(x, y, width, height).toFloat().reduced(10);

		auto radius = jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
		auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
		auto lineW = jmin(11.0f, radius * 0.5f);
		auto arcRadius = radius - lineW * 0.5f;

		Path backgroundArc;
		backgroundArc.addCentredArc(bounds.getCentreX(),
			bounds.getCentreY(),
			arcRadius,
			arcRadius,
			0.0f,
			rotaryStartAngle,
			rotaryEndAngle,
			true);

		g.setColour(outline);
		g.strokePath(backgroundArc, PathStrokeType(lineW, PathStrokeType::curved, PathStrokeType::rounded));

		if (slider.isEnabled()) {
			Path valueArc;
			valueArc.addCentredArc(bounds.getCentreX(),
				bounds.getCentreY(),
				arcRadius,
				arcRadius,
				0.0f,
				rotaryStartAngle,
				toAngle,
				true);
            
            const auto normalizedValue = (slider.getValue() - slider.getRange().getStart())
                                         / slider.getRange().getLength();
            const auto scaledNormalizedValue = pow(normalizedValue, getSkewFactor());
            valueIndicator.setBrightness(std::sqrt(static_cast<float>(scaledNormalizedValue)));

			const auto trackColour = Colours::black.interpolatedWith(Colours::white, valueIndicator.getBrightness());

			g.setColour(outline);
			g.strokePath(valueArc, PathStrokeType(lineW, PathStrokeType::curved, PathStrokeType::rounded));

			g.setColour(trackColour);
			g.strokePath(valueArc, PathStrokeType(lineW*.5f, PathStrokeType::curved, PathStrokeType::rounded));
		}

		auto thumbWidth = lineW * 2.0f;
		juce::Point<float> thumbPoint(bounds.getCentreX() + arcRadius * std::cos(toAngle - MathConstants<float>::halfPi),
			bounds.getCentreY() + arcRadius * std::sin(toAngle - MathConstants<float>::halfPi));

		g.setColour(slider.findColour(Slider::thumbColourId));
		const auto thumbBounds = Rectangle<float>(thumbWidth, thumbWidth).withCentre(thumbPoint);
		g.fillEllipse(thumbBounds);

		const auto indicatorDimension = std::min(static_cast<int>(round(thumbBounds.getWidth() * .5f)), static_cast<int>(round(thumbBounds.getHeight() * .5f)));
		valueIndicator.setSize(indicatorDimension, indicatorDimension);
		valueIndicator.setCentrePosition(static_cast<int>(round(thumbBounds.getCentreX())), static_cast<int>(round(thumbBounds.getCentreY())));
	}

private:
	Rectangle<float> sliderBounds;
	Indicator<IndicatorShapes::Circle> valueIndicator = Indicator<IndicatorShapes::Circle>(Colours::white.withAlpha(.0f));
};

//wrapper for a slider that adds a label below the slider that displays the slider's name
template<typename KnobType>
class LabeledKnob : public Component
{
public:
	LabeledKnob(const String& Name,
                const String& textBoxSuffix = "",
                const String& tooltipText = "") {
        static_assert(std::is_base_of<juce::Slider, KnobType>::value, "KnobType needs to be some sort of slider!");
        addAndMakeVisible(&knob);
		addAndMakeVisible(&label);

		knob.setTooltip(tooltipText);
		knob.setTextValueSuffix(textBoxSuffix);
        knob.setTextBoxIsEditable(true);

		label.setJustificationType(Justification::centred);
		label.setColour(Label::textColourId, Colours::black);
		label.setText(Name, NotificationType::dontSendNotification);
		label.setTooltip(tooltipText);
	}

	void resized() override {
		auto bounds = this->getLocalBounds();
		const auto knobBounds = bounds.removeFromTop(roundToInt(bounds.getHeight() * .8f));
		knob.setBounds(knobBounds);
		label.setBounds(bounds);
	}

private:
	KnobType knob;
	Label label;
};
