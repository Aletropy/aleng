#pragma once
#include <string>

namespace Aleng {

    struct SourceLocation {
        int Line = 0;
        int Column = 0;
    };

    struct SourceRange {
        SourceLocation Start;
        SourceLocation End;
        std::string FilePath;

        bool Contains(const int line, const int column) const {
            if (line < Start.Line || line > End.Line) return false;
            if (line == Start.Line && column < Start.Column) return false;
            if (line == End.Line && column > End.Column) return false;
            return true;
        }
    };
}