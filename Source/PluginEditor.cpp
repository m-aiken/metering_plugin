/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
void Goniometer::paint(juce::Graphics& g)
{
    using namespace juce;
    
    auto bounds = getLocalBounds();
    auto centre = bounds.getCentre();
    auto width = bounds.getWidth();
    auto padding = width / 10;
    auto diameter = width - (padding * 2);
    auto radius = diameter / 2;
    
    g.drawImage(this->canvas, bounds.toFloat());
    
    g.setColour(MyColours::getColour(MyColours::GoniometerPath));
    
    Path p;
    
    auto numSamples = buffer.getNumSamples();

    for (auto i = 0; i < numSamples; ++i)
    {
        auto left = buffer.getSample(0, i);
        auto right = buffer.getSample(1, i);

        auto side = jlimit<float>(-1.f, 1.f, (left - right) * Decibels::decibelsToGain(-3.f));
        
        auto mid = jlimit<float>(-1.f, 1.f, (left + right) * Decibels::decibelsToGain(-3.f));
        
        Point<float> point(centre.getX() + radius * scale * side,
                           centre.getY() + radius * scale * mid);
        
        if ( i == 0 )
            p.startNewSubPath(point);
        else
            p.lineTo(point);
    }
    
    g.strokePath(p, PathStrokeType(1.f));
}

void Goniometer::resized()
{
    using namespace juce;
    
    auto bounds = getLocalBounds();
    auto centre = bounds.getCentre();
    auto height = bounds.getHeight();
    auto width = bounds.getWidth();
    auto padding = width / 10;
    auto diameter = width - (padding * 2);
    
    auto backgroundColour = MyColours::getColour(MyColours::Background);
    auto textColour = MyColours::getColour(MyColours::Text);
    auto ellipseColour = textColour.withAlpha(0.1f);
    auto lineColour = textColour.withAlpha(0.025f);
    
    canvas = Image(Image::RGB, width, height, true);
    
    Graphics g (canvas);
    
    // inner lines
    Path linePath;
    Rectangle<float> lineRect;
    lineRect.setLeft(centre.getX() - 1);
    lineRect.setRight(centre.getX() + 1);
    lineRect.setTop(padding);
    lineRect.setBottom(centre.getY());
    linePath.addRectangle(lineRect);
    
    Path labelPath;
    Rectangle<float> labelRect;
    labelRect.setLeft(centre.getX() - (padding / 2));
    labelRect.setRight(centre.getX() + (padding / 2));
    labelRect.setTop(0);
    labelRect.setBottom(padding);
    labelPath.addRectangle(labelRect);
    
    float angle;
    
    for ( auto i = 0; i < 8; ++i)
    {
        angle = degreesToRadians( i * 45.f );
        auto affineT = AffineTransform().rotated(angle, centre.getX(), centre.getY());
        
        // lines
        linePath.applyTransform(affineT);
        g.setColour(lineColour);
        g.fillPath(linePath);
        
        // label boxes
        labelPath.applyTransform(affineT);
        g.setColour(backgroundColour);
        g.fillPath(labelPath);
    }
    
    // draw labels separately - if drawn in the same loop as lines and text boxes 90 degrees doesn't draw??
    std::vector<String> labels { "M", "R", "-S", "", "", "", "+S", "L" };
    g.setColour(textColour);
    for ( auto i = 0; i < 8; ++i)
    {
        angle = degreesToRadians( i * 45.f );
        auto affineT = AffineTransform().rotated(angle, centre.getX(), centre.getY());
        
        g.drawText(labels[i], labelRect.transformedBy(affineT), Justification::centred);
    }
    
    // draw ellipse last so that it overlaps label rectanges
    g.setColour(ellipseColour);
    
    g.drawEllipse(padding,  // x
                  padding,  // y
                  diameter, // width
                  diameter, // height
                  2.f);     // line thickness
}

void Goniometer::update(juce::AudioBuffer<float>& incomingBuffer)
{
    if ( incomingBuffer.getNumSamples() >= 265 )
        buffer = incomingBuffer;
    else
        buffer.applyGain(juce::Decibels::decibelsToGain(-3.f));
    
    repaint();
}

void Goniometer::setScale(const double& rotaryValue)
{
    scale = juce::jmap<double>(rotaryValue, 50.0, 200.0, 0.2, 0.8);
    repaint();
}

//==============================================================================
void Histogram::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto height = bounds.getHeight();
    auto width = bounds.getWidth();
    
    /*
    Filling the entire container with the gradient and threshold colours
    The path to fill is the negative of the buffer data
    That path hides anything not from the proper signal with "Background Colour"
    */
    auto gradient = MyColours::getMeterGradient(bounds.getHeight(), 0, MyColours::GradientOrientation::Vertical);
    
    g.setGradientFill(gradient);
    g.fillAll();

    auto mappedThresh = juce::jmap<float>(threshold.getValue(),
                                          NegativeInfinity,
                                          MaxDecibels,
                                          bounds.getHeight(),
                                          0);
    
    juce::Rectangle<float> redRect (0,
                                    0,
                                    bounds.getWidth(),
                                    mappedThresh);
    
    g.setColour(MyColours::getColour(MyColours::Red));
    g.fillRect(redRect);
    
    auto& data = circularBuffer.getData();
    auto readIdx = circularBuffer.getReadIndex();
    auto bufferSize = circularBuffer.getSize();

    if ( view == HistView::columns )
    {
        readIdx = (readIdx + (bufferSize / 2)) % bufferSize;
    }
    
    g.setColour(MyColours::getColour(MyColours::Background));
    juce::Path p;
    
    // manually setting first and last pixel's column (x) outside of the loops
    p.startNewSubPath(0, 0);
    
    auto x = 1;
    
    for ( auto i = readIdx; i < bufferSize - 1; ++i )
    {
        auto scaledValue = juce::jmap<float>(data[i], NegativeInfinity, MaxDecibels, height, 0);
        
        p.lineTo(x, scaledValue);
        
        ++x;
    }

    for ( auto j = 0; j < readIdx; ++j )
    {
        
        auto scaledValue = juce::jmap<float>(data[j], NegativeInfinity, MaxDecibels, height, 0);
        
        p.lineTo(x, scaledValue);

        ++x;
    }
    
    p.lineTo(bufferSize - 1, 0);
    p.closeSubPath();
    g.fillPath(p);
    
    g.setColour(MyColours::getColour(MyColours::Text));
    g.setFont(16.0f);
    g.drawFittedText(label,                              // text
                     bounds.reduced(4),                  // area
                     juce::Justification::centredTop, // justification
                     1);                                 // max num lines
    
    
    g.setColour(MyColours::getColour(MyColours::Background).contrasting(0.05f));
    // startX, startY, endX, endY, thickness
    g.drawLine(0, 0, 0, height, 2.f);
    g.drawLine(width, 0, width, height, 2.f);
    
    g.drawLine(0, 0, width, 0, 2.f);
    g.drawLine(0, height, width, height, 2.f);
}

void Histogram::update(const float& inputL, const float& inputR)
{
    auto average = (inputL + inputR) / 2;
    circularBuffer.write(average);
    
    repaint();
}

void Histogram::setThreshold(const float& threshAsDecibels)
{
    threshold.setValue(threshAsDecibels);
}

void Histogram::setView(const HistView& v)
{
    view = v;
    repaint();
}

//==============================================================================
HistogramContainer::HistogramContainer()
{
    addAndMakeVisible(rmsHistogram);
    addAndMakeVisible(peakHistogram);
}

