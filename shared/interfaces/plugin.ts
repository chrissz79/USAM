export interface PluginVendor {
  id: number;
  name: string;
  website?: string | null;
  createdAt: string;
}

export interface PluginCategory {
  id: number;
  name: string;
  description?: string | null;
  createdAt: string;
}

export interface PluginRecord {
  id: number;
  name: string;
  vendorId?: number | null;
  categoryId?: number | null;
  version?: string | null;
  pluginType?: string | null;
  filePath: string;
  scanSessionId?: number | null;
  createdAt: string;
  updatedAt: string;
}
