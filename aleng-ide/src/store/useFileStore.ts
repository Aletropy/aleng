import { create } from 'zustand';

export interface FileNode {
    id: string;
    name: string;
    content: string;
    isMain: boolean;
}

interface FileState {
    files: FileNode[];
    activeFileId: string;

    addFile: (name: string) => void;
    removeFile: (id: string) => void;
    selectFile: (id: string) => void;
    updateFileContent: (id: string, content: string) => void;
    getActiveFile: () => FileNode | undefined;
}

const INITIAL_CODE = `# Bem-vindo ao Aleng IDE! 🚀
# Pressione "Run" para executar.

# 1. Exemplo de Função Recursiva
Fn fibonacci(n)
    If n <= 1
        Return n
    End
    Return fibonacci(n - 1) + fibonacci(n - 2)
End

# 2. Exemplo de Estruturas de Dados (Mapas e Listas)
config = {
    "nome": "Aleng Studio",
    "versao": 1.0,
    "modulos": ["Core", "Wasm", "LSP"]
}

# 3. Fluxo de Execução Principal
Print("--- Inicializando " + config.nome + " v" + config.versao + " ---")

Print("Carregando módulos:")
For mod in config.modulos
    Print(" 🟩 [OK] Carregado: " + mod)
End

Print("Calculando sequência de Fibonacci:")
For i = 0 .. 6
    val = fibonacci(i)
    Print(" 🔹 Fib(" + i + ") = " + val)
End

Print("--- Execução Finalizada com Sucesso ---")
`;

export const useFileStore = create<FileState>((set, get) => ({
    files: [
        { id: '1', name: 'main.aleng', content: INITIAL_CODE, isMain: true }
    ],
    activeFileId: '1',

    addFile: (name) => {
        const fileName = name.endsWith('.aleng') ? name : `${name}.aleng`;

        if (get().files.some(f => f.name === fileName)) return;

        const newFile = {
            id: crypto.randomUUID(),
            name: fileName,
            content: `# ${fileName}\n\n`,
            isMain: false
        };

        set(state => ({
            files: [...state.files, newFile],
            activeFileId: newFile.id
        }));
    },

    removeFile: (id) => {
        set(state => {
            const fileToRemove = state.files.find(f => f.id === id);
            if (fileToRemove?.isMain) return state; // Protege o main

            const newFiles = state.files.filter(f => f.id !== id);
            const newActiveId = state.activeFileId === id ? '1' : state.activeFileId;

            return { files: newFiles, activeFileId: newActiveId };
        });
    },

    selectFile: (id) => set({ activeFileId: id }),

    updateFileContent: (id, content) => set(state => ({
        files: state.files.map(f => f.id === id ? { ...f, content } : f)
    })),

    getActiveFile: () => {
        const { files, activeFileId } = get();
        return files.find(f => f.id === activeFileId);
    }
}));