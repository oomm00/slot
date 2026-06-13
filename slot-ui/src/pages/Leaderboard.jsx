import { useEffect, useState } from 'react';
import { getTopBy } from '../services/leaderboardService';
import DataTable from '../components/DataTable';
import LoadingSpinner from '../components/LoadingSpinner';
import { fmt } from '../utils/format';

const metrics = [
  { key: 'balance', label: 'Balance' },
  { key: 'winrate', label: 'Win Rate' },
  { key: 'winnings', label: 'Winnings' },
  { key: 'games', label: 'Games Played' },
  { key: 'biggestwin', label: 'Biggest Win' },
];

const kOptions = [5, 10, 15, 20, 25, 50];

export default function Leaderboard() {
  const [active, setActive] = useState('balance');
  const [k, setK] = useState(10);
  const [data, setData] = useState([]);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    async function load() {
      setLoading(true);
      try { setData(await getTopBy(active, k)); } catch {}
      setLoading(false);
    }
    load();
  }, [active, k]);

  const cols = [
    { key: 'rank', label: '#' },
    { key: 'name', label: 'Name' },
    { key: 'balance', label: 'Balance', render: (v) => `${fmt(v)} Credits` },
    { key: 'winRate', label: 'Win Rate', render: (v) => (v * 100).toFixed(1) + '%' },
    { key: 'gamesPlayed', label: 'Games' },
    { key: 'totalWagered', label: 'Wagered', render: (v) => `${fmt(v)} Credits` },
  ];

  const dataWithRank = data.map((row, i) => ({ ...row, rank: i + 1 }));

  return (
    <div className="space-y-6">
      <div className="flex items-center justify-between flex-wrap gap-4">
        <h1 className="text-2xl font-bold">Leaderboard</h1>
        <div className="flex items-center gap-2">
          <label className="text-sm text-slate-400">Show top</label>
          <select
            value={k}
            onChange={(e) => setK(Number(e.target.value))}
            className="bg-slate-700 border border-slate-600 rounded-lg px-3 py-1.5 text-sm"
          >
            {kOptions.map((n) => (
              <option key={n} value={n}>{n}</option>
            ))}
          </select>
        </div>
      </div>

      <div className="flex gap-2 flex-wrap">
        {metrics.map((m) => (
          <button
            key={m.key}
            onClick={() => setActive(m.key)}
            className={`px-4 py-2 rounded-lg text-sm font-medium transition-colors ${
              active === m.key ? 'bg-indigo-600 text-white' : 'bg-slate-700 text-slate-300 hover:bg-slate-600'
            }`}
          >
            {m.label}
          </button>
        ))}
      </div>

      {loading ? (
        <LoadingSpinner />
      ) : (
        <div className="bg-slate-800 rounded-xl border border-slate-600 p-4">
          <DataTable columns={cols} data={dataWithRank} />
        </div>
      )}
    </div>
  );
}
