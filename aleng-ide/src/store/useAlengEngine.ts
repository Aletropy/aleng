import { create } from "zustand"
import { useFileStore } from './useFileStore';

interface AlengInstance
{
    execute : (code : string) => string;
    lint : (code : string) => string;
    getSemanticTokens : () => string;
    getHover: (code: string, line: number, col: number) => string;
    complete: (code: string, line: number, col: number) => string;
    delete : () => void;
}

interface EditorState {
    isReady: boolean;
    output: string[];
    isCompiling: boolean;

    init: () => Promise<void>;
    run: () => void;
    lint: (code: string) => any[];
    complete: (code: string, line: number, col: number) => any[];
    getHover: (code: string, line: number, col: number) => string;
    getSemanticTokens: (code: string) => number[];
    clearOutput: () => void;
}

export const useAlengEngine = create<EditorState>((set, get) => ({
    isReady: false,
    output: [],
    isCompiling: false,
    instance: null as AlengInstance | null,
    fsModule: null as any,

    init: async () => {
        if (get().isReady) return;

        try {
            // @ts-ignore - createAlengModule
            const moduleFactory = window.createAlengModule;

            if (!moduleFactory) {
                console.error("AlengWasm.js not loaded properly via index.html");
                return;
            }

            const module = await moduleFactory({
                print: (text: string) => set(s => ({ output: [...s.output, text] })),
                printErr: (text: string) => set(s => ({ output: [...s.output, `Error: ${text}`] })),
            });

            try {
                module.FS.mkdir('/virtual_fs');
            } catch (e) {}

            const instance = new module.Aleng();

            (get() as any).instance = instance;
            (get() as any).fsModule = module;

            set({ isReady: true });
            console.log("Aleng VM Ready");
        } catch (e) {
            console.error("Failed to load WASM", e);
            set({ output: ["Fatal: Could not load Aleng Engine."] });
        }
    },

    run: () => {
        const instance = (get() as any).instance as AlengInstance;
        const fs = (get() as any).fsModule?.FS;
        if (!instance || !fs) return;

        const { files } = useFileStore.getState();
        const mainFile = files.find(f => f.isMain)

        if(!mainFile) return;

        set({ isCompiling: true, output: [] });

        setTimeout(() => {
            try {
                files.forEach(file => {
                    try {
                        fs.writeFile(`/virtual_fs/${file.name}`, file.content)
                    } catch (err) {}
                })

                const result = instance.execute(mainFile.content);

                if(result) {
                    set(s => ({ output: [...s.output, result] }));
                }
            } catch (e) {
                set(s => ({ output: [...s.output, `JS/Wasm Error: ${e}`] }));
            } finally {
                set({ isCompiling: false });
            }
        }, 10);
    },

    lint: (code: string) => {
        const instance = (get() as any).instance as AlengInstance;
        if (!instance) return [];
        try {
            return JSON.parse(instance.lint(code));
        } catch (e) { return []; }
    },

    complete: (code: string, line: number, col: number) => {
        const instance = (get() as any).instance as AlengInstance;
        if (!instance) return [];
        try {
            return JSON.parse(instance.complete(code, line, col));
        } catch (e) { return []; }
    },

    getHover: (code: string, line: number, col: number) => {
        const instance = (get() as any).instance as AlengInstance;
        if (!instance) return "";
        try {
            return instance.getHover(code, line, col);
        } catch (e) { return ""; }
    },

    getSemanticTokens: (_code: string) => {
        const instance = (get() as any).instance as AlengInstance;
        if (!instance) return [];
        try {
            const raw = instance.getSemanticTokens();
            return JSON.parse(raw);
        } catch (e) { return []; }
    },

    clearOutput: () => set({ output: [] })
}));