#include "Analyzer.h"
#include "Core/AST.h"
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

namespace AlengLSP
{
    TypeInfo ResolveType(const Aleng::ASTNode *node)
    {
        if (!node) return {"Unknown", {}, ""};

        if (dynamic_cast<const Aleng::IntegerNode *>(node) ||
            dynamic_cast<const Aleng::FloatNode *>(node))
        {
            return {"Number", {}, ""};
        }
        if (dynamic_cast<const Aleng::StringNode *>(node))
        {
            return {"String", {}, ""};
        }
        if (dynamic_cast<const Aleng::BooleanNode *>(node))
        {
            return {"Boolean", {}, ""};
        }
        if (dynamic_cast<const Aleng::ListNode *>(node))
        {
            return {"List", {}, ""};
        }
        if (const auto mapNode = dynamic_cast<const Aleng::MapNode *>(node))
        {
            std::vector<std::string> keys;
            for (const auto &key: mapNode->Elements | std::views::keys)
            {
                if (const auto keyStr = dynamic_cast<const Aleng::StringNode *>(key.get()))
                {
                    keys.push_back(keyStr->Value);
                } else if (const auto keyId = dynamic_cast<const Aleng::IdentifierNode *>(key.get()))
                {
                    keys.push_back(keyId->Value);
                }
            }
            return {"Map", keys, ""};
        }
        if (const auto funcNode = dynamic_cast<const Aleng::FunctionDefinitionNode *>(node))
        {
            std::string sig = "Fn(";
            for (size_t i = 0; i < funcNode->Parameters.size(); ++i)
            {
                sig += funcNode->Parameters[i].Name;
                if (i < funcNode->Parameters.size() - 1) sig += ", ";
            }
            sig += ")";
            return {"Function", {}, sig};
        }

        return {"Unknown", {}, ""};
    }

    void Analyzer::Analyze(const Aleng::ProgramNode &program)
    {
        m_RootScope = std::make_shared<ScopeRange>();
        m_RootScope->StartLine = 0;
        m_RootScope->EndLine = -1;
        m_CurrentScope = m_RootScope;

        m_TokenBuilder = SemanticTokenBuilder();

        for (const auto &stmt: program.Statements)
        {
            VisitNode(stmt.get());
        }
    }

    std::vector<SymbolInfo> Analyzer::GetSymbolsAt(const int line) const
    {
        std::vector<SymbolInfo> result;

        std::shared_ptr<ScopeRange> target = m_RootScope;
        bool changed = true;

        while (changed)
        {
            changed = false;
            for (const auto &child: target->Children)
            {
                if (line >= child->StartLine)
                {
                    target = child;
                    changed = true;
                    break;
                }
            }
        }

        auto curr = target;
        while (curr)
        {
            for (const auto &sym: curr->Symbols)
            {
                bool alreadyAdded = false;
                for (const auto &res: result)
                {
                    if (res.Name == sym.Name)
                    {
                        alreadyAdded = true;
                        break;
                    }
                }
                if (!alreadyAdded) result.push_back(sym);
            }
            curr = curr->Parent;
        }
        return result;
    }

    const SymbolInfo *Analyzer::FindSymbol(const std::string &name, int line) const
    {
        std::shared_ptr<ScopeRange> target = m_RootScope;
        bool changed = true;
        while (changed)
        {
            changed = false;
            for (const auto &child: target->Children)
            {
                if (line >= child->StartLine)
                {
                    target = child;
                    changed = true;
                    break;
                }
            }
        }

        auto curr = target;
        while (curr)
        {
            for (const auto &sym: curr->Symbols)
            {
                if (sym.Name == name) return &sym;
            }
            curr = curr->Parent;
        }
        return nullptr;
    }

    void Analyzer::UpdateOrDefineSymbol(const std::string &name, const std::string &kind, int line,
                                        const TypeInfo &type) const
    {
        for (auto &sym: m_CurrentScope->Symbols)
        {
            if (sym.Name == name)
            {
                if (sym.Type.Name == "Unknown" && type.Name != "Unknown")
                {
                    sym.Type = type;
                }
                return;
            }
        }

        m_CurrentScope->Symbols.push_back({name, kind, line, type});
    }

    void Analyzer::VisitBlock(const Aleng::BlockNode *node)
    {
        if (!node) return;

        for (const auto& stmt : node->Statements) {
            VisitNode(stmt.get());
        }
    }

    void Analyzer::VisitNode(const Aleng::ASTNode *node)
    {
        if (!node) return;

        if (const auto func = dynamic_cast<const Aleng::FunctionDefinitionNode *>(node))
        {
            const TypeInfo funcType = ResolveType(func);

            if (func->FunctionName)
            {
                UpdateOrDefineSymbol(*func->FunctionName, "Function", func->Location.Line, funcType);
                // Semantic Token: Function (Type 1)
                m_TokenBuilder.Push(func->Location.Line - 1, func->Location.Column - 1, func->FunctionName->length(), 1,
                                    1);
            }

            const auto funcScope = std::make_shared<ScopeRange>();
            funcScope->StartLine = func->Location.Line;
            funcScope->Parent = m_CurrentScope;
            m_CurrentScope->Children.push_back(funcScope);
            m_CurrentScope = funcScope;

            for (const auto &param: func->Parameters)
            {
                UpdateOrDefineSymbol(param.Name, "Parameter", func->Location.Line, {"Unknown", {}, ""});
            }

            VisitNode(func->Body.get());
            m_CurrentScope = funcScope->Parent;
        }

        else if (const auto assign = dynamic_cast<const Aleng::AssignExpressionNode *>(node))
        {
            const TypeInfo inferredType = ResolveType(assign->Right.get());
            VisitNode(assign->Right.get());

            if (const auto id = dynamic_cast<const Aleng::IdentifierNode *>(assign->Left.get()))
            {
                UpdateOrDefineSymbol(id->Value, "Variable", id->Location.Line, inferredType);

                m_TokenBuilder.Push(id->Location.Line - 1, id->Location.Column - 1, id->Value.length(), 0, 0);
            }
            else if (const auto member = dynamic_cast<const Aleng::MemberAccessNode *>(assign->Left.get()))
            {
                VisitNode(member);
            } else if (const auto listAcc = dynamic_cast<const Aleng::ListAccessNode *>(assign->Left.get()))
            {
                VisitNode(listAcc);
            }
        }

        else if (const auto block = dynamic_cast<const Aleng::BlockNode *>(node))
        {
            VisitBlock(block);
        } else if (const auto ifNode = dynamic_cast<const Aleng::IfNode *>(node))
        {
            VisitNode(ifNode->Condition.get());
            VisitNode(ifNode->ThenBranch.get());
            if (ifNode->ElseBranch) VisitNode(ifNode->ElseBranch.get());
        } else if (const auto forNode = dynamic_cast<const Aleng::ForStatementNode *>(node))
        {
            const auto loopScope = std::make_shared<ScopeRange>();
            loopScope->StartLine = forNode->Location.Line;
            loopScope->Parent = m_CurrentScope;
            m_CurrentScope->Children.push_back(loopScope);
            m_CurrentScope = loopScope;

            if (forNode->NumericLoopInfo)
            {
                UpdateOrDefineSymbol(forNode->NumericLoopInfo->IteratorVariableName, "Variable", forNode->Location.Line,
                                     {"Number", {}, ""});
                VisitNode(forNode->NumericLoopInfo->StartExpression.get());
                VisitNode(forNode->NumericLoopInfo->EndExpression.get());
            } else if (forNode->CollectionLoopInfo)
            {
                // Inferir tipo da iteração baseado na coleção (TODO)
                UpdateOrDefineSymbol(forNode->CollectionLoopInfo->IteratorVariableName, "Variable",
                                     forNode->Location.Line, {"Unknown", {}, ""});
                VisitNode(forNode->CollectionLoopInfo->CollectionExpression.get());
            }

            VisitNode(forNode->Body.get());
            m_CurrentScope = loopScope->Parent;
        }

        else if (const auto bin = dynamic_cast<const Aleng::BinaryExpressionNode *>(node))
        {
            VisitNode(bin->Left.get());
            VisitNode(bin->Right.get());
        } else if (const auto call = dynamic_cast<const Aleng::FunctionCallNode *>(node))
        {
            VisitNode(call->CallableExpression.get());
            for (const auto &arg: call->Arguments) VisitNode(arg.get());
        } else if (const auto id = dynamic_cast<const Aleng::IdentifierNode *>(node))
        {
            m_TokenBuilder.Push(id->Location.Line - 1, id->Location.Column - 1, id->Value.length(), 0, 0);
        } else if (const auto member = dynamic_cast<const Aleng::MemberAccessNode *>(node))
        {
            VisitNode(member->Object.get());
            m_TokenBuilder.Push(member->Location.Line - 1, member->MemberIdentifier.Location.Column - 1,
                                member->MemberIdentifier.Value.length(), 0, 0); // 0 = Variable/Property
        } else if (const auto str = dynamic_cast<const Aleng::StringNode *>(node))
        {
            m_TokenBuilder.Push(str->Location.Line - 1, str->Location.Column - 1, str->Value.length() + 2, 3, 0);
        } else if (const auto num = dynamic_cast<const Aleng::IntegerNode *>(node))
        {
            m_TokenBuilder.Push(num->Location.Line - 1, num->Location.Column - 1, std::to_string(num->Value).length(),
                                4, 0);
        }
    }

    json Analyzer::GetSemanticTokens()
    {
        return m_TokenBuilder.GetData();
    }
} // AlengLSP
