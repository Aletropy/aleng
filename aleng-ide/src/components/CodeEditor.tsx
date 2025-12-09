import { useCallback, useEffect, useRef } from 'react';
import Editor, { type BeforeMount, type Monaco, type OnMount } from '@monaco-editor/react';
import { useAlengEngine } from '../store/useAlengEngine';

const BASIC_SYNTAX = {
    tokenizer: {
        root: [
            [/\b(If|Else|While|For|Fn|Return|Break|Continue|Module|Import|End)\b/, "keyword"],
            [/\b(True|False)\b/, "keyword.constant"],
            [/"/, { token: "string.quote", bracket: "@open", next: "@string" }],

            [/[{}()\[\]]/, "delimiter.bracket"],
            [/[.,;]/, "delimiter"],
            [/[+\-*/%^=<>!&|]+/, "operator"],

            [/#.*$/, "comment"],
        ],
        string: [
            [/[^\\"]+/, "string"],
            [/"/, { token: "string.quote", bracket: "@close", next: "@pop" }]
        ]
    }
};

const SEMANTIC_LEGEND = {
    tokenTypes: [
        'variable', 'function', 'parameter', 'property', 'class',
        'string', 'number', 'keyword', 'operator'
    ],
    tokenModifiers: []
};

interface Props {
    initialCode: string;
    onChange: (val: string) => void;
}

export const CodeEditor = ({ initialCode, onChange }: Props) => {
    const { isReady, lint, complete, getHover, getSemanticTokens, run } = useAlengEngine();

    const monacoRef = useRef<Monaco | null>(null);
    const editorRef = useRef<any>(null);

    const handleBeforeMount: BeforeMount = (monaco) => {
        monaco.languages.register({ id: 'aleng' });

        monaco.languages.setMonarchTokensProvider('aleng', BASIC_SYNTAX as any);

        monaco.editor.defineTheme('aleng-dark', {
            base: 'vs-dark',
            inherit: true,
            rules: [
                { token: 'keyword', foreground: 'C678DD', fontStyle: 'bold' },
                { token: 'variable', foreground: 'E06C75' },
                { token: 'function', foreground: '61AFEF' },
                { token: 'parameter', foreground: 'D19A66', fontStyle: 'italic' },
                { token: 'operator', foreground: 'C678DD' },
                { token: 'string', foreground: '98C379' },
                { token: 'number', foreground: 'D19A66' },
                { token: 'comment', foreground: '5C6370' },
                { token: 'delimiter', foreground: 'ABB2BF' },
                { token: 'delimiter.bracket', foreground: 'E5C07B' },
                { token: 'identifier', foreground: 'ABB2BF' },
            ],
            colors: {
                'editor.background': '#09090b',
                'editor.lineHighlightBackground': '#18181b',
                'editorCursor.foreground': '#528bff',
            }
        });

        monaco.languages.registerCompletionItemProvider('aleng', {
            provideCompletionItems: (model : any, position : any) => {
                const code = model.getValue();
                const suggestions = complete(code, position.lineNumber, position.column);

                return {
                    suggestions: suggestions.map((s: any) => ({
                        label: s.label,
                        kind: s.kind,
                        insertText: s.insertText,
                        insertTextRules: s.insertTextRules,
                        detail: s.detail,
                        documentation: s.documentation
                    }))
                };
            },
            triggerCharacters: ['.']
        });

        monaco.languages.registerHoverProvider('aleng', {
            provideHover: (model : any, position : any) => {
                const code = model.getValue();
                const hoverText = getHover(code, position.lineNumber, position.column);

                if (!hoverText) return null;

                return {
                    contents: [
                        { value: hoverText }
                    ]
                };
            }
        });

        monaco.languages.registerDocumentSemanticTokensProvider('aleng', {
            getLegend: () => SEMANTIC_LEGEND,
            provideDocumentSemanticTokens: (model : any) => {
                const code = model.getValue();
                lint(code);

                const data = getSemanticTokens(code);

                return {
                    data: new Uint32Array(data),
                    resultId: undefined
                };
            },
            releaseDocumentSemanticTokens: () => {}
        });
    };

    const handleEditorDidMount: OnMount = (editor, monaco) => {
        editorRef.current = editor;
        monacoRef.current = monaco;

        editor.addCommand(monaco.KeyCode.F5, () => {
            if (isReady) run();
        });

        if(isReady) validateCode(initialCode);
    };

    const validateCode = useCallback((codeToValidate: string) => {
        if (!monacoRef.current || !editorRef.current || !isReady) return;

        const errors = lint(codeToValidate);
        const model = editorRef.current.getModel();

        if (model) {
            monacoRef.current.editor.setModelMarkers(model, 'owner', errors.map((err: any) => ({
                startLineNumber: err.line,
                startColumn: err.col,
                endLineNumber: err.line,
                endColumn: err.col + (err.length || 1),
                message: err.message,
                severity: err.severity === 'Error'
                    ? monacoRef.current!.MarkerSeverity.Error
                    : monacoRef.current!.MarkerSeverity.Warning
            })));
        }
    }, [isReady, lint]);

    useEffect(() => {
        if (isReady && editorRef.current) {
            validateCode(editorRef.current.getValue());
        }
    }, [isReady, validateCode]);

    return (
        <Editor
            height="100%"
            defaultLanguage="aleng"
            theme="aleng-dark"
            beforeMount={handleBeforeMount}
            onMount={handleEditorDidMount}
            value={initialCode}
            onChange={(val) => {
                const code = val || '';
                onChange(code);
                validateCode(code);
            }}
            options={{
                minimap: { enabled: false },
                fontSize: 14,
                fontFamily: 'var(--font-mono)',
                padding: { top: 16, bottom: 16 },
                scrollBeyondLastLine: false,
                automaticLayout: true,
                contextmenu: true,
                'semanticHighlighting.enabled': true
            }}
        />
    );
};