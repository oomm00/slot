import { useEffect, useState } from 'react';
import { useApp } from '../context/AppContext';
import { getSystemAnalytics } from '../services/analyticsService';
import { getFraudAlerts } from '../services/fraudService';
import { getGameHistory } from '../services/bettingService';
import StatCard from '../components/StatCard';
import DataTable from '../components/DataTable';
import LoadingSpinner from '../components/LoadingSpinner';
import { fmt } from '../utils/format';

export default function Dashboard() {
  const { user, lb, loadLeaderboard, loadPlayers, players } = useApp();
  const [sys, setSys] = useState(null);
  const [fraud, setFraud] = useState(null);
  const [recentGames, setRecentGames] = useState([]);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    async function init() {
      setLoading(true);
      await Promise.all([loadPlayers(), loadLeaderboard(5)]);
      try { setSys(await getSystemAnalytics()); } catch {}
      try { setFraud(await getFraudAlerts()); } catch {}
      try {
        if (user?.playerId) {
          const games = await getGameHistory(user.playerId);
          setRecentGames(games.slice(-5).reverse());
        }
      } catch {}
      setLoading(false);
    }
    init();
  }, []);

  if (loading) return <LoadingSpinner text="Loading dashboard..." />;

  const totalRevenue = players.reduce((s, p) => s + (p.totalWagered || 0), 0);
  const totalPayouts = players.reduce((s, p) => s + (p.totalWon || 0), 0);

  return (
    <div className="space-y-6">
      <h1 className="text-2xl font-bold">Dashboard</h1>
      {user && (
        <p className="text-sm text-slate-400">
          Welcome, {user.name}. Balance: <span className="text-indigo-300 font-semibold">{fmt(user.balance)} Credits</span>
        </p>
      )}

      <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-4">
        <StatCard label="Total Players" value={players.length} color="bg-indigo-800" />
        <StatCard label="Total Wagered" value={`${fmt(totalRevenue)} Credits`} color="bg-slate-700" />
        <StatCard label="Total Payouts" value={`${fmt(totalPayouts)} Credits`} color="bg-slate-700" />
        <StatCard label="Fraud Alerts" value={fraud?.flaggedPlayers ?? 0} color="bg-red-800" />
      </div>

      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
        <div className="bg-slate-800 rounded-xl border border-slate-600 p-4">
          <h2 className="text-lg font-semibold mb-3">System Analytics</h2>
          {sys ? (
            <div className="grid grid-cols-2 gap-3 text-sm">
              <div><span className="text-slate-400">House Edge</span><br /><span className="font-medium">{(sys.houseEdge * 100).toFixed(1)}%</span></div>
              <div><span className="text-slate-400">Win Rate</span><br /><span className="font-medium">{sys.overallWinRate?.toFixed(1)}%</span></div>
              <div><span className="text-slate-400">Avg Streak</span><br /><span className="font-medium">{sys.avgStreakLength?.toFixed(1)}</span></div>
              <div><span className="text-slate-400">Avg Ruin Risk</span><br /><span className="font-medium">{(sys.avgRuinRisk * 100).toFixed(1)}%</span></div>
            </div>
          ) : <p className="text-sm text-slate-400">No analytics</p>}
        </div>

        <div className="bg-slate-800 rounded-xl border border-slate-600 p-4">
          <h2 className="text-lg font-semibold mb-3">Leaderboard Top 5</h2>
          <DataTable
            columns={[
              { key: 'rank', label: '#' },
              { key: 'name', label: 'Name' },
              { key: 'balance', label: 'Balance', render: (v) => `${fmt(v)} Credits` },
            ]}
            data={lb}
          />
        </div>
      </div>

      {user && recentGames.length > 0 && (
        <div className="bg-slate-800 rounded-xl border border-slate-600 p-4">
          <h2 className="text-lg font-semibold mb-3">Your Recent Spins</h2>
          <DataTable
            columns={[
              { key: 'betType', label: 'Type' },
              { key: 'betAmount', label: 'Bet', render: (v) => `${fmt(v)} Credits` },
              { key: 'win', label: 'Result', render: (v) => v ? 'Win' : 'Loss' },
              { key: 'payout', label: 'Payout', render: (v) => `${fmt(v)} Credits` },
            ]}
            data={recentGames}
          />
        </div>
      )}
    </div>
  );
}
