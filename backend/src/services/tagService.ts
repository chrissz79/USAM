import db from './db';

export function getTags() {
  const stmt = db.prepare('SELECT * FROM tag ORDER BY name');
  return stmt.all();
}

export function createTag(name: string, description?: string) {
  const insert = db.prepare('INSERT INTO tag (name, description) VALUES (?, ?)');
  const info = insert.run(name, description || null);
  return { id: info.lastInsertRowid };
}

export default { getTags, createTag };
