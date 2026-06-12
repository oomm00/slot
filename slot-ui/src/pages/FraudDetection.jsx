import { useEffect, useState } from 'react';
import { useApp } from '../context/AppContext';
import { getFraudAlerts, getFraudCheck } from '../services/fraudService';
import StatCard from '../components/StatCard';
import DataTable from '../components/DataTable';
import FraudAlertCard from '../components/FraudAlertCard';
import LoadingSpinner from '../components/LoadingSpinner';

export default function FraudDetection() {
  const { players, loadPlayers } = useApp();
  const [stats, setStats] = useState(null);
  const [results, setResults] = useState([]);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    async function load() {
      setLoading(true);
      await loadPlayers();
      try { setStats(await getFraudAlerts()); } catch {}
      setLoading(false);
    }
    load();
  }, []);

  useEffect(() => {
    if (players.length > 0 && !loading) {
      (async () => {
        const fr = [];
        for (const p of players.slice(0, 20)) {
          try { fr.push(await getFraudCheck(p.playerId)); } catch {}
        }
        setResults(fr.sort((a, b) => b.fraudScore - a.fraudScore));
      })();
    }
  }, [players]);

  if (loading) return <LoadingSpinner text="Scanning for fraud..." />;

  return (
    <div className="space-y-6">
      <h1 className="text-2xl font-bold">Fraud Detection</h1>

      <div className="grid grid-cols-1 sm:grid-cols-3 gap-4">
        <StatCard label="Players Scanned" value={stats?.totalPlayersScanned ?? 0} color="bg-slate-700" />
        <StatCard label="Flagged" value={stats?.flaggedPlayers ?? 0} color="bg-red-800" />
        <StatCard label="Avg Score" value={stats ? (stats.averageFraudScore * 100).toFixed(1) + '%' : '\u2014'} color="bg-slate-700" />
      </div>

      <div className="space-y-2">
        <h2 className="text-lg font-semibold">Player Risk Scores</h2>
        {results.length === 0 ? (
          <p className="text-sm text-slate-400">No results</p>
        ) : (
          <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-3">
            {results.map((r, i) => (
              <FraudAlertCard key={i} playerId={r.playerId} score={r.fraudScore} flagged={r.flagged} />
            ))}
          </div>
        )}
      </div>

      <div className="bg-slate-800 rounded-xl border border-slate-600 p-4">
        <h2 className="font-semibold mb-3">Fraud Analysis Table</h2>
        <DataTable
          columns={[
            { key: 'playerId', label: 'Player' },
            { key: 'fraudScore', label: 'Score', render: (v) => (v * 100).toFixed(1) + '%' },
            { key: 'flagged', label: 'Flagged', render: (v) => v ? 'Yes' : '\u2014' },
          ]}
          data={results}
        />
      </div>
    </div>
  );
}
