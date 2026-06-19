#include "pch.h"

#include <CCRepl/CommandBuilder.h>
#include <CCRepl/CommandSet.h>
#include <CCRepl/ReplContext.h>
#include <util/ObjTbl.h>

namespace CCRepl {

	CMD_H(Handler) {
		ctx.WriteLine("Nodal command (No handler, or execute function not yet implemented).\n");
		ctx.WriteLine(args.Cmd->PrintFull(ctx.MaxWidth.value_or(150)));

		if (args.Cmd->Children.size() > 0) {			
			ctx.WriteLine("\nTree:\n");
			ctx.WriteLine(args.Cmd->PrintTree("", ""));
		}
	}

	int WaitSeconds(std::size_t seconds) {
		std::this_thread::sleep_for(std::chrono::seconds(seconds));
		return 0;
	}

	CMD_H(TestSpinner) {
		std::future<int> ft = std::async(std::launch::async, WaitSeconds, args.GetRequired<std::size_t>(0));
		int r = ctx.WaitSpinner<int>(std::move(ft));
		ctx.WriteLine(r);
	}

	CMD_H(TruncateList) {
		std::size_t start = args.GetR<std::size_t>(0);
		std::size_t end = args.GetR<std::size_t>(1);

		std::vector<std::string> stringVec;
		for (std::size_t i = 1; i < 101; i++) {
			stringVec.push_back(std::format("This is item #{}", i));
		}

		ctx.WriteLine("StringVec defined: " + STR_VAR_DEF(stringVec.size()));
		ctx.WriteLine(STR_VAR_DEF(start));
		ctx.WriteLine(STR_VAR_DEF(end));
		ctx.WriteLine("Truncating:\n");
		ctx.WriteLine(str::TruncateList<std::string>(stringVec, "Items", start, end));
				
		ctx.WriteLine("\n Truncating w/ conversion overload:\n");
		ctx.WriteLine(
			str::TruncateList<std::string>(stringVec,
				[](const std::string& i) { return i;},
				"Items", start, end
			));
		ctx.WriteLine("\nDone.");

		// Some code to test this from Claude <3:
		struct TestItem {
			int id;
			double value;
			std::string tag;
		};

		std::vector<TestItem> itemVec;
		for (std::size_t i = 1; i < 101; i++) {
			itemVec.push_back({ (int)i, i * 1.5, i % 2 == 0 ? "even" : "odd" });
		}

		ctx.WriteLine("TestItem vec defined: " + STR_VAR_DEF(itemVec.size()));
		ctx.WriteLine("Truncating w/ conversion:\n");
		ctx.WriteLine(
			str::TruncateList<TestItem>(itemVec,
				[](const TestItem& item) {
					return std::format("Item #{}: value={:.2f}, tag={}", item.id, item.value, item.tag);
				},
				"TestItems", start, end
			)
		);
		ctx.WriteLine("\nDone.");
	}

