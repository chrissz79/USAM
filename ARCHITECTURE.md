# USAM Architectuur

## Overzicht
USAM is ontworpen als een modulair desktopplatform voor het beheren van sound assets, met een nadruk op uitbreidbaarheid, performance en lokale betrouwbaarheid.
De architectuur splitst de applicatie in duidelijke lagen: frontend, backend, database en specifieke domeincomponenten zoals de plugin scanner, search engine en tagging system.

## Systeemarchitectuur
USAM gebruikt een hybride desktoparchitectuur waarin een Electron shell de gebruikersinterface host en de backendservices coördineert.
Belangrijke architectuurprincipes zijn:
- **Modulariteit:** losse componenten met duidelijk gedefinieerde interfaces.
- **Cross-platform:** werken op Windows, macOS en Linux.
- **Database-gedreven logica:** SQLite fungeert als single source of truth.
- **Separation of concerns:** UI, business logic en persistence blijven gescheiden.

### Lagen
- **Presentatielaag:** React-gestuurde UI in Electron.
- **Applicatielaag:** services, use cases en domeinlogica.
- **Persistencelaag:** SQLite-database en data access services.
- **Integratielaag:** plugin scanner en externe adapters.

## Frontend
Het frontendgedeelte is gericht op een responsieve, intuïtieve ervaring voor asset discovery en organisatie.

### Kerncomponenten
- **Electron container:** lanceert de desktopapplicatie, verzorgt native bestands- en systeemtoegang en beheert het venster- en menu-systeem.
- **React UI:** bouwt de interface voor browserweergave, zoekresultaten en metadata-paneel.
- **State management:** houdt UI-state, zoekfilters, geselecteerde assets en tagdata bij.
- **Interactiepatronen:** plugin browser, zoekbalk, tagbeheer en detailpagina's.

### UX-principes
- **Snelheid:** directe weergave van plugin-lijsten en zoekresultaten.
- **Filterbaarheid:** bepalende filters voor vendor, plugin-type en tags.
- **Feedback:** scan- en zoekstatus zichtbaar maken tijdens taken.

## Backend
De backend beheert de core business logic en fungeert als brug tussen de UI en de database.

### Diensten
- **Scanner service:** verwerkt VST scanoperaties en roept de database-persistentie aan.
- **Search service:** bouwt query's voor full-text search en verwerkt resultaten.
- **Tag service:** beheert tagcreatie, koppelingen en zoekfilters op tags.
- **Data service:** abstraheert database-oproepen naar domeinmodellen.

### Communicatie
- **IPC tussen frontend en backend:** Electron IPC-berichten of vergelijkbare mechanismen zorgen voor veilige requests/responses.
- **Asynchrone workflows:** scanning en zoekopdrachten lopen asynchroon om UI-blocking te voorkomen.

## Database
SQLite is gekozen als lokale, lichtgewicht en betrouwbare database voor MVP v0.1.
Het datamodel ondersteunt plugin metadata, tagging-relaties en zoekindexen.

### Database eigenschappen
- **Lokaal en embedded:** geen extra servercomponent.
- **Transacties:** veilige updates tijdens scans en metadata wijzigingen.
- **FTS-index:** full-text search op samengestelde metadata.
- **Connectiviteit:** backend services gebruiken een database-abstracitielaag.

### Data flow
- De scanner schrijft plugin records naar de database.
- Tags en relaties worden opgeslagen via koppeltabellen.
- Zoekdocumenten worden opgebouwd voor snelle full-text queries.

## Plugin Scanner
De plugin scanner is een centraal domeincomponent voor de MVP en detecteert lokale VST-plugins.

### Functie
- Verkent gedefinieerde plugin-paden.
- Identificeert VST-bestanden en leest metadata zoals naam, vendor, versie en type.
- Detecteert nieuwe, gewijzigde en verwijderde plugins.

### Architectuur
- **Scanner engine:** kernel die bestanden identificeert en metadata verwerkt.
- **Adapterlaag:** maakt het mogelijk later andere formats of library scanners toe te voegen.
- **Scan sessions:** bewaart status, tijdstippen en resultaten voor reproduceerbaarheid.

## Search Engine
De search engine maakt het mogelijk om snel relevante plugins te vinden op basis van metadata en tags.

### Doel
- Volledige tekst doorzoeken van pluginnaam, vendor, categorie en tags.
- Ondersteunen van zoekopdrachten met filters en ranking.

### Implementatie
- **SQLite FTS:** full-text indexering voor zoekprestaties.
- **Search document materiaal:** gecombineerde content van plugin metadata en tag labels.
- **Query builder:** genereert zoekopdrachten op basis van zoekterm en filters.
- **Resultaatverrijking:** later uitbreidbaar met AI-assisted suggesties.

## Tagging System
Het tagging system biedt flexibele organisatie van plugins door gebruiker-gedefinieerde labels.

### Doel
- Enable custom categorization beyond traditionele plugin metadata.
- Support many-to-many relations between plugins and tags.

### Architectuur
- **Tags table:** bewaart unieke labels en beschrijvingen.
- **Link table:** `plugin_tag` koppelt plugins aan tags.
- **Tag services:** creëren, bijwerken, verwijderen en zoeken op tags.
- **UI-integratie:** tagging in de plugin detailweergave en zoekfilters.

### Gebruiksscenario's
- Plugins markeren als "favorites", "drums", "synths" of "sound design".
- combinatie van tags en tekstzoekopdrachten om snel relevante assets te vinden.

## Samenvatting
USAM's architectuur legt de basis voor een schaalbaar desktopplatform waarin frontend, backend en database helder gescheiden zijn.
Voor de MVP ligt de focus op een betrouwbare VST scanner, een snelle SQLite-gebaseerde search engine en een krachtig tagging systeem dat later kan uitbreiden naar extra assettypes en AI-assisted workflows.
