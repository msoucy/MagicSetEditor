//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2012 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// -----------------------------------------------------------------------------
// : Includes

#include <util/prec.hpp>
#include <util/file_utils.hpp>
#include <wx/filename.h>
#include <wx/dir.h>
#include <errno.h>
#include <sys/stat.h>
#include <boost/range/adaptor/reversed.hpp>

// -----------------------------------------------------------------------------
// : File names

String normalize_filename(const String &name) {
    wxFileName fn(name);
    fn.Normalize();
    return fn.GetFullPath();
}

String normalize_internal_filename(const String &name) {
    String ret;
    for (const auto &c : name) {
        if (c == (L'\\'))
            ret += (L'/');
        else
            ret += toLower(c);
    }
    return ret;
}

bool ignore_file(const String &name) {
    // Files that are never part of a package,
    // i.e. random stuff the OS file manager dumps without being asked
    return name == (L"Thumbs.db"); // winXP explorer thumbnails
}

String add_extension(const String &filename, String const &extension) {
    if (extension.size() <= filename.size() &&
        is_substr(filename, filename.size() - extension.size(), extension)) {
        return filename;
    } else {
        return filename + extension;
    }
}

bool is_filename_char(Char c) {
    return isAlnum(c) || c == (L' ') || c == (L'_') || c == (L'-') ||
           c == (L'.');
}

String clean_filename(const String &name) {
    String clean;
    // allow only valid characters, and remove leading whitespace
    bool start = true;
    for (const auto &c : name) {
        if (is_filename_char(c) && !(start && c == (L' '))) {
            start = false;
            clean += c;
        }
    }
    // remove trailing whitespace
    while (!clean.empty() && clean[clean.size() - 1] == (L' ')) {
        clean.resize(clean.size() - 1);
    }
    if (clean.empty() || starts_with(clean, (L"."))) {
        clean = (L"no-name") + clean;
    }
    return clean;
}

bool resolve_filename_conflicts(wxFileName &fn, FilenameConflicts conflicts,
                                set<String> &used) {
    switch (conflicts) {
    case CONFLICT_KEEP_OLD:
        return !fn.FileExists();
    case CONFLICT_OVERWRITE:
        return true;
    case CONFLICT_NUMBER: {
        int i = 0;
        String ext = fn.GetExt();
        while (fn.FileExists()) {
            fn.SetExt(String() << ++i << (L".") << ext);
        }
        return true;
    }
    case CONFLICT_NUMBER_OVERWRITE: {
        int i = 0;
        String ext = fn.GetExt();
        while (used.find(fn.GetFullPath()) != used.end()) {
            fn.SetExt(String() << ++i << (L".") << ext);
        }
        return true;
    }
    default: {
        throw InternalError((L"resolve_filename_conflicts: default case"));
    }
    }
}

// -----------------------------------------------------------------------------
// : File info

time_t file_modified_time(const String &path) {
    // Note: wxFileName also provides a function for this, but that is very
    // slow.
    struct stat statbuf;
    if (stat(path.mb_str(), &statbuf) != 0) {
        if (errno == ENOENT) {
            return 0;
        } else {
            throw InternalError((L"could not stat ") + path);
        }
    }
    return statbuf.st_mtime;
}

// -----------------------------------------------------------------------------
// : Directories

bool create_parent_dirs(const String &file) {
    for (size_t pos = file.find_first_of((L"\\/"), 1); pos != String::npos;
         pos = file.find_first_of((L"\\/"), pos + 1)) {
        String part = file.substr(0, pos);
        if (!wxDirExists(part)) {
            if (!wxMkdir(part))
                return false;
        }
    }
    return true;
}

// -----------------------------------------------------------------------------
// : Removing

class RecursiveDeleter : public wxDirTraverser {
  public:
    RecursiveDeleter(const String &start) {
        to_delete.push_back(start);
        ok = true;
    }

    bool ok;

    void remove() {
        for (auto &dir : boost::adaptors::reverse(to_delete)) {
            if (!wxRmdir(dir)) {
                ok = false;
                handle_error(L"Cannot delete " + dir +
                             L"\n"
                             L"The remainder of the package has still been "
                             L"removed, if possible.\n"
                             L"Other packages may have been removed, including "
                             L"packages that this on is dependent on. Please "
                             L"remove manually.");
            }
        }
    }

    wxDirTraverseResult OnFile(const String &filename) {
        if (!wxRemoveFile(filename)) {
            ok = false;
            handle_error(L"Cannot delete " + filename +
                         L"\n"
                         L"The remainder of the package has still been "
                         L"removed, if possible.\n"
                         L"Other packages may have been removed, including "
                         L"packages that this on is dependent on. Please "
                         L"remove manually.");
        }
        return wxDIR_CONTINUE;
    }
    wxDirTraverseResult OnDir(const String &dirname) {
        to_delete.push_back(dirname);
        return wxDIR_CONTINUE;
    }

  private:
    vector<String> to_delete;
};

bool remove_file_or_dir(const String &name) {
    if (wxFileExists(name)) {
        return wxRemoveFile(name);
    } else if (wxDirExists(name)) {
        RecursiveDeleter rd(name);
        {
            wxDir dir(name);
            dir.Traverse(rd);
        }
        rd.remove();
        return rd.ok;
    } else {
        return true;
    }
}

// -----------------------------------------------------------------------------
// : Renaming

bool rename_file_or_dir(const String &from, const String &to) {
    create_parent_dirs(to);
    return wxRenameFile(from, to);
}

// -----------------------------------------------------------------------------
// : Moving

class IgnoredMover : public wxDirTraverser {
  public:
    IgnoredMover(const String &from, const String &to) : from(from), to(to) {}
    wxDirTraverseResult OnFile(const String &filename) {
        tryMove(filename);
        return wxDIR_CONTINUE;
    }
    wxDirTraverseResult OnDir(const String &dirname) {
        return tryMove(dirname) ? wxDIR_IGNORE : wxDIR_CONTINUE;
    }

  private:
    String from, to;
    bool tryMove(const String &from_path) {
        if (is_substr(from_path, 0, from)) {
            String to_path = to + from_path.substr(from.size());
            return rename_file_or_dir(from_path, to_path);
        } else {
            // This shouldn't happen
            return false;
        }
    }
};

void move_ignored_files(const String &from_dir, const String &to_dir) {
    if (wxDirExists(from_dir) && wxDirExists(to_dir)) {
        wxDir dir(from_dir);
        IgnoredMover im(from_dir, to_dir);
        dir.Traverse(im);
    }
}
