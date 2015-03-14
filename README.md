# Magic Set Editor

## About

Magic Set Editor is a tool that can be used to create custom Magic the Gathering cards.

It was originally located on [SourceForge](http://magicseteditor.sourceforge.net), but this fork is meant to be a modernization.

## Changes

Currently, the big changes from the original version are mainly behind the scenes:

- Uses [CMake](http://cmake.org) instead of separate build systems
- Allows installing into any directory
- Native Linux build
- Uses C++11 instead of Boost where possible

Future plans involve going through the codebase, attempting to fix some of the known issues and enhance portability.

## Dependencies

- [Boost](http://boost.org)
- [wxWidgets](http://wxwidgets.org/)
- [Hunspell](http://hunspell.sourceforge.net/)

## Disclaimer

I don't claim to know what I'm doing
