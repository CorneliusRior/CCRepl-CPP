#pragma once
#include "CommandArgs.h"

namespace CCRepl {

	/*
	Scripts: We will not bother with JSON formatting for the time being. The way this will work is it will generate a series of commandargs.

	Make a parser which will generate them with a constructor which does not prompt upon absent arguments or formatting issues but just throws.

	ReplContext will have two new functions:
	 - ParseScript
	 - RunScript

	These will refer to static functions in ScriptParser.cpp (I think we'll call it).
	Let's just keep tokenizers in tokenizers beacause that works fine for now!
	*/
}