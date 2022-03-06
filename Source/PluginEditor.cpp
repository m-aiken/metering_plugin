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
    
    g.setColour(Colour(96u, 188u, 224u));
    
    Path p;
    
    auto numSamples = buffer.getNumSamples();

    for (auto i = 0; i < numSamples; ++i)
    {
        auto left = buffer.getSample(0, i);
        auto right = buffer.getSample(1, i);

        auto side = jlimit<float>(-1.f, 1.f, (left - right) * Decibels::decibelsToGain(-3.f));
        
        auto mid = jlimit<float>(-1.f, 1.f, (left + right) * Decibels::decibelsToGain(-3.f));
        
        Point<float> point(centre.getX() + radius * 0.5f * side,
                           centre.getY() + radius * 0.5f * mid);
        
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
    
    auto backgroundColour = Colour(13u, 17u, 23u);
    auto textColour = Colour(201u, 209u, 217u);
    auto ellipseColour = Colour(201u, 209u, 217u).withAlpha(0.1f);
    auto lineColour = Colour(201u, 209u, 217u).withAlpha(0.025f);
    
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

//==============================================================================
HistogramViewCombo::HistogramViewCombo()
{
    addAndMakeVisible(label);
    label.setText("HISTOGRAM", juce::NotificationType::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(comboBox);
    comboBox.addItemList(choices, 1);
    comboBox.setSelectedId(1);
}

void HistogramViewCombo::resized()
{
    auto bounds = getLocalBounds();
    auto width = bounds.getWidth();
    auto height = bounds.getHeight();
    
    label.setBounds(0, 0, width, height / 2);
    comboBox.setBounds(0, height / 2, width, height / 2);
}

//==============================================================================
void Histogram::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto height = bounds.getHeight();

    g.fillAll(juce::Colour(13u, 17u, 23u).contrasting(0.05f)); // background
    
    g.setColour(juce::Colour(201u, 209u, 217u)); // text colour
    g.setFont(16.0f);
    g.drawFittedText(label,                              // text
                     bounds,                             // area
                     juce::Justification::centredBottom, // justification
                     1);                                 // max num lines
    
    auto& data      = viewId == 1 ? circularBufferStacked.getData()
                                  : circularBufferSideBySide.getData();
    
    auto readIdx    = viewId == 1 ? circularBufferStacked.getReadIndex()
                                  : circularBufferSideBySide.getReadIndex();
    
    auto bufferSize = viewId == 1 ? circularBufferStacked.getSize()
                                  : circularBufferSideBySide.getSize();

    juce::Path p;
    
    // manually setting first and last pixel's column (x) outside of the loops
    p.startNewSubPath(0, height);
    
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
    
    p.lineTo(bufferSize - 1, height);
    p.closeSubPath();
    
    auto green = juce::Colour(89u, 255u, 103u).withAlpha(0.6f);
    auto red = juce::Colour(196u, 55u, 55u);
    
    auto thresholdProportion = juce::jmap<float>(threshold,
                                                 NegativeInfinity,
                                                 MaxDecibels,
                                                 0.f,
                                                 1.f);
    
    colourGrad.clearColours();
    
    // bottom to boundary = green, boundary+ = red
    colourGrad.addColour(0, green);                   // negative infinity
    colourGrad.addColour(thresholdProportion, green); // threshold boundary
    colourGrad.addColour(thresholdProportion, red);   // threshold boundary

    g.setGradientFill(colourGrad);
    g.fillPath(p);
}

void Histogram::resized()
{
    colourGrad.point1 = juce::Point<float>(0, getLocalBounds().getHeight()); // bottom
    colourGrad.point2 = juce::Point<float>(0, 0);      // top
}

void Histogram::update(const float& inputL, const float& inputR)
{
    auto average = (inputL + inputR) / 2;
    circularBufferStacked.write(average);
    circularBufferSideBySide.write(average);
    
    repaint();
}

void Histogram::setThreshold(const float& threshAsDecibels)
{
    threshold = threshAsDecibels;
}

void Histogram::setView(const int& selectedId)
{
    viewId = selectedId;
}

//==============================================================================
HistogramContainer::HistogramContainer()
{
    addAndMakeVisible(rmsHistogram);
    addAndMakeVisible(peakHistogram);
}

void HistogramContainer::resized()
{
    auto rms = juce::FlexItem(rmsHistogram).withFlex(1.f).withMargin(2.f);
    auto peak = juce::FlexItem(peakHistogram).withFlex(1.f).withMargin(2.f);
    
    fb.items.add(rms);
    fb.items.add(peak);
    
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

void HistogramContainer::setFlexDirection(const int& selectedId)
{
    if ( selectedId == 1 )
        fb.flexDirection = juce::FlexBox::Direction::column;
    else if ( selectedId == 2)
        fb.flexDirection = juce::FlexBox::Direction::row;
    
    rmsHistogram.setView(selectedId);
    peakHistogram.setView(selectedId);
    
    fb.performLayout(getLocalBounds());
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
    g.setColour(juce::Colour(201u, 209u, 217u)); // text colour
    // draw fitted text args = text, x, y, width, height, justification, maxNumLines
    g.drawFittedText("-1", 0, 0, padding, height, juce::Justification::centred, 1);
    g.drawFittedText("+1", width - padding, 0, padding, height, juce::Justification::centred, 1);
    
    // meter background
    g.setColour(juce::Colour(13u, 17u, 23u).contrasting(0.05f)); // background
    
    auto meterBounds = juce::Rectangle<int>(bounds.getCentreX() - (meterWidth / 2), // x
                                            bounds.getY(),                          // y
                                            meterWidth,                             // width
                                            height);                                // height

    g.fillRect(meterBounds);
    
    // meters
    g.setColour(juce::Colour(89u, 255u, 103u).withAlpha(0.6f)); // green
    
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

//==============================================================================
void ValueHolderBase::timerCallback()
{
    now = juce::Time::currentTimeMillis();
    if ( now - peakTime > holdTime )
        handleOverHoldTime();
}

//==============================================================================
void DecayingValueHolder::updateHeldValue(const float& input)
{
    if (input > currentValue)
    {
        currentValue = input;
        peakTime = juce::Time::currentTimeMillis();
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
                                currentValue - decayRatePerFrame);
        
    if ( currentValue == NegativeInfinity )
        setDecayRate(initDecayRate); // reset decayRatePerFrame
    else
        decayRatePerFrame *= 1.04f;
}

//==============================================================================
void ValueHolder::setThreshold(const float& threshAsDecibels)
{
    threshold = threshAsDecibels;
}

void ValueHolder::updateHeldValue(const float& input)
{
    currentValue = input;
    
    if (input > threshold)
    {
        isOverThreshold = true;
        peakTime = juce::Time::currentTimeMillis();
        if (input > heldValue)
            heldValue = input;
    }
}

void ValueHolder::handleOverHoldTime()
{
    isOverThreshold = currentValue > threshold;
    heldValue = NegativeInfinity;
}

//==============================================================================
void TextMeter::paint(juce::Graphics& g)
{
    juce::String str;
    
    if ( valueHolder.getIsOverThreshold() )
    {
        str = juce::String(valueHolder.getHeldValue(), 1);
        g.fillAll(juce::Colour(196u, 55u, 55u)); // red background
    }
    else
    {
        str = juce::String(valueHolder.getCurrentValue(), 1);
    }
    
    g.setColour(juce::Colour(201u, 209u, 217u)); // text colour
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
    
    g.setColour(juce::Colour(201u, 209u, 217u)); // text colour
    
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
    
    g.setColour(juce::Colour(13u, 17u, 23u).contrasting(0.05f)); // background colour
    g.fillRect(bounds);
    
    auto green = juce::Colour(89u, 255u, 103u).withAlpha(0.6f);
    auto red = juce::Colour(196u, 55u, 55u);
    g.setColour(green); // green
    auto levelJmap = juce::jmap<float>(level, NegativeInfinity, MaxDecibels, h, 0);
    auto thrshJmap = juce::jmap<float>(threshold, NegativeInfinity, MaxDecibels, h, 0);
    
    g.setColour(green);
    if ( threshold <= level )
    {
        g.setColour(red);
        g.fillRect(bounds.withHeight((h * levelJmap) - (thrshJmap - 1)).withY(levelJmap));
        g.setColour(green);
        g.fillRect(bounds.withHeight(h * (thrshJmap + 1)).withY(thrshJmap + 1));
    }
    else
    {
        g.fillRect(bounds.withHeight(h * levelJmap).withY(levelJmap));
    }
    
    // falling tick
    auto yellow = juce::Colour(217, 193, 56);
    g.setColour(yellow);
    
    auto ftJmap = juce::jmap<float>(fallingTick.getCurrentValue(),
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
    
    auto averageMeterWidth = static_cast<int>(w * 0.75f);
    auto instantMeterWidth = static_cast<int>(w * 0.20f);
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

void MacroMeter::setMeterView(const int& newViewId)
{
    if ( newViewId == 1 ) // Both
    {
        averageMeter.setAlpha(1.f);
        instantMeter.setAlpha(1.f);
    }
    else if ( newViewId == 2 ) // Peak
    {
        averageMeter.setAlpha(0.07f);
        instantMeter.setAlpha(1.f);
    }
    else if ( newViewId == 3 ) // Average
    {
        averageMeter.setAlpha(1.f);
        instantMeter.setAlpha(0.07f);
    }
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
    g.setColour(juce::Colour(196u, 55u, 55u)); // red
    g.fillRect(threshold);
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
    
    g.setColour(juce::Colour(201u, 209u, 217u)); // text colour
    
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

void StereoMeter::setDecayRate(const float& dbPerSecond)
{
    macroMeterL.setDecayRate(dbPerSecond);
    macroMeterR.setDecayRate(dbPerSecond);
}

void StereoMeter::setMeterView(const int& newViewId)
{
    macroMeterL.setMeterView(newViewId);
    macroMeterR.setMeterView(newViewId);
}

//==============================================================================
MeterComboGroup::MeterComboGroup()
{
    addAndMakeVisible(decayRateLabel);
    decayRateLabel.setText("DECAY", juce::NotificationType::dontSendNotification);
    decayRateLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(avgDurationLabel);
    avgDurationLabel.setText("AVG", juce::NotificationType::dontSendNotification);
    avgDurationLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(meterViewLabel);
    meterViewLabel.setText("METER", juce::NotificationType::dontSendNotification);
    meterViewLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(decayRateCombo);
    decayRateCombo.addItemList(decayRateChoices, 1);
    decayRateCombo.setSelectedId(3);
    
    addAndMakeVisible(avgDurationCombo);
    avgDurationCombo.addItemList(avgDurationChoices, 1);
    
    addAndMakeVisible(meterViewCombo);
    meterViewCombo.addItemList(meterViewChoices, 1);
    meterViewCombo.setSelectedId(1);
}

void MeterComboGroup::resized()
{
    auto bounds = getLocalBounds();
    auto boundsHeight = bounds.getHeight();
    
    auto boxHeight = 30;
    auto width = bounds.getWidth();

    decayRateLabel.setBounds(0,
                             (boundsHeight * 0.15f) - boxHeight,
                             width,
                             boxHeight);
    decayRateCombo.setBounds(0,
                             (boundsHeight * 0.15f),
                             width,
                             boxHeight);
    
    avgDurationLabel.setBounds(0,
                               (boundsHeight * 0.5f) - boxHeight,
                               width,
                               boxHeight);
    avgDurationCombo.setBounds(0,
                               (boundsHeight * 0.5f),
                               width,
                               boxHeight);
    
    meterViewLabel.setBounds(0,
                             (boundsHeight * 0.85f) - boxHeight,
                             width,
                             boxHeight);
    meterViewCombo.setBounds(0,
                             (boundsHeight * 0.85f),
                             width,
                             boxHeight);
}

//==============================================================================
PFMProject10AudioProcessorEditor::PFMProject10AudioProcessorEditor (PFMProject10AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), stereoImageMeter(p.getSampleRate(), p.getBlockSize())
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    startTimerHz(30);
        
    addAndMakeVisible(stereoMeterRms);
    addAndMakeVisible(stereoMeterPeak);

    addAndMakeVisible(histViewSelect);
    addAndMakeVisible(histograms);
    
    addAndMakeVisible(stereoImageMeter);
    addAndMakeVisible(meterCombos);
    
    // set inital values
    histograms.setFlexDirection(histViewSelect.comboBox.getSelectedId());
    
    // handle change events
    stereoMeterRms.threshCtrl.onValueChange = [this]
    {
        auto v = stereoMeterRms.threshCtrl.getValue();
        stereoMeterRms.setThreshold(v);
        histograms.setThreshold(HistogramTypes::RMS, v);
    };
    
    stereoMeterPeak.threshCtrl.onValueChange = [this]
    {
        auto v = stereoMeterPeak.threshCtrl.getValue();
        stereoMeterPeak.setThreshold(v);
        histograms.setThreshold(HistogramTypes::PEAK, v);
    };
    
    meterCombos.decayRateCombo.onChange = [this]
    {
        float dbPerSecond;
        switch (meterCombos.decayRateCombo.getSelectedId()) {
            case 1:  dbPerSecond = 3.f;  break;
            case 2:  dbPerSecond = 6.f;  break;
            case 3:  dbPerSecond = 12.f; break;
            case 4:  dbPerSecond = 24.f; break;
            case 5:  dbPerSecond = 36.f; break;
            default: dbPerSecond = 12.f; break;
        }
        
        stereoMeterRms.setDecayRate(dbPerSecond);
        stereoMeterPeak.setDecayRate(dbPerSecond);
    };
    
    meterCombos.avgDurationCombo.onChange = [this] { };
    
    meterCombos.meterViewCombo.onChange = [this]
    {
        auto selectedId = meterCombos.meterViewCombo.getSelectedId();
        stereoMeterRms.setMeterView(selectedId);
        stereoMeterPeak.setMeterView(selectedId);
    };
    
    histViewSelect.comboBox.onChange = [this]
    {
        histograms.setFlexDirection(histViewSelect.comboBox.getSelectedId());
    };
    
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
    g.fillAll(juce::Colour(13u, 17u, 23u));
}

void PFMProject10AudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();
    auto width = bounds.getWidth();
    auto margin = 10;
    auto stereoMeterWidth = 100;
    auto stereoMeterHeight = 350;
    
    // setBounds args (int x, int y, int width, int height)
    stereoMeterRms.setBounds(margin,
                             margin,
                             stereoMeterWidth,
                             stereoMeterHeight);
    
    stereoMeterPeak.setBounds(width - (stereoMeterWidth + margin),
                              margin,
                              stereoMeterWidth,
                              stereoMeterHeight);
    
    histograms.setBounds(margin,
                         stereoMeterRms.getBottom() + (margin * 2),
                         width - (margin * 2),
                         210);
    
    auto stereoImageMeterWidth = 300; // this will also be the height of the goniometer
    stereoImageMeter.setBounds((width / 2) - (stereoImageMeterWidth / 2),
                              (histograms.getY() - stereoImageMeterWidth) / 2,
                              stereoImageMeterWidth,
                              stereoImageMeterWidth + 20); // +20 to account for correlation meter
    
    meterCombos.setBounds(stereoMeterRms.getRight() + 20,
                          margin,
                          80,
                          stereoMeterHeight);
    
    auto histComboMaxWidth = (stereoMeterPeak.getX() - stereoImageMeter.getRight());
    auto histComboWidth = histComboMaxWidth * 0.8f;
    auto padding = (histComboMaxWidth * 0.2f) / 2;
    auto histComboHeight = 60;
    
    histViewSelect.setBounds(stereoImageMeter.getRight() + padding,
                             stereoImageMeter.getBottom() - histComboHeight,
                             histComboWidth,
                             histComboHeight);
    
#if defined(GAIN_TEST_ACTIVE)
    gainSlider.setBounds(stereoMeterRms.getRight(), margin * 2, 20, 320);
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
