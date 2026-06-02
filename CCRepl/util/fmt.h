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
	std::string TxtBoxLeft(std::string body, std::string title = "", std::size_t w = 100, TextAlign titleAlignment = TextAlign::Left);
	std::string TxtBoxCenter(std::string body, std::string title = "", std::size_t w = 100, TextAlign titleAlignment = TextAlign::Left);
	std::string TxtBoxRight(std::string body, std::string title = "", std::size_t w = 100, TextAlign titleAlignment = TextAlign::Left);
	
	struct TextTableColumn {
		std::string Header;
		size_t Width = 10;
		TextAlign HeaderAlignment = TextAlign::Left;
		TextAlign DataAlignment = TextAlign::Left;
		std::string HLine;
		std::string HLineAscii;

		TextTableColumn(std::string header, size_t width, TextAlign headerAln = TextAlign::Left, TextAlign dataAln = TextAlign::Left);
	};

	struct TextTable {
		std::vector<TextTableColumn> Columns;
		std::vector<std::vector<std::string>> Items;

		// Constructors:
		TextTable() = default;
		TextTable(const std::vector<TextTableColumn>& columns);
		TextTable(const std::vector<TextTableColumn>& columns, std::vector<std::vector<std::string>> items);

		// Add Columns:
		TextTable& AddColumnLeft(std::string header, std::size_t width = 10, TextAlign headerAlignment = TextAlign::Left);
		TextTable& AddColumnRight(std::string header, std::size_t width = 10, TextAlign headerAlignment = TextAlign::Left);
		TextTable& AddColumnCenter(std::string header, std::size_t width = 10, TextAlign headerAlignment = TextAlign::Left);

		// Add items:
		void AddItem(const std::vector<std::string>& item);
		void AddItems(std::vector<std::vector<std::string>> items);
		TextTable& operator<<(const std::vector<std::string>& item);

		// Validation:	
		void Validate() const;
		void Validate(const std::vector<std::string>& item) const;

		// Output:
		std::string Print(bool ascii = false) const;
		std::string PrintCompact(bool ascii = false) const;
		std::vector<std::string> PrintVec() const;
	};
	std::ostream& operator<<(std::ostream& os, const TextTable& table);

	/// <summary>
	/// Generates a table of a type T which needs to have functions std::vector<fmt::TextTableColumn> GetTableColumns(), and std::vector<std::string> GetTableRow(). 
	/// If n is positive, print the first n items. If n is negative, print the last n items. If n is 0, print all items.
	/// </summary>
	/// <typeparam name="T">Datatype with functions GetTableColumns() and GetTableRow().</typeparam>
	/// <param name="items">Vector of type T.</param>
	/// <param name="n">Number of items to display. 0 to display all, positive to display first n items, negative to displat last n items.</param>
	/// <param name="compact">Bool to render table compact or expanded (default: true).</param>
	/// <param name="ascii">Bool to render using ascii symbols and no UTF-8 box characters (default: true).</param>
	/// <returns></returns>
	template <typename T>
	std::string BuildTable(const std::vector<T>& items, int n = 0, bool compact = true, bool ascii = false) {
		TextTable t(T::GetTableColumns());
		if (n == 0) for (const T& row : items) t << row.GetTableRow();
		else if (n > 0) for (std::size_t i = 0; i < items.size() && i < n; i++) t << items[i].GetTableRow();
		else {
			std::size_t start = items.size() - std::min<std::size_t>(items.size(), static_cast<std::size_t>(std::abs(n)));
			for (std::size_t i = start; i < items.size(); i++) t << items[i].GetTableRow();
		}
		if (compact) return t.PrintCompact(ascii);
		else return t.Print(ascii);
	}

	/// <summary>
	/// Generates a table of a type T, using column items and a function to convert a row into a string vector (getRowFunc).
	/// If n is positive, print the first n items. If n is negative, print the last n items. If n is 0, print all items.
	/// </summary>
	/// <typeparam name="T">Datatype to display.</typeparam>
	/// <param name="items">Vector of type T.</param>
	/// <param name="columns">TextTableColumn vector.</param>
	/// <param name="getRowFunc">Function to generate string vector (TextTable row).</param>
	/// <param name="n">umber of items to display. 0 to display all, positive to display first n items, negative to displat last n items.</param>
	/// <param name="compact">Bool to render table compact or expanded (default: true).</param>
	/// <param name="ascii">Bool to render using ascii symbols and no UTF-8 box characters (default: true).</param>
	/// <returns></returns>
	template <typename T>
	std::string BuildTable(const std::vector<T>& items, std::vector<TextTableColumn> columns, std::function<std::vector<std::string>(const T&)> getRowFunc, int n = 0, bool compact = true, bool ascii = false) {
		TextTable t(columns);
		if (n == 0) for (const T& row : items) t << getRowFunc(row);
		else if (n > 0) for (std::size_t i = 0; i < items.size() && i < n; i++) t << getRowFunc(items[i]);
		else {
			std::size_t start = items.size() - std::min<std::size_t>(items.size(), static_cast<std::size_t>(std::abs(n)));
			for (std::size_t i = start; i < items.size(); i++) t << getRowFunc(items[i]);
		}
		if (compact) return t.PrintCompact(ascii);
		else return t.Print(ascii);
	}
}
