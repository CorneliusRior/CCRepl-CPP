#include "pch.h"

#include "CommandBuilder.h"
#include "CommandSet.h"
#include "ReplContext.h"

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
		ctx.WriteLineV("This is a thing", "And this is also a thing", "so many things!", 29, 24, "hello", 87.5 / 12);
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
				.Exec(VariadicPrintLine)

			)

		);
	}

}