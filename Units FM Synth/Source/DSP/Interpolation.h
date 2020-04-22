#pragma once

#include "../../Interval/include/Interval.h"

//A simple function object for lerping
template<typename ValueType>
struct LinearInterpolation
{
    constexpr auto operator()(const ValueType& a, const ValueType& b, const ValueType& coefficient){
        return a+(b-a)*coefficient;
    }
};

// An container around an array with a modulor read index.
// Returns two ways to access the data:
//    A writer, which gives back values in the table, allowing them to be modified
//    A reader, which takes a fractional index, interpolating between the two adjacent table values
//    based on the fractional part of the index.
//       This returns by value, because modifying a value in the table via an interpolated output
//       doesn't make sense
template<typename ValueType, size_t Size, typename InterpolationMethod = LinearInterpolation<ValueType>, IntervalWrapModes WrapMode = IntervalWrapModes::Wrap>
class InterpolatedIntervalArray : public ArrayWithIntervalRead<ValueType, Size, IntervalWrapModes::Wrap>
{
    using TableType = ArrayWithIntervalRead<ValueType, Size, IntervalWrapModes::Wrap>;
public:
    auto getReader() const noexcept { return Reader(*this); }
    auto getWriter() noexcept { return Writer(*this); }
    
private:
    // A simple class that allows the reading of the table.
    // By having each read happen through an instance of this class
    // the state of the interpolator is preserved.
    class Reader
    {
        using ParentContainer = InterpolatedIntervalArray<ValueType, Size, InterpolationMethod,  WrapMode>;
    public:
        Reader(const ParentContainer& parent) : parentContainer(parent) {}
        
        template<typename T>
        constexpr decltype(auto) operator[](const T& index) const {
            if constexpr(std::is_integral<T>::value)
                return parentContainer[index];
            else
                return floatLookupImpl(index);
        }

        constexpr auto& operator[](const typename ParentContainer::TableType::IntervalType& interval) const noexcept {
            return parentContainer[interval];
        }
        
    private:
        const ParentContainer& parentContainer;
        
        template<typename IndexType>
        constexpr auto floatLookupImpl(const IndexType& index) const noexcept {
                return InterpolationMethod()(parentContainer[static_cast<size_t>(std::floor(index))],
                                             parentContainer[static_cast<size_t>(std::ceil(index))],
                                             index-std::floor(index));
        }
        
        template<typename Bound1, typename Bound2, IntervalWrapModes Mode, typename Default>
        constexpr auto floatLookupImpl(const Interval<Bound1, Bound2, Mode, Default>& interval) const noexcept {
            using ArrayIntervalType = typename ArrayWithIntervalRead<ValueType, Size, IntervalWrapModes::Wrap>::IntervalType;
            return floatLookupImpl(ArrayIntervalType{interval}.getValue());
        }
    };
    
    // A simple class that allows writing to the table.
    class Writer
    {
        using ParentContainer = InterpolatedIntervalArray<ValueType, Size, InterpolationMethod,  WrapMode>;
    public:
        Writer(ParentContainer& parent) : parentContainer(parent) {}

        constexpr auto& operator[](const size_t& index) {
            return parentContainer[index];
        }

        constexpr auto& operator[](const typename ParentContainer::TableType::IntervalType& interval) noexcept {
            return parentContainer[interval];
        }
        
        constexpr ParentContainer& getContainer() {return parentContainer;}
        
    private:
        ParentContainer& parentContainer;
    };
};
