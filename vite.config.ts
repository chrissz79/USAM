import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import path from 'path';

export default defineConfig({
  root: 'ui',
  plugins: [react()],
  resolve: {
    alias: {
      shared: path.resolve(__dirname, 'shared')
    }
  },
  server: {
    port: 5173
  },
  build: {
    outDir: path.resolve(__dirname, 'dist/renderer'),
    emptyOutDir: true
  }
});
