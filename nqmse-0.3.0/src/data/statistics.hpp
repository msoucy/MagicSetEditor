//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#ifndef HEADER_DATA_STATISTICS
#define HEADER_DATA_STATISTICS

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/reflect.hpp>
#include <script/scriptable.hpp>

class Field;
DECLARE_POINTER_TYPE(StatsDimension);
DECLARE_POINTER_TYPE(StatsCategory);

// ----------------------------------------------------------------------------- : Statistics dimension

/// A dimension that can be plotted as an axis in a graph
/** Dimensions can be generated automatically based on card fields */
class StatsDimension {
  public:
	StatsDimension();
	StatsDimension(const Field&);
	
	String         name;			///< Name of this dimension
	String         description;		///< Description, used in status bar
	String         icon_filename;	///< Icon for lists
	OptionalScript script;			///< Script that determines the value(s)
	bool           numeric;			///< Are the values numeric? If so, they require special sorting
	bool           automatic;		///< Based on a card field?
	
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : Statistics category

/// Types of graphs
enum GraphType
{	GRAPH_TYPE_BAR
,	GRAPH_TYPE_PIE
};

/// A category for statistics
/** Can be generated automatically based on a dimension */
class StatsCategory {
  public:
	StatsCategory();
	StatsCategory(const StatsDimensionP&);
	
	String                  name;			///< Name/label
	String                  description;	///< Description, used in status bar
	String                  icon_filename;	///< Icon for lists
	Bitmap                  icon;			///< The loaded icon (optional of course)
	vector<StatsDimensionP> dimensions;		///< The dimensions to use, higher dimensions may be null
	GraphType               type;			///< Type of graph to use
	bool                    automatic;		///< Automatically generated?
	
	DECLARE_REFLECTION();
};

// ----------------------------------------------------------------------------- : EOF
#endif