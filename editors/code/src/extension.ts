import * as path from 'path';
import * as fs from 'fs';
import {
    workspace,
    ExtensionContext,
    window,
    commands
} from "vscode";

import {
    LanguageClient,
    LanguageClientOptions,
    ServerOptions,
    TransportKind
} from "vscode-languageclient/node";

let client: LanguageClient;

function resolveServerPath(context: ExtensionContext): string | undefined {
    // 1. Check Configuration
    const config = workspace.getConfiguration('aleng');
    const configPath = config.get<string>('serverPath');
    if (configPath && fs.existsSync(configPath)) {
        return configPath;
    }

    const devPath = '/home/gabriel/Projects/aleng/build/AlengLSP';
    if (fs.existsSync(devPath)) {
        return devPath;
    }

    const bundledPath = context.asAbsolutePath(path.join('bin', 'AlengLSP'));
    if (fs.existsSync(bundledPath)) {
        return bundledPath;
    }

    if (process.platform === 'win32') {
        const windowsPath = context.asAbsolutePath(path.join('bin', 'AlengLSP.exe'));
        if (fs.existsSync(windowsPath)) return windowsPath;
    }

    return undefined;
}

export async function activate(context: ExtensionContext) {
    console.log('Activating Aleng LSP Extension...');

    const serverExecutable = resolveServerPath(context);

    if (!serverExecutable) {
        window.showErrorMessage(
            "Aleng LSP server not found. Please configure 'aleng.serverPath' in settings or build the project at the default location."
        );
        return;
    }

    if (process.platform !== 'win32') {
        try {
            fs.accessSync(serverExecutable, fs.constants.X_OK);
        } catch (err) {
            window.showWarningMessage(`Aleng LSP found at ${serverExecutable} but might not be executable. Attempting to chmod +x...`);
            try {
                fs.chmodSync(serverExecutable, '755');
            } catch (chmodErr) {
                window.showErrorMessage(`Failed to make LSP executable: ${chmodErr}`);
                return;
            }
        }
    }

    const serverOptions: ServerOptions = {
        run: {
            command: serverExecutable,
            transport: TransportKind.stdio,
            args: []
        },
        debug: {
            command: serverExecutable,
            transport: TransportKind.stdio,
            args: []
        }
    };

    const clientOptions: LanguageClientOptions = {
        documentSelector: [{ scheme: 'file', language: 'aleng' }],
        synchronize: {
            fileEvents: workspace.createFileSystemWatcher('**/.alengrc')
        },
        outputChannelName: 'Aleng Language Server',
        traceOutputChannel: window.createOutputChannel("Aleng LSP Trace")
    };

    client = new LanguageClient(
        'alengLsp',
        'Aleng Language Server',
        serverOptions,
        clientOptions
    );

    await client.start();

    window.setStatusBarMessage("Aleng LSP: Active", 3000);
}

export function deactivate(): Thenable<void> | undefined {
    if (!client) {
        return undefined;
    }
    return client.stop();
}