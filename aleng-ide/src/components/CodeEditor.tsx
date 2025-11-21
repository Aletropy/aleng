import {useCallback, useEffect, useRef} from 'react';
import Editor, {type BeforeMount, type Monaco, type OnMount} from '@monaco-editor/react';
import {useAlengEngine} from '../store/useAlengEngine';

const ALENG_SYNTAX = {
    tokenizer: {
        root: [
            [/\b(If|Else|While|For|Fn|Return|Break|Continue|Module|Import|End|in|step|until)\b/, "keyword"],
            [/\b(True|False)\b/, "keyword.constant"],
            [/\b(Print|Append|Len|Pop)\b/, "support.function"],
            [/[a-zA-Z_]\w*/, "identifier"],
            [/"([^"\\]|\\.)*$/, "string.invalid"],
            [/"/, { token: "string.quote", bracket: "@open", next: "@string" }],
            [/\d+(\.\d+)?/, "number"],
            [/#.*$/, "comment"],
        ],
        string: [
            [/[^\\"]+/, "string"],
            [/\\./, "string.escape"],
            [/"/, { token: "string.quote", bracket: "@close", next: "@pop" }]
        ]
    }
};

interface Props {
    initialCode: string;
    onChange: (val: string) => void;
}

export const CodeEditor = ({ initialCode, onChange }: Props) => {
    const { isReady, lint, complete, run } = useAlengEngine();

    const monacoRef = useRef<Monaco | null>(null);
    const editorRef = useRef<any>(null);

    const handleBeforeMount: BeforeMount = (monaco) => {
        monaco.languages.register({ id: 'aleng' });
        monaco.languages.setMonarchTokensProvider('aleng', ALENG_SYNTAX as any);

        monaco.editor.defineTheme('aleng-dark', {
            base: 'vs-dark',
            inherit: true,
            rules: [
                { token: 'keyword', foreground: 'C678DD', fontStyle: 'bold' },
                { token: 'keyword.constant', foreground: 'D19A66' },
                { token: 'support.function', foreground: '61AFEF' },
                { token: 'identifier', foreground: 'E5C07B' },
                { token: 'string', foreground: '98C379' },
                { token: 'number', foreground: 'D19A66' },
                { token: 'comment', foreground: '5C6370', fontStyle: 'italic' },
            ],
            colors: {
                'editor.background': '#09090b',
                'editor.lineHighlightBackground': '#18181b',
                'editorCursor.foreground': '#528bff',
            }
        });

        monaco.languages.registerCompletionItemProvider('aleng', {
            provideCompletionItems: (model: { getValue: () => any; }, position: { lineNumber: number; }) => {
                const code = model.getValue();
                return complete(code, position.lineNumber);
            }
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
            const markers = errors.map((err: any) => ({
                startLineNumber: err.line,
                startColumn: err.col,
                endLineNumber: err.line,
                endColumn: err.col + (err.length || 5),
                message: err.message,
                severity: monacoRef.current!.MarkerSeverity.Error
            }));
            monacoRef.current!.editor.setModelMarkers(model, 'owner', markers);
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
                contextmenu: false,
            }}
        />
    );
};