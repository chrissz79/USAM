PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS plugin (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  name TEXT NOT NULL,
  vendor_id INTEGER,
  category_id INTEGER,
  version TEXT,
  plugin_type TEXT,
  file_path TEXT NOT NULL UNIQUE,
  scan_session_id INTEGER,
  created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
  updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
  FOREIGN KEY (vendor_id) REFERENCES plugin_vendor(id),
  FOREIGN KEY (category_id) REFERENCES plugin_category(id),
  FOREIGN KEY (scan_session_id) REFERENCES scan_session(id)
);

CREATE TABLE IF NOT EXISTS plugin_vendor (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  name TEXT NOT NULL UNIQUE,
  website TEXT,
  created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS plugin_category (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  name TEXT NOT NULL UNIQUE,
  description TEXT,
  created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS tag (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  name TEXT NOT NULL UNIQUE,
  description TEXT,
  created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS plugin_tag (
  plugin_id INTEGER NOT NULL,
  tag_id INTEGER NOT NULL,
  assigned_at DATETIME DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (plugin_id, tag_id),
  FOREIGN KEY (plugin_id) REFERENCES plugin(id),
  FOREIGN KEY (tag_id) REFERENCES tag(id)
);

CREATE TABLE IF NOT EXISTS scan_session (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  started_at DATETIME DEFAULT CURRENT_TIMESTAMP,
  completed_at DATETIME,
  status TEXT NOT NULL,
  summary TEXT,
  plugin_count INTEGER DEFAULT 0,
  created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS search_document (
  plugin_id INTEGER PRIMARY KEY,
  content TEXT NOT NULL,
  FOREIGN KEY (plugin_id) REFERENCES plugin(id)
);

-- FTS index (scaffold)
CREATE VIRTUAL TABLE IF NOT EXISTS search_document_fts USING fts5(content);

-- Indexes
CREATE INDEX IF NOT EXISTS idx_plugin_file_path ON plugin(file_path);
CREATE INDEX IF NOT EXISTS idx_plugin_vendor_id ON plugin(vendor_id);
CREATE INDEX IF NOT EXISTS idx_plugin_category_id ON plugin(category_id);
CREATE INDEX IF NOT EXISTS idx_plugin_scan_session_id ON plugin(scan_session_id);
CREATE INDEX IF NOT EXISTS idx_plugin_tag_tag_id ON plugin_tag(tag_id);
CREATE INDEX IF NOT EXISTS idx_tag_name ON tag(name);
CREATE INDEX IF NOT EXISTS idx_search_document_plugin_id ON search_document(plugin_id);