	CMD_H(BuildTable) {
		int n = args.GetR<int>(0);
		ctx.WriteLine("Testing fmt::BuildTable()");

		struct TestItem {
			int id;
			double value;
			std::string tag;
			static std::vector<fmt::TextTableColumn> GetTableColumns() {
				return {
					fmt::TextTableColumn("Id:", 5),
					fmt::TextTableColumn("Value:", 6, fmt::TextAlign::Left, fmt::TextAlign::Right),
					fmt::TextTableColumn("Tag:", 6)
				};
			}
			std::vector<std::string> GetTableRow() const {
				return {
					std::to_string(id),
					str::ToString(value, 1),
					tag
				};
			}
		};

		std::vector<TestItem> itemVec;
		for (std::size_t i = 1; i < 101; i++) {
			itemVec.push_back({ (int)i, i * 1.5, i % 2 == 0 ? "even" : "odd" });
		}

		struct TestItemConvert {
			int id;
			double value;
			std::string tag;
		};

		std::vector<TestItemConvert> convTtemVec;
		for (std::size_t i = 1; i < 101; i++) {
			convTtemVec.push_back({ (int)i, i * 1.5, i % 2 == 0 ? "even" : "odd" });
		}

		// Do tests here:

		ctx.WriteLine("\nDirect, compact, ascii = false:\n");
		ctx.WriteLine(fmt::BuildTable(itemVec, n, true, false));

		ctx.WriteLine("\nDirect, compact, ascii = true:\n");
		ctx.WriteLine(fmt::BuildTable(itemVec, n, true, true));

		ctx.WriteLine("\nDirect, not compact, ascii = false:\n");
		ctx.WriteLine(fmt::BuildTable(itemVec, n, false, false));

		ctx.WriteLine("\nDirect, not compact, ascii = true:\n");
		ctx.WriteLine(fmt::BuildTable(itemVec, n, false, true));

		std::vector<fmt::TextTableColumn> columns{
			fmt::TextTableColumn("Id:", 5),
			fmt::TextTableColumn("Value:", 6, fmt::TextAlign::Left, fmt::TextAlign::Right),
			fmt::TextTableColumn("Tag:", 6)
		};

		auto Convert = [](const TestItemConvert& row) {
			std::vector<std::string> r{
				std::to_string(row.id),
				str::ToString(row.value, 1),
				row.tag
			};
			return r;
			};

		ctx.WriteLine("\n------------------------------\n");

		ctx.WriteLine("\nConvert, compact, ascii = false:\n");
		ctx.WriteLine(fmt::BuildTable<TestItemConvert>(convTtemVec, columns, Convert, n, true, false));

		ctx.WriteLine("\nConvert, compact, ascii = true:\n");
		ctx.WriteLine(fmt::BuildTable<TestItemConvert>(convTtemVec, columns, Convert, n, true, true));

		ctx.WriteLine("\nConvert, not compact, ascii = false:\n");
		ctx.WriteLine(fmt::BuildTable<TestItemConvert>(convTtemVec, columns, Convert, n, false, false));

		ctx.WriteLine("\nConvert, not compact, ascii = true:\n");
		ctx.WriteLine(fmt::BuildTable<TestItemConvert>(convTtemVec, columns, Convert, n, false, true));

		ctx.WriteLine("\n------------------------------\n");

		ctx.WriteLine("\nTest Add Column functions:\n");
		fmt::TextTable t;
		t.AddColumnLeft("Id").AddColumnRight("Value").AddColumnCenter("Tag:");
		for (const TestItem& i : itemVec) t << i.GetTableRow();
		ctx.WriteLine(t.PrintCompact(false));
	}

	CMD_H(GetByName) {
		ctx.WriteLine("Testing Get() 'by name' overrides (name instead of id). Case sensitive.");

		int argA = args.GetR<int>("argA");
		int argB = args.GetOr<int>("argB", 10);
		std::optional<int> argC = args.Get<int>("argC");

		ctx.WriteLine(STR_VAR_DEF(argA));
		ctx.WriteLine(STR_VAR_DEF(argB));
		ctx.Write("argC = ");
		ctx.WriteLine(argC ? str::ToString(*argC) : "Undefined");
	}

	CMD_H(DoubleToString) {
		double value = args.GetR<double>(0);
		std::size_t prec = args.GetR<std::size_t>(1);
		bool compact = args.GetR<bool>(2);
		std::string expected = args.GetR<std::string>(3);

		std::string actual = str::ToString(value, prec, compact);

		if (expected == actual)
			ctx.WriteLine("[x] Exp.: " + str::TruncatePadRight(expected, 10) + " == Act.: " + actual);
		else {
			ctx.WriteLine("[ ] Exp.: " + str::TruncatePadRight(expected, 10) + " != Act.: " + actual);
		}
			
	}

	CMD_H(VariadicPrintLine) {
		ctx.WriteLine("This is a thing", "And this is also a thing", "so many things!", 29, 24, "hello", 87.5 / 12);
		ctx.WriteLine("This is a new line I think.");
		ctx.WriteLine();
		ctx.WriteLine("yoo");
	}

