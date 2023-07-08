#ifndef UIHELPERS_H
#define UIHELPERS_H

#pragma once

#include <imgui.h>

namespace mrs
{
	void HelpMarker(const char *desc);

    void TextCentered(std::string text);

    bool ButtonCentered(std::string text);

    bool ButtonBottom(std::string text);
}

#endif