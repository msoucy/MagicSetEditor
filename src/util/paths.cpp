//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2012 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

/** @file util/paths.cpp
 *
 *  @brief Contains functions for getting file locations
 */

// -----------------------------------------------------------------------------
// : Includes

#include <util/prec.hpp>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <util/paths.hpp>

// -----------------------------------------------------------------------------
// : Path selection

String getDataDir() {
	wxFileName fn{wxStandardPaths::Get().GetExecutablePath()};
	fn.AppendDir(_("share"));
	fn.AppendDir(_("magicseteditor"));
	return fn.GetPath();
}

String getUserDataDir() { return wxStandardPaths::Get().GetUserDataDir(); }

// -----------------------------------------------------------------------------
// : EOF