void HistogramContainer::resized()
{
    juce::FlexBox fb;
    fb.flexDirection = ( view == rows ? juce::FlexBox::Direction::column
                                      : juce::FlexBox::Direction::row);
    
    auto rms = juce::FlexItem(rmsHistogram).withFlex(1.f).withMargin(2.f);
    auto peak = juce::FlexItem(peakHistogram).withFlex(1.f).withMargin(2.f);
    
    fb.items.add(rms);
    fb.items.add(peak);
    
    fb.performLayout(getLocalBounds());
}

void HistogramContainer::update(const HistogramTypes& histoType,
                                const float& inputL,
                                const float& inputR)
{
    if ( histoType == HistogramTypes::RMS )
        rmsHistogram.update(inputL, inputR);
    else
        peakHistogram.update(inputL, inputR);
}

void HistogramContainer::setThreshold(const HistogramTypes& histoType,
                                      const float& threshAsDecibels)
{
    if ( histoType == HistogramTypes::RMS )
        rmsHistogram.setThreshold(threshAsDecibels);
    else
        peakHistogram.setThreshold(threshAsDecibels);
}

void HistogramContainer::setView(const int& selectedId)
{
    auto v = (selectedId == HistView::rows) ? HistView::rows : HistView::columns;
    view = v;
    resized();
    
    rmsHistogram.setView(v);
    peakHistogram.setView(v);
}

juce::Value& HistogramContainer::getThresholdValueObject(const HistogramTypes& histoType)
{
    if ( histoType == HistogramTypes::RMS )
        return rmsHistogram.getThresholdValueObject();
    else
        return peakHistogram.getThresholdValueObject();
}

//==============================================================================
CorrelationMeter::CorrelationMeter(double _sampleRate, size_t _blockSize)
    : sampleRate(_sampleRate), blockSize(_blockSize)
{
    prepareFilters();
}

void CorrelationMeter::prepareFilters()
{
    juce::dsp::ProcessSpec spec;
    spec.numChannels = 1;
    
    using FilterDesign = juce::dsp::FilterDesign<float>;
    using WindowingFunction = juce::dsp::WindowingFunction<float>;
    
    auto coefficientsPtr = FilterDesign::designFIRLowpassWindowMethod(100.f,                                     // frequency
                                                                      sampleRate,                                // sample rate
                                                                      2,                                         // order
                                                                      WindowingFunction::WindowingMethod::hann); // windowing method
    
    for ( auto& filter : filters )
    {
        filter.prepare(spec);
        filter.coefficients = coefficientsPtr;
    }
}

void CorrelationMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto width = bounds.getWidth();
    auto height = bounds.getHeight();
    auto padding = width / 10;
    auto meterWidth = width - (padding * 2);
    
    // labels
    g.setColour(MyColours::getColour(MyColours::Text));
    // draw fitted text args = text, x, y, width, height, justification, maxNumLines
    g.drawFittedText("-1", 0, 0, padding, height, juce::Justification::centred, 1);
    g.drawFittedText("+1", width - padding, 0, padding, height, juce::Justification::centred, 1);
    
    // meter background
    auto meterBounds = juce::Rectangle<int>(bounds.getCentreX() - (meterWidth / 2), // x
                                            bounds.getY(),                          // y
                                            meterWidth,                             // width
                                            height);                                // height

    auto shadow = MyColours::getDropShadow();
    shadow.drawForRectangle(g, meterBounds);
    
    // meters
    auto gradient = MyColours::getMeterGradient(bounds.getCentreX(), bounds.getRight(), MyColours::GradientOrientation::Horizontal);
    g.setGradientFill(gradient);
    
    juce::Rectangle<int> averageCorrelationMeter = paintMeter(meterBounds,            // container bounds
                                                   meterBounds.getY(),                // y
                                                   static_cast<int>(height * 0.2f),   // height
                                                   averagedCorrelation.getAverage()); // value
    
    juce::Rectangle<int> instantCorrelationMeter = paintMeter(meterBounds,                                           // container bounds
                                                              averageCorrelationMeter.getBottom() + (height * 0.1f), // y
                                                              static_cast<int>(height * 0.7f),                       // height
                                                              instantaneousCorrelation.getAverage());                // value
    
    g.fillRect(averageCorrelationMeter);
    g.fillRect(instantCorrelationMeter);
}

juce::Rectangle<int> CorrelationMeter::paintMeter(const juce::Rectangle<int>& containerBounds, const int& y, const int& height, const float& value)
{
    auto jmap = juce::jmap<float>(value,                       // source
                                  -1.f,                        // source min
                                  1.f,                         // source max
                                  containerBounds.getX(),      // target min
                                  containerBounds.getRight()); // target max
    
    juce::Rectangle<int> rectangle;
    rectangle.setY(y);
    rectangle.setHeight(height);
    
    if ( jmap > containerBounds.getCentreX() )
    {
        rectangle.setX(containerBounds.getCentreX());
        rectangle.setWidth(std::floor(jmap) - containerBounds.getCentreX());
    }
    else
    {
        rectangle.setX(std::floor(jmap));
        rectangle.setWidth(containerBounds.getCentreX() - std::floor(jmap));
    }
    
    return rectangle;
}

void CorrelationMeter::update(juce::AudioBuffer<float>& incomingBuffer)
{
    for ( auto i = 0; i < incomingBuffer.getNumSamples(); ++i )
    {
        auto sampleL = incomingBuffer.getSample(0, i);
        auto sampleR = incomingBuffer.getSample(1, i);
        
        auto denominator = std::sqrt( filters[1].processSample( std::pow(sampleL, 2) ) * filters[2].processSample( std::pow(sampleR, 2) ) );
        
        if ( denominator != 0.f && !std::isinf(denominator) )
        {
            auto numerator = filters[0].processSample(sampleL * sampleR);
            auto correlation = numerator / denominator;
            
            instantaneousCorrelation.add(correlation);
            averagedCorrelation.add(correlation);
        }
        else
        {
            instantaneousCorrelation.add(0.f);
            averagedCorrelation.add(0.f);
        }
    }
    
    repaint();
}

//==============================================================================
StereoImageMeter::StereoImageMeter(double _sampleRate, size_t _blockSize)
    : correlationMeter(_sampleRate, _blockSize)
{
    addAndMakeVisible(goniometer);
    addAndMakeVisible(correlationMeter);
}

void StereoImageMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto goniometerDims = bounds.getWidth();
    
    goniometer.setBounds(0, 0, goniometerDims, goniometerDims);
    correlationMeter.setBounds(0, goniometer.getBottom(), goniometerDims, 20);
}

void StereoImageMeter::update(juce::AudioBuffer<float>& incomingBuffer)
{
    goniometer.update(incomingBuffer);
    correlationMeter.update(incomingBuffer);
}

void StereoImageMeter::setGoniometerScale(const double& rotaryValue)
{
    goniometer.setScale(rotaryValue);
}

//==============================================================================
void ValueHolderBase::timerCallback()
{
    if ( getNow() - peakTime > holdTime )
        handleOverHoldTime();
}

//==============================================================================
void DecayingValueHolder::updateHeldValue(const float& input)
{
    if (input > currentValue)
    {
        peakTime = getNow();
        currentValue = input;
        resetDecayRateMultiplier();
    }
}

void DecayingValueHolder::setDecayRate(const float& dbPerSecond)
{
    decayRatePerFrame = dbPerSecond / timerFrequency;
}

