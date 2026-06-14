#include <util/str.h>
#include <util/fmt.h>
#include <ranges>

#define FMT_OTCOL_ORDER_DEF(varname) [](const auto* a, const auto* b) { return a->varname < b->varname; }
#define FMT_OTCOL_ORDER_ENUM(varname) [](const auto* a, const auto* b) { return static_cast<int>(a->varname) < static_cast<int>(b->varname); }

// String named varname in ObjType, e.g.: FMT_OTCOL_STR(Item, "Name:", 20, name).
#define FMT_OTCOL_STR(ObjType, header, width, varname) fmt::ObjTblCol<ObjType>(header, width, \
    [](const ObjType* itm) { return itm->varname; }, \
    FMT_OTCOL_ORDER_DEF(varname), fmt::TextAlign::Left, fmt::TextAlign::Left)

// Int named varname in ObjType. For compact, use FMT_OTCOL_INT_C.
#define FMT_OTCOL_INT(ObjType, header, width, varname) fmt::ObjTblCol<ObjType>(header, width, \
    [](const ObjType* itm) { return std::to_string(itm->varname); }, \
    FMT_OTCOL_ORDER_DEF(varname), fmt::TextAlign::Left, fmt::TextAlign::Right)

// Int named varname in ObjType, truncates like 1,234,567 -> 1.2M.
#define FMT_OTCOL_INT_C(ObjType, header, width, varname, prec) fmt::ObjTblCol<ObjType>(header, width, \
    [](const ObjType* itm) { return str::ToString(itm->varname, prec, true); }, \
    FMT_OTCOL_ORDER_DEF(varname), fmt::TextAlign::Left, fmt::TextAlign::Right)

// Double named varname in ObjType, print w/ prec decimal places. For compact, use FMT_OTCOL_DBL_C.
#define FMT_OTCOL_DBL(ObjType, header, width, varname, prec) fmt::ObjTblCol<ObjType>(header, width, \
    [](const ObjType* itm) { return str::ToString(itm->varname, prec, false); }, \
    FMT_OTCOL_ORDER_DEF(varname), fmt::TextAlign::Left, fmt::TextAlign::Right)

// Double named varname in ObjType, truncates like 1,234,567 -> 1.2M.
#define FMT_OTCOL_DBL_C(ObjType, header, width, varname, prec) fmt::ObjTblCol<ObjType>(header, width, \
    [](const ObjType* itm) { return str::ToString(itm->varname, prec, true); }, \
    FMT_OTCOL_ORDER_DEF(varname), fmt::TextAlign::Left, fmt::TextAlign::Right)

// Double named varname in ObjType, printed as a percentage, with prec decimal places.
#define FMT_OTCOL_PCT(ObjType, header, width, varname, prec) fmt::ObjTblCol<ObjType>(header, width, \
    [](const ObjType* itm) { return str::AsPct(itm->varname, prec); }, \
    FMT_OTCOL_ORDER_DEF(varname), fmt::TextAlign::Left, fmt::TextAlign::Right)

// String named varname in ObjType, calls member function (ObjTbl::AddColumn)
#define FMT_OTCOL_STR_M(header, width, varname) AddColumn(header, width, \
    [](const auto* itm) { return itm->varname; }, \
    FMT_OTCOL_ORDER_DEF(varname), fmt::TextAlign::Left, fmt::TextAlign::Left)

// Int named varname in ObjType, calls member function (ObjTbl::AddColumn). For compact, use FMT_OTCOL_INT_CM.
#define FMT_OTCOL_INT_M(header, width, varname) AddColumn(header, width, \
    [](const auto* itm) { return std::to_string(itm->varname); }, \
    FMT_OTCOL_ORDER_DEF(varname), fmt::TextAlign::Left, fmt::TextAlign::Right)

// Int named varname in ObjType, truncates like 1,234,567 -> 1.2M. Calls member function (ObjTbl::AddColumn).
#define FMT_OTCOL_INT_CM(header, width, varname, prec) AddColumn(header, width, \
    [](const auto* itm) { return str::ToString(itm->varname, prec, true); }, \
    FMT_OTCOL_ORDER_DEF(varname), fmt::TextAlign::Left, fmt::TextAlign::Right)

// Double named varname in ObjType, print w/ prec decimal places. Calls member function (ObjTbl::AddColumn). For compact, use FMT_OTCOL_DBL_CM.
#define FMT_OTCOL_DBL_M(header, width, varname, prec) AddColumn(header, width, \
    [](const auto* itm) { return str::ToString(itm->varname, prec, false); }, \
    FMT_OTCOL_ORDER_DEF(varname), fmt::TextAlign::Left, fmt::TextAlign::Right)

// Double named varname in ObjType, truncates like 1,234,567 -> 1.2M. Calls member function (ObjTbl::AddColumn).
#define FMT_OTCOL_DBL_CM(header, width, varname, prec) AddColumn(header, width, \
    [](const auto* itm) { return str::ToString(itm->varname, prec, true); }, \
    FMT_OTCOL_ORDER_DEF(varname), fmt::TextAlign::Left, fmt::TextAlign::Right)