	CMD_H(TextTableMultiline) {
		auto Col = [](const std::size_t& mult) {
			std::vector<std::string> r{
				"Wow",
				str::Repeat("Wow ", 5 * mult),
				str::Repeat("wow", 15 * mult),
				str::Repeat("ok ", 1 * mult)
			};
			return r;
		};
		
		fmt::TextTable ttbl;
		ttbl.AddColumnLeft("Header 1", 15)
			.AddColumnLeft("Header 2", 30)
			.AddColumnLeft("Header 3", 50)
			.AddColumnRight("Header 4", 6);
		
		for (std::size_t i = 1; i < 8; i++) {
			ttbl << Col(i);
		}

		ctx.WriteLine(ttbl.PrintML());
		ctx.WriteLine(ttbl.PrintML(true));

	}

	// Some stuff for ObjTableTests:
	// Represents the status/category of an item — good candidate for a column
	enum class Status {
	    Active,
	    Inactive,
	    Pending,
	    Deprecated
	};

	std::string ToString(Status status) {
		switch (status) {
		case Status::Active: 		return "Active";
		case Status::Inactive: 		return "Inactive";
		case Status::Pending: 		return "Pending";
		case Status::Deprecated: 	return "Deprecated";
		default: return "Unknown Status";
		}
	}
	
	// Another column-worthy type: a simple tagged priority level
	enum class Priority {
	    Low,
	    Medium,
	    High,
	    Critical
	};

	std::string ToString(Priority priority) {
		switch (priority) {
		case Priority::Low:			return "Low";
	    case Priority::Medium:		return "Medium";
	    case Priority::High:		return "High";
	    case Priority::Critical:	return "Critical";
		default: return "Unknown Priority";
		}
	}
 
	class Item {
	public:
	    std::string name;
	    std::string description;
	    int         quantity;
	    double      market_value;   // large double, e.g. 1'234'567.89
	    double      change_pct;     // small double, e.g. 0.042 (4.2%)
	    Status      status;
	    Priority    priority;
	
	    Item(std::string name,
	         std::string description,
	         int         quantity,
	         double      market_value,
	         double      change_pct,
	         Status      status,
	         Priority    priority)
	        : name(std::move(name))
	        , description(std::move(description))
	        , quantity(quantity)
	        , market_value(market_value)
	        , change_pct(change_pct)
	        , status(status)
	        , priority(priority)
	    {}
	};
	
