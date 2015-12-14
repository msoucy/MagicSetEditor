//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2012 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// -----------------------------------------------------------------------------
// : Includes

#include <util/prec.hpp>
#include <data/format/formats.hpp>
#include <data/set.hpp>

DECLARE_POINTER_TYPE(FileFormat);

// -----------------------------------------------------------------------------
// : Formats

// All supported file formats
vector<FileFormatP> file_formats;

void init_file_formats() {
    file_formats.push_back(mse2_file_format());
    file_formats.push_back(mse1_file_format());
    file_formats.push_back(mtg_editor_file_format());
}

String import_formats() {
    String all_extensions; // type1;type2
    String type_strings;   // |name1|type1|name2|type2
    for (auto &f : file_formats) {
        if (f->canImport()) {
            if (!all_extensions.empty())
                all_extensions += (L";");
            all_extensions += f->matches();
            type_strings += (L"|") + f->name() + (L"|") + f->matches();
        }
    }
    return (L"Set files|") + all_extensions + type_strings +
           (L"|All files (*.*)|*");
}

String export_formats(const Game &game) {
    String type_strings; // name1|type1|name2|type2
    for (auto &f : file_formats) {
        if (f->canExport(game)) {
            if (!type_strings.empty())
                type_strings += (L"|");
            type_strings += f->name() + (L"|") + f->matches();
        }
    }
    return type_strings;
}

void export_set(Set &set, const String &filename, size_t format_index,
                bool is_copy) {
    FileFormatP format = file_formats.at(format_index);
    if (!format->canExport(*set.game)) {
        throw InternalError((L"File format doesn't apply to set"));
    }
    format->exportSet(set, filename, is_copy);
}

SetP import_set(String name) {
    size_t pos = name.find_last_of((L'.'));
    String extension = pos == String::npos ? (L"") : name.substr(pos + 1);
    // determine format
    for (auto &f : file_formats) {
        if (f->extension() == extension) {
            return f->importSet(name);
        }
    }
    // default: use first format = MSE2 format
    assert(!file_formats.empty() && file_formats[0]->canImport());
    return file_formats[0]->importSet(name);
}
