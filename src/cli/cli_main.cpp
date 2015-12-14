//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2012 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include "util/prec.hpp"
#include "util/error.hpp"
#include "cli/cli_main.hpp"
#include "cli/text_io_handler.hpp"
#include "script/functions/functions.hpp"
#include "script/profiler.hpp"
#include "data/format/formats.hpp"
#include <wx/process.h>
#include <wx/wfstream.h>
#include <boost/range/adaptor/reversed.hpp>

using std::min;

String read_utf8_line(wxInputStream& input, bool eat_bom = true, bool until_eof = false);


// ----------------------------------------------------------------------------- : Command line interface

CLISetInterface::CLISetInterface(const SetP& set, bool quiet)
	: quiet(quiet)
	, our_context(nullptr)
{
	if (!cli.haveConsole()) {
		throw Error(L"Can not run command line interface without a console;\nstart MSE with \"mse.com --cli\"");
	}
	ei.allow_writes_outside = true;
	setExportInfoCwd();
	setSet(set);
	// show welcome logo
	if (!quiet) showWelcome();
	cli.print_pending_errors();
}

CLISetInterface::~CLISetInterface() {
	delete our_context;
}

Context& CLISetInterface::getContext() {
	if (set) {
		return set->getContext();
	} else {
		if (!our_context) {
			our_context = new Context();
			init_script_functions(*our_context);
		}
		return *our_context;
	}
}

void CLISetInterface::onBeforeChangeSet() {
	if (set || our_context) {
		Context& ctx = getContext();
		ctx.closeScope(scope);
	}
}

void CLISetInterface::onChangeSet() {
	Context& ctx = getContext();
	scope = ctx.openScope();
	ei.set = set;
}

void CLISetInterface::setExportInfoCwd() {
	// write to the current directory
	ei.directory_relative = ei.directory_absolute = wxGetCwd();
	// read from the current directory
	ei.export_template = intrusive(new Package());
	ei.export_template->open(ei.directory_absolute, true);
}


// ----------------------------------------------------------------------------- : Running

void CLISetInterface::run_interactive() {
	// loop
	running = true;
	while (running) {
		// show prompt
		if (!quiet) {
			cli << GRAY << L"> " << NORMAL;
			cli.flush();
		}
		// read line from stdin
		String command = cli.getLine();
		if (command.empty() && !cli.canGetLine()) break;
		handleCommand(command);
		cli.print_pending_errors();
		cli.flush();
		cli.flushRaw();
	}
}

bool CLISetInterface::run_script(ScriptP const& script) {
	try {
		WITH_DYNAMIC_ARG(export_info, &ei);
		Context& ctx = getContext();
		ScriptValueP result = ctx.eval(*script,false);
		// show result (?)
		cli << result->toCode() << ENDL;
		return true;
	} CATCH_ALL_ERRORS(true);
	return false;
}

bool CLISetInterface::run_script_string(String const& command, bool multiline) {
	vector<ScriptParseError> errors;
	ScriptP script = parse(command,nullptr,false,errors);
	if (errors.empty()) {
		return run_script(script);
	} else {
		for(auto& error :errors) {
			if (multiline) {
				cli.show_message(MESSAGE_ERROR, String::Format(L"On line %d:\t", error.line) + error.what());
			} else {
				cli.show_message(MESSAGE_ERROR, error.what());
			}
		}
		return false;
	}
}

bool CLISetInterface::run_script_file(String const& filename) {
	// read file
	if (!wxFileExists(filename)) {
		cli.show_message(MESSAGE_ERROR, L"File not found: "+filename);
		return false;
	}
	wxFileInputStream is(filename);
	if (!is.Ok()) {
		cli.show_message(MESSAGE_ERROR, L"Unable to open file: "+filename);
		return false;
	}
	wxBufferedInputStream bis(is);
	String code = read_utf8_line(bis, true, true);
	run_script_string(code);
	return true;
}

