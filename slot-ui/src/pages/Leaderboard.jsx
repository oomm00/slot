import { useEffect, useState } from 'react';
import { getTopBy } from '../services/leaderboardService';
import DataTable from '../components/DataTable';
import LoadingSpinner from '../components/LoadingSpinner';
import { fmt } from '../utils/format';

const metrics = [
  { key: 'balance', label: 'Top Balances' },
  { key: 'winrate', label: 'Top Win Rates' },
  { key: 'winnings', label: 'Top Winnings' },
  { key: 'games', label: 'Most Games' },
  { key: 'biggestwin', label: 'Biggest Wins' },
];

export default function Leaderboard() {
  const [active, setActive] = useState('balance');
  const [data, setData] = useState([]);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    async function load() {
      setLoading(true);
      try { setData(await getTopBy(active, 20)); } catch {}
      setLoading(false);
    }
    load();
  }, [active]);

  const cols = [
    { key: 'rank', label: '#' },
    { key: 'name', label: 'Name' },
    { key: 'balance', label: 'Balance', render: (v) => `${fmt(v)} Credits` },
    { key: 'winRate', label: 'Win Rate', render: (v) => (v * 100).toFixed(1) + '%' },
    { key: 'gamesPlayed', label: 'Games' },
    { key: 'totalWagered', label: 'Wagered', render: (v) => `${fmt(v)} Credits` },
  ];

  return (
    <div className="space-y-6">
      <h1 className="text-2xl font-bold">Leaderboard</h1>

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
          <DataTable columns={cols} data={data} />
        </div>
      )}
    </div>
  );
}