void DecayingValueHolder::handleOverHoldTime()
{
    currentValue = juce::jlimit(NegativeInfinity,
                                MaxDecibels,
                                currentValue - (decayRatePerFrame * decayRateMultiplier));
    
    decayRateMultiplier *= 1.04f;
    
    if ( currentValue == NegativeInfinity )
        resetDecayRateMultiplier();
}

//==============================================================================
void ValueHolder::setThreshold(const float& threshAsDecibels)
{
    threshold = threshAsDecibels;
}

void ValueHolder::updateHeldValue(const float& input)
{
    currentValue = input;
    
    if (isOverThreshold())
    {
        peakTime = juce::Time::currentTimeMillis();
        if (input > heldValue)
            heldValue = input;
    }
}

bool ValueHolder::isOverThreshold() const
{
    return currentValue > threshold;
}

void ValueHolder::handleOverHoldTime()
{
    heldValue = NegativeInfinity;
}

//==============================================================================
void TextMeter::paint(juce::Graphics& g)
{
    juce::String str;
    
    if ( valueHolder.isOverThreshold() )
    {
        str = juce::String(valueHolder.getHeldValue(), 1);
        g.fillAll(MyColours::getColour(MyColours::Red)); // background
    }
    else
    {
        str = juce::String(valueHolder.getCurrentValue(), 1);
    }
    
    g.setColour(MyColours::getColour(MyColours::Text));
    g.setFont(14.0f);
    g.drawFittedText(str,                                      // text
                     getLocalBounds(),                         // area
                     juce::Justification::horizontallyCentred, // justification
                     1);                                       // max num lines
}

void TextMeter::update(const float& input)
{
    valueHolder.updateHeldValue(input);
    repaint();
}

void TextMeter::setThreshold(const float& threshAsDecibels)
{
    valueHolder.setThreshold(threshAsDecibels);
}

//==============================================================================
void DbScale::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    int textHeight = 8;
    
    g.setColour(MyColours::getColour(MyColours::Text));
    
    for ( int i = 0; i < ticks.size(); ++i)
    {
        auto dbStr = juce::String(ticks[i].db);
        
        g.drawFittedText((ticks[i].db > 0 ? '+' + dbStr : dbStr),   // text
                         bounds.getX(),                             // x
                         ticks[i].y + (yOffset - (textHeight / 2)), // y
                         bounds.getWidth(),                         // width
                         textHeight,                                // height
                         juce::Justification::horizontallyCentred,  // justification
                         1);                                        // maxLines
    }
}

//==============================================================================
void Meter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto h = bounds.getHeight();
    
    auto shadow = MyColours::getDropShadow();
    shadow.drawForRectangle(g, bounds);
    
    auto underThreshColour = MyColours::getColour(MyColours::Green);
    auto overThreshColour = MyColours::getColour(MyColours::Red);
    g.setColour(underThreshColour);
    auto levelJmap = juce::jmap<float>(level, NegativeInfinity, MaxDecibels, h, 0);
    auto thrshJmap = juce::jmap<float>(threshold, NegativeInfinity, MaxDecibels, h, 0);
    
    auto underThreshGradient = MyColours::getMeterGradient(bounds.getHeight(), bounds.getHeight() / 3, MyColours::GradientOrientation::Vertical);
    
    g.setGradientFill(underThreshGradient);
    if ( threshold <= level )
    {
        g.setColour(overThreshColour);
        g.fillRect(bounds.withHeight((h * levelJmap) - (thrshJmap - 1)).withY(levelJmap));
        
        g.setGradientFill(underThreshGradient);
        g.fillRect(bounds.withHeight(h * (thrshJmap + 1)).withY(thrshJmap + 1));
    }
    else
    {
        g.fillRect(bounds.withHeight(h * levelJmap).withY(levelJmap));
    }
    
    // falling tick
    if ( fallingTickEnabled )
    {
        g.setColour(MyColours::getColour(MyColours::Yellow));
        
        auto tickValue = fallingTick.getHoldTime() == 0 ? level :   fallingTick.getCurrentValue();
        
        auto ftJmap = juce::jmap<float>(tickValue,
                                        NegativeInfinity,
                                        MaxDecibels,
                                        h,
                                        0);
        
        g.drawLine(bounds.getX(),     // startX
                   ftJmap,            // startY
                   bounds.getRight(), // endX
                   ftJmap,            // endY
                   3.f);              // line thickness
    }
}

void Meter::resized()
{
    ticks.clear();
    int h = getHeight();
    
    for ( int i = static_cast<int>(NegativeInfinity); i <= static_cast<int>(MaxDecibels); ++i ) // <= maxDb to include max value
    {
        Tick tick;
        if ( i % 6 == 0 )
        {
            tick.db = static_cast<float>(i);
            tick.y = juce::jmap<int>(i, NegativeInfinity, MaxDecibels, h, 0);
            ticks.push_back(tick);
        }
    }
}

void Meter::update(const float& newLevel)
{
    level = newLevel;
    fallingTick.updateHeldValue(newLevel);
    repaint();
}

void Meter::setThreshold(const float& threshAsDecibels)
{
    threshold = threshAsDecibels;
}

void Meter::setDecayRate(const float& dbPerSecond)
{
    fallingTick.setDecayRate(dbPerSecond);
}

void Meter::setHoldTime(const long long& ms)
{
    fallingTick.setHoldTime(ms);
}

void Meter::resetValueHolder()
{
    fallingTick.reset();
}

void Meter::setTickVisibility(const bool& toggleState)
{
    fallingTickEnabled = toggleState;
    repaint();
}

//==============================================================================
MacroMeter::MacroMeter(const Channel& channel)
    : channel(channel)
{
    addAndMakeVisible(textMeter);
    addAndMakeVisible(averageMeter);
    addAndMakeVisible(instantMeter);
}

void MacroMeter::resized()
{
    auto bounds = getLocalBounds();
    auto h = bounds.getHeight();
    auto textBoxHeight = static_cast<int>(h * 0.06f);
    auto meterHeight = h - textBoxHeight;
    
    auto w = bounds.getWidth();
    
    // setBounds args (int x, int y, int width, int height)
    textMeter.setBounds(0, 0, w, textBoxHeight);
    
    auto averageMeterWidth = static_cast<int>(w * 0.7f);
    auto instantMeterWidth = static_cast<int>(w * 0.25f);
    auto meterPadding = static_cast<int>(w * 0.05f);
    
    auto avgMeterX = (channel == Channel::Left ? 0 : instantMeterWidth + meterPadding);
    
    auto averageMeterRect = juce::Rectangle<int>(avgMeterX,
                                                 textMeter.getBottom(),
                                                 averageMeterWidth,
                                                 meterHeight);
    
    auto instMeterX = channel == Channel::Left ? averageMeterWidth + meterPadding : 0;
    
    auto instantMeterRect = juce::Rectangle<int>(instMeterX,
                                                 textMeter.getBottom(),
                                                 instantMeterWidth,
                                                 meterHeight);
    
    if (channel == Channel::Left)
    {
        averageMeter.setBounds(averageMeterRect);
        instantMeter.setBounds(instantMeterRect);
    }
    else
    {
        instantMeter.setBounds(instantMeterRect);
        averageMeter.setBounds(averageMeterRect);
    }
}

void MacroMeter::update(const float& input)
{
    averager.add(input);
    
    textMeter.update(input);
    averageMeter.update(averager.getAverage());
    instantMeter.update(input);
}

void MacroMeter::setThreshold(const float& threshAsDecibels)
{
    averageMeter.setThreshold(threshAsDecibels);
    instantMeter.setThreshold(threshAsDecibels);
    textMeter.setThreshold(threshAsDecibels);
}

