//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2012 Twan van Laarhoven and Sean Hunt             |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/io/package_manager.hpp>
#include <util/spell_checker.hpp>
#include <data/game.hpp>
#include <data/set.hpp>
#include <data/settings.hpp>
#include <data/locale.hpp>
#include <data/installer.hpp>
#include <data/format/formats.hpp>
#include <cli/cli_main.hpp>
#include <cli/text_io_handler.hpp>
#include <gui/welcome_window.hpp>
#include <gui/update_checker.hpp>
#include <gui/packages_window.hpp>
#include <gui/set/window.hpp>
#include <gui/symbol/window.hpp>
#include <gui/thumbnail_thread.hpp>
#include <wx/fs_inet.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/socket.h>

ScriptValueP export_set(SetP const& set, vector<CardP> const& cards, ExportTemplateP const& exp, String const& outname);


// ----------------------------------------------------------------------------- : Main function/class

/// The application class for MSE.
/** This class is used by wxWidgets as a kind of 'main function'
 */
class MSE : public wxApp {
  public:
	/// Do nothing. The command line parsing, etc. is done in OnRun
	bool OnInit() { return true; }
	/// Main startup function of the program
	/** Use OnRun instead of OnInit, so we can determine whether or not we need a main loop
	 *  Also, OnExit is always run.
	 */
	int OnRun();
	/// Actually start the GUI mainloop
	int runGUI();
	/// On exit: write the settings to the config file
	int OnExit();
	/// On exception: display error message
	void HandleEvent(wxEvtHandler *handler, wxEventFunction func, wxEvent& event) const;
	/// Hack around some wxWidget idiocies
	int FilterEvent(wxEvent& ev);
	/// Fancier assert
	#if defined(_MSC_VER) && defined(_DEBUG) && defined(_CRT_WIDE)
		void OnAssert(const wxChar *file, int line, const wxChar *cond, const wxChar *msg);
	#endif
};

IMPLEMENT_APP(MSE)


// ----------------------------------------------------------------------------- : Initialization

