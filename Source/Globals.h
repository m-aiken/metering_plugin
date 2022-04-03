/*
  ==============================================================================

    Globals.h
    Created: 3 Apr 2022 4:45:18pm
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
namespace Globals
{
inline float maxDb() { return 6.f; }
inline float negInf() { return -48.f; }
inline juce::Font font() { return juce::Font(juce::Font::getDefaultMonospacedFontName(), 12.f, 0); }
}
