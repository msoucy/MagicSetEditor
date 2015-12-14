//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2012 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// Includes {{{
#include "error.hpp"
#include <wx/defs.h>                // for wxICON_ERROR, wxOK
#include <wx/msgdlg.h>              // for wxMessageBox
#include <wx/setup.h>               // for wxUSE_STACKWALKER
#include <wx/thread.h>              // for wxMutexLocker, wxThread, wxMutex
#if wxUSE_STACKWALKER
#  include <wx/stackwalk.h>         // for wxStackFrame, wxStackWalker
#endif
#include <wx/wxchar.h>              // for wxT
#include <cli/text_io_handler.hpp>  // for TextIOHandler, cli
#include <deque>                    // for deque
#include <util/error.hpp>           // for MessageType, Error, etc
#include <utility>                  // for make_pair, pair
#include "util/string.hpp"          // for String

using std::deque;
using std::make_pair;
using std::pair;

// Includes }}}

// -----------------------------------------------------------------------------
// : Debug utilities

#if defined(_MSC_VER) && defined(_DEBUG) && defined(_CRT_WIDE)
void msvc_assert(const wchar_t *msg, const wchar_t *expr, const wchar_t *file,
                 unsigned line) {
    if (IsDebuggerPresent()) {
        wchar_t buffer[1024];
        if (msg) {
            wsprintf(buffer, L"Assertion failed: %s: %s, file %s, line %d\n",
                     msg, expr, file, line);
        } else {
            wsprintf(buffer, L"Assertion failed: %s, file %s, line %d\n", expr,
                     file, line);
        }
        OutputDebugStringW(buffer);
        __debugbreak();
    } else {
        _wassert(expr, file, line);
    }
}
#endif

// -----------------------------------------------------------------------------
// : Error types

Error::Error(const String &message) : message(message) {}

Error::~Error() {}

String Error::what() const { return message; }

// Stolen from wx/appbase.cpp
// we can't just call it, because of static linkage
#if wxUSE_STACKWALKER
String get_stack_trace() {
    wxString stackTrace;

    class StackDumper : public wxStackWalker {
      public:
        StackDumper() {}

        const wxString &GetStackTrace() const { return m_stackTrace; }

      protected:
        virtual void OnStackFrame(const wxStackFrame &frame) {
            m_stackTrace << wxString::Format((L"[%02d] "), frame.GetLevel());

            wxString name = frame.GetName();
            if (!name.empty()) {
                m_stackTrace << wxString::Format((L"%-40s"), name.c_str());
            } else {
                m_stackTrace
                    << wxString::Format((L"%p"), (void *)frame.GetAddress());
            }

            if (frame.HasSourceLocation()) {
                m_stackTrace << (L'\t') << frame.GetFileName() << (L':')
                             << (unsigned int)frame.GetLine();
            }

            m_stackTrace << (L'\n');
        }

      private:
        wxString m_stackTrace;
    };

    StackDumper dump;
    dump.Walk(2); // don't show InternalError() call itself
    stackTrace = dump.GetStackTrace();

    // don't show more than maxLines or we could get a dialog too tall to be
    // shown on screen: 20 should be ok everywhere as even with 15 pixel high
    // characters it is still only 300 pixels...
    static const int maxLines = 20;
    const int count = stackTrace.Freq(wxT('\n'));
    for (int i = 0; i < count - maxLines; i++)
        stackTrace = stackTrace.BeforeLast(wxT('\n'));

    return stackTrace;
}
#else
String get_stack_trace() {
    return (L""); // not supported
}
#endif // wxUSE_STACKWALKER

InternalError::InternalError(const String &str)
    : Error(L"An internal error occured:\n\n" + str +
            L"\n"
            L"Please save your work (use 'save as' to so you don't overwrite "
            L"things)\n"
            L"and restart Magic Set Editor.\n\n"
            L"You should leave a bug report on "
            L"http://magicseteditor.sourceforge.net/\n"
            L"Press Ctrl+C to copy this message to the clipboard.") {
    // add a stacktrace
    const String stack_trace = get_stack_trace();
    if (!stack_trace.empty()) {
        message << (L"\n\nCall stack:\n") << stack_trace;
    }
}

// -----------------------------------------------------------------------------
// : Parse errors

ScriptParseError::ScriptParseError(size_t pos, int line, const String &filename,
                                   const String &error)
    : ParseError(error), start(pos), end(pos), line(line), filename(filename) {}
ScriptParseError::ScriptParseError(size_t pos, int line, const String &filename,
                                   const String &exp, const String &found)
    : ParseError((L"Expected '") + exp + (L"' instead of '") + found + (L"'")),
      start(pos), end(pos + found.size()), line(line), filename(filename) {}
ScriptParseError::ScriptParseError(size_t pos1, size_t pos2, int line,
                                   const String &filename, const String &open,
                                   const String &close, const String &found)
    : ParseError((L"Expected closing '") + close + (L"' for this '") + open +
                 (L"' instead of '") + found + (L"'")),
      start(pos1), end(pos2 + found.size()), line(line), filename(filename) {}
String ScriptParseError::what() const {
    return String((L"(")) << (int)start << (L"): ") << Error::what();
}

String concat(const vector<ScriptParseError> &errors) {
    String total;
    for (const auto &e : errors) {
        if (!total.empty())
            total += (L"\n");
        total += e.what();
    }
    return total;
}
ScriptParseErrors::ScriptParseErrors(const vector<ScriptParseError> &errors)
    : ParseError(concat(errors)) {}

// -----------------------------------------------------------------------------
// : Error handling

// messages can be posted from other threads, this mutex protects the message
// list
wxMutex crit_error_handling;
typedef pair<MessageType, String> Message;
deque<Message> message_queue;
bool show_message_box_for_fatal_errors = true;
bool write_errors_to_cli = false;

void queue_message(MessageType type, String const &msg) {
    if (write_errors_to_cli && wxThread::IsMain()) {
        cli.show_message(type, msg);
        return; // TODO: is this the right thing to do?
    }
    if (show_message_box_for_fatal_errors && type == MESSAGE_FATAL_ERROR &&
        wxThread::IsMain()) {
        // bring this to the user's attention right now!
        wxMessageBox(msg, (L"Error"), wxOK | wxICON_ERROR);
    }
    // Thread safety
    wxMutexLocker lock(crit_error_handling);
    // Only show errors in the main thread
    message_queue.push_back(make_pair(type, msg));
}

void handle_error(const Error &e) {
    queue_message(e.is_fatal() ? MESSAGE_FATAL_ERROR : MESSAGE_ERROR, e.what());
}

bool have_queued_message() {
    wxMutexLocker lock(crit_error_handling);
    return !message_queue.empty();
}

bool get_queued_message(MessageType &type, String &msg) {
    wxMutexLocker lock(crit_error_handling);
    if (message_queue.empty()) {
        return false;
    } else {
        type = message_queue.back().first;
        msg = message_queue.back().second;
        message_queue.pop_back();
        return true;
    }
}