void CLISetInterface::showWelcome() {
    // clang-format off
    cli << LR"++(\
+---------------------------------------------------------------------------+
|                                                                     ___   |
|  __  __           _       ___     _      ___    _ _ _              |__ \  |
| |  \/  |__ _ __ _(_)__   / __|___| |_   | __|__| (_) |_ ___ _ _       ) | |
| | |\/| / _` / _` | / _|  \__ | -_)  _|  | _|/ _` | |  _/ _ \ '_|     / /  |
| |_|  |_\__,_\__, |_\__|  |___|___|\__|  |___\__,_|_|\__\___/_|      / /_  |
|             |___/                                                  |____| |
|                                                                           |
+---------------------------------------------------------------------------+
)++";
    // clang-format on
    cli.flush();
}

void CLISetInterface::showUsage() {
	cli << L" Commands available from the prompt:\n\n";
	cli << L"   <expression>        Execute a script expression, display the result\n";
	cli << L"   :help               Show this help page.\n";
	cli << L"   :load <setfile>     Load a different set file.\n";
	cli << L"   :quit               Exit the MSE command line interface.\n";
	cli << L"   :reset              Clear all local variable definitions.\n";
	cli << L"   :pwd                Print the current working directory.\n";
	cli << L"   :cd                 Change the working directory.\n";
	cli << L"   :! <command>        Perform a shell command.\n";
	cli << L"\n Commands can be abreviated to their first letter if there is no ambiguity.\n\n";
}

void CLISetInterface::handleCommand(const String& command) {
	try {
		if (command.empty()) {
			// empty, ignore
		} else if (command.GetChar(0) == L':') {
			// :something
			size_t space = min(command.find_first_of(L' '), command.size());
			String before = command.substr(0,space);
			String arg    = space + 1 < command.size() ? command.substr(space+1) : wxEmptyString;
			if (before == L":q" || before == L":quit") {
				if (!quiet) {
					cli << L"Goodbye\n";
				}
				running = false;
			} else if (before == L":?" || before == L":h" || before == L":help") {
				showUsage();
			} else if (before == L":l" || before == L":load") {
				if (arg.empty()) {
					cli.show_message(MESSAGE_ERROR, L"Give a filename to open.");
				} else {
					setSet(import_set(arg));
				}
			} else if (before == L":r" || before == L":reset") {
				Context& ctx = getContext();
				ei.exported_images.clear();
				ctx.closeScope(scope);
				scope = ctx.openScope();
			} else if (before == L":i" || before == L":info") {
				if (set) {
					cli << L"set:      " << set->identification() << ENDL;
					cli << L"filename: " << set->absoluteFilename() << ENDL;
					cli << L"relative: " << set->relativeFilename() << ENDL;
					cli << String::Format(L"#cards:   %d", set->cards.size()) << ENDL;
				} else {
					cli << L"No set loaded" << ENDL;
				}
			} else if (before == L":c" || before == L":cd") {
				if (arg.empty()) {
					cli.show_message(MESSAGE_ERROR, L"Give a new working directory.");
				} else {
					if (!wxSetWorkingDirectory(arg)) {
						cli.show_message(MESSAGE_ERROR,L"Can't change working directory to "+arg);
					} else {
						setExportInfoCwd();
					}
				}
			} else if (before == L":pwd" || before == L":p") {
				cli << ei.directory_absolute << ENDL;
			} else if (before == L":!") {
				if (arg.empty()) {
					cli.show_message(MESSAGE_ERROR, L"Give a shell command to execute.");
				} else {
                    #ifdef __WXMSW__
                        _wsystem(arg.c_str()); // TODO: is this function available on other platforms?
                    #else
                        wxCharBuffer buf = arg.fn_str();
                        system(buf);
                    #endif
				}
			#if USE_SCRIPT_PROFILING
				} else if (before == L":profile") {
					if (arg == L"full") {
						showProfilingStats(profile_root);
					} else {
						long level = 1;
						arg.ToLong(&level);
						showProfilingStats(profile_aggregated(level));
					}
			#endif
			} else {
				cli.show_message(MESSAGE_ERROR, L"Unknown command, type :help for help.");
			}
		} else if (command == L"exit" || command == L"quit") {
			cli << L"Use :quit to quit\n";
		} else if (command == L"help") {
			cli << L"Use :help for help\n";
		} else {
			run_script_string(command);
		}
	} catch (const Error& e) {
		cli.show_message(MESSAGE_ERROR,e.what());
	}
}

#if USE_SCRIPT_PROFILING
	void CLISetInterface::showProfilingStats(const FunctionProfile& item, int level) {
		// show parent
		if (level == 0) {
			cli << GRAY << L"Time(s)   Avg (ms)  Calls   Function" << ENDL;
			cli <<         L"========  ========  ======  ===============================" << NORMAL << ENDL;
		} else {
			for (int i = 1 ; i < level ; ++i) cli << L"  ";
			cli << String::Format(L"%8.5f  %8.5f  %6d  %s",
                    item.total_time(), 1000 * item.avg_time(),
                    item.calls, item.name.c_str()) << ENDL;
		}
		// show children
		vector<FunctionProfileP> children;
		item.get_children(children);
		for(auto& c : boost::adaptors::reverse(children)) {
			showProfilingStats(*c, level + 1);
		}
	}
#endif
