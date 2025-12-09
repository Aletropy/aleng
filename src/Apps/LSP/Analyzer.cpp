#include "Analyzer.h"
#include "Core/AST.h"
#include <sstream>
#include <iostream>

#include "Core/Error.h"

namespace AlengLSP
{
    std::string TypeInfo::ToString() const
    {
        switch (kind)
        {
            case Kind::Unknown: return "Unknown";
            case Kind::Any: return "Any";
            case Kind::Void: return "Void";
            case Kind::Number: return "Number";
            case Kind::String: return "String";
            case Kind::Boolean: return "Boolean";
            case Kind::List: return "List<" + (innerType ? innerType->ToString() : "Any") + ">";
            case Kind::Map: return "Map";
            case Kind::Function:
            {
                std::stringstream ss;
                ss << "Fn(";
                for (size_t i = 0; i < paramTypes.size(); ++i)
                {
                    ss << (paramTypes[i] ? paramTypes[i]->ToString() : "Any");
                    if (i < paramTypes.size() - 1) ss << ", ";
                }
                ss << ")";
                if (returnType && returnType->kind != Kind::Void)
                    ss << " -> " << returnType->ToString();
                return ss.str();
            }
            default: return "Unknown";
        }
    }

    bool TypeInfo::operator==(const TypeInfo &other) const
    {
        if (kind != other.kind) return false;
        // Simplified equality check (deep check omitted to not go crazy)
        return true;
    }

    void Symbol::AddReference(const Aleng::SourceRange &range)
    {
        references.push_back(range);
    }

    Scope::Scope(std::shared_ptr<Scope> p) : parent(std::move(p))
    {
        level = parent ? parent->level + 1 : 0;
    }

    void Scope::Define(const std::shared_ptr<Symbol> &sym)
    {
        if (!sym) return;
        symbols[sym->name] = sym;
    }

    std::shared_ptr<Symbol> Scope::Resolve(const std::string &name)
    {
        if (symbols.contains(name)) return symbols[name];
        if (parent) return parent->Resolve(name);
        return nullptr;
    }

    void Analyzer::Analyze(const Aleng::ProgramNode &program, const std::string &uri)
    {
        FileAnalysisContext ctx;
        ctx.globalScope = std::make_shared<Scope>();

        auto currentScope = ctx.globalScope;
        for (const auto &stmt: program.Statements)
        {
            VisitNode(stmt.get(), currentScope, ctx);
        }

        std::sort(ctx.spatialIndex.begin(), ctx.spatialIndex.end());

        m_Contexts[uri] = std::move(ctx);
    }

