import { contextBridge, ipcRenderer } from 'electron';
import { IPC_CHANNELS } from 'shared/constants';

contextBridge.exposeInMainWorld('usam', {
  search: (query: string, filters?: Record<string, any>) => ipcRenderer.invoke(IPC_CHANNELS.SEARCH, { query, filters }),
  getPlugins: () => ipcRenderer.invoke(IPC_CHANNELS.GET_PLUGINS),
  getTags: () => ipcRenderer.invoke(IPC_CHANNELS.GET_TAGS),
  assignTag: (pluginId: number, tagId: number) => ipcRenderer.invoke(IPC_CHANNELS.ASSIGN_TAG, { pluginId, tagId })
});
