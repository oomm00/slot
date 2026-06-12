import { useEffect, useState } from 'react';
import api from '../services/api';
import DataTable from '../components/DataTable';
import StatCard from '../components/StatCard';
import LoadingSpinner from '../components/LoadingSpinner';
import { fmt } from '../utils/format';

export default function Admin() {
  const [tab, setTab] = useState('players');
  const [players, setPlayers] = useState([]);
  const [games, setGames] = useState([]);
  const [bets, setBets] = useState([]);
  const [stats, setStats] = useState(null);
  const [loading, setLoading] = useState(true);
  const [msg, setMsg] = useState('');

  const loadData = async () => {
    setLoading(true);
    setMsg('');
    try {
      const [p, g, b, s] = await Promise.all([
        api.get('/admin/players').then((r) => r.data).catch(() => []),
        api.get('/admin/games').then((r) => r.data).catch(() => []),
        api.get('/admin/bets').then((r) => r.data).catch(() => []),
        api.get('/admin/stats').then((r) => r.data).catch(() => null),
      ]);
      setPlayers(p);
      setGames(g);
      setBets(b);
      setStats(s);
    } catch {}
    setLoading(false);
  };

  useEffect(() => { loadData(); }, []);

  const handleDelete = async (playerId) => {
    if (!confirm(`Delete player ${playerId}? This cannot be undone.`)) return;
    try {
      await api.delete(`/players/${playerId}`);
      setMsg(`Deleted player ${playerId}`);
      loadData();
    } catch (err) {
      setMsg(err.response?.data?.error || err.message);
    }
  };

  if (loading) return <LoadingSpinner text="Loading admin panel..." />;

  const tabs = ['players', 'games', 'bets', 'stats'];

  return (
    <div className="space-y-6">
      <h1 className="text-2xl font-bold">Admin Panel</h1>

      {msg && (
        <div className="bg-blue-900/30 border border-blue-700/30 rounded-lg px-4 py-2.5 text-sm text-blue-300">
          {msg}
        </div>
      )}

      <div className="flex gap-2 flex-wrap">
        {tabs.map((t) => (
          <button
            key={t}
            onClick={() => setTab(t)}
            className={`px-4 py-2 rounded-lg text-sm font-medium transition-colors ${
              tab === t ? 'bg-indigo-600 text-white' : 'bg-slate-700 text-slate-300 hover:bg-slate-600'
            }`}
          >
            {t.charAt(0).toUpperCase() + t.slice(1)}
          </button>
        ))}
      </div>

      {tab === 'stats' && stats && (
        <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-4">
          <StatCard label="Total Players" value={stats.totalPlayers} color="bg-indigo-800" />
          <StatCard label="Total Bets" value={stats.totalBets} color="bg-slate-700" />
          <StatCard label="Total Games" value={stats.totalGames} color="bg-slate-700" />
          <StatCard label="Total Wagered" value={`${fmt(stats.totalWagered)} Credits`} color="bg-slate-700" />
          <StatCard label="Total Payouts" value={`${fmt(stats.totalPayouts)} Credits`} color="bg-green-800" />
          <StatCard label="Active Players" value={stats.activePlayers} color="bg-blue-800" />
        </div>
      )}

      {tab === 'players' && (
        <div className="bg-slate-800 rounded-xl border border-slate-600 p-4">
          <h2 className="font-semibold mb-3">All Players ({players.length})</h2>
          <DataTable
            columns={[
              { key: 'playerId', label: 'ID' },
              { key: 'name', label: 'Name' },
              { key: 'username', label: 'Username' },
              { key: 'role', label: 'Role' },
              { key: 'balance', label: 'Balance', render: (v) => `${fmt(v)} Credits` },
              { key: 'age', label: 'Age' },
              { key: 'gamesPlayed', label: 'Games' },
              { key: 'fraudFlagged', label: 'Flagged', render: (v) => v ? 'Yes' : 'No' },
              {
                key: 'actions', label: 'Actions',
                render: (_, row) => (
                  <button
                    onClick={(e) => { e.stopPropagation(); handleDelete(row.playerId); }}
                    className="bg-red-700 hover:bg-red-600 text-white text-xs px-3 py-1 rounded transition-colors"
                  >
                    Delete
                  </button>
                ),
              },
            ]}
            data={players}
          />
        </div>
      )}

      {tab === 'games' && (
        <div className="bg-slate-800 rounded-xl border border-slate-600 p-4">
          <h2 className="font-semibold mb-3">All Games ({games.length})</h2>
          <DataTable
            columns={[
              { key: 'gameId', label: 'Game ID' },
              { key: 'playerId', label: 'Player' },
              { key: 'betType', label: 'Type' },
              { key: 'symbols', label: 'Symbols', render: (v) => v?.join(' ') || '-' },
              { key: 'betAmount', label: 'Bet', render: (v) => `${fmt(v)} Credits` },
              { key: 'payout', label: 'Payout', render: (v) => `${fmt(v)} Credits` },
              { key: 'win', label: 'Result', render: (v) => v ? 'Win' : 'Loss' },
            ]}
            data={games}
          />
        </div>
      )}

      {tab === 'bets' && (
        <div className="bg-slate-800 rounded-xl border border-slate-600 p-4">
          <h2 className="font-semibold mb-3">All Bets ({bets.length})</h2>
          <DataTable
            columns={[
              { key: 'betId', label: 'Bet ID' },
              { key: 'playerId', label: 'Player' },
              { key: 'betType', label: 'Type' },
              { key: 'prediction', label: 'Prediction', render: (v) => v?.join(' ') || '-' },
              { key: 'betAmount', label: 'Amount', render: (v) => `${fmt(v)} Credits` },
              { key: 'expectedValue', label: 'Expected', render: (v) => `${fmt(v)} Credits` },
            ]}
            data={bets}
          />
        </div>
      )}
    </div>
  );
}
