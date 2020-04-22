#pragma once

enum class IndicatorShapes {
	Square,
	Circle
};


//a component that displays a specified colour at a specified brightness, as a square or circle.
template<IndicatorShapes Shape>
class Indicator : public Component
{
public:
	Indicator(Colour newColour = Colours::white) : indicatorColour(newColour) {}

	void paint(Graphics& g) override {
        //fill the bounds of the component with a transparent grey
		const auto indicatorBackgroundColour = Colour(0x33101010);
		g.setColour(indicatorBackgroundColour);

		if constexpr(Shape == IndicatorShapes::Square)
			g.fillRect(getLocalBounds());
		else
			g.fillEllipse(getLocalBounds().toFloat());
        
        //fill almost all of the bounds with a lighter version of the indicators color
		g.setColour(indicatorBackgroundColour.interpolatedWith(Colours::white, indicatorBrightness));
		Rectangle<int> innerRect = getLocalBounds().reduced(1);
		innerRect.setCentre(getLocalBounds().getCentre());
		Rectangle<int> innerRect2 = innerRect.reduced(1);
		innerRect2.setCentre(getLocalBounds().getCentre());
		if constexpr (Shape == IndicatorShapes::Square)
			g.fillRect(innerRect2);
		else
			g.fillEllipse(innerRect2.toFloat());

        //draw a thin border of the actual indicator background
		g.setColour(indicatorColour);
		if constexpr (Shape == IndicatorShapes::Square)
			g.fillRect(innerRect);
		else
			g.fillEllipse(innerRect.toFloat());
	}

	auto getBrightness() const noexcept { return indicatorBrightness;}

	void setBrightness(const float& newBrightness) noexcept { indicatorBrightness = newBrightness; }

private:
    float indicatorBrightness{0};
    Colour indicatorColour, brightness{0xFF202020};
};


//a component that briefly flashes a specified colour when its state is true
class MomentaryIndicator : public AnimatedAppComponent
{
public:
	MomentaryIndicator(Colour newColour, std::reference_wrapper<std::atomic<bool>> state) : indicator(newColour), onOrOff(state) {
		setOpaque(false);
		addAndMakeVisible(&indicator);

		setFramesPerSecond(20);
		indicator.setBounds(getLocalBounds());
	}

	void resized() override {indicator.setBounds(getLocalBounds());}

	void update() override {
        //if the indicator has been activated, max the brightness and deactivate the indicator.
		if (onOrOff.get().load()) {
			indicator.setBrightness(1.0);
			onOrOff.get().store(false);
		}

        //if there is brightness, decay the brightness.
		if (indicator.getBrightness() > .001f) {
			const auto newBrightness = indicator.getBrightness() - .13f;
			indicator.setBrightness(newBrightness);
		}
	}

private:
	Indicator<IndicatorShapes::Square> indicator;
	std::reference_wrapper<std::atomic<bool>> onOrOff;
};
