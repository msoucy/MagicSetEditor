//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2012 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/vcs/subversion.hpp>

// ----------------------------------------------------------------------------- : SVN File Manipulation

bool run_svn(const Char** arguments) {
	switch (wxExecute(const_cast<Char**>(arguments), wxEXEC_SYNC)) { // Yuck, const_cast
		// Success
		case 0:
			return true;
		// Couldn't run SVN
		case -1:
			handle_error(String((L"Can't run SVN.")));
			return false;
		// SVN error
		default:
			handle_error(String((L"SVN encountered an error")));
			return false;
	}
	
}

void SubversionVCS::addFile(const wxFileName& filename)
{
	String name = filename.GetFullPath();
	const Char* name_c[] = {(L"svn"), (L"add"), name.c_str(), nullptr};
	if (!run_svn(name_c)) {
		VCS::addFile(filename);
	}
}

void SubversionVCS::moveFile(const wxFileName& source, const wxFileName& dest)
{
	String source_name = source.GetFullPath(), dest_name = dest.GetFullPath();
	const Char* name_c[] = {(L"svn"), (L"mv"), source_name.c_str(), dest_name.c_str(), nullptr};
	if (!run_svn(name_c)) {
		VCS::moveFile(source, dest);
	}
}

void SubversionVCS::removeFile(const wxFileName& filename)
{
	String name = filename.GetFullPath();
	const Char* name_c[] = {(L"svn"), (L"rm"), name.c_str(), nullptr};
	queue_message(MESSAGE_WARNING, String(name_c[0]) + name_c[1] + name_c[2]);
	// TODO: do we really need to remove the file before calling "svn remove"?
	VCS::removeFile(filename);
	if (!run_svn(name_c)) {
		VCS::removeFile(filename);
	}
}

IMPLEMENT_REFLECTION(SubversionVCS) {
	REFLECT_IF_NOT_READING {
		String type = (L"subversion");
		REFLECT(type);
	}
}

// ----------------------------------------------------------------------------- : EOF