// Double named varname in ObjType, printed as a percentage, with prec decimal places. Calls member function (ObjTbl::AddColumn).
#define FMT_OTCOL_PCT_M(header, width, varname, prec) AddColumn(header, width, \
    [](const auto* itm) { return str::AsPct(itm->varname, prec); }, \
    FMT_OTCOL_ORDER_DEF(varname), fmt::TextAlign::Left, fmt::TextAlign::Right)

namespace fmt {    

    // Column for a ObjTbl table:
    template <typename Obj> 
    class ObjTblCol {
    public:
        std::string Header;
        std::size_t Width;
        std::function<std::string(const Obj*)> Render;
        std::function<bool(const Obj*, const Obj*)> Order; 
        TextAlign HeaderAlignment;
        TextAlign DataAlignment;

        /*Column for a Object table.
        
        ObjTblCol<MyClass>(
            "Header 1:", 12, 
            [](const MyClass* itm) { return var1.ToString(); },
            [](const MyClass* a, const MyClass* b) { return a->value < b-> value; },
            TextAlign::Left, TextAlign::Right
        );
        */
        ObjTblCol(
            std::string header, 
            std::size_t width, 
            std::function<std::string(const Obj*)> renderFunc, 
            std::function<bool (const Obj*, const Obj*)> orderFunc, 
            TextAlign headerAlignment = TextAlign::Left, 
            TextAlign dataAlignment = TextAlign::Left) 
            : Header(header), Width(width), Render(renderFunc), Order(orderFunc), HeaderAlignment(headerAlignment), DataAlignment(dataAlignment) {}    
    };


    enum class TblRenderType {
        Box,
        BoxCompact,
        Ascii,
        AsciiCompact,
        Borderless
    };

    template <typename Obj>
    class ObjTbl {
    private:
        std::vector<ObjTblCol<Obj>> columns_;
        std::vector<Obj*> objects_;
    
    public:       
        ObjTbl();
        ObjTbl(std::vector<Obj*> rows) : objects_(rows) {}
        ObjTbl(std::vector<ObjTblCol<Obj>> columns) : columns_(columns) {}
        ObjTbl(std::vector<ObjTblCol<Obj>> columns, std::vector<Obj*> rows) : columns_(columns), objects_(rows) {}

        ObjTbl& operator<<(Obj* item) {
            objects_.push_back(item); 
            return *this;
        }

        ObjTbl& operator<<(Obj& item) {
            objects_.push_back(&item);
            return *this;
        }        

        ObjTbl& operator<<(const std::vector<Obj*>& items) {
            AddRows(items);
            return *this;
        }

        ObjTbl& operator<<(const std::vector<Obj>& items) {
            AddRows(items);
            return *this;
        }

        ObjTbl& operator<<(ObjTblCol<Obj> column) {
            columns_.push_back(column);
            return *this;
        }

        // Returns pointer to the object on specified row.
        Obj* GetRow(std::size_t row) {
            if (row < objects_.size()) return objects_[row];
            else return nullptr;    // or could also throw?
        }

        // Returns new instance, only with object which satisfy filter function.
        ObjTbl Where(std::function<bool(const Obj*)> filter) {
            std::vector<Obj*> filtered;
            std::ranges::copy_if(objects_, std::back_inserter(filtered), filter);
            return ObjTbl<Obj>(columns_, filtered);
        }

        // Mutates this instance, only keeps objects which satisfy filter function.
        ObjTbl& Filter(std::function<bool(const Obj*)> filter) {
            std::vector<Obj*> filtered;
            std::ranges::copy_if(objects_, std::back_inserter(filtered), filter);
            objects_ = std::move(filtered);
            return *this;
        }

        // Returns new instance, only first n if positive, last n if negative, or all if 0.
        ObjTbl FirstN(int n) {
            if (n == 0) return *this;
            std::size_t len = std::min<std::size_t>(objects_.size(), static_cast<std::size_t>(std::abs(n)));
            if (n > 0) return ObjTbl(columns_, std::vector<Obj*>(objects_.begin(), objects_.begin() + len));
            return ObjTbl(columns_, std::vector<Obj*>(objects_.end() - len, objects_.end()));
        }

        // Mutatues this instance, removes all but first n entries if positive, last n if negative, no change if 0.
        ObjTbl& FilterFirstN(int n) {
            if (n == 0) return *this;
            std::size_t len = std::min<std::size_t>(objects_.size(), static_cast<std::size_t>(std::abs(n)));
            if (n > 0) objects_ = std::vector<Obj*>(objects_.begin(), objects_.begin() + len);
            else objects_ = std::vector<Obj*>(objects_.end() - len, objects_.end());
            return *this;
        }

        // Change the order of rows in accordance with a given column.
        ObjTbl& OrderBy(int column = -1, bool desc = true) {
            if (column >= 0 && column < columns_.size()) {
                std::ranges::sort(objects_, columns_[column].Order);
                if (desc) std::ranges::reverse(objects_);
            }
            return *this;
        }                