    void Analyzer::VisitNode(const Aleng::ASTNode* node, std::shared_ptr<Scope>& currentScope, FileAnalysisContext& ctx) {
        if (!node) return;

        if (const auto strNode = dynamic_cast<const Aleng::StringNode*>(node)) {
            if (strNode->Location.Start.Line == strNode->Location.End.Line) {
                auto loc = strNode->Location;
                loc.End.Column -= 1;
                AddSpatialToken(loc, SemanticType::String, ctx);
            }
        }
        else if (dynamic_cast<const Aleng::IntegerNode*>(node) || dynamic_cast<const Aleng::FloatNode*>(node)) {
            AddSpatialToken(node->Location, SemanticType::Number, ctx);
        }
        else if (dynamic_cast<const Aleng::BooleanNode*>(node)) {
            AddSpatialToken(node->Location, SemanticType::Keyword, ctx);
        }
        else if (const auto id = dynamic_cast<const Aleng::IdentifierNode*>(node)) {
            auto sym = currentScope->Resolve(id->Value);
            if (sym) {
                sym->AddReference(id->Location);
                AddToSpatialIndex(node, sym, sym->type, ctx);
            } else {
                AddSpatialToken(id->Location, SemanticType::Variable, ctx);
            }
        }


        else if (const auto forNode = dynamic_cast<const Aleng::ForStatementNode*>(node)) {
            AddSpatialToken(forNode->Location, SemanticType::Keyword, ctx);

            auto loopScope = std::make_shared<Scope>(currentScope);
            RegisterScope(forNode->Location, loopScope, ctx);

            if (forNode->NumericLoopInfo) {
                VisitNode(forNode->NumericLoopInfo->StartExpression.get(), currentScope, ctx);
                VisitNode(forNode->NumericLoopInfo->EndExpression.get(), currentScope, ctx);
                if (forNode->NumericLoopInfo->StepExpression) {
                    VisitNode(forNode->NumericLoopInfo->StepExpression.get(), currentScope, ctx);
                }
                auto iterType = std::make_shared<TypeInfo>(TypeInfo{TypeInfo::Kind::Number});
                DefineSymbol(forNode->NumericLoopInfo->IteratorVariableName, Symbol::Category::Variable, iterType, forNode->Location, loopScope, ctx);
            }
            else if (forNode->CollectionLoopInfo) {
                VisitNode(forNode->CollectionLoopInfo->CollectionExpression.get(), currentScope, ctx);
                auto iterType = std::make_shared<TypeInfo>(TypeInfo{TypeInfo::Kind::Any});
                DefineSymbol(forNode->CollectionLoopInfo->IteratorVariableName, Symbol::Category::Variable, iterType, forNode->Location, loopScope, ctx);
            }

            auto prevScope = currentScope;
            currentScope = loopScope;
            VisitNode(forNode->Body.get(), currentScope, ctx);

            Aleng::SourceRange endKwRange;
            endKwRange.Start.Line = forNode->Location.End.Line;
            endKwRange.End.Line = forNode->Location.End.Line;
            endKwRange.End.Column = forNode->Location.End.Column;
            endKwRange.Start.Column = std::max(1, endKwRange.End.Column - 2);
            AddSpatialToken(endKwRange, SemanticType::Keyword, ctx);

            currentScope = prevScope;
        }

        else if (const auto whileNode = dynamic_cast<const Aleng::WhileStatementNode*>(node)) {
            AddSpatialToken(whileNode->Location, SemanticType::Keyword, ctx);

            VisitNode(whileNode->Condition.get(), currentScope, ctx);
            VisitNode(whileNode->Body.get(), currentScope, ctx);

            Aleng::SourceRange endKwRange;
            endKwRange.Start.Line = whileNode->Location.End.Line;
            endKwRange.End.Line = whileNode->Location.End.Line;
            endKwRange.End.Column = whileNode->Location.End.Column;
            endKwRange.Start.Column = std::max(1, endKwRange.End.Column - 2);
            AddSpatialToken(endKwRange, SemanticType::Keyword, ctx);
        }

        else if (const auto ifNode = dynamic_cast<const Aleng::IfNode*>(node)) {
            Aleng::SourceRange kwRange = ifNode->Location;
            kwRange.End.Line = kwRange.Start.Line;
            kwRange.End.Column = kwRange.Start.Column + 1;
            AddSpatialToken(kwRange, SemanticType::Keyword, ctx);

            VisitNode(ifNode->Condition.get(), currentScope, ctx);
            VisitNode(ifNode->ThenBranch.get(), currentScope, ctx);
            if (ifNode->ElseBranch)
            {
                VisitNode(ifNode->ElseBranch.get(), currentScope, ctx);
            }
            Aleng::SourceRange endKwRange;
            endKwRange.Start.Line = ifNode->Location.End.Line;
            endKwRange.End.Line = ifNode->Location.End.Line;
            endKwRange.End.Column = ifNode->Location.End.Column;
            endKwRange.Start.Column = std::max(1, endKwRange.End.Column - 2);
            AddSpatialToken(endKwRange, SemanticType::Keyword, ctx);
        }

        else if (const auto func = dynamic_cast<const Aleng::FunctionDefinitionNode*>(node))
        {
            auto fnKeywordRange = Aleng::SourceRange(
                func->Location.Start,
                Aleng::SourceLocation(
                    func->Location.Start.Line,
                    func->Location.Start.Column + 2
                )
            );

            AddSpatialToken(fnKeywordRange, SemanticType::Keyword, ctx);

            auto funcType = std::make_shared<TypeInfo>();
            funcType->kind = TypeInfo::Kind::Function;
            funcType->returnType = std::make_shared<TypeInfo>(TypeInfo{TypeInfo::Kind::Any});

            std::shared_ptr<Symbol> funcSym = nullptr;
            if (func->FunctionName)
            {
                auto nameRange = Aleng::SourceRange();
                nameRange.Start.Line = func->Location.End.Line;
                nameRange.End.Line = func->Location.End.Line;
                nameRange.End.Column = func->Location.End.Column + 3 + static_cast<int>(func->FunctionName->size());
                nameRange.Start.Column = func->Location.End.Column + 2;
                funcSym = DefineSymbol(*func->FunctionName, Symbol::Category::Function, funcType, nameRange, currentScope, ctx);
            }

            auto funcScope = std::make_shared<Scope>(currentScope);
            RegisterScope(func->Location, funcScope, ctx);

            for (const auto& param : func->Parameters) {
                auto paramType = std::make_shared<TypeInfo>(TypeInfo{TypeInfo::Kind::Any});
                if (param.TypeName) {
                    if (*param.TypeName == "Number") paramType->kind = TypeInfo::Kind::Number;
                    else if (*param.TypeName == "String") paramType->kind = TypeInfo::Kind::String;
                    else if (*param.TypeName == "Boolean") paramType->kind = TypeInfo::Kind::Boolean;
                }

                funcType->paramTypes.push_back(paramType);
                DefineSymbol(param.Name, Symbol::Category::Parameter, paramType, param.Range, funcScope, ctx);
            }
            if (funcSym) funcSym->type = funcType;

            auto prevScope = currentScope;
            currentScope = funcScope;
            VisitNode(func->Body.get(), currentScope, ctx);

            auto endKwRange = func->EndLocation;
            endKwRange.Start.Column = std::max(1, endKwRange.End.Column - 2);

            AddSpatialToken(endKwRange, SemanticType::Keyword, ctx);

            currentScope = prevScope;
        }

        else if (const auto ret = dynamic_cast<const Aleng::ReturnNode*>(node)) {
            AddSpatialToken(ret->Location, SemanticType::Keyword, ctx);
            if (ret->ReturnValueExpression) VisitNode(ret->ReturnValueExpression.get(), currentScope, ctx);
        }
        else if (const auto brk = dynamic_cast<const Aleng::BreakNode*>(node)) {
            AddSpatialToken(brk->Location, SemanticType::Keyword, ctx);
        }
        else if (const auto cont = dynamic_cast<const Aleng::ContinueNode*>(node)) {
            AddSpatialToken(cont->Location, SemanticType::Keyword, ctx);
        }
        else if (const auto imp = dynamic_cast<const Aleng::ImportModuleNode*>(node)) {
            AddSpatialToken(imp->Location, SemanticType::Keyword, ctx);
            AddSpatialToken(imp->ModuleLocation, SemanticType::String, ctx);
        }

        else if (const auto block = dynamic_cast<const Aleng::BlockNode*>(node)) {
            for (const auto& stmt : block->Statements) VisitNode(stmt.get(), currentScope, ctx);
        }
        else if (const auto assign = dynamic_cast<const Aleng::AssignExpressionNode*>(node)) {
            VisitNode(assign->Right.get(), currentScope, ctx);
            auto rhsType = InferType(assign->Right.get(), currentScope);

            if (const auto ident = dynamic_cast<const Aleng::IdentifierNode*>(assign->Left.get()))
            {
                if (auto existing = currentScope->Resolve(ident->Value))
                {
                    existing->AddReference(ident->Location);
                    AddToSpatialIndex(ident, existing, existing->type, ctx);
                } else
                {
                    DefineSymbol(ident->Value, Symbol::Category::Variable, rhsType, ident->Location, currentScope, ctx);
                }
            }
            else if (const auto member = dynamic_cast<const Aleng::MemberAccessNode*>(assign->Left.get())) {
                VisitNode(member, currentScope, ctx);
            }
            else if (const auto listAcc = dynamic_cast<const Aleng::ListAccessNode*>(assign->Left.get())) {
                VisitNode(listAcc, currentScope, ctx);
            }
        }
        else if (const auto call = dynamic_cast<const Aleng::FunctionCallNode*>(node)) {
            VisitNode(call->CallableExpression.get(), currentScope, ctx);
            for (const auto& arg : call->Arguments) VisitNode(arg.get(), currentScope, ctx);
        }
        else if (const auto bin = dynamic_cast<const Aleng::BinaryExpressionNode*>(node)) {
            VisitNode(bin->Left.get(), currentScope, ctx);
            VisitNode(bin->Right.get(), currentScope, ctx);
        }
        else if (const auto un = dynamic_cast<const Aleng::UnaryExpressionNode*>(node)) {
            VisitNode(un->Right.get(), currentScope, ctx);
        }
        else if (const auto list = dynamic_cast<const Aleng::ListNode*>(node)) {
            for(const auto& el : list->Elements) VisitNode(el.get(), currentScope, ctx);
        }
        else if (const auto map = dynamic_cast<const Aleng::MapNode*>(node)) {
            for(const auto&[key, val] : map->Elements) {
                VisitNode(key.get(), currentScope, ctx);
                VisitNode(val.get(), currentScope, ctx);
            }
        }
        else if (const auto member = dynamic_cast<const Aleng::MemberAccessNode*>(node)) {
            VisitNode(member->Object.get(), currentScope, ctx);
            Aleng::SourceRange propRange = member->Location;
            propRange.Start.Column = propRange.End.Column - member->MemberIdentifier.Value.length() + 1;
            if (propRange.Start.Column > member->Location.Start.Column) {
                 AddSpatialToken(propRange, SemanticType::Property, ctx);
            }
        }
        else if (const auto listAcc = dynamic_cast<const Aleng::ListAccessNode*>(node)) {
            VisitNode(listAcc->Object.get(), currentScope, ctx);
            VisitNode(listAcc->Index.get(), currentScope, ctx);
        }
    }

