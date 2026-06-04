import React, { useState } from 'react';

declare global { interface Window { usam: any } }

export default function SearchBar() {
  const [q, setQ] = useState('');

  async function onSearch(e?: React.FormEvent) {
    e?.preventDefault();
    if (window.usam?.search) {
      const results = await window.usam.search(q);
      console.log('Search results', results);
    }
  }

  return (
    <form onSubmit={onSearch}>
      <input value={q} onChange={e => setQ(e.target.value)} placeholder="Search plugins..." />
      <button type="submit">Search</button>
    </form>
  );
}