        std::string Print(TblRenderType type = TblRenderType::BoxCompact, int orderBy = -1, bool desc = true) {
            // Order if applicable:
            OrderBy(orderBy, desc);

            // Define cell seperator & row seperator:
            std::string cellSep;        // String we put between each cell (one char, "│" or "|" or " "
            std::ostringstream rss;     // streamstream for string we put between each row.
            std::string truncStr;       // Truncated string: … if utf-8, '-' if ascii.
            std::string orderStr;       // Marker for column we order by: "▲" or "▼" if utf-8, just * if ascii (I don't like how 'v' & '^' look)

            switch (type) {
            case TblRenderType::Ascii: {
                cellSep = "|";
                rss << '\n';
                for (const auto& col : columns_) 
                    rss << '|' << str::Repeat('-', col.Width);
                rss << "|\n";
                truncStr = "-";
                orderStr = "*";
                break;                    
            }
            case TblRenderType::AsciiCompact: {
                cellSep = "|";
                rss << '\n';
                truncStr = "-";
                orderStr = "*";
                break;  
            }
            case TblRenderType::Borderless: {
                cellSep = " ";
                rss << '\n';
                truncStr = "-";
                orderStr = "*" ;
                break;
            }
            case TblRenderType::Box: {
                cellSep = "│";
                rss << "\n├";
                for (std::size_t i = 0; i < columns_.size(); i++) {
                    if (i > 0) rss << "┼";
                    rss << str::Repeat("─", columns_[i].Width);
                }                        
                rss << "┤\n";
                truncStr = "…";
                orderStr = desc ? "▼ " : "▲ ";
                break;
            }
            case TblRenderType::BoxCompact: {
                cellSep = "│";
                rss << '\n';
                truncStr = "…";
                orderStr = desc ? "▼ " : "▲ ";
                break;
            }
            }
            std::string rowSep = rss.str();
            std::ostringstream oss;

            // Draw top based on rendertype:
            switch (type) {
            case TblRenderType::Ascii:           
            case TblRenderType::AsciiCompact:    
            case TblRenderType::Borderless:      break; // Only Box & BoxCompact have this, rest is blank.
            case TblRenderType::Box: 
            case TblRenderType::BoxCompact: 
                oss << "┌";
                for (std::size_t i = 0; i < columns_.size(); i++) {
                    if (i != 0) oss << "┬";
                    oss << str::Repeat("─", columns_[i].Width);
                }
                oss << "┐\n";
                break;
            }

            // Draw headers:
            for (std::size_t i = 0; i < columns_.size(); i++) 
                    oss << cellSep << AlignText(i == orderBy ? orderStr + columns_[i].Header : columns_[i].Header, columns_[i].HeaderAlignment, columns_[i].Width, truncStr);                
                oss << cellSep;

            // Draw header bottom if applicable:
            switch(type) {
            case TblRenderType::AsciiCompact:
                oss << '\n';
                for (const auto& col : columns_)
                    oss << "|" << str::Repeat('-', col.Width);
                oss << "|";
                break;
            case TblRenderType::Borderless:
                oss << '\n';
                break;
            case TblRenderType::BoxCompact:
                oss << "\n├";
                for (std::size_t i = 0; i < columns_.size(); i++) { 
                    if (i > 0) oss << "┼";
                    oss << str::Repeat("─", columns_[i].Width);
                }
                oss << "┤";
                break;
            default: break;
            }
            
            // Draw data:
            for (const Obj* row : objects_) {
                oss << rowSep;
                for (const ObjTblCol<Obj>& col : columns_) {
                    oss << cellSep << AlignText(col.Render(row), col.DataAlignment, col.Width);
                }
                oss << cellSep;
            }

            // Draw bottom:
            switch (type) {
            case TblRenderType::Ascii:
                oss << rowSep;
                break;
            case TblRenderType::AsciiCompact:
            case TblRenderType::Borderless:
                break;
            case TblRenderType::Box:
            case TblRenderType::BoxCompact:
                oss << "\n└";
                for (std::size_t i = 0; i < columns_.size(); i++) {
                    if (i > 0) oss << "┴";
                    oss << str::Repeat("─", columns_[i].Width);
                }
                oss << "┘";
                break;
            }

            return oss.str();
        }

        ObjTbl& AddColumn(std::string header, std::size_t width, std::function<std::string(const Obj*)> renderFunc, std::function<bool(const Obj*, const Obj*)> orderFunc, TextAlign headerAlignment = TextAlign::Left, TextAlign dataAlignment = TextAlign::Left) {
            columns_.push_back(ObjTblCol<Obj>(header, width, renderFunc, orderFunc, headerAlignment, dataAlignment));
            return *this;
        }

        ObjTbl& AddRows(const std::vector<Obj*>& rows) {
            objects_.insert(objects_.end(), rows.begin(), rows.end());
        }

        ObjTbl& AddRows(const std::vector<Obj>& rows) {
            objects_.reserve(objects_.size() + rows.size());
            for (const Obj& o : rows) objects_.push_back(&o);
        }
    };
}