#include "pch.h"
#include "fmt.h"

namespace fmt
{

	TextAlign ParseTextAlign(const std::string& text) {
		static const std::unordered_set<std::string> leftStrs = {
			"left", "l"
		};
		static const std::unordered_set<std::string> centreStrs = {
			"centre", "center", "c", "middle", "m"
		};
		static const std::unordered_set<std::string> rightStrs = {
			"right", "r"
		};
		std::string t = str::ToLower(text);
		if (leftStrs.contains(t)) return TextAlign::Left;
		if (centreStrs.contains(t)) return TextAlign::Center;
		if (rightStrs.contains(t)) return TextAlign::Right;
		throw std::runtime_error(std::format("Cannot parse TextAlign '{}'.", text));
	}

	bool TryParseTextAlign(const std::string& text, TextAlign& v) {
		try {
			v = ParseTextAlign(text);
			return true;
		}
		catch (...) {
			return false;
		}
	}

	std::string ToString(const TextAlign& value) {
		std::ostringstream oss;
		oss << value;
		return oss.str();
	}

	std::ostream& operator<<(std::ostream& os, TextAlign value) {
		switch (value) {
		case TextAlign::Left:
			return os << "Left";
		case TextAlign::Center:
			return os << "Center";
		case TextAlign::Right:
			return os << "Right";
		default:
			return os << "Unknown alignment";
		}
	}

	std::string AlignText(const std::string& text, const TextAlign& alignment, std::size_t w, const std::string& truncateString) {
		switch (alignment) {
		case TextAlign::Left: return str::TruncatePadRight(text, w, truncateString);
		case TextAlign::Right: return str::TruncatePadLeft(text, w, truncateString);
		case TextAlign::Center: return str::TruncatePadCenter(text, w, truncateString);
		default: return str::TruncatePadRight(text, w, truncateString);
		}
		
	}

	TextLine::TextLine() : Text("") {}
	TextLine::TextLine(const std::string& text, bool indent, TextAlign alignment, std::size_t padLeft, std::size_t padRight)
		: Text(text), Indent(indent), Alignment(alignment), PadRight(padRight), PadLeft(padLeft) { }

	TextLine::TextLine(const std::string& text, TextAlign alignment) : Text(text), Alignment(alignment) {}

	std::string TextLine::PrintStr (std::size_t w) const {
		return AlignText(Text, Alignment, w);
	}

	std::vector<std::string> TextLine::Print(std::size_t w) const {
		std::vector<std::string> r;
		
		// If it is empty, consider it a "new line", and just return a vector with an empty string:
		if (Text.empty()) {
			r.push_back(str::Repeat(" ", w));
			return r;
		}

		// Make sure there is enough space:
		if (w <= PadLeft + PadRight) return r;		// Alternatively, we could just ignore padding, or throw.
		std::size_t sp = w - PadLeft - PadRight;
		std::string lPad = str::Repeat(" ", PadLeft);
		std::string rPad = str::Repeat(" ", PadRight);

		// Wrap, and Indent if required:
		std::vector<std::string> wrapped = Indent ? str::Wrap("00" + Text, sp) : str::Wrap(Text, sp);
		if (Indent) {
			wrapped[0][0] = ' ';
			wrapped[0][1] = ' ';
		}

		// Align & add left padding:
		for (std::string l : wrapped) r.push_back(lPad + AlignText(l, Alignment, sp) + rPad);

		return r;
	}

	std::ostream& operator<<(std::ostream& os, const TextLine& value) {
		return os << value.Text;
	}

	std::string PrintTxtLineStr(const TextLine& value, std::size_t w) { return value.PrintStr(w); }
	std::vector<std::string> PrintTxtLine(const TextLine& value, std::size_t w) { return value.Print(w); }

	// TextBlock
	std::vector<std::string> TextBlock::Print(std::size_t w) const {
		std::vector<std::string> r;
		for (TextLine l : Lines) {
			std::vector<std::string> v = l.Print(w);
			r.insert(r.end(), v.begin(), v.end());
		}
		return r;
	}

