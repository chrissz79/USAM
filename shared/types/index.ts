export type ISODateString = string;

export interface Plugin {
  id: number;
  name: string;
  vendorId?: number | null;
  categoryId?: number | null;
  version?: string | null;
  pluginType?: string | null;
  filePath: string;
  scanSessionId?: number | null;
  createdAt: ISODateString;
  updatedAt: ISODateString;
}

export interface Tag {
  id: number;
  name: string;
  description?: string | null;
  createdAt: ISODateString;
}

export interface ScanSession {
  id: number;
  startedAt: ISODateString;
  completedAt?: ISODateString | null;
  status: string;
  summary?: string | null;
  pluginCount: number;
}

export interface SearchResult {
  plugin: Plugin;
  score?: number;
}
