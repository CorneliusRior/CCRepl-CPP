#include <util/str.h>
#include <util/fmt.h>
#include <ranges>

#define FMT_OTCOL_EXT(header, width, varname) header, width, [](const auto* obj){ return obj->varname; }

namespace fmt {    

    // Column for a ObjTbl table:
    template <typename Obj> 
    class ObjTblCol {
    public:
        std::string Header;
        std::size_t Width;
        TextAlign HeaderAlignment;
        TextAlign DataAlignment;
        std::function<std::string(const Obj*)> Render;
        std::function<bool(const Obj*, const Obj*)> Order; 

        ObjTblCol(
            std::string header, 
            std::size_t width, 
            std::function<std::string(const Obj*)> renderFunc, 
            std::function<bool (const Obj*, const Obj*)> orderFunc, 
            TextAlign headerAlignment = TextAlign::Left, 
            TextAlign dataAlignment = TextAlign::Left) 
            : Header(header), Width(width), HeaderAlignment(headerAlignment), DataAlignment(dataAlignment), Render(renderFunc), Order(orderFunc) {}    
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

        // Adds a string column.
        ObjTbl& StrCol(const std::string& header, std::size_t width, std::function<std::string(const Obj*)> getFunc, TextAlign headerAlignment = TextAlign::Left, TextAlign dataAlignment = TextAlign::Left) {
            columns_.push_back(ObjTblCol<Obj>(
                header, width, getFunc, [&getFunc](const Obj* a, const Obj* b){ return getFunc(a) < getFunc(b); }, headerAlignment, dataAlignment 
            ));
            return *this;
        }

        ObjTbl& IntCol(const std::string& header, std::size_t width, std::function<int(const Obj*)> getFunc, TextAlign headerAlignment = TextAlign::Left, TextAlign dataAlignment = TextAlign::Right) {
            columns_.push_back(ObjTblCol<Obj>(
                header, width, [&getFunc](const Obj* obj){ return std::to_string(getFunc(obj)); }, [&getFunc](const Obj* a, const Obj* b){ return getFunc(a) < getFunc(b); }, headerAlignment, dataAlignment
            ));
            return *this;
        }

        ObjTbl& DblCol(const std::string& header, std::size_t width, std::function<double(const Obj*)> getFunc, std::size_t prec = 2, bool compact = false, TextAlign headerAlignment = TextAlign::Left, TextAlign dataAlignment = TextAlign::Right) {
            columns_.push_back(ObjTblCol<Obj>(
                header, width, [&getFunc, &prec, &compact](const Obj* obj){ return str::ToString(getFunc(obj), prec, compact); }, [&getFunc](const Obj* a, const Obj* b){ return getFunc(a) < getFunc(b); }, headerAlignment, dataAlignment
            ));
            return *this;
        }

        ObjTbl& PctCol(const std::string& header, std::size_t width, std::function<double(const Obj*)> getFunc, std::size_t prec = 2, TextAlign headerAlignment = TextAlign::Left, TextAlign dataAlignment = TextAlign::Right) {
            columns_.push_back(ObjTblCol<Obj>(
                header, width, [&getFunc, &prec](const Obj* obj){ return str::AsPct(getFunc(obj), prec); }, [&getFunc](const Obj* a, const Obj* b){ return getFunc(a) < getFunc(b); }, headerAlignment, dataAlignment
            ));
            return *this;
        }
    };
}