	inline std::vector<Item> make_sample_items() {
	    return {
	        { "Yttrium Capacitor",  "High-freq resonance cap",   412,  1'482'300.00,  0.034,  Status::Active,     Priority::High     },
	        { "Beryllium Rod",      "Structural support alloy",   87,    256'740.50, -0.012,  Status::Active,     Priority::Medium   },
	        { "Cobalt Mesh",        "EMI shielding fabric",     1'203,    98'510.75,  0.007,  Status::Inactive,   Priority::Low      },
	        { "Palladium Foil",     "Catalyst substrate",         34,  3'901'200.00,  0.151,  Status::Active,     Priority::Critical },
	        { "Osmium Disc",        "Dense inertial dampener",    19,  7'234'000.00, -0.003,  Status::Pending,    Priority::High     },
	        { "Hafnium Wire",       "Neutron absorber spool",    560,    487'230.25,  0.022,  Status::Active,     Priority::Medium   },
	        { "Rhenium Plate",      "High-temp turbine lining",   73,  1'105'800.00, -0.041,  Status::Deprecated, Priority::Low      },
	        { "Indium Slab",        "LCD electrode backing",    2'800,    312'450.00,  0.009,  Status::Active,     Priority::Medium   },
	        { "Tellurium Crystal",  "Thermoelectric element",    145,    678'900.50,  0.063,  Status::Pending,    Priority::High     },
	        { "Gallium Ingot",      "Semiconductor melt stock",  390,    204'175.00, -0.018,  Status::Inactive,   Priority::Low      },
	    };
	}

	CMD_H(ObjTableTests) {
		std::vector<Item> itemVector = make_sample_items();
		std::vector<Item*> itemPtrs;
		itemPtrs.reserve(itemVector.size());
		for (Item& itm : itemVector) itemPtrs.emplace_back(&itm);

		/* fmt::ObjTbl<Item> tbl({
			fmt::ObjTblCol<Item>(
				"Name:", 20, 
				[](const Item* itm){ return itm->name; },
				[](const Item* a, const Item* b){ return a->name < b->name; }
			),
			fmt::ObjTblCol<Item>(
				"Description:", 30,
				[](const Item* itm){ return itm->description; },
				[](const Item* a, const Item* b){ return a->description < b->description; }
			),
			fmt::ObjTblCol<Item>(
				"Quantity:", 10,
				[](const Item* itm){ return str::ToString(itm->quantity, 2, true); },
				[](const Item* a, const Item* b){ return a->quantity < b->quantity; },
				fmt::TextAlign::Left, fmt::TextAlign::Right
			),
			fmt::ObjTblCol<Item>(
				"Market Val.:", 15,
				[](const Item* itm){ return str::ToString(itm->market_value, 2, true); },
				[](const Item* a, const Item* b){ return a->market_value < b->market_value; },
				fmt::TextAlign::Left, fmt::TextAlign::Right
			),
			fmt::ObjTblCol<Item>(
				"Change %:", 10,
				[](const Item* itm){ return str::AsPct(itm->change_pct, 2); },
				[](const Item* a, const Item* b){ return a->change_pct < b->change_pct; },
				fmt::TextAlign::Left, fmt::TextAlign::Right
			),
			fmt::ObjTblCol<Item>(
				"Status:", 10,
				[](const Item* itm){ return ToString(itm->status); },
				[](const Item* a, const Item* b){ return static_cast<int>(a->status) < static_cast<int>(b->status); }
			),
			fmt::ObjTblCol<Item>(
				"Priority:", 10,
				[](const Item* itm){ return ToString(itm->priority); },
				[](const Item* a, const Item* b){ return static_cast<int>(a->priority) < static_cast<int>(b->priority); }
			)
		}, itemPtrs);

		tbl.StrCol(FMT_OTCOL_EXT("Name (3):", 20, name))
		.PctCol(FMT_OTCOL_EXT("% (2)", 8, change_pct), 1)
		.DblCol(FMT_OTCOL_EXT("Market V(2):", 15, market_value), 1, true); */

		//Item appended = Item("New entry", "Entry put in with << Operator", 100, 10000, 0.05, Status::Active, Priority::Low);
		//tbl << &appended;

		fmt::ObjTbl<Item> tbl({
			FMT_OTCOL_STR(Item, "Name:", 20, name),
			FMT_OTCOL_STR(Item, "Description:", 30, description),
			FMT_OTCOL_INT_C(Item, "Quantity:", 10, quantity, 2),
			FMT_OTCOL_DBL_C(Item, "Market Value:", 15, market_value, 2),
			FMT_OTCOL_PCT(Item, "Change (%):", 12, change_pct, 2),
			fmt::ObjTblCol<Item>("Status:", 10, [](const Item* itm) {return ToString(itm->status); }, FMT_OTCOL_ORDER_ENUM(status)),
			fmt::ObjTblCol<Item>("Priority:", 10, [](const Item* itm) {return ToString(itm->priority); }, FMT_OTCOL_ORDER_ENUM(priority))
		});

		tbl << itemPtrs;
		/* 
		// Alternatively you could do this:
		tbl.FMT_OTCOL_STR_M("Name:", 20, name)
			.FMT_OTCOL_STR_M("Description:", 30, description)
			.FMT_OTCOL_INT_CM("Quantity:", 10, quantity, 2)
			.FMT_OTCOL_DBL_CM("Market Value:", 15, market_value, 2)
			.FMT_OTCOL_PCT_M("Change:", 12, change_pct, 2)
			.AddColumn("Status:", 10, [](const Item* itm) {return ToString(itm->status); }, FMT_OTCOL_ORDER_ENUM(status))
			.AddColumn("Priority:", 10, [](const Item* itm) {return ToString(itm->priority); }, FMT_OTCOL_ORDER_ENUM(priority));
 		*/
		ctx.WriteLine(tbl.Print(fmt::TblRenderType::BoxCompact, 4, true));
		ctx.WriteLine(tbl.FirstN(-3).Print());		
		ctx.WriteLine(tbl.Print());
		tbl.FilterFirstN(5);
		ctx.WriteLine(tbl.Print());
	}	

	// some stuff for objtablmacrotests, feel free to delete:
	// claude wrote this:
	class ObjEntry {
	public:
		// --- String ---
		std::string name;
		std::optional<std::string> nameOpt;

		// --- Bools --- (this is my own addition)
		bool boolVar;
		std::optional<bool> boolVarOpt;

		// --- Integers ---
		int smallInt;
		int largeInt;
		std::optional<int> smallIntOpt;
		std::optional<int> largeIntOpt;

		// --- Doubles ---
		double smallDouble;
		double largeDouble;
		double percentage;   // e.g. 0.0-100.0
		std::optional<double> smallDoubleOpt;
		std::optional<double> largeDoubleOpt;
		std::optional<double> percentageOpt;

		// --- std::tm ---
		std::tm dateTime{};   // full date & time
		std::tm dateOnly{};   // date only (time fields unused)
		std::tm timeOnly{};   // time only (date fields unused)
		std::optional<std::tm> dateTimeOpt;
		std::optional<std::tm> dateOnlyOpt;
		std::optional<std::tm> timeOnlyOpt;

		ObjEntry() = default;
	};

	// Small helper to build a std::tm from explicit fields.
	static std::tm makeTm(int year, int month, int day, int hour = 0, int min = 0, int sec = 0) {
		std::tm t{};
		t.tm_year = year - 1900;
		t.tm_mon  = month - 1;
		t.tm_mday = day;
		t.tm_hour = hour;
		t.tm_min  = min;
		t.tm_sec  = sec;
		t.tm_isdst = -1;
		std::mktime(&t); // normalize (weekday, yday, etc.)
		return t;
	}

	std::vector<ObjEntry> buildObjEntries() {
		std::vector<ObjEntry> entries;
		entries.reserve(10);

		for (int i = 0; i < 10; ++i) {
			ObjEntry e;

			e.name        = "Entry " + std::to_string(i);
			e.boolVar	  = i % 3 == 1;
			e.smallInt    = i;
			e.largeInt    = i * 1'000'000;
			e.smallDouble = i * 0.5;
			e.largeDouble = i * 1.0e9;
			e.percentage  = i * 10.0; // 0%, 10%, 20%, ...

			e.dateTime = makeTm(2026, 1, i + 1, 9, 30, 0);
			e.dateOnly = makeTm(2026, 1, i + 1);
			e.timeOnly = makeTm(1900, 1, 1, i % 24, 0, 0); // date fields irrelevant here

			// Populate the optional fields on every other entry, to show both states.
			if (i % 2 == 0) {
				e.nameOpt        = e.name + " (opt)";
				e.boolVarOpt	 = e.boolVar;
				e.smallIntOpt    = e.smallInt + 1;
				e.largeIntOpt    = e.largeInt + 1;
				e.smallDoubleOpt = e.smallDouble + 0.1;
				e.largeDoubleOpt = e.largeDouble + 1.0;
				e.percentageOpt  = e.percentage / 2.0;
				e.dateTimeOpt    = e.dateTime;
				e.dateOnlyOpt    = e.dateOnly;
				e.timeOnlyOpt    = e.timeOnly;
			}

			entries.push_back(e);
		}

		return entries;
	}

	CMD_H(ObjTableMacroTests) {
		// Claude wrote this:
		std::vector<ObjEntry> rows = buildObjEntries();
		fmt::ObjTbl<ObjEntry> otbl({
			FMT_OTCOL_STR(ObjEntry, "Name:", 10, name),
			FMT_OTCOL_BOOL(ObjEntry, "BoolVar:", 10, boolVar, "[x]", "[ ]"),
			FMT_OTCOL_INT(ObjEntry, "SmallInt:", 10, smallInt),
			FMT_OTCOL_INT_C(ObjEntry, "LargeInt:", 10, largeInt, 2),
			FMT_OTCOL_DBL(ObjEntry, "SmallDbl:", 10, smallDouble, 2),
			FMT_OTCOL_DBL_C(ObjEntry, "LargeDbl:", 20, largeDouble, 2),
			FMT_OTCOL_PCT(ObjEntry, "Percent:", 10, percentage, 2),
			FMT_OTCOL_DTM(ObjEntry, "DateTime:", 20, dateTime),
			FMT_OTCOL_DATE(ObjEntry, "DateOnly:", 10, dateOnly),
			FMT_OTCOL_TIME(ObjEntry, "TimeOnly:", 10, timeOnly),
			FMT_OTCOL_OPT_STR(ObjEntry, "Name:", 10, nameOpt),
			FMT_OTCOL_OPT_BOOL(ObjEntry, "BoolVar:", 10, boolVarOpt, "[x]", "[ ]"),			
			FMT_OTCOL_OPT_INT(ObjEntry, "SmallInt:", 10, smallIntOpt),
			FMT_OTCOL_OPT_INT_C(ObjEntry, "LargeInt:", 10, largeIntOpt, 2),
			FMT_OTCOL_OPT_DBL(ObjEntry, "SmallDbl:", 10, smallDoubleOpt, 2),
			FMT_OTCOL_OPT_DBL_C(ObjEntry, "LargeDbl:", 20, largeDoubleOpt, 2),
			FMT_OTCOL_OPT_PCT(ObjEntry, "Percent:", 10, percentageOpt, 2),
			FMT_OTCOL_OPT_DTM(ObjEntry, "DateTime:", 20, dateTimeOpt),
			FMT_OTCOL_OPT_DATE(ObjEntry, "DateOnly:", 10, dateOnlyOpt),
			FMT_OTCOL_OPT_TIME(ObjEntry, "TimeOnly:", 10, timeOnlyOpt),
		});
		otbl << rows;
		ctx << otbl.Print() << '\n';
		ctx << otbl.Print(fmt::TblRenderType::BoxCompact, 1, false) << '\n';
		otbl.OrderBy(0);
		ctx << otbl.Print(fmt::TblRenderType::BoxCompact, 11) << '\n';
	}

	CMD_H(TestPrsTime) {

		std::optional<std::tm> testTime = args.Get<std::tm>(0);
		if (testTime) {
			ctx << "Parsed arg as: " << std::put_time(&testTime.value(), "%Y-%m-%d") << '\n';
			return;
		}

		ctx.WriteLine("Too Lazy to bother with writing inputs, so we'll just do manual:");

		// PrsTime	
		ctx.WriteLine("\n\n--parsers::PrsTime():--");
		std::vector<std::string> PrsTimeStrs {
			"2025-01-01",
			"2025-05-16 23:01:00",
			"1987/01/30 06:30:00"
		};
		for (const std::string& str : PrsTimeStrs) {
			std::tm t = parsers::PrsTime(str);
			ctx << "'" << str << "':\n"
				<< std::put_time(&t, "%Y-%m-%d %H:%M:%S")
				<< "\n\n";			
		}

		// PrsDate (str)
		ctx.WriteLine("\n\n--parsers::PrsDate(std::string):--");
		std::vector<std::string> PrsDateStrs {
			"2077-09-15",
			"1900-01-01",
			"2026/06/16"
		};
		for (const std::string& str : PrsDateStrs) {
			std::tm t = parsers::PrsDate(str);
			ctx << "'" << str << "':\n"
				<< std::put_time(&t, "%Y-%m-%d")
				<< "\n\n";
		}

		// PrsDate (int)
		ctx.WriteLine("\n\n--parsers::PrsDate(int):--");
		std::vector<int> PrsDateInts {
			20260616,
			//19010101,
			20270415,
			20000415,
			20000101
		};
		for (const int& nt : PrsDateInts) {
			std::tm t = parsers::PrsDate(nt);
			ctx << "'" << nt << "':\n"
				<< std::put_time(&t, "%Y-%m-%d")
				<< "\n\n";
		}

		// TryTime
		ctx.WriteLine("\n\n--parsers::TryTime():--");
		std::vector<std::string> TryTimeStrs {
			"2025-01-01",
			"2025-05-16 23:01:00",
			"1987/01/30 06:30:00",
			"1899-05-15 23:23:23",
			"2000-01-05 25:60:60",
			"2026-06-15 A billion o clock"
		};
		for (const std::string& str : TryTimeStrs) {
			ctx << "'" << str << "':";
			std::tm t{};
			if (parsers::TryTime(str, t))
				ctx << " [x]\n" << std::put_time(&t, "%Y-%m-%d %H:%M:%S");
			else ctx << " [ ]";
			ctx << "\n\n";			
		}

		// TryDate (str)
		ctx.WriteLine("\n\n--parsers::TryDate(std::string):--");
		std::vector<std::string> TryDateStrs {
			"2077-09-15",
			"1900-01-01",
			"2026/06/16",
			"1899-05-16",
			"2025-04-31",
			"Yesterday"
		};
		for (const std::string& str : TryDateStrs) {
			ctx << "'" << str << "':";
			std::tm t{};
			if (parsers::TryDate(str, t))
				ctx << " [x]\n" << std::put_time(&t, "%Y-%m-%d");
			else ctx << " [ ]";
			ctx << "\n\n";			
		}

		// TryDate (int)
		ctx.WriteLine("\n\n--parsers::TryDate(int):--");
		std::vector<int> TryDateInts {
			20260616,
			19010101,
			20270415,
			20000415,
			20000101,
			29,
			999999999,
			18990516,
			20250431,
			20251300
		};
		for (const int& nt : TryDateInts) {
			ctx << "'" << nt << "':";
			std::tm t{};
			if (parsers::TryIntDate(nt, t))
				ctx << " [x]\n" << std::put_time(&t, "%Y-%m-%d");
			else ctx << " [ ]";
			ctx << "\n\n";			
		}

		// ToDateInt
		ctx.WriteLine("\n\n--parsers::ToDateInt:--");
		std::vector<std::tm> ToDateTms {
			parsers::PrsDate(20000101),
			parsers::PrsDate(20231215),
			parsers::PrsDate(19990704),
			parsers::PrsDate(20240229),
			parsers::PrsDate(20100615),
			parsers::PrsDate(20221031)
		};
		for (const std::tm& date : ToDateTms) {
			ctx << std::put_time(&date, "%Y-%m-%d") << '\n'
				<< parsers::ToIntDate(date) << '\n';
		}
	}

	CMD_H(ReqInput) {

		// Test 1: RequireInput with int, normal flow
		ctx << "Test 1: RequireInput<int> (try entering a non-int first, then an int)\n";
		int i1 = ctx.RequireInput<int>(parsers::TryInt, "Enter an int: ", "Not a valid int, try again: ");
		ctx << "Got: " << i1 << "\n\n";

		// Test 2: RequireInput with cancel string
		ctx << "Test 2: RequireInput<int> (type \\ to cancel, should throw)\n";
		try {
			int i2 = ctx.RequireInput<int>(parsers::TryInt, "Enter an int or \\ to cancel: ", "Not valid: ");
			ctx << "Got: " << i2 << "\n\n";
		} catch (const ReplCancel& e) {
			ctx << "Caught ReplCancel as expected: " << e.what() << "\n\n";
		}

		// Test 3: RequestInput with fallback, cancel string used
		ctx << "Test 3: RequestInput<int> w/ fallback=42 (type \\ to cancel)\n";
		int i3 = ctx.RequestInput<int>(parsers::TryInt, 42, "Enter an int or \\ to cancel: ", "Not valid: ");
		ctx << "Got: " << i3 << " (should be 42 if cancelled)\n\n";

		// Test 4: RequestInput with fallback, normal value entered
		ctx << "Test 4: RequestInput<int> w/ fallback=42 (enter a real int)\n";
		int i4 = ctx.RequestInput<int>(parsers::TryInt, 42, "Enter an int: ", "Not valid: ");
		ctx << "Got: " << i4 << "\n\n";

		// Test 5: RequestInput returning optional, cancel string used
		ctx << "Test 5: RequestInput<int> optional overload (type \\ to cancel)\n";
		std::optional<int> i5 = ctx.RequestInput<int>(parsers::TryInt, "Enter an int or \\ to cancel: ", "Not valid: ");
		ctx << "Got: " << (i5.has_value() ? std::to_string(*i5) : "nullopt") << " (should be nullopt if cancelled)\n\n";

		// Test 6: RequestInput returning optional, value entered
		ctx << "Test 6: RequestInput<int> optional overload (enter a real int)\n";
		std::optional<int> i6 = ctx.RequestInput<int>(parsers::TryInt, "Enter an int: ", "Not valid: ");
		ctx << "Got: " << (i6.has_value() ? std::to_string(*i6) : "nullopt") << "\n\n";

		// Test 7: with std::tm via TryDate
		ctx << "Test 7: RequireInput<std::tm> via TryDate (enter e.g. 2024-01-01)\n";
		std::tm d1 = ctx.RequireInput<std::tm>(static_cast<bool(*)(const std::string&, std::tm&)>(parsers::TryDate), "Enter a date (YYYYMMDD): ", "Invalid date: ");
		ctx << "Got date with year: " << (d1.tm_year + 1900) << "\n\n";

		// Test 8: with std::string via TryString
		ctx << "Test 8: RequireInput<std::string> via TryString\n";
		std::string s1 = ctx.RequireInput<std::string>(parsers::TryString, "Enter any text: ", "Invalid: ");
		ctx << "Got: " << s1 << "\n\n";

		ctx << "All tests complete.\n";

	}

	TestCommands::TestCommands() {
		Define(

			Cmd("Test")
			.Aliases("tst")
			.Exec(Handler)
			.Desc("Nodal command for various testing functions.")
			.Group("Test")
			.Children(

				Cmd("Spinner")
				.Aliases("waitspinner")
				.Exec(TestSpinner)
				.Args(
					SztArg("Seconds", ArgMode::Optional, 5),
					StrArg("Wait Message", ArgMode::Optional, "Processing"),
					StrArg("Done Message", ArgMode::Optional, "Done.")
				)
				.Group("Test"),

				Cmd("TruncateList")
				.Exec(TruncateList)
				.Args(
					SztArg("Start", ArgMode::Optional, 1),
					SztArg("StarEnd", ArgMode::Optional, 1)
				)
				.Group("Test"),

				Cmd("BuildTable")
				.Exec(BuildTable)
				.Args(IntArg("N", ArgMode::Optional, 10))
				.Desc("Test fmt::BuildTable in different configurations.")
				.Group("Test"),

				Cmd("GetByName")
				.Exec(GetByName)
				.Args(
					IntArg("argA", ArgMode::Required),
					IntArg("argB", ArgMode::Optional),
					IntArg("argC", ArgMode::Optional)
				)
				.Group("Test"),

				Cmd("DoubleToString")
				.Aliases("dts")
				.Exec(DoubleToString)
				.Args(
					DblArg("Amount", ArgMode::Required),
					SztArg("Prec", ArgMode::Required),
					CmdArg<bool>(
						"Compact", 
						[](const std::string& text, bool& v){ 
							if (text == "true" || text == "false") {
								v = text == "true" ? true : false;
								return true;
							}
							return false;
						}
					),
					StrArg("Expected")
				)
				.Desc("We really didn't bake a bool arg huh?"),

				Cmd("VariadicWriteLine")
				.Aliases("vwl")
				.Exec(VariadicPrintLine),

				Cmd("TextTableMultiline")
				.Aliases("ttm")
				.Exec(TextTableMultiline),

				Cmd("ObjTbl")
				.Exec(ObjTableTests)
				.Children(

					Cmd("Macro")
					.Aliases("macros", "macrotest")
					.Exec(ObjTableMacroTests)

				),

				Cmd("PrsTime")
				.Exec(TestPrsTime)
				.Args(DatArg("DateTest", ArgMode::Optional)),

				Cmd("ReqInput")
				.Exec(ReqInput)

			)

		);
	}

}