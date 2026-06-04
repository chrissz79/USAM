# Start Vite dev server and then Electron (Windows PowerShell)
Start-Process -NoNewWindow -FilePath "npm" -ArgumentList "run","dev"
Start-Sleep -Seconds 2
Start-Process -NoNewWindow -FilePath "npm" -ArgumentList "run","start"