    std::shared_ptr<TypeInfo> Analyzer::InferType(const Aleng::ASTNode *node, const std::shared_ptr<Scope> &scope)
    {
        if (!node) return std::make_shared<TypeInfo>();

        if (dynamic_cast<const Aleng::IntegerNode *>(node) || dynamic_cast<const Aleng::FloatNode *>(node))
            return std::make_shared<TypeInfo>(TypeInfo{TypeInfo::Kind::Number});

        if (dynamic_cast<const Aleng::StringNode *>(node))
            return std::make_shared<TypeInfo>(TypeInfo{TypeInfo::Kind::String});

        if (const auto id = dynamic_cast<const Aleng::IdentifierNode *>(node))
        {
            if (const auto sym = scope->Resolve(id->Value); sym && sym->type) return sym->type;
        }

        // TODO: BinaryExpression inference, FunctionCall return type inference
        return std::make_shared<TypeInfo>(TypeInfo{TypeInfo::Kind::Unknown});
    }

    void Analyzer::RegisterScope(const Aleng::SourceRange &range, std::shared_ptr<Scope> scope,
                                 FileAnalysisContext &ctx)
    {
        ctx.scopeIndex.push_back({range, scope});
    }

    std::shared_ptr<Scope> Analyzer::FindScopeAt(const std::string &uri, const int line, const int col) const
    {
        if (!m_Contexts.contains(uri)) return nullptr;
        const auto &ctx = m_Contexts.at(uri);

        std::shared_ptr<Scope> bestScope = ctx.globalScope;
        int bestDepth = -1;

        for (const auto &entry: ctx.scopeIndex)
        {
            bool inLine = (line >= entry.range.Start.Line && line <= entry.range.End.Line);
            if (!inLine) continue;

            if (line == entry.range.Start.Line && col < entry.range.Start.Column) continue;
            if (line == entry.range.End.Line && col > entry.range.End.Column) continue;

            if (entry.scope->level > bestDepth)
            {
                bestDepth = entry.scope->level;
                bestScope = entry.scope;
            }
        }
        return bestScope;
    }