	std::string TextBlock::PrintStr(std::size_t w) const {
		return str::PrintList(Print(w));
	}

	std::ostream& operator<<(std::ostream& os, const TextBlock& value) {		
		return os << "[Textblock, printed with width = 100]:\n" << value.PrintStr(100);
	}

	std::string PrintTxtBlkStr(const TextBlock& value, std::size_t w) { return value.PrintStr(w); }
	std::vector<std::string> PrintTxtBlk(const TextBlock& value, std::size_t w) { return value.Print(w); }

	// Box (TextBox):
	TextBox::TextBox(TextBlock body, std::string title, TextAlign titleAlignment, std::size_t vPadding, std::size_t hPadding) : Body(body), Title(title), TitleAlignment(titleAlignment), VPadding(vPadding), HPadding(hPadding) {}

	TextBox::TextBox(std::string body, std::string title, TextAlign bodyAlignment, TextAlign titleAlignment, std::size_t vPadding, std::size_t hPadding) : Title(title), TitleAlignment(titleAlignment), VPadding(vPadding), HPadding(hPadding) {
		TextBlock bd;
		bd.Lines = std::vector<TextLine>{ TextLine(body, bodyAlignment) };
		Body = bd;
	}

	TextBox::TextBox(std::vector<std::string> body, std::string title, TextAlign titleAlignment, std::size_t vPadding, std::size_t hPadding) : Title(title), TitleAlignment(titleAlignment), VPadding(vPadding), HPadding(hPadding) {
		TextBlock bd;
		for (std::string l : body) bd.Lines.push_back(TextLine(l));
		Body = bd;
	}

	std::string TextBox::PrintStr(size_t w) const {
		std::size_t fullWidth = w + (HPadding * 2);
		std::string titleString = Title.empty() ? "──" : std::format("─[{}]─", str::Truncate(Title, fullWidth - 4));
		std::vector<std::string> lines = Body.Print(w);
		std::string hPad = str::Repeat(" ", HPadding);
		std::string vPad = str::Repeat(" ", fullWidth);
		std::ostringstream oss;

		// Draw top:
		oss << "┌";
		size_t topSize = fullWidth - str::StrLength(titleString);
		if (TitleAlignment == TextAlign::Center) {
			size_t seg = topSize / 2;
			oss << str::Repeat("─", seg) << titleString << str::Repeat("─", topSize % 2 == 1 ? seg + 1 : seg);
		}
		else {
			std::string seg = str::Repeat("─", topSize);
			if (TitleAlignment == TextAlign::Left) oss << titleString << seg;
			else oss << seg << titleString;
		}
		oss << "┐" << std::endl;

		// Draw VPadding, each line then VPadding again:
		for (int i = 0; i < VPadding; i++) oss << "│" + vPad + "│" << std::endl;
		for (std::string l : lines) oss << "│" + hPad + l + hPad + "│" << std::endl;
		for (int i = 0; i < VPadding; i++) oss << "│" + vPad + "│" << std::endl;
		oss << "└" << str::Repeat("─", fullWidth) << "┘";

		return oss.str();
	}

	std::vector<std::string> TextBox::Print(size_t w) const {
		std::vector<std::string> r;
		std::size_t fullWidth = w + (HPadding * 2);
		std::string titleString = Title.empty() ? "──" : std::format("─[{}]─", str::Truncate(Title, fullWidth - 4));
		std::vector<std::string> lines = Body.Print(w);
		std::string hPad = str::Repeat(" ", HPadding);
		std::string vPad = str::Repeat(" ", fullWidth);
		std::ostringstream oss;
		auto NewLine = [&oss, &r]() {
			r.push_back(oss.str());
			oss.str("");
			oss.clear();
			};

		// Draw top:
		oss << "┌";
		size_t topSize = fullWidth - str::StrLength(titleString);
		if (TitleAlignment == TextAlign::Center) {
			size_t seg = topSize / 2;
			oss << str::Repeat("─", seg) << titleString << str::Repeat("─", topSize % 2 == 1 ? seg + 1 : seg);
		}
		else {
			std::string seg = str::Repeat("─", topSize);
			if (TitleAlignment == TextAlign::Left) oss << titleString << seg;
			else oss << seg << titleString;
		}
		oss << "┐";
		NewLine();

		// Draw VPadding, each line then VPadding again:
		for (int i = 0; i < VPadding; i++) {
			oss << "│" + vPad + "│";
			NewLine();
		}

		for (std::string l : lines) {
			oss << "│" + hPad + l + hPad + "│";
			NewLine();
		}

		for (int i = 0; i < VPadding; i++) {
			oss << "│" + vPad + "│";
			NewLine();
		}
		oss << "└" << str::Repeat("─", fullWidth) << "┘";
		NewLine();
		return r;
	}

