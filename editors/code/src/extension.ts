import {
    ExtensionContext,
    workspace
} from "vscode"

import {
    LanguageClient,
    LanguageClientOptions,
    ServerOptions,
    TransportKind
} from "vscode-languageclient/node"

let client : LanguageClient

export function activate(context : ExtensionContext)
{
    const serverExecutable = '/home/gabriel/Projects/aleng/build/AlengLSP'

    const serverOptions : ServerOptions = {
        run: { command: serverExecutable, transport: TransportKind.stdio },
        debug: { command: serverExecutable, transport: TransportKind.stdio }
    }

    const clientOptions : LanguageClientOptions = {
        documentSelector: [{scheme: 'file', language: 'aleng'}],
        synchronize: {
            fileEvents: workspace.createFileSystemWatcher('**/.clientrc')
        }
    }

    client = new LanguageClient('alengLsp', 'Aleng Language Server', serverOptions, clientOptions)

    client.start()
}

export function deactivate(): Thenable<void> | undefined {
  if (!client) {
    return undefined;
  }
  return client.stop();
}