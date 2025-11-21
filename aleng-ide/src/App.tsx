import { useEffect, useState } from 'react';
import { Panel, PanelGroup, PanelResizeHandle } from 'react-resizable-panels';
import {
    Play, Terminal, Trash2, FileCode2, Loader2,
    X, FilePlus, GripVertical
} from 'lucide-react';

import { useAlengEngine } from './store/useAlengEngine';
import { useFileStore } from './store/useFileStore';
import { CodeEditor } from './components/CodeEditor';

const ResizeHandle = ({ className = "" }: { className?: string }) => (
    <PanelResizeHandle className={`w-1.5 bg-editor-border hover:bg-purple-500/50 transition-colors flex items-center justify-center group ${className}`}>
        <GripVertical className="h-4 w-4 text-gray-600 group-hover:text-white opacity-50" />
    </PanelResizeHandle>
);

function App() {
    const { init, run, isReady, output, clearOutput, isCompiling } = useAlengEngine();

    const {
        files, activeFileId, getActiveFile,
        addFile, removeFile, selectFile, updateFileContent
    } = useFileStore();

    const [isCreatingFile, setIsCreatingFile] = useState(false);
    const [newFileName, setNewFileName] = useState("");

    useEffect(() => {
        init();
    }, []);

    const activeFile = getActiveFile();

    const handleCreateFile = (e: React.FormEvent) => {
        e.preventDefault();
        if (newFileName.trim()) {
            addFile(newFileName.trim());
            setNewFileName("");
            setIsCreatingFile(false);
        }
    };

    useEffect(() => {
        const handleKeyDown = (e: KeyboardEvent) => {
            if ((e.ctrlKey || e.metaKey) && e.key === 'Enter') {
                e.preventDefault();

                if (isReady && !isCompiling) {
                    run();
                }
            }
        };

        window.addEventListener('keydown', handleKeyDown);

        return () => window.removeEventListener('keydown', handleKeyDown);
    }, [isReady, isCompiling, run]);

    return (
        <div className="flex h-screen w-screen flex-col bg-editor-bg text-editor-fg font-sans overflow-hidden">

            {/* --- HEADER --- */}
            <header className="flex h-14 shrink-0 items-center justify-between border-b border-editor-border bg-editor-sidebar px-4 z-10">
                <div className="flex items-center gap-3">
                    <div className="flex h-8 w-8 items-center justify-center rounded bg-purple-600 font-bold text-white select-none">
                        AL
                    </div>
                    <h1 className="font-semibold tracking-wide text-gray-200 hidden sm:block">Aleng Studio</h1>
                </div>

                <div className="flex items-center gap-3">
                    <div className="hidden md:flex items-center gap-2 text-xs px-3 py-1 rounded-full border border-editor-border bg-white/5">
                        <div className={`w-2 h-2 rounded-full ${isReady ? 'bg-green-500' : 'bg-yellow-500 animate-pulse'}`}></div>
                        {isReady ? 'Engine Ready' : 'Loading Wasm...'}
                    </div>

                    <button
                        onClick={run}
                        disabled={!isReady || isCompiling}
                        title="Shortcut: Ctrl + Enter"
                        className="flex items-center gap-2 rounded bg-green-600 px-4 py-1.5 text-sm font-medium text-white hover:bg-green-700 disabled:opacity-50 disabled:cursor-not-allowed transition-colors shadow-lg shadow-green-900/20"
                    >
                        {isCompiling ? <Loader2 className="h-4 w-4 animate-spin"/> : <Play className="h-4 w-4 fill-current" />}
                        Run Project
                    </button>
                </div>
            </header>

            {/* --- RESIZABLE LAYOUT --- */}
            <div className="flex-1 overflow-hidden">
                <PanelGroup direction="horizontal">

                    {/* SIDEBAR (FILE EXPLORER) */}
                    <Panel defaultSize={20} minSize={10} maxSize={30} className="bg-editor-sidebar flex flex-col border-r border-editor-border">
                        <div className="p-3 flex items-center justify-between text-xs font-bold uppercase text-gray-500 tracking-wider">
                            <span>Explorer</span>
                            <button
                                onClick={() => setIsCreatingFile(true)}
                                className="p-1 hover:bg-white/10 rounded text-gray-400 hover:text-white transition"
                                title="New File"
                            >
                                <FilePlus className="h-4 w-4" />
                            </button>
                        </div>

                        {/* File List */}
                        <div className="flex-1 overflow-y-auto px-2 space-y-0.5">
                            {files.map(file => (
                                <div
                                    key={file.id}
                                    onClick={() => selectFile(file.id)}
                                    className={`group flex items-center justify-between w-full gap-2 rounded px-3 py-2 text-sm cursor-pointer transition-all border border-transparent ${
                                        activeFileId === file.id
                                            ? 'bg-white/10 text-white border-white/5'
                                            : 'text-gray-400 hover:bg-white/5 hover:text-gray-200'
                                    }`}
                                >
                                    <div className="flex items-center gap-2 truncate">
                                        <FileCode2 className={`h-4 w-4 ${file.isMain ? 'text-purple-400' : 'text-blue-400'}`} />
                                        <span className="truncate">{file.name}</span>
                                    </div>

                                    {!file.isMain && (
                                        <button
                                            onClick={(e) => { e.stopPropagation(); removeFile(file.id); }}
                                            className="opacity-0 group-hover:opacity-100 p-1 hover:bg-red-500/20 hover:text-red-400 rounded"
                                        >
                                            <X className="h-3 w-3" />
                                        </button>
                                    )}
                                </div>
                            ))}

                            {/* Inline New File Input */}
                            {isCreatingFile && (
                                <form onSubmit={handleCreateFile} className="px-1 mt-1">
                                    <input
                                        autoFocus
                                        type="text"
                                        className="w-full bg-editor-bg border border-blue-500 rounded px-2 py-1 text-sm text-white outline-none"
                                        placeholder="filename..."
                                        value={newFileName}
                                        onBlur={() => setIsCreatingFile(false)}
                                        onChange={e => setNewFileName(e.target.value)}
                                    />
                                </form>
                            )}
                        </div>
                    </Panel>

                    <ResizeHandle />

                    {/* MAIN CONTENT (EDITOR + TERMINAL) */}
                    <Panel>
                        <PanelGroup direction="vertical">

                            {/* EDITOR */}
                            <Panel defaultSize={70} minSize={30}>
                                <div className="h-full w-full relative">
                                    {activeFile ? (
                                        <CodeEditor
                                            initialCode={activeFile.content}
                                            onChange={(val) => updateFileContent(activeFile.id, val)}
                                        />
                                    ) : (
                                        <div className="flex h-full items-center justify-center text-gray-500">
                                            Select a file to edit
                                        </div>
                                    )}
                                </div>
                            </Panel>

                            <PanelResizeHandle className="h-1.5 bg-editor-border hover:bg-purple-500/50 transition-colors flex items-center justify-center cursor-row-resize">
                                <div className="w-8 h-1 bg-gray-600 rounded-full opacity-50" />
                            </PanelResizeHandle>

                            {/* TERMINAL */}
                            <Panel defaultSize={30} minSize={10} className="bg-editor-bg flex flex-col">
                                <div className="flex h-9 shrink-0 items-center justify-between border-b border-editor-border bg-editor-sidebar px-4 select-none">
                                    <div className="flex items-center gap-2 text-xs font-medium text-gray-400 uppercase tracking-wider">
                                        <Terminal className="h-4 w-4" />
                                        Output Console
                                    </div>
                                    <button onClick={clearOutput} className="text-gray-500 hover:text-gray-300 p-1 rounded hover:bg-white/10 transition">
                                        <Trash2 className="h-4 w-4" />
                                    </button>
                                </div>

                                <div className="flex-1 overflow-auto p-4 font-mono text-sm">
                                    {output.length === 0 && (
                                        <div className="text-gray-600 italic opacity-50">Run the project to see output...</div>
                                    )}
                                    {output.map((line, i) => (
                                        <div key={i} className={`${
                                            line.startsWith('Error:') || line.startsWith('Fatal:') || line.includes('Runtime Error')
                                                ? 'text-red-400 bg-red-500/5'
                                                : 'text-gray-300'
                                        } border-b border-white/5 last:border-0 break-words whitespace-pre-wrap font-fira`}
                                        >
                                            {line}
                                        </div>
                                    ))}
                                </div>
                            </Panel>

                        </PanelGroup>
                    </Panel>

                </PanelGroup>
            </div>
        </div>
    );
}

export default App;