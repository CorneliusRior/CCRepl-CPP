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
		}, itemPtrs);
		/* 
		// Alternatively you could do this:
		tblFMT_OTCOL_STR_M("Name:", 20, name)
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

			)

		);
	}

}