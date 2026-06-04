import React, { useEffect, useState } from 'react';

declare global { interface Window { usam: any } }

export default function TagList() {
  const [tags, setTags] = useState([]);

  useEffect(() => {
    (async () => {
      if (window.usam?.getTags) {
        const t = await window.usam.getTags();
        setTags(t || []);
      }
    })();
  }, []);

  return (
    <section>
      <h3>Tags</h3>
      <ul>
        {tags.map((tag: any) => (
          <li key={tag.id}>{tag.name}</li>
        ))}
      </ul>
    </section>
  );
}