void MacroMeter::setDecayRate(const float& dbPerSecond)
{
    averageMeter.setDecayRate(dbPerSecond);
    instantMeter.setDecayRate(dbPerSecond);
}

void MacroMeter::setHoldTime(const long long& ms)
{
    averageMeter.setHoldTime(ms);
    instantMeter.setHoldTime(ms);
}

void MacroMeter::resetValueHolder()
{
    averageMeter.resetValueHolder();
    instantMeter.resetValueHolder();
}

void MacroMeter::setMeterView(const int& newViewId)
{
    if ( newViewId == 1 ) // Both
    {
        averageMeter.setVisible(true);
        instantMeter.setVisible(true);
    }
    else if ( newViewId == 2 ) // Peak
    {
        averageMeter.setVisible(false);
        instantMeter.setVisible(true);
    }
    else if ( newViewId == 3 ) // Average
    {
        averageMeter.setVisible(true);
        instantMeter.setVisible(false);
    }
}

void MacroMeter::setTickVisibility(const bool& toggleState)
{
    averageMeter.setTickVisibility(toggleState);
    instantMeter.setTickVisibility(toggleState);
}

void MacroMeter::resizeAverager(const int& durationId)
{
    // timerCallback called every 25ms
    size_t newSize;
    
    switch (durationId)
    {
        case 1:  newSize = 4;  break; // 100ms
        case 2:  newSize = 10; break; // 250ms
        case 3:  newSize = 20; break; // 500ms
        case 4:  newSize = 40; break; // 1000ms
        case 5:  newSize = 80; break; // 2000ms
        default: newSize = 20; break; // 500ms
    }
    
    averager.resize(newSize, averager.getAverage());
}

//==============================================================================
void CustomLookAndFeel::drawLinearSlider(juce::Graphics& g,
                                         int x, int y, int width, int height,
                                         float sliderPos,
                                         float minSliderPos,
                                         float maxSliderPos,
                                         const juce::Slider::SliderStyle style,
                                         juce::Slider& slider)
{
    slider.setSliderStyle(style);
    
    auto threshold = juce::Rectangle<float>(x, sliderPos, width, 2.f);
    g.setColour(MyColours::getColour(MyColours::Red));
    g.fillRect(threshold);
}

void CustomLookAndFeel::drawComboBox(juce::Graphics& g,
                                     int width, int height,
                                     bool isButtonDown,
                                     int buttonX, int buttonY,
                                     int buttonW, int buttonH,
                                     juce::ComboBox& comboBox)
{
    g.fillAll(MyColours::getColour(MyColours::Background).contrasting(0.05f));
    comboBox.setColour(juce::ComboBox::textColourId, MyColours::getColour(MyColours::Text));
}

void CustomLookAndFeel::drawToggleButton(juce::Graphics& g,
                                         juce::ToggleButton& toggleButton,
                                         bool shouldDrawButtonAsHighlighted,
                                         bool shouldDrawButtonAsDown)
{
    auto bounds = toggleButton.getLocalBounds();
    
    // background
    g.fillAll(toggleButton.getToggleState()
              ? MyColours::getColour(MyColours::Yellow)
              : MyColours::getColour(MyColours::Background).contrasting(0.05f));
    
    // text colour
    g.setColour(toggleButton.getToggleState()
                ? MyColours::getColour(MyColours::Background).contrasting(0.05f)
                : MyColours::getColour(MyColours::Text));
    
    g.drawFittedText(toggleButton.getButtonText(),
                     bounds.getX(),
                     bounds.getY(),
                     bounds.getWidth(),
                     bounds.getHeight(),
                     juce::Justification::centred,
                     1);
}

void CustomLookAndFeel::drawButtonBackground(juce::Graphics& g,
                                             juce::Button& button,
                                             const juce::Colour& backgroundColour,
                                             bool shouldDrawButtonAsHighlighted,
                                             bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds();
    
    g.fillAll(backgroundColour);
    
    g.setColour(MyColours::getColour(MyColours::Text));
    g.drawFittedText(button.getButtonText(),
                     bounds.getX(),
                     bounds.getY(),
                     bounds.getWidth(),
                     bounds.getHeight(),
                     juce::Justification::centred,
                     1);
}

void CustomLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                         int x, int y, int width, int height,
                                         float sliderPosProportional,
                                         float rotaryStartAngle,
                                         float rotaryEndAngle,
                                         juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float>(x, y, width, height);
    
    g.setColour(MyColours::getColour(MyColours::Background).contrasting(0.05f));
    g.fillEllipse(bounds);
    
    g.setColour(MyColours::getColour(MyColours::Text));
    
    juce::Path p;
    
    auto centre = bounds.getCentre();
    
    juce::Rectangle<float> r;
    r.setLeft(centre.getX() - 2);
    r.setRight(centre.getX() + 2);
    r.setTop(bounds.getY());
    r.setBottom(centre.getY());
    
    p.addRoundedRectangle(r, 2.f);
    
    jassert(rotaryStartAngle < rotaryEndAngle);
    
    auto transform = juce::AffineTransform().rotated(sliderPosProportional,
                                                     centre.getX(),
                                                     centre.getY());
    
    p.applyTransform(transform);
    
    g.fillPath(p);
}

//==============================================================================
ThresholdSlider::ThresholdSlider()
{
    setLookAndFeel(&lnf);
    setRange(NegativeInfinity, MaxDecibels);
    setValue(0.f);
}

ThresholdSlider::~ThresholdSlider() { setLookAndFeel(nullptr); }

void ThresholdSlider::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto valueToDraw = juce::jmap<float>(getValue(),
                                         NegativeInfinity,
                                         MaxDecibels,
                                         bounds.getBottom(),
                                         bounds.getY());
    
    getLookAndFeel().drawLinearSlider(g,
                                      bounds.getX(),
                                      bounds.getY(),
                                      bounds.getWidth(),
                                      bounds.getHeight(),
                                      valueToDraw,          // sliderPos
                                      bounds.getBottom(),   // minSliderPos
                                      bounds.getY(),        // maxSliderPos
                                      juce::Slider::SliderStyle::LinearVertical,
                                      *this);
}

//==============================================================================
StereoMeter::StereoMeter(const juce::String& labelText)
    : label(labelText)
{
    addAndMakeVisible(macroMeterL);
    addAndMakeVisible(dbScale);
    addAndMakeVisible(macroMeterR);
    addAndMakeVisible(threshCtrl);
}

void StereoMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto h = bounds.getHeight();
    auto w = bounds.getWidth();
    auto labelContainerY = static_cast<int>(h * dbScaleLabelCrossover);
    auto labelContainerH = static_cast<int>(h - labelContainerY);
    
    g.setColour(MyColours::getColour(MyColours::Text));
    
    std::vector<juce::String> labels{"L", label, "R"};
    std::vector<int> xPositions{0, static_cast<int>(w / 3), static_cast<int>(w - (w / 3))};
    
    for (auto i = 0; i < labels.size(); ++i)
    {
        g.drawFittedText(labels[i],                                // text
                         xPositions[i],                            // x
                         labelContainerY,                          // y
                         static_cast<int>(w / 3),                  // width
                         labelContainerH,                          // height
                         juce::Justification::horizontallyCentred, // justification
                         1);                                       // max num lines
    }
}