    std::shared_ptr<Symbol> Analyzer::DefineSymbol(const std::string &name, Symbol::Category cat,
                                                   std::shared_ptr<TypeInfo> type, const Aleng::SourceRange &range,
                                                   std::shared_ptr<Scope> &scope, FileAnalysisContext &ctx)
    {
        auto sym = std::make_shared<Symbol>();
        sym->name = name;
        sym->category = cat;
        sym->type = type;
        sym->definitionRange = range;

        scope->Define(sym);
        ctx.allSymbols.push_back(sym);

        AddToSpatialIndex(nullptr, sym, type, ctx);

        SpatialEntry entry;
        entry.range = range;
        entry.symbol = sym;
        entry.type = type;
        ctx.spatialIndex.push_back(entry);

        return sym;
    }

    void Analyzer::AddToSpatialIndex(const Aleng::ASTNode *node, const std::shared_ptr<Symbol> &sym,
                                     const std::shared_ptr<TypeInfo> &type, FileAnalysisContext &ctx)
    {
        if (node)
        {
            SpatialEntry entry;
            entry.range = node->Location;
            entry.symbol = sym;
            entry.type = type;
            ctx.spatialIndex.push_back(entry);
        }
    }

    void Analyzer::AddSpatialToken(const Aleng::SourceRange &range, SemanticType type, FileAnalysisContext &ctx)
    {
        SpatialEntry entry;
        entry.range = range;
        entry.symbol = nullptr;
        entry.type = nullptr;
        entry.customType = type;
        ctx.spatialIndex.push_back(entry);
    }

