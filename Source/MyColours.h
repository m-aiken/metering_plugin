/*
  ==============================================================================

    MyColours.h
    Created: 2 Apr 2022 12:37:40am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
namespace MyColours
{

enum Palette
{
    Red,
    RedBright,
    Text,
    Background,
    Yellow,
    GoniometerPath
};

inline std::map<Palette, juce::Colour> colourMap =
{
    { Red,            juce::Colour(187u, 62u, 3u).withAlpha(0.9f) },
    { RedBright,      juce::Colour(202u, 103u, 2u)                },
    { Text,           juce::Colour(233u, 216u, 166u)              },
    { Background,     juce::Colour(0u, 18u, 25u)                  },
    { Yellow,         juce::Colour(238u, 155u, 0u)                },
    { GoniometerPath, juce::Colour(153u, 226u, 180u)              }
};

inline juce::Colour getColour(Palette c) { return colourMap[c]; }

enum GradientOrientation
{
    Vertical,
    Horizontal
};

inline juce::ColourGradient getMeterGradient(const float startCoord, const float endCoord, const GradientOrientation orientation)
{
    juce::ColourGradient gradient;
    if ( orientation == Vertical )
    {
        gradient = juce::ColourGradient().vertical(juce::Colour(153u,226u,180u),
                          startCoord,
                          juce::Colour(3u,102u,102u),
                          endCoord);
    }
    else
    {
        gradient = juce::ColourGradient().horizontal(juce::Colour(153u,226u,180u),
                            startCoord,
                            juce::Colour(3u,102u,102u),
                            endCoord);
    }
    
    gradient.addColour(0.2, juce::Colour(136u,212u,171u));
    gradient.addColour(0.3, juce::Colour(120u,198u,163u));
    gradient.addColour(0.4, juce::Colour(103u,185u,154u));
    gradient.addColour(0.5, juce::Colour(86u,171u,145u));
    gradient.addColour(0.6, juce::Colour(70u,157u,137u));
    gradient.addColour(0.7, juce::Colour(53u,143u,128u));
    gradient.addColour(0.8, juce::Colour(36u,130u,119u));
    gradient.addColour(0.9, juce::Colour(20u,116u,111u));
    
    return gradient;
}

inline juce::DropShadow getDropShadow()
{
    return juce::DropShadow
    {
        getColour(Background).contrasting(0.03f),
        10,
        juce::Point<int>(0,0)
    };
}

}
