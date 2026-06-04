# USAM - Universal Sound Asset Manager

USAM is een modulair platform voor het beheren, doorzoeken en organiseren van sound assets in moderne muziekproductie.
Het richt zich op een database-gedreven desktopervaring voor producers, sound designers en studio-engineers die VST-plugins, samples, presets en sound libraries centraal willen beheren.

## Missie
USAM heeft als doel een future-proof asset management ecosysteem te bieden dat eenvoudig uitbreidbaar is naar meerdere formats, intelligente zoekfuncties en hardware-integratie.

## Wat maakt USAM uniek?
- **Modulair ontwerp:** elk onderdeel is losjes gekoppeld, zodat nieuwe assettypen en adapters later eenvoudig kunnen worden toegevoegd.
- **Database-gedreven workflow:** alle assetmetadata wordt centraal opgeslagen in SQLite voor offline betrouwbaarheid en snelle zoekprestaties.
- **Plugin-first MVP:** de eerste versie richt zich op VST-plugins met een solide scanner, browser en tag-based organisatie.
- **AI-ready architectuur:** het systeem is ontworpen om later AI-assisted search en metadata-verrijking te ondersteunen.

## Doelgroep
- Muziekproducenten
- Sound designers
- Studio-engineers
- Plugin-collectors
- Componisten met grote sample- en presetbibliotheken

## Kerncomponenten (MVP v0.1)
- **VST Scanner:** detecteert en inventariseert lokale VST-plugins.
- **SQLite Database:** slaat pluginmetadata, tags en scanresultaten op.
- **Plugin Browser:** toont een overzicht van gevonden plugins met filters en metadata.
- **Search Engine:** ondersteunt full-text zoekopdrachten over plugininformatie en tags.
- **Tagging System:** biedt flexibele labeling en organisatie van assets.

## Architectuurbenadering
USAM is ontworpen als een cross-platform desktopapplicatie met een scheiding tussen:
- **Frontend:** React-gebaseerde UI voor snelle browser- en zoekervaring.
- **Backend:** Node.js services voor scanning, persistence en business logic.
- **Database:** SQLite als lokale single source of truth.

## Releaseplanning
- **MVP v0.1:** VST plugin discovery, browser, zoekfunctie en tagging.
- **Beta v0.5:** toevoeging van samples, presets, library-adapters en AI-assisted search.
- **Release v1.0:** volledige multi-asset ondersteuning, hardware foundation en productierijpe desktoprelease.

## Documentatie
- `ARCHITECTURE.md` — beschrijft de architectuurprincipes en systeembouw.
- `ROADMAP.md` — geeft de releaseplanning en versie-indeling weer.
- `DATABASE_SCHEMA.md` — presenteert het relationele datamodel voor assets en zoekindexen.

## Status
Dit document geeft de definitie weer voor de eerste fase van USAM. De huidige focus is conceptueel en richt zich op het vastleggen van de architectuur en roadmap zonder broncodeimplementatie.
