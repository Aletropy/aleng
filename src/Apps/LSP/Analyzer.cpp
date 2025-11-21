#include "Analyzer.h"

namespace AlengLSP
{
    void Analyzer::Analyze(const Aleng::ProgramNode &program)
    {
        m_RootScope = std::make_shared<ScopeRange>();
        m_RootScope->StartLine = 0;
        m_RootScope->EndLine = -1;
        m_CurrentScope = m_RootScope;

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
            result.insert(result.end(), curr->Symbols.begin(), curr->Symbols.end());
            curr = curr->Parent;
        }
        return result;
    }

    void Analyzer::VisitNode(const Aleng::ASTNode *node)
    {
        if (!node) return;

        if (const auto func = dynamic_cast<const Aleng::FunctionDefinitionNode *>(node))
        {
            if (func->FunctionName)
            {
                m_CurrentScope->Symbols.push_back({*func->FunctionName, "Function", func->Location.Line});
            }

            auto funcScope = std::make_shared<ScopeRange>();
            funcScope->StartLine = func->Location.Line;
            funcScope->Parent = m_CurrentScope;
            m_CurrentScope->Children.push_back(funcScope);

            m_CurrentScope = funcScope;

            for (const auto &param: func->Parameters)
            {
                m_CurrentScope->Symbols.push_back({param.Name, "Parameter", func->Location.Line});
            }

            VisitNode(func->Body.get());

            if (auto bodyBlock = dynamic_cast<const Aleng::BlockNode *>(func->Body.get()))
            {
            }

            m_CurrentScope = funcScope->Parent;
        } else if (const auto assign = dynamic_cast<const Aleng::AssignExpressionNode *>(node))
        {
            if (const auto id = dynamic_cast<const Aleng::IdentifierNode *>(assign->Left.get()))
            {
                bool exists = false;
                for (const auto &s: m_CurrentScope->Symbols) if (s.Name == id->Value) exists = true;

                if (!exists)
                {
                    m_CurrentScope->Symbols.push_back({id->Value, "Variable", id->Location.Line});
                }
            }
            VisitNode(assign->Right.get());
        } else if (const auto block = dynamic_cast<const Aleng::BlockNode *>(node))
        {
            VisitBlock(block);
        } else if (const auto ifNode = dynamic_cast<const Aleng::IfNode *>(node))
        {
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
                m_CurrentScope->Symbols.push_back({
                    forNode->NumericLoopInfo->IteratorVariableName, "Variable", forNode->Location.Line
                });
            else if (forNode->CollectionLoopInfo)
                m_CurrentScope->Symbols.push_back({
                    forNode->CollectionLoopInfo->IteratorVariableName, "Variable", forNode->Location.Line
                });

            VisitNode(forNode->Body.get());
            m_CurrentScope = loopScope->Parent;
        }
    }

    void Analyzer::VisitBlock(const Aleng::BlockNode *node)
    {
        for (const auto& stmt : node->Statements) {
            VisitNode(stmt.get());
        }
    }
} // AlengLSP