void StereoMeter::resized()
{
    auto bounds = getLocalBounds();
    auto h = bounds.getHeight();
    auto w = bounds.getWidth();
    auto meterWidth = static_cast<int>(w * 0.35f);
    auto meterHeight = static_cast<int>(h * 0.92f);
    
    auto dbScaleWidth = w - (meterWidth * 2);
    
    macroMeterL.setBounds(0, 0, meterWidth, meterHeight);
    
    dbScale.ticks = macroMeterL.getTicks();
    auto tickYoffset = macroMeterL.getTickYoffset();
    dbScale.yOffset = macroMeterL.getY() + tickYoffset;
    dbScale.setBounds(macroMeterL.getRight(),
                      0,
                      dbScaleWidth,
                      static_cast<int>(h * dbScaleLabelCrossover));
    
    macroMeterR.setBounds(dbScale.getRight(), 0, meterWidth, meterHeight);
    
    auto offset = macroMeterL.getTickYoffset();
    threshCtrl.setBounds(dbScale.getX(),
                         macroMeterL.getY() + offset,
                         dbScale.getWidth(),
                         macroMeterL.getHeight() - offset);
}

void StereoMeter::update(const float& inputL, const float& inputR)
{
    macroMeterL.update(inputL);
    macroMeterR.update(inputR);
}

void StereoMeter::setThreshold(const float& threshAsDecibels)
{
    macroMeterL.setThreshold(threshAsDecibels);
    macroMeterR.setThreshold(threshAsDecibels);
}

void StereoMeter::setDecayRate(const int& selectedId)
{
    float dbPerSecond;
    
    switch (selectedId)
    {
        case 1:  dbPerSecond = 3.f;  break;
        case 2:  dbPerSecond = 6.f;  break;
        case 3:  dbPerSecond = 12.f; break;
        case 4:  dbPerSecond = 24.f; break;
        case 5:  dbPerSecond = 36.f; break;
        default: dbPerSecond = 12.f; break;
    }
    
    macroMeterL.setDecayRate(dbPerSecond);
    macroMeterR.setDecayRate(dbPerSecond);
}

void StereoMeter::setTickVisibility(const bool& toggleState)
{
    macroMeterL.setTickVisibility(toggleState);
    macroMeterR.setTickVisibility(toggleState);
}

void StereoMeter::setTickHoldTime(const int& selectedId)
{
    long long holdTimeMs;
    
    switch (selectedId)
    {
        case 1:  holdTimeMs = 0;        break;
        case 2:  holdTimeMs = 500;      break;
        case 3:  holdTimeMs = 2000;     break;
        case 4:  holdTimeMs = 4000;     break;
        case 5:  holdTimeMs = 6000;     break;
        case 6:  holdTimeMs = INFINITY; break;
        default: holdTimeMs = 500;      break;
    }
    
    macroMeterL.setHoldTime(holdTimeMs);
    macroMeterR.setHoldTime(holdTimeMs);
}

void StereoMeter::resetValueHolder()
{
    macroMeterL.resetValueHolder();
    macroMeterR.resetValueHolder();
}

void StereoMeter::setMeterView(const int& newViewId)
{
    macroMeterL.setMeterView(newViewId);
    macroMeterR.setMeterView(newViewId);
}

void StereoMeter::resizeAverager(const int& durationId)
{
    macroMeterL.resizeAverager(durationId);
    macroMeterR.resizeAverager(durationId);
}

//==============================================================================
CustomComboBox::CustomComboBox(const juce::StringArray& choices)
{
    setLookAndFeel(&lnf);
    addItemList(choices, 1);
}

void CustomComboBox::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    getLookAndFeel().drawComboBox(g,
                                  bounds.getWidth(),  // width
                                  bounds.getHeight(), // height
                                  true,               // isButtonDown
                                  bounds.getX(),      // button x
                                  bounds.getY(),      // button y
                                  bounds.getWidth(),  // button width
                                  bounds.getHeight(), // button height
                                  *this);             // combo box
}

CustomLabel::CustomLabel(const juce::String& labelText)
{
    setText(labelText, juce::NotificationType::dontSendNotification);
}

void CustomLabel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    g.setColour(MyColours::getColour(MyColours::Text));
    
    g.drawFittedText(getText(),
                     bounds.getX(),
                     bounds.getY(),
                     bounds.getWidth(),
                     bounds.getHeight(),
                     juce::Justification::centred,
                     1);
}

CustomToggle::CustomToggle(const juce::String& buttonText)
{
    setLookAndFeel(&lnf);
    setButtonText(buttonText);
}

void CustomToggle::paint(juce::Graphics& g)
{
    getLookAndFeel().drawToggleButton(g,
                                      *this, // toggle button
                                      true,  // draw as highlighted
                                      true); // draw as down
}

CustomTextBtn::CustomTextBtn(const juce::String& buttonText)
{
    setLookAndFeel(&lnf);
    setButtonText(buttonText);
}

void CustomTextBtn::paint(juce::Graphics& g)
{
    auto buttonColour = (inClickState
                         ? MyColours::getColour(MyColours::RedBright)
                         : MyColours::getColour(MyColours::Red));
    
    getLookAndFeel().drawButtonBackground(g,
                                          *this,        // button
                                          buttonColour, // colour
                                          true,         // draw as highlighted
                                          false);       // draw as down
}

void CustomTextBtn::animateButton()
{
    inClickState = true;
    repaint();
    
    juce::Timer::callAfterDelay(100, resetColour);
}

CustomRotary::CustomRotary()
{
    setLookAndFeel(&lnf);
}

void CustomRotary::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto diameter = bounds.getWidth() * 0.7f;
    auto radius = diameter / 2;
    
    auto startAngle = juce::degreesToRadians(180.f + 45.f);
    auto endAngle = juce::degreesToRadians(180.f - 45.f) + juce::MathConstants<float>::twoPi;
    
    auto range = getRange();
    
    auto valueToDraw = juce::jmap<float>(getValue(),
                                         range.getStart(),
                                         range.getEnd(),
                                         startAngle,
                                         endAngle);
    
    getLookAndFeel().drawRotarySlider(g,
                                      bounds.getCentreX() - radius, // x
                                      bounds.getCentreY() - radius, // y
                                      diameter,                     // width
                                      diameter,                     // height
                                      valueToDraw,                  // position
                                      startAngle,                   // start angle
                                      endAngle,                     // end angle
                                      *this);                       // slider
}

//==============================================================================
juce::Grid ToggleGroupBase::generateGrid(std::vector<CustomToggle*>& toggles)
{
    juce::Grid grid;
     
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;
    
    grid.templateColumns = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1)) };
    grid.autoRows = Track(Fr(1));
    
    for ( auto& toggle : toggles )
        grid.items.add(juce::GridItem(*toggle));
    
    grid.setGap(juce::Grid::Px{4});
    return grid;
}

DecayRateToggleGroup::DecayRateToggleGroup()
{
    for ( auto& toggle : toggles )
    {
        addAndMakeVisible(toggle);
        toggle->setRadioGroupId(1);
    }
}

void DecayRateToggleGroup::resized()
{
    juce::Grid grid = generateGrid(toggles);
    grid.performLayout(getLocalBounds());
}

void DecayRateToggleGroup::setSelectedToggleFromState()
{
    using nt = juce::NotificationType;
    switch (static_cast<int>(getValueObject().getValue()))
    {
        case 1:  optionA.setToggleState(true, nt::dontSendNotification); break;
        case 2:  optionB.setToggleState(true, nt::dontSendNotification); break;
        case 3:  optionC.setToggleState(true, nt::dontSendNotification); break;
        case 4:  optionD.setToggleState(true, nt::dontSendNotification); break;
        case 5:  optionE.setToggleState(true, nt::dontSendNotification); break;
        default: optionC.setToggleState(true, nt::dontSendNotification); break;
    }
}

