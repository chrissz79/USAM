# USAM Database Schema

Deze databasebeschrijving beschrijft het SQLite-datamodel voor USAM MVP v0.1, gericht op VST-pluginbeheer, tagging en zoekoptimalisatie.

## Tabellen

### plugin
- `id` INTEGER PRIMARY KEY AUTOINCREMENT
- `name` TEXT NOT NULL
- `vendor_id` INTEGER REFERENCES plugin_vendor(id)
- `category_id` INTEGER REFERENCES plugin_category(id)
- `version` TEXT
- `plugin_type` TEXT
- `file_path` TEXT NOT NULL UNIQUE
- `scan_session_id` INTEGER REFERENCES scan_session(id)
- `created_at` DATETIME DEFAULT CURRENT_TIMESTAMP
- `updated_at` DATETIME DEFAULT CURRENT_TIMESTAMP

Beschrijft elke gedetecteerde plugin met basismetadata en verwijzingen naar vendor, categorie en de scan waarin de plugin is geïnventariseerd.

### plugin_vendor
- `id` INTEGER PRIMARY KEY AUTOINCREMENT
- `name` TEXT NOT NULL UNIQUE
- `website` TEXT
- `created_at` DATETIME DEFAULT CURRENT_TIMESTAMP

Stelt pluginfabrikanten voor en faciliteert consistente vendor-lookup en filtering.

### plugin_category
- `id` INTEGER PRIMARY KEY AUTOINCREMENT
- `name` TEXT NOT NULL UNIQUE
- `description` TEXT
- `created_at` DATETIME DEFAULT CURRENT_TIMESTAMP

Bevat categorielabels voor plugins, zoals synth, effect, utility of instrument.

### tag
- `id` INTEGER PRIMARY KEY AUTOINCREMENT
- `name` TEXT NOT NULL UNIQUE
- `description` TEXT
- `created_at` DATETIME DEFAULT CURRENT_TIMESTAMP

Definieert gebruikerslabels die aan plugins gekoppeld kunnen worden voor flexibele organisatie.

### plugin_tag
- `plugin_id` INTEGER NOT NULL REFERENCES plugin(id)
- `tag_id` INTEGER NOT NULL REFERENCES tag(id)
- `assigned_at` DATETIME DEFAULT CURRENT_TIMESTAMP
- PRIMARY KEY (`plugin_id`, `tag_id`)

Legt de veel-op-veel relatie vast tussen plugins en tags.

### scan_session
- `id` INTEGER PRIMARY KEY AUTOINCREMENT
- `started_at` DATETIME DEFAULT CURRENT_TIMESTAMP
- `completed_at` DATETIME
- `status` TEXT NOT NULL
- `summary` TEXT
- `plugin_count` INTEGER DEFAULT 0
- `created_at` DATETIME DEFAULT CURRENT_TIMESTAMP

Bewaart informatie over scanvondsten, de status van een beurt en het aantal gevonden plugins.

### search_document
- `rowid` INTEGER PRIMARY KEY
- `plugin_id` INTEGER NOT NULL REFERENCES plugin(id)
- `content` TEXT NOT NULL

Deze tabel wordt gebruikt voor SQLite full-text search (FTS). `content` bevat samengestelde metadata en tagwaarden per plugin.

## Relaties
- `plugin.vendor_id` verwijst naar `plugin_vendor.id`.
- `plugin.category_id` verwijst naar `plugin_category.id`.
- `plugin.scan_session_id` verwijst naar `scan_session.id`.
- `plugin_tag.plugin_id` verwijst naar `plugin.id` en `plugin_tag.tag_id` verwijst naar `tag.id`.
- `search_document.plugin_id` verwijst naar `plugin.id`.

## Indexen
- `idx_plugin_file_path` op `plugin(file_path)`
  - Ondersteunt snelle detectie van dubbele plugins en zoekopdrachten op bestandspad.
- `idx_plugin_vendor_id` op `plugin(vendor_id)`
  - Versnelt queries voor vendor-gebaseerde filtering.
- `idx_plugin_category_id` op `plugin(category_id)`
  - Versnelt category filtering en analyse.
- `idx_plugin_scan_session_id` op `plugin(scan_session_id)`
  - Versnelt scanspecifieke queries en rapportage.
- `idx_plugin_tag_tag_id` op `plugin_tag(tag_id)`
  - Ondersteunt snelle taggerelateerde zoekacties.
- `idx_tag_name` op `tag(name)`
  - Verbetert snelle lookup bij tagselectie.
- `idx_search_document_plugin_id` op `search_document(plugin_id)`
  - Houdt de mapping tussen zoekdocumenten en plugins efficiënt.

## Full-Text Search Strategie
Voor MVP wordt een SQLite FTS-tabel gebruikt op `search_document.content`.
- `content` bevat gecombineerde waarden zoals pluginnaam, vendornaam, categorie, type en geassocieerde tags.
- Search queries lopen via FTS-match op deze samengestelde tekst.
- Bij updates aan plugins of tags wordt het corresponderende `search_document` record vernieuwd.

## Ontwerpnotities
- Het schema is genormaliseerd voor plugin metadata, vendor- en categoriegegevens.
- `plugin_tag` maakt dynamische, veel-op-veel tagging mogelijk zonder redundante data.
- `scan_session` zorgt voor traceerbaarheid van scanoperaties.
- Indexen zijn gekozen om de meest voorkomende filter- en zoekquery's te versnellen.
