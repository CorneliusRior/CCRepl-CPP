#pragma once
#include "str.h"
#include <format>
#include <ostream>
#include <iomanip>
#include <string>
#include <unordered_set>

namespace fmt
{
	enum class TextAlign {
		Left,
		Center,
		Right
	};


	TextAlign ParseTextAlign(const std::string& text);
	bool TryParseTextAlign(const std::string& text, TextAlign& v);
	std::string ToString(const TextAlign& value);
	std::string AlignText(const std::string& text, const TextAlign& alignment, std::size_t w, const std::string& truncateString = "…");
	std::ostream& operator<<(std::ostream& os, TextAlign value);

	struct TextLine {
		std::string Text;
		bool Indent = false;
		TextAlign Alignment = TextAlign::Left;
		std::size_t PadLeft = 0;
		std::size_t PadRight = 0;

		TextLine();
		TextLine(const std::string& text, bool indent, TextAlign alignment = TextAlign::Left, std::size_t padLeft = 0, std::size_t padRight = 0);
		TextLine(const std::string& text, TextAlign alignment = TextAlign::Left);

		std::string PrintStr(std::size_t w) const;
		std::vector<std::string> Print(std::size_t w) const;
	};
	std::ostream& operator<<(std::ostream& os, const TextLine& value);

	// External to struct, mainly for testing:
	std::string PrintTxtLineStr(const TextLine& value, std::size_t w);
	std::vector<std::string> PrintTxtLine(const TextLine& value, std::size_t w);

	struct TextBlock {
		std::vector<TextLine> Lines;							// Maybe should be called `Paragraphs`?
		std::string PrintStr(std::size_t w) const;
		std::vector<std::string> Print(std::size_t w) const;
	};
	std::ostream& operator<<(std::ostream& os, const TextBlock& value);

	// External to struct, mainly for testing:
	std::string PrintTxtBlkStr(const TextBlock& value, std::size_t w);
	std::vector<std::string> PrintTxtBlk(const TextBlock& value, std::size_t w);

	struct TextBox {
		TextBlock Body;
		std::string Title = "";
		TextAlign TitleAlignment = TextAlign::Left;
		std::size_t VPadding = 1;
		std::size_t HPadding = 3;

		TextBox();
		TextBox(TextBlock body, std::string title = "", TextAlign titleAlignment = TextAlign::Left, std::size_t vPadding = 1, std::size_t hPadding = 3);
		TextBox(std::string body, std::string title = "", TextAlign bodyAlignment = TextAlign::Left, TextAlign titleAlignment = TextAlign::Left, std::size_t = 1, std::size_t = 3);
		TextBox(std::vector<std::string> body, std::string title = "", TextAlign titleAlignment = TextAlign::Left, std::size_t = 1, std::size_t = 3);

		std::string PrintStr(size_t w = 100) const;
		std::vector<std::string> Print(size_t w = 100) const;
	};

	std::string PrintTxtBxStr(const TextBox& value, std::size_t w = 100);
	std::vector<std::string> PrintTxtBx(const TextBox& value, std::size_t w = 100);

	// Functions for quickly making boxes:
	std::string TxtBoxLeft(std::string body, std::string title, std::size_t w = 100, TextAlign titleAlignment = TextAlign::Left);
	std::string TxtBoxCenter(std::string body, std::string title, std::size_t w = 100, TextAlign titleAlignment = TextAlign::Left);
	std::string TxtBoxRight(std::string body, std::string title, std::size_t w = 100, TextAlign titleAlignment = TextAlign::Left);
	
	struct TextTableColumn {
		std::string Header;
		size_t Width = 10;
		TextAlign HeaderAlignment = TextAlign::Left;
		TextAlign DataAlignment = TextAlign::Left;
		std::string HLine;

		TextTableColumn(std::string header, size_t width, TextAlign headerAln = TextAlign::Left, TextAlign dataAln = TextAlign::Left);
	};

	struct TextTable {
		const std::vector<TextTableColumn>& Columns;
		std::vector<std::vector<std::string>> Items;

		// Constructors:
		TextTable(const std::vector<TextTableColumn>& columns);
		TextTable(const std::vector<TextTableColumn>& columns, std::vector<std::vector<std::string>> items);

		// Add items:
		void AddItem(const std::vector<std::string>& item);
		void AddItems(std::vector<std::vector<std::string>> items);
		TextTable& operator<<(const std::vector<std::string>& item);

		// Validation:	
		void Validate() const;
		void Validate(const std::vector<std::string>& item) const;

		// Output:
		std::string Print() const;
		std::string PrintCompact(bool ascii = false) const;
		std::vector<std::string> PrintVec() const;
	};
	std::ostream& operator<<(std::ostream& os, const TextTable& table);
}
