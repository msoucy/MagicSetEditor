//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2007 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <data/game.hpp>
#include <data/field.hpp>
#include <data/keyword.hpp>
#include <data/statistics.hpp>
#include <data/pack.hpp>
#include <util/io/package_manager.hpp>
#include <script/script.hpp>

DECLARE_TYPEOF_COLLECTION(FieldP);
DECLARE_TYPEOF_COLLECTION(StatsDimensionP);

// ----------------------------------------------------------------------------- : Game

IMPLEMENT_DYNAMIC_ARG(Game*, game_for_reading, nullptr);

Game::Game()
	: has_keywords(false)
	, dependencies_initialized(false)
{}

GameP Game::byName(const String& name) {
	return packages.open<Game>(name + _(".mse-game"));
}

bool Game::isMagic() const {
	return name() == _("magic");
}

String Game::typeNameStatic() { return _("game"); }
String Game::typeName() const { return _("game"); }

IMPLEMENT_REFLECTION(Game) {
	REFLECT_BASE(Packaged);
	REFLECT_NO_SCRIPT(init_script);
	REFLECT_NO_SCRIPT(set_fields);
	REFLECT_IF_READING {
		default_set_style.init(set_fields);
	}
	REFLECT_NO_SCRIPT(default_set_style);
	REFLECT_NO_SCRIPT(card_fields);
	REFLECT_NO_SCRIPT(statistics_dimensions);
	REFLECT_NO_SCRIPT(statistics_categories);
	REFLECT_NO_SCRIPT(pack_types);
	REFLECT(has_keywords);
	REFLECT(keyword_modes);
	REFLECT(keyword_parameter_types);
	REFLECT_NO_SCRIPT(keywords);
//	REFLECT(word_lists);
}

void Game::validate(Version v) {
	Packaged::validate(v);
	// automatic statistics dimensions
	{
		vector<StatsDimensionP> dims;
		FOR_EACH(f, card_fields) {
			if (f->show_statistics) {
				dims.push_back(new_intrusive1<StatsDimension>(*f));
			}
		}
		statistics_dimensions.insert(statistics_dimensions.begin(), dims.begin(), dims.end()); // push front
	}
	// automatic statistics categories
	{
		vector<StatsCategoryP> cats;
		FOR_EACH(dim, statistics_dimensions) {
			cats.push_back(new_intrusive1<StatsCategory>(dim));
		}
		statistics_categories.insert(statistics_categories.begin(), cats.begin(), cats.end()); // push front
	}
}

void addStatsDimensionsForFields();

// special behaviour of reading/writing GamePs: only read/write the name

void Reader::handle(GameP& game) {
	game = Game::byName(getValue());
}
void Writer::handle(const GameP& game) {
	if (game) handle(game->name());
}