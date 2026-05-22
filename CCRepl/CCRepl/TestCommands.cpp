#include "pch.h"

#include "CommandBuilder.h"
#include "CommandSet.h"
#include "ReplContext.h"

namespace CCRepl {

	CMD_H(Handler) {
		ctx.WriteLine("Nodal command (No handler, or execute function not yet implemented).\n");
		ctx.WriteLine(args.Cmd->PrintFull());

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
				.Group("Test")

			)

		);
	}

}