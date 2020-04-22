#pragma once

class CenteredTabComponent : public TabbedComponent, public LookAndFeel_V4
{
public:
    CenteredTabComponent(TabbedButtonBar::Orientation orientation = TabbedButtonBar::Orientation::TabsAtTop) : TabbedComponent(orientation) { setLookAndFeel(this);
    }
    
    ~CenteredTabComponent() override { setLookAndFeel(nullptr); }
    
    auto getTabHeight() const noexcept { return tabs->getHeight(); }
    
    void resized() override {
        TabbedComponent::resized();
        const auto tabWidth = tabs->getTabButton(0)->getWidth()*tabs->getNumTabs();
        tabs->setBounds(roundToInt(getWidth()*.5-(tabWidth*.5)), 0, tabWidth, getTabHeight());
    }
};

// A component that acts as a container and layout engine for several other components
// These are fed to a CenteredTabComponent to make a set of tabbed controls
class TabbedContainerComponent : public Component
{
public:
    //takes a set of unique_ptrs to components and moves them into a vector
    template<typename... Args>
    TabbedContainerComponent(Args&&... widgetPointers) {
        //move component unique_ptrs into the vector
        std::unique_ptr<Component> itemArr[] = {std::move(widgetPointers)...};
        widgets = std::vector<std::unique_ptr<Component>> {std::make_move_iterator(std::begin(itemArr)), std::make_move_iterator(std::end(itemArr))};

        for(auto&& widget : widgets)
            addAndMakeVisible(widget.get());
    }
    
    void resized() override {
        const auto sideHeight = static_cast<float>(std::min(getHeight(), 150));
        const auto sideWidth  = std::max(getWidth() / static_cast<float>(widgets.size()), 150.0f);
        
        envelopeControlsContainer.items.clearQuick();
        for(auto&& widget : widgets)
            envelopeControlsContainer.items.add(FlexItem(*widget)
                                                .withMinWidth(sideWidth)
                                                .withMinHeight(sideHeight));
        
        envelopeControlsContainer.performLayout(getBounds().withY(0));
    }

private:
    std::vector<std::unique_ptr<Component>> widgets;
    
    FlexBox envelopeControlsContainer { FlexBox::Direction::row,
                                        FlexBox::Wrap::noWrap,
                                        FlexBox::AlignContent::center,
                                        FlexBox::AlignItems::center,
                                        FlexBox::JustifyContent::spaceBetween };
};
