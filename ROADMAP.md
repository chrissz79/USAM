# USAM Roadmap

Deze roadmap beschrijft de geplande uitrol van USAM van MVP tot productrelease, gebaseerd op de bestaande projectvisie en architectuurprincipes.

## Visie
USAM wil een modulair, database-gedreven assetbeheerplatform worden voor de muziekproductie-wereld.
De focus ligt bij de eerste releases op VST-pluginbeheer, uitgebreide zoekmogelijkheden en flexibele tagging.

## MVP v0.1
### Doel
Een betrouwbare basis leggen voor VST discovery, lokale metadata-opslag en efficiënte assetnavigatie.

### Scope
- VST Scanner
- SQLite Database
- Plugin Browser
- Search Engine
- Tagging System

### Kernfeatures
- Detectie van VST-plugins op het lokale systeem.
- Opslag van plugin metadata in een SQLite-database.
- Een browserinterface voor het verkennen van gedetecteerde plugins.
- Full-text search over plugin metadata en tags.
- Flexibel tagbeheer voor custom organisatie en filtering.
- Scan-sessies en statusinformatie voor reproduceerbare scans.

### Succescriteria
- Plugins worden consistent en betrouwbaar gescand.
- Metadata wordt correct opgeslagen en kan worden opgehaald.
- De gebruiker kan plugins zoeken, filteren en taggen.
- UI en database blijven responsief bij realistische bibliotheken.

## Beta v0.5
### Doel
Het platform uitbreiden naar meerdere assettypen, diepere metadata-workflows en betere zoek- en organisatiefunctionaliteit.

### Scope
- Sample Management
- Preset Management
- Support voor Kontakt, Falcon en HALion library adapters
- Verbeterde search experience
- Metadata import/export
- Basis AI-assisted search capabilities

### Kernfeatures
- Browser voor samples en presets naast VST-plugins.
- Library adapters voor populaire sound libraryformaten.
- Uitgebreide filteropties op assettype, vendor, categorie en tags.
- Geavanceerde zoekfunctionaliteit met ranking en suggesties.
- Relaties tussen plugins, presets en libraries.
- Basisniveau AI-assisted search of metadata-suggesties voor zoekopdrachten.

### Succescriteria
- Het systeem ondersteunt meerdere assettypen naast VST-plugins.
- Zoekresultaten blijven snel en relevant na uitbreiding.
- Gebruikers kunnen metadata en tags delen tussen assets.
- De architectuur blijft modular en uitbreidbaar.

## Release v1.0
### Doel
Een productierijpe release leveren met volledige multi-asset dekking en een foundation voor toekomstige hardware-integratie.

### Scope
- Volledige multi-format asset workflow
- Productierijpe desktop packaging
- Stabiliteit, performance en QA
- Foundations voor toekomstige hardware-integratie

### Kernfeatures
- Volledige ondersteuning voor VST, samples, presets en library assets.
- Geavanceerde tagging en organisatorische workflows.
- Desktop packaging voor Windows, macOS en Linux.
- Prestatieoptimalisatie en data-integriteit.
- Basisarchitectuur voor hardware- en device-integratie.

### Succescriteria
- USAM is bruikbaar als end-to-end asset management oplossing.
- De applicatie voldoet aan productiestandaarden voor stabiliteit en performance.
- De architectuur biedt een solide basis voor toekomstige uitbreidingen.

## Planning en verdere uitbreidingen
- Na v1.0 wordt de roadmap uitgebreid met AI-assisted tagging, collaboratieve workflows en hardware-synchronisatie.
- De modulaire architectuur maakt het mogelijk om later nieuwe assettypen en externe adapters toe te voegen zonder de kernopzet te wijzigen.