	std::string PrintTxtBxStr(const TextBox& value, std::size_t w) {
		return value.PrintStr(w);
	}

	std::vector<std::string> PrintTxtBx(const TextBox& value, std::size_t w) {
		return value.Print(w);
	}

	// Functions for quickly making boxes:
	std::string TxtBoxLeft(std::string body, std::string title, std::size_t w, TextAlign titleAlignment) {
		return TextBox(body, title, TextAlign::Left, titleAlignment).PrintStr(w);
	}
	std::string TxtBoxCenter(std::string body, std::string title, std::size_t w, TextAlign titleAlignment) {
		return TextBox(body, title, TextAlign::Center, titleAlignment).PrintStr(w);
	}
	std::string TxtBoxRight(std::string body, std::string title, std::size_t w, TextAlign titleAlignment) {
		return TextBox(body, title, TextAlign::Right, titleAlignment).PrintStr(w);
	}



	// TextTable stuff:

	TextTableColumn::TextTableColumn(std::string header, size_t width, TextAlign headerAln, TextAlign dataAln) :
		Header(std::move(header)), Width(std::move(width)), HeaderAlignment(std::move(headerAln)), DataAlignment(std::move(dataAln)) { HLine = str::Repeat("─", Width); }

	TextTable::TextTable(const std::vector<TextTableColumn>& columns) : Columns(columns) {}
	TextTable::TextTable(const std::vector<TextTableColumn>& columns, std::vector<std::vector<std::string>> items) : Columns(columns), Items(std::move(items)) {}

	void TextTable::AddItem(const std::vector<std::string>& item) {
		Validate(item);
		Items.push_back(item);
	}

	void TextTable::AddItems(std::vector<std::vector<std::string>> items) {
		for (std::vector<std::string> item : items) AddItem(item);
	}

	TextTable& TextTable::operator<<(const std::vector<std::string>& item) {
		AddItem(item);
	}

	void TextTable::Validate() const {
		for (std::vector<std::string> item : Items) Validate(item);
	}

	void TextTable::Validate(const std::vector<std::string>& item) const {
		if (item.size() != Columns.size()) throw std::runtime_error(std::format("Table and item line have different column amounts, must be identical. Columns.size() = '{}', item.size() = '{}'\n{}", Columns.size(), item.size(), str::PresentList(item, "Item: ", " | ")));
	}

	std::string TextTable::Print() const {
		// Ensure data lines up:
		Validate();

		std::ostringstream oss;

		// Draw Banner, top:
		oss << "┌";
		for (int i = 0; i < Columns.size(); i++) {
			oss << Columns[i].HLine;
			if (i != Columns.size() - 1) oss << "┬";
		}
		oss << "┐" << std::endl;

		// Draw Banner, headers:
		for (TextTableColumn c : Columns) {
			oss << "│" << AlignText(c.Header, c.HeaderAlignment, c.Width);
		}
		oss << "│" << std::endl;

		// Draw each row (item):
		for (std::vector<std::string> item : Items) {
			// Draw top:
			oss << "├";
			for (int i = 0; i < Columns.size(); i++) {
				oss << Columns[i].HLine;
				if (i != Columns.size() - 1) oss << "┼";
			}
			oss << "┤" << std::endl;

			// Draw data:
			for (int i = 0; i < Columns.size(); i++) {
				oss << "│" << AlignText(item[i], Columns[i].DataAlignment, Columns[i].Width);
			}
			oss << "│" << std::endl;
		}

		// Draw bottom:
		oss << "└";
		for (int i = 0; i < Columns.size(); i++) {
			oss << Columns[i].HLine;
			if (i != Columns.size() - 1) oss << "┴";
		}
		oss << "┘";

		return oss.str();
	}

