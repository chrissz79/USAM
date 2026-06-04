export {};

declare global {
  interface Window {
    usam?: {
      search?: (query: string, filters?: Record<string, any>) => Promise<any>;
      getPlugins?: () => Promise<any[]>;
      getTags?: () => Promise<any[]>;
      assignTag?: (pluginId: number, tagId: number) => Promise<any>;
    };
  }
}
