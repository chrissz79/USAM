import Database from 'better-sqlite3';
import fs from 'fs';
import path from 'path';
import { DB_FILE } from 'shared/constants';

const dbPath = path.resolve(process.cwd(), 'database', DB_FILE);
const dir = path.dirname(dbPath);
if (!fs.existsSync(dir)) fs.mkdirSync(dir, { recursive: true });

export const db = new Database(dbPath);

// For scaffold: ensure foreign keys
try {
  db.pragma('foreign_keys = ON');
} catch (e) {
  // ignore
}

export default db;
