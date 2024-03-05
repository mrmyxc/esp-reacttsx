import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [react()],
  build: {
    rollupOptions: {
      output: {
        // Shorten filenames
        assetFileNames: "a/[hash:6][extname]",
        chunkFileNames: "c/[hash:6].js",
        entryFileNames: "e/[hash:6].js"
      }
    }
  }
})