    std::shared_ptr<Symbol> Analyzer::FindSymbolAt(const std::string &uri, int line, int col) const
    {
        if (!m_Contexts.contains(uri)) return nullptr;
        const auto &idx = m_Contexts.at(uri).spatialIndex;

        SpatialEntry target;
        target.range.Start.Line = line;
        target.range.Start.Column = col;

        const auto it = std::lower_bound(idx.begin(), idx.end(), target);

        const int startIdx = static_cast<int>(std::distance(idx.begin(), it));
        for (int i = std::max(0, startIdx - 5); i < std::min(static_cast<int>(idx.size()), startIdx + 5); ++i)
        {
            if (const auto &entry = idx[i]; line >= entry.range.Start.Line && line <= entry.range.End.Line)
            {
                const bool afterStart = (line > entry.range.Start.Line) || (col >= entry.range.Start.Column);
                const bool beforeEnd = (line < entry.range.End.Line) || (col <= entry.range.End.Column);

                if (afterStart && beforeEnd && entry.symbol)
                {
                    return entry.symbol;
                }
            }
        }
        return nullptr;
    }

    std::string Analyzer::GetHoverInfo(const std::string &uri, const int line, const int col) const
    {
        if (const auto sym = FindSymbolAt(uri, line, col))
        {
            std::string md = "**" + sym->name + "**\n\n";
            md += "Type: `" + (sym->type ? sym->type->ToString() : "Unknown") + "`\n";
            if (!sym->documentation.empty()) md += "\n" + sym->documentation;
            return md;
        }
        return "";
    }

    std::vector<Aleng::SourceRange> Analyzer::GetReferences(const std::string &uri, const int line, const int col) const
    {
        if (const auto sym = FindSymbolAt(uri, line, col)) return sym->references;
        return {};
    }

    std::vector<std::shared_ptr<Symbol> > Analyzer::GetCompletions(const std::string &uri, int line, int col) const
    {
        if (!m_Contexts.contains(uri)) return {};

        const auto currentScope = FindScopeAt(uri, line, col);
        if (!currentScope) return {};

        std::vector<std::shared_ptr<Symbol> > results;
        std::unordered_map<std::string, bool> seen;

        auto scopeIter = currentScope;
        while (scopeIter)
        {
            for (const auto &[fst, snd]: scopeIter->symbols)
            {
                if (!seen.contains(fst))
                {
                    results.push_back(snd);
                    seen[fst] = true;
                }
            }
            scopeIter = scopeIter->parent;
        }

        return results;
    }

    json Analyzer::GetSemanticTokens(const std::string& uri) const {
        if (!m_Contexts.contains(uri)) return json::array();

        auto sortedIndex = m_Contexts.at(uri).spatialIndex;
        std::sort(sortedIndex.begin(), sortedIndex.end());

        std::vector<int> data;

        int prevLine = 0;
        int prevChar = 0;

        int lastTokenLine = -1;
        int lastTokenEndCol = -1;

        for (const auto& entry : sortedIndex) {
            if (entry.range.Start.Line != entry.range.End.Line) continue;

            int line = std::max(0, entry.range.Start.Line - 1);
            int col = std::max(0, entry.range.Start.Column - 1);

            int tokenType = 0;
            if (entry.customType != SemanticType::None) {
                tokenType = static_cast<int>(entry.customType);
            } else if (entry.symbol) {
                switch(entry.symbol->category) {
                    case Symbol::Category::Variable:  tokenType = 0; break;
                    case Symbol::Category::Function:  tokenType = 1; break;
                    case Symbol::Category::Parameter: tokenType = 2; break;
                    case Symbol::Category::Property:  tokenType = 3; break;
                    case Symbol::Category::Class:     tokenType = 4; break;
                    default: tokenType = 0;
                }
            } else {
                continue;
            }

            int length = 0;

            if (entry.symbol) {
                length = static_cast<int>(entry.symbol->name.length());
            }
            else {
                length = entry.range.End.Column - entry.range.Start.Column;
            }

             if (length <= 0) length = 1;

            if (line == lastTokenLine && col < lastTokenEndCol) {
                continue;
            }

            int deltaLine = line - prevLine;
            int deltaChar = (deltaLine == 0) ? (col - prevChar) : col;

            data.push_back(deltaLine);
            data.push_back(deltaChar);
            data.push_back(length);
            data.push_back(tokenType);
            data.push_back(0); // Modifiers

            prevLine = line;
            prevChar = col;

            lastTokenLine = line;
            lastTokenEndCol = col + length;
        }

        return data;
    }
} // namespace AlengLSP
