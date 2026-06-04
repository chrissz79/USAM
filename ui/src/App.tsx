import React from 'react';
import PluginBrowser from './components/PluginBrowser';
import SearchBar from './components/SearchBar';
import TagList from './components/TagList';

export default function App() {
  return (
    <div>
      <header>
        <h1>USAM</h1>
      </header>
      <main>
        <SearchBar />
        <TagList />
        <PluginBrowser />
      </main>
    </div>
  );
}