int MSE::OnRun() {
	try {
		#ifdef __WXMSW__
			SetAppName((L"Magic Set Editor"));
		#else
			// Platform friendly appname
			SetAppName((L"magicseteditor"));
		#endif
		wxInitAllImageHandlers();
		wxFileSystem::AddHandler(new wxInternetFSHandler); // needed for update checker
		wxSocketBase::Initialize();
		init_script_variables();
		init_file_formats();
		cli.init();
		package_manager.init();
		settings.read();
		the_locale = Locale::byName(settings.locale);

		// interpret command line
		{
			vector<String> args;
			for (int i = 1 ; i < argc ; ++i) {
				args.push_back(argv[i]);
				if (args.back() == (L"--color")) args.pop_back(); // ingnore the --color argument, it is handled by cli.init()
			}
			if (!args.empty()) {
				// Find the extension
				wxFileName f(args[0].Mid(0,args[0].find_last_not_of((L"\\/"))+1));
				if (f.GetExt() == (L"mse-symbol")) {
					// Show the symbol editor
					Window* wnd = new SymbolWindow(nullptr, args[0]);
					wnd->Show();
					return runGUI();
				} else if (f.GetExt() == (L"mse-set") || f.GetExt() == (L"mse") || f.GetExt() == (L"set")) {
					// Show the set window
					Window* wnd = new SetWindow(nullptr, import_set(args[0]));
					wnd->Show();
					return runGUI();
				} else if (f.GetExt() == (L"mse-installer")) {
					// Installer; install it
					InstallType type = settings.install_type;
					if (args.size() >= 2) {
						if (starts_with(args[1], (L"--"))) {
							parse_enum(String(args[1]).substr(2), type);
						}
					}
					InstallerP installer = open_package<Installer>(args[0]);
					PackagesWindow wnd(nullptr, installer);
					wnd.ShowModal();
					return EXIT_SUCCESS;
				} else if (args[0] == (L"--symbol-editor")) {
					Window* wnd = new SymbolWindow(nullptr);
					wnd->Show();
					return runGUI();
				} else if (args[0] == (L"--create-installer")) {
					// create an installer
					Installer inst;
					for(auto& arg : args) {
						if (!starts_with(arg,(L"--"))) {
							inst.addPackage(arg);
						}
					}
					if (inst.prefered_filename.empty()) {
						throw Error((L"Specify packages to include in installer"));
					} else {
						inst.saveAs(inst.prefered_filename, false);
					}
					return EXIT_SUCCESS;
				} else if (args[0] == (L"--help") || args[0] == (L"-?") || args[0] == (L"/?")) {
					// command line help
					cli << (L"Magic Set Editor\n\n");
					cli << (L"Usage: ") << BRIGHT << argv[0] << NORMAL << (L" [") << PARAM << (L"OPTIONS") << NORMAL << (L"]");
					cli << (L"\n\n  no options");
					cli << (L"\n         \tStart the MSE user interface showing the welcome window.");
					cli << (L"\n\n  ") << BRIGHT << (L"-?") << NORMAL << (L", ")
									   << BRIGHT << (L"--help") << NORMAL;
					cli << (L"\n         \tShows this help screen.");
					cli << (L"\n\n  ") << BRIGHT << (L"-v") << NORMAL << (L", ")
									   << BRIGHT << (L"--version") << NORMAL;
					cli << (L"\n         \tShow version information.");
					cli << (L"\n\n  ") << PARAM << (L"FILE") << FILE_EXT << (L".mse-set") << NORMAL << (L", ")
									   << PARAM << (L"FILE") << FILE_EXT << (L".set") << NORMAL << (L", ")
									   << PARAM << (L"FILE") << FILE_EXT << (L".mse") << NORMAL;
					cli << (L"\n         \tLoad the set file in the MSE user interface.");
					cli << (L"\n\n  ") << PARAM << (L"FILE") << FILE_EXT << (L".mse-symbol") << NORMAL;
					cli << (L"\n         \tLoad the symbol into the MSE symbol editor.");
					cli << (L"\n\n  ") << PARAM << (L"FILE") << FILE_EXT << (L".mse-installer")
									   << NORMAL << (L" [") << BRIGHT << (L"--local") << NORMAL << (L"]");
					cli << (L"\n         \tInstall the packages from the installer.");
					cli << (L"\n         \tIf the ") << BRIGHT << (L"--local") << NORMAL << (L" flag is passed, install packages for this user only.");
					cli << (L"\n\n  ") << BRIGHT << (L"--symbol-editor") << NORMAL;
					cli << (L"\n         \tShow the symbol editor instead of the welcome window.");
					cli << (L"\n\n  ") << BRIGHT << (L"--create-installer") << NORMAL << (L" [")
									   << PARAM << (L"OUTFILE") << FILE_EXT << (L".mse-installer") << NORMAL << (L"] [")
									   << PARAM << (L"PACKAGE") << NORMAL << (L" [") << PARAM << (L"PACKAGE") << NORMAL << (L" ...]]");
					cli << (L"\n         \tCreate an instaler, containing the listed packages.");
					cli << (L"\n         \tIf no output filename is specified, the name of the first package is used.");
					cli << (L"\n\n  ") << BRIGHT << (L"--export") << NORMAL << PARAM << (L" TEMPLATE SETFILE ") << NORMAL << (L" [") << PARAM << (L"OUTFILE") << NORMAL << (L"]");
					cli << (L"\n         \tExport a set using an export template.");
					cli << (L"\n         \tIf no output filename is specified, the result is written to stdout.");
					cli << (L"\n\n  ") << BRIGHT << (L"--export-images") << NORMAL << PARAM << (L" SETFILE") << NORMAL << (L" [") << PARAM << (L"IMAGE") << NORMAL << (L"]");
					cli << (L"\n         \tExport the cards in a set to image files,");
					cli << (L"\n         \tIMAGE is the same format as for 'export all card images'.");
					cli << (L"\n\n  ") << BRIGHT << (L"--cli") << NORMAL << (L" [")
									   << BRIGHT << (L"--quiet") << NORMAL << (L"] [")
									   << BRIGHT << (L"--raw") << NORMAL << (L"] [")
									   << BRIGHT << (L"--script ") << NORMAL << PARAM << (L"FILE") << NORMAL << (L"] [")
									   << PARAM << (L"SETFILE") << NORMAL << (L"]");
					cli << (L"\n         \tStart the command line interface for performing commands on the set file.");
					cli << (L"\n         \tUse ") << BRIGHT << (L"-q") << NORMAL << (L" or ") << BRIGHT << (L"--quiet") << NORMAL << (L" to supress the startup banner and prompts.");
					cli << (L"\n         \tUse ") << BRIGHT << (L"--raw") << NORMAL << (L" for raw output mode.");
					cli << (L"\n         \tUse ") << BRIGHT << (L"--script") << NORMAL << (L" to execute a script file.");
					cli << (L"\n\nRaw output mode is intended for use by other programs:");
					cli << (L"\n    - The only output is only in response to commands.");
					cli << (L"\n    - For each command a single 'record' is written to the standard output.");
					cli << (L"\n    - The record consists of:");
					cli << (L"\n        - A line with an integer status code, 0 for ok, 1 for warnings, 2 for errors");
					cli << (L"\n        - A line containing an integer k, the number of lines to follow");
					cli << (L"\n        - k lines, each containing UTF-8 encoded string data.");
					cli << ENDL;
					cli.flush();
					return EXIT_SUCCESS;
				} else if (args[0] == (L"--version") || args[0] == (L"-v") || args[0] == (L"-V")) {
					// dump version
					cli << (L"Magic Set Editor\n");
					cli << (L"Version ") << app_version.toString() << version_suffix << ENDL;
					cli.flush();
					return EXIT_SUCCESS;
				} else if (args[0] == (L"--cli")) {
					// command line interface
					SetP set;
					vector<String> scripts;
					bool quiet = false;
					for (size_t i = 1 ; i < args.size() ; ++i) {
						String const& arg = args[i];
						wxFileName f(arg);
						if (f.GetExt() == (L"mse-set") || f.GetExt() == (L"mse") || f.GetExt() == (L"set")) {
							set = import_set(arg);
						} else if (arg == (L"-q") || arg == (L"--quiet") || arg == (L"--silent")) {
							quiet = true;
						} else if (arg == (L"-r") || arg == (L"--raw")) {
							quiet = true;
							cli.enableRaw();
						} else if ((arg == (L"-s") || arg == (L"--script")) && i+1 < args.size()) {
							scripts.push_back(args[i+1]);
							++i;
						} else if (arg == (L"--color")) {
							// ignore
						} else {
							throw Error((L"Invalid command line argument: ") + arg);
						}
					}
					CLISetInterface cli_interface(set,quiet);
					if (scripts.empty()) {
						cli_interface.run_interactive();
					} else {
						for(auto& script : scripts) {
							cli_interface.run_script_file(script);
						}
					}
					return EXIT_SUCCESS;
				} else if (args[0] == (L"--export-images")) {
					if (args.size() < 2) {
						throw Error((L"No input file specified for --export-images"));
					}
					SetP set = import_set(args[1]);
					// path
					String out = args.size() >= 3
							   ? args[2]
							   : settings.gameSettingsFor(*set->game).images_export_filename;
					String path = (L".");
					size_t pos = out.find_last_of((L"/\\"));
					if (pos != String::npos) {
						path = out.substr(0, pos);
						if (!wxDirExists(path)) wxMkdir(path);
						path += (L"/x");
						out  = out.substr(pos + 1);
					}
					// export
					export_images(set, set->cards, path, out, CONFLICT_NUMBER_OVERWRITE);
					return EXIT_SUCCESS;
				} else if (args[0] == (L"--export")) {
					if (args.size() < 2) {
						throw Error((L"No export template specified for --export"));
					} else if (args.size() < 3) {
						throw Error((L"No input set file specified for --export"));
					}
					String export_template = args[1];
					ExportTemplateP exp = ExportTemplate::byName(export_template);
					SetP set = import_set(args[2]);
					String out = args.size() >= 4 ? args[3] : (L"");
					ScriptValueP result = export_set(set, set->cards, exp, out);
					if (out.empty()) {
						cli << result->toString();
					}
					return EXIT_SUCCESS;
				} else {
					throw Error((L"Invalid command line argument: ") + args[0]);
				}
			}
		}

		// no command line arguments, or error, show welcome window
		(new WelcomeWindow())->Show();
		return runGUI();

	} CATCH_ALL_ERRORS(true);
	cli.print_pending_errors();
	return EXIT_FAILURE;
}