	std::string TextTable::PrintCompact(bool ascii) const {
		Validate();
		std::ostringstream oss;
		if (ascii) {
			// Draw Banner:
			for (TextTableColumn c : Columns) {
				oss << '|'<< AlignText(c.Header, c.HeaderAlignment, c.Width);
			}
			oss << "|\n";

			// Draw Banner, bottom:
			oss << '|';
			for (std::size_t i = 0; i < Columns.size(); i++) {
				oss << str::Repeat("-", Columns[i].Width) << '|';
			}

			// Draw each row:
			for (std::vector<std::string> item : Items) {
				oss << "\n|";
				for (std::size_t i = 0; i < Columns.size(); i++) {
					oss << AlignText(item[i], Columns[i].DataAlignment, Columns[i].Width)
						<< '|';
				}
			}
		}
		else {
			// Draw Banner, top:
			oss << "┌";
			for (int i = 0; i < Columns.size(); i++) {
				oss << Columns[i].HLine;
				if (i != Columns.size() - 1) oss << "┬";
			}
			oss << "┐" << std::endl;

			// Draw Banner, headers:
			for (TextTableColumn c : Columns) {
				oss << "│" << AlignText(c.Header, c.HeaderAlignment, c.Width);
			}
			oss << "│" << std::endl;

			// Draw Banner, bottom:
			oss << "├";
			for (int i = 0; i < Columns.size(); i++) {
				oss << Columns[i].HLine;
				if (i != Columns.size() - 1) oss << "┼";
			}
			oss << "┤" << std::endl;

			// Draw each row:
			for (std::vector<std::string> item : Items) {
				for (int i = 0; i < Columns.size(); i++) {
					oss << "│" << AlignText(item[i], Columns[i].DataAlignment, Columns[i].Width);
				}
				oss << "│" << std::endl;
			}

			// Draw bottom:
			oss << "└";
			for (int i = 0; i < Columns.size(); i++) {
				oss << Columns[i].HLine;
				if (i != Columns.size() - 1) oss << "┴";
			}
			oss << "┘";
		}
		return oss.str();
	}

	std::vector<std::string> TextTable::PrintVec() const{
		// Ensure data lines up:
		Validate();
		std::vector<std::string> r;
		std::ostringstream oss;
		auto NewLine = [&oss, &r]() {
			r.push_back(oss.str());
			oss.str("");
			oss.clear();
			};

		// Draw Banner, top:
		oss << "┌";
		for (int i = 0; i < Columns.size(); i++) {
			oss << Columns[i].HLine;
			if (i != Columns.size() - 1) oss << "┬";
		}
		oss << "┐";
		NewLine();

		// Draw Banner, headers:
		for (TextTableColumn c : Columns) {
			oss << "│" << AlignText(c.Header, c.HeaderAlignment, c.Width);
		}
		oss << "│";
		NewLine();

		// Draw each row (item):
		for (std::vector<std::string> item : Items) {
			// Draw top:
			oss << "├";
			for (int i = 0; i < Columns.size(); i++) {
				oss << Columns[i].HLine;
				if (i != Columns.size() - 1) oss << "┼";
			}
			oss << "┤";
			NewLine();

			// Draw data:
			for (int i = 0; i < Columns.size(); i++) {
				oss << "│" << AlignText(item[i], Columns[i].DataAlignment, Columns[i].Width);
			}
			oss << "│";
			NewLine();
		}

		// Draw bottom:
		oss << "└";
		for (int i = 0; i < Columns.size(); i++) {
			oss << Columns[i].HLine;
			if (i != Columns.size() - 1) oss << "┴";
		}
		oss << "┘";
		NewLine();

		return r;
	}

	std::ostream& operator<<(std::ostream& os, const TextTable& table) {
		return os << table.Print();
	}
}