AverageTimeToggleGroup::AverageTimeToggleGroup()
{
    for ( auto& toggle : toggles )
    {
        addAndMakeVisible(toggle);
        toggle->setRadioGroupId(2);
    }
}

void AverageTimeToggleGroup::resized()
{
    juce::Grid grid = generateGrid(toggles);
    grid.performLayout(getLocalBounds());
}

void AverageTimeToggleGroup::setSelectedToggleFromState()
{
    using nt = juce::NotificationType;
    switch (static_cast<int>(getValueObject().getValue()))
    {
        case 1:  optionA.setToggleState(true, nt::dontSendNotification); break;
        case 2:  optionB.setToggleState(true, nt::dontSendNotification); break;
        case 3:  optionC.setToggleState(true, nt::dontSendNotification); break;
        case 4:  optionD.setToggleState(true, nt::dontSendNotification); break;
        case 5:  optionE.setToggleState(true, nt::dontSendNotification); break;
        default: optionC.setToggleState(true, nt::dontSendNotification); break;
    }
}

MeterViewToggleGroup::MeterViewToggleGroup()
{
    for ( auto& toggle : toggles )
    {
        addAndMakeVisible(toggle);
        toggle->setRadioGroupId(3);
    }
}

void MeterViewToggleGroup::resized()
{
    juce::Grid grid = generateGrid(toggles);
    grid.performLayout(getLocalBounds());
}

void MeterViewToggleGroup::setSelectedToggleFromState()
{
    using nt = juce::NotificationType;
    switch (static_cast<int>(getValueObject().getValue()))
    {
        case 1:  optionA.setToggleState(true, nt::dontSendNotification); break;
        case 2:  optionB.setToggleState(true, nt::dontSendNotification); break;
        case 3:  optionC.setToggleState(true, nt::dontSendNotification); break;
        default: optionA.setToggleState(true, nt::dontSendNotification); break;
    }
}

HoldTimeToggleGroup::HoldTimeToggleGroup()
{
    for ( auto& toggle : toggles )
    {
        addAndMakeVisible(toggle);
        toggle->setRadioGroupId(4);
    }
}

void HoldTimeToggleGroup::resized()
{
    juce::Grid grid = generateGrid(toggles);
    grid.performLayout(getLocalBounds());
}

void HoldTimeToggleGroup::setSelectedToggleFromState()
{
    using nt = juce::NotificationType;
    switch (static_cast<int>(getValueObject().getValue()))
    {
        case 1:  optionA.setToggleState(true, nt::dontSendNotification); break;
        case 2:  optionB.setToggleState(true, nt::dontSendNotification); break;
        case 3:  optionC.setToggleState(true, nt::dontSendNotification); break;
        case 4:  optionD.setToggleState(true, nt::dontSendNotification); break;
        case 5:  optionE.setToggleState(true, nt::dontSendNotification); break;
        case 6:  optionF.setToggleState(true, nt::dontSendNotification); break;
        default: optionB.setToggleState(true, nt::dontSendNotification); break;
    }
}

HistViewToggleGroup::HistViewToggleGroup()
{
    for ( auto& toggle : toggles )
    {
        addAndMakeVisible(toggle);
        toggle->setRadioGroupId(5);
    }
}

void HistViewToggleGroup::resized()
{
    juce::Grid grid = generateGrid(toggles);
    grid.performLayout(getLocalBounds());
}

void HistViewToggleGroup::setSelectedToggleFromState()
{
    using nt = juce::NotificationType;
    switch (static_cast<int>(getValueObject().getValue()))
    {
        case 1:  optionA.setToggleState(true, nt::dontSendNotification); break;
        case 2:  optionB.setToggleState(true, nt::dontSendNotification); break;
        default: optionA.setToggleState(true, nt::dontSendNotification); break;
    }
}

juce::Grid HistViewToggleGroup::generateGrid(std::vector<CustomToggle*>& toggles)
{
    juce::Grid grid;
     
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;
    
    grid.templateColumns = { Track(Fr(1)), Track(Fr(1)) };
    grid.autoRows = Track(Fr(1));
    
    for ( auto& toggle : toggles )
        grid.items.add(juce::GridItem(*toggle));
    
    grid.setGap(juce::Grid::Px{4});
    return grid;
}

//==============================================================================
HoldResetButtons::HoldResetButtons()
{
    addAndMakeVisible(holdButton);
    addAndMakeVisible(resetButton);
}

void HoldResetButtons::resized()
{
    juce::Grid grid;
     
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;
    
    grid.templateColumns = { Track(Fr(2)), Track(Fr(1)) };
    grid.autoRows = Track(Fr(1));
    grid.items = { juce::GridItem(holdButton), juce::GridItem(resetButton) };
    
    grid.setGap(juce::Grid::Px{4});
    grid.performLayout(getLocalBounds());
}

//==============================================================================
void LineBreak::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    g.setColour(juce::Colour(201u, 209u, 217u).withAlpha(0.025f));
    g.drawLine(0, bounds.getCentreY(), bounds.getRight(), bounds.getCentreY(), 2.f);
}

TimeControls::TimeControls()
{
    addAndMakeVisible(holdTimeLabel);
    addAndMakeVisible(decayRateLabel);
    addAndMakeVisible(avgDurationLabel);
    
    addAndMakeVisible(holdTime);
    addAndMakeVisible(decayRate);
    addAndMakeVisible(avgDuration);
    
    addAndMakeVisible(lineBreak1);
    addAndMakeVisible(lineBreak2);
}

void TimeControls::resized()
{
    auto bounds = getLocalBounds();
    auto buttonHeight = bounds.getHeight() / 10;
    
    juce::Grid grid;
     
    using Track = juce::Grid::TrackInfo;
    using Px = juce::Grid::Px;
    
    grid.autoColumns = Track(Px(bounds.getWidth()));
    grid.templateRows =
    {
        Track(Px(buttonHeight)),
        Track(Px(buttonHeight * 2)),
        Track(Px(buttonHeight / 2)), // line break
        Track(Px(buttonHeight)),
        Track(Px(buttonHeight * 2)),
        Track(Px(buttonHeight / 2)), // line break
        Track(Px(buttonHeight)),
        Track(Px(buttonHeight * 2))
    };
    
    grid.items =
    {
        juce::GridItem(holdTimeLabel),
        juce::GridItem(holdTime),
        juce::GridItem(lineBreak1),
        juce::GridItem(decayRateLabel),
        juce::GridItem(decayRate),
        juce::GridItem(lineBreak2),
        juce::GridItem(avgDurationLabel),
        juce::GridItem(avgDuration)
    };
    
    grid.performLayout(bounds);
}

//==============================================================================
GonioScaleControl::GonioScaleControl()
{
    addAndMakeVisible(gonioScaleLabel);
    addAndMakeVisible(gonioScaleKnob);
    gonioScaleKnob.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    gonioScaleKnob.setRange(50.0, 200.0);
    gonioScaleKnob.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    gonioScaleKnob.setValue(100.0);
    
    addAndMakeVisible(lineBreak);
}

