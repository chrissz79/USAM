import db from './db';

export function getPlugins(limit = 200) {
  const stmt = db.prepare('SELECT * FROM plugin ORDER BY name LIMIT ?');
  return stmt.all(limit);
}

export default { getPlugins };
