import React, { useEffect, useState } from 'react';
import type { Plugin } from 'shared/types';

declare global {
  interface Window { usam: any }
}

export default function PluginBrowser() {
  const [plugins, setPlugins] = useState<Plugin[]>([]);

  useEffect(() => {
    (async () => {
      if (window.usam?.getPlugins) {
        const list = await window.usam.getPlugins();
        setPlugins(list || []);
      }
    })();
  }, []);

  return (
    <section>
      <h2>Plugins</h2>
      <ul>
        {plugins.map(p => (
          <li key={p.id}>{p.name} — {p.pluginType || 'unknown'}</li>
        ))}
      </ul>
    </section>
  );
}