int MSE::runGUI() {
	check_updates();
	return wxApp::OnRun();
}

// ----------------------------------------------------------------------------- : Exit

int MSE::OnExit() {
	thumbnail_thread.abortAll();
	settings.write();
	package_manager.destroy();
	SpellChecker::destroyAll();
	return 0;
}

// ----------------------------------------------------------------------------- : Exception handling

void MSE::HandleEvent(wxEvtHandler *handler, wxEventFunction func, wxEvent& event) const {
	try {
		wxApp::HandleEvent(handler, func, event);
		return;
	} CATCH_ALL_ERRORS(true);
	if (wxNotifyEvent* nev = dynamic_cast<wxNotifyEvent*>(&event)) {
		// notifications about for instance list box selection will fail if an exception is thrown
		nev->Veto();
	}
}

#if defined(_MSC_VER) && defined(_DEBUG) && defined(_CRT_WIDE)
	// Print assert failures to debug output
	void MSE::OnAssert(const wxChar *file, int line, const wxChar *cond, const wxChar *msg) {
        msvc_assert(msg, cond, file, line);
	}
#endif

// ----------------------------------------------------------------------------- : Events

int MSE::FilterEvent(wxEvent& ev) {
	/*if (ev.GetEventType() == wxEVT_MOUSE_CAPTURE_LOST) {
		return 1;
	} else {
		return -1;
	}*/
	return -1;
}
