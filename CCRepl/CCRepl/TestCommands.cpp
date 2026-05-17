#include "pch.h"

#include "CommandBuilder.h"
#include "CommandSet.h"
#include "ReplContext.h"

namespace CCRepl {

	static void Handler(ReplContext& ctx, CommandArgs& args) {
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

	static void TestSpinner(ReplContext& ctx, CommandArgs& args) {
		std::future<int> ft = std::async(std::launch::async, WaitSeconds, args.GetRequired<std::size_t>(0));
		int r = ctx.WaitSpinner<int>(std::move(ft));
		ctx.WriteLine(r);
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
				.Group("Test")

			)

		);
	}

}