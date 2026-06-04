import db from './db';

export function searchPlugins(query: string, filters?: Record<string, any>) {
  // Placeholder: use FTS virtual table when implemented
  const stmt = db.prepare("SELECT p.* FROM plugin p WHERE p.name LIKE '%' || ? || '%' LIMIT 100");
  const rows = stmt.all(query);
  return rows;
}

export default { searchPlugins };