void GonioScaleControl::resized()
{
    auto bounds = getLocalBounds();
    auto buttonHeight = bounds.getHeight() / 5.5f;
    
    juce::Grid grid;
     
    using Track = juce::Grid::TrackInfo;
    using Px = juce::Grid::Px;
    
    grid.autoColumns = Track(Px(bounds.getWidth()));
    
    grid.templateRows =
    {
        Track(Px(buttonHeight)),
        Track(Px(buttonHeight * 3)),
        Track(Px(buttonHeight)),
        Track(Px(buttonHeight / 2)), // line break
    };
    
    grid.items =
    {
        juce::GridItem(gonioScaleLabel),
        juce::GridItem(gonioScaleKnob),
        juce::GridItem(),
        juce::GridItem(lineBreak)
    };
    
    grid.performLayout(bounds);
}

//==============================================================================
ViewControls::ViewControls()
{
    addAndMakeVisible(meterViewLabel);
    addAndMakeVisible(meterView);
    addAndMakeVisible(histViewLabel);
    addAndMakeVisible(histView);
    
    addAndMakeVisible(lineBreak);
}

void ViewControls::resized()
{
    auto bounds = getLocalBounds();
    auto buttonHeight = bounds.getHeight() / 4.5f;
    
    juce::Grid grid;
     
    using Track = juce::Grid::TrackInfo;
    using Px = juce::Grid::Px;
    
    grid.autoColumns = Track(Px(bounds.getWidth()));
    
    grid.templateRows =
    {
        Track(Px(buttonHeight)),
        Track(Px(buttonHeight)),
        Track(Px(buttonHeight / 2)), // line break
        Track(Px(buttonHeight)),
        Track(Px(buttonHeight))
    };
    
    grid.items =
    {
        juce::GridItem(meterViewLabel),
        juce::GridItem(meterView),
        juce::GridItem(lineBreak),
        juce::GridItem(histViewLabel),
        juce::GridItem(histView)
    };
    
    grid.performLayout(bounds);
}

//==============================================================================
PFMProject10AudioProcessorEditor::PFMProject10AudioProcessorEditor (PFMProject10AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), stereoImageMeter(p.getSampleRate(), p.getBlockSize())
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    startTimerHz(40);
    
    addAndMakeVisible(stereoMeterRms);
    addAndMakeVisible(stereoMeterPeak);
    addAndMakeVisible(histograms);
    addAndMakeVisible(stereoImageMeter);
    addAndMakeVisible(holdResetBtns);
    addAndMakeVisible(timeToggles);
    addAndMakeVisible(gonioControl);
    addAndMakeVisible(viewToggles);
    
    auto& state = audioProcessor.valueTree;
    
    // link widgets to valueTree
    holdResetBtns.holdButton.getToggleStateValue().referTo(state.getPropertyAsValue("EnableHold", nullptr));
    
    timeToggles.decayRate.getValueObject().referTo(state.getPropertyAsValue("DecayTime", nullptr));
    timeToggles.avgDuration.getValueObject().referTo(state.getPropertyAsValue("AverageTime", nullptr));
    timeToggles.holdTime.getValueObject().referTo(state.getPropertyAsValue("HoldTime", nullptr));
    
    gonioControl.gonioScaleKnob.getValueObject().referTo(state.getPropertyAsValue("GoniometerScale", nullptr));
    viewToggles.meterView.getValueObject().referTo(state.getPropertyAsValue("MeterViewMode", nullptr));
    viewToggles.histView.getValueObject().referTo(state.getPropertyAsValue("HistogramView", nullptr));
    
    stereoMeterRms.threshCtrl.getValueObject().referTo(state.getPropertyAsValue("RMSThreshold", nullptr));
    stereoMeterPeak.threshCtrl.getValueObject().referTo(state.getPropertyAsValue("PeakThreshold", nullptr));
    
    histograms.getThresholdValueObject(HistogramTypes::RMS).referTo(state.getPropertyAsValue("RMSThreshold", nullptr));
    histograms.getThresholdValueObject(HistogramTypes::PEAK).referTo(state.getPropertyAsValue("PeakThreshold", nullptr));
    
    // set initial values
    bool holdButtonState = state.getPropertyAsValue("EnableHold", nullptr).getValue();
    stereoMeterRms.setTickVisibility(holdButtonState);
    stereoMeterPeak.setTickVisibility(holdButtonState);
    
    updateParams(ToggleGroup::DecayRate, state.getPropertyAsValue("DecayTime", nullptr).getValue());
    timeToggles.decayRate.setSelectedToggleFromState();
    
    updateParams(ToggleGroup::AverageTime, state.getPropertyAsValue("AverageTime", nullptr).getValue());
    timeToggles.avgDuration.setSelectedToggleFromState();
    
    updateParams(ToggleGroup::HoldTime, state.getPropertyAsValue("HoldTime", nullptr).getValue());
    timeToggles.holdTime.setSelectedToggleFromState();
    
    double gonioScale = state.getPropertyAsValue("GoniometerScale", nullptr).getValue();
    stereoImageMeter.setGoniometerScale(gonioScale);
    
    updateParams(ToggleGroup::MeterView, state.getPropertyAsValue("MeterViewMode", nullptr).getValue());
    viewToggles.meterView.setSelectedToggleFromState();
    
    updateParams(ToggleGroup::HistView, state.getPropertyAsValue("HistogramView", nullptr).getValue());
    viewToggles.histView.setSelectedToggleFromState();
    
    float rmsThresh = state.getPropertyAsValue("RMSThreshold", nullptr).getValue();
    stereoMeterRms.setThreshold(rmsThresh);
    histograms.setThreshold(HistogramTypes::RMS, rmsThresh);
    
    float peakThresh = state.getPropertyAsValue("PeakThreshold", nullptr).getValue();
    stereoMeterPeak.setThreshold(peakThresh);
    histograms.setThreshold(HistogramTypes::PEAK, peakThresh);
    
    // handle change events
    stereoMeterRms.threshCtrl.onValueChange = [this]
    {
        stereoMeterRms.setThreshold(stereoMeterRms.threshCtrl.getValue());
    };
    
    stereoMeterPeak.threshCtrl.onValueChange = [this]
    {
        stereoMeterPeak.setThreshold(stereoMeterPeak.threshCtrl.getValue());
    };
    
    holdResetBtns.holdButton.onClick = [this]
    {
        auto toggleState = holdResetBtns.holdButton.getToggleState();
        stereoMeterRms.setTickVisibility(toggleState);
        stereoMeterPeak.setTickVisibility(toggleState);
        
        auto resetIsVisible = holdResetBtns.resetButton.isVisible();
        auto holdTimeId = timeToggles.holdTime.getValueObject().getValue();
        if ( !toggleState && resetIsVisible )
        {
            holdResetBtns.resetButton.setVisible(false);
        }
        else if ( toggleState && static_cast<int>(holdTimeId) == 6 && !resetIsVisible )
        {
            holdResetBtns.resetButton.setVisible(true);
        }
    };
    
    holdResetBtns.resetButton.onClick = [this]
    {
        stereoMeterRms.resetValueHolder();
        stereoMeterPeak.resetValueHolder();
        holdResetBtns.resetButton.animateButton();
    };
    
    timeToggles.decayRate.optionA.onClick = [this]{ updateParams(ToggleGroup::DecayRate, 1); };
    timeToggles.decayRate.optionB.onClick = [this]{ updateParams(ToggleGroup::DecayRate, 2); };
    timeToggles.decayRate.optionC.onClick = [this]{ updateParams(ToggleGroup::DecayRate, 3); };
    timeToggles.decayRate.optionD.onClick = [this]{ updateParams(ToggleGroup::DecayRate, 4); };
    timeToggles.decayRate.optionE.onClick = [this]{ updateParams(ToggleGroup::DecayRate, 5); };
    
    timeToggles.avgDuration.optionA.onClick = [this]{ updateParams(ToggleGroup::AverageTime, 1); };
    timeToggles.avgDuration.optionB.onClick = [this]{ updateParams(ToggleGroup::AverageTime, 2); };
    timeToggles.avgDuration.optionC.onClick = [this]{ updateParams(ToggleGroup::AverageTime, 3); };
    timeToggles.avgDuration.optionD.onClick = [this]{ updateParams(ToggleGroup::AverageTime, 4); };
    timeToggles.avgDuration.optionE.onClick = [this]{ updateParams(ToggleGroup::AverageTime, 5); };
    
    timeToggles.holdTime.optionA.onClick = [this]{ updateParams(ToggleGroup::HoldTime, 1); };
    timeToggles.holdTime.optionB.onClick = [this]{ updateParams(ToggleGroup::HoldTime, 2); };
    timeToggles.holdTime.optionC.onClick = [this]{ updateParams(ToggleGroup::HoldTime, 3); };
    timeToggles.holdTime.optionD.onClick = [this]{ updateParams(ToggleGroup::HoldTime, 4); };
    timeToggles.holdTime.optionE.onClick = [this]{ updateParams(ToggleGroup::HoldTime, 5); };
    timeToggles.holdTime.optionF.onClick = [this]{ updateParams(ToggleGroup::HoldTime, 6); };
    
    gonioControl.gonioScaleKnob.onValueChange = [this]
    {
        auto rotaryValue = gonioControl.gonioScaleKnob.getValue();
        stereoImageMeter.setGoniometerScale(rotaryValue);
    };
    
    viewToggles.meterView.optionA.onClick = [this]{ updateParams(ToggleGroup::MeterView, 1); };
    viewToggles.meterView.optionB.onClick = [this]{ updateParams(ToggleGroup::MeterView, 2); };
    viewToggles.meterView.optionC.onClick = [this]{ updateParams(ToggleGroup::MeterView, 3); };
    
    viewToggles.histView.optionA.onClick = [this]{ updateParams(ToggleGroup::HistView, 1); };
    viewToggles.histView.optionB.onClick = [this]{ updateParams(ToggleGroup::HistView, 2); };
    
#if defined(GAIN_TEST_ACTIVE)
    addAndMakeVisible(gainSlider);
    gainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
#endif

    setSize (800, 600);
}

PFMProject10AudioProcessorEditor::~PFMProject10AudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void PFMProject10AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(MyColours::getColour(MyColours::Background));
}

void PFMProject10AudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();
    auto width = bounds.getWidth();
    auto padding = 10;
    auto stereoMeterWidth = 90;
    auto stereoMeterHeight = 350;
    
    // setBounds args (int x, int y, int width, int height)
    stereoMeterRms.setBounds(padding,
                             padding,
                             stereoMeterWidth,
                             stereoMeterHeight);
    
    stereoMeterPeak.setBounds(width - (stereoMeterWidth + padding),
                              padding,
                              stereoMeterWidth,
                              stereoMeterHeight);
    
    histograms.setBounds(padding,
                         stereoMeterRms.getBottom() + (padding * 2),
                         width - (padding * 2),
                         210);
    
    auto stereoImageMeterWidth = 280; // this will also be the height of the goniometer
    auto stereoImageMeterHeight = 300;
    
    stereoImageMeter.setBounds(bounds.getCentreX() - (stereoImageMeterWidth / 2),
                              (histograms.getY() / 2) - (stereoImageMeterHeight / 2),
                              stereoImageMeterWidth,
                              stereoImageMeterHeight);
    
    auto comboWidth = stereoImageMeter.getX() - stereoMeterRms.getRight() - (padding * 4);
    
    auto toggleContainerHeight = 300;
    timeToggles.setBounds(stereoMeterRms.getRight() + (padding * 2),
                          stereoImageMeter.getBottom() - toggleContainerHeight,
                          comboWidth,
                          toggleContainerHeight);
    
    auto btnHeight = timeToggles.getY() - (padding * 2);
    holdResetBtns.setBounds(timeToggles.getX(),
                            timeToggles.getY() - btnHeight,
                            comboWidth,
                            btnHeight);
    
    gonioControl.setBounds(stereoMeterPeak.getX() - (padding * 2) - comboWidth,
                           stereoImageMeter.getBottom() - 300,
                           comboWidth,
                           165);
    
    viewToggles.setBounds(stereoMeterPeak.getX() - (padding * 2) - comboWidth,
                           stereoImageMeter.getBottom() - 135,
                           comboWidth,
                           135);
    
#if defined(GAIN_TEST_ACTIVE)
    gainSlider.setBounds(stereoMeterRms.getRight(), padding * 2, 20, 320);
#endif
}

void PFMProject10AudioProcessorEditor::timerCallback()
{
    if ( audioProcessor.fifo.getNumAvailable() > 0 )
    {
        while ( audioProcessor.fifo.pull(incomingBuffer) )
        {
            // do nothing else - just looping through until incomingBuffer = most recent available buffer
        }
        
        auto numSamples = incomingBuffer.getNumSamples();
                
        auto rmsL = incomingBuffer.getRMSLevel(0, 0, numSamples);
        auto rmsR = incomingBuffer.getRMSLevel(1, 0, numSamples);
        auto rmsDbL = juce::Decibels::gainToDecibels(rmsL, NegativeInfinity);
        auto rmsDbR = juce::Decibels::gainToDecibels(rmsR, NegativeInfinity);
        stereoMeterRms.update(rmsDbL, rmsDbR);
        
        auto peakL = incomingBuffer.getMagnitude(0, 0, numSamples);
        auto peakR = incomingBuffer.getMagnitude(1, 0, numSamples);
        auto peakDbL = juce::Decibels::gainToDecibels(peakL, NegativeInfinity);
        auto peakDbR = juce::Decibels::gainToDecibels(peakR, NegativeInfinity);
        stereoMeterPeak.update(peakDbL, peakDbR);
        
        histograms.update(HistogramTypes::RMS, rmsDbL, rmsDbR);
        histograms.update(HistogramTypes::PEAK, peakDbL, peakDbR);
        
        stereoImageMeter.update(incomingBuffer);
    }
}

void PFMProject10AudioProcessorEditor::updateParams(const ToggleGroup& toggleGroup, const int& selectedId)
{
    switch (toggleGroup)
    {
        case ToggleGroup::DecayRate:
            stereoMeterRms.setDecayRate(selectedId);
            stereoMeterPeak.setDecayRate(selectedId);
            timeToggles.decayRate.setSelectedValue(selectedId);
            break;
        case ToggleGroup::AverageTime:
            stereoMeterRms.resizeAverager(selectedId);
            stereoMeterPeak.resizeAverager(selectedId);
            timeToggles.avgDuration.setSelectedValue(selectedId);
            break;
        case ToggleGroup::MeterView:
            stereoMeterRms.setMeterView(selectedId);
            stereoMeterPeak.setMeterView(selectedId);
            viewToggles.meterView.setSelectedValue(selectedId);
            break;
        case ToggleGroup::HoldTime:
            stereoMeterRms.setTickHoldTime(selectedId);
            stereoMeterPeak.setTickHoldTime(selectedId);
            timeToggles.holdTime.setSelectedValue(selectedId);
            holdResetBtns.resetButton.setVisible( (selectedId == 6 && holdResetBtns.holdButton.getToggleState()) );
            break;
        case ToggleGroup::HistView:
            histograms.setView(selectedId);
            viewToggles.histView.setSelectedValue(selectedId);
            break;
    }
}
