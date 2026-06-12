import { useEffect, useState } from 'react';
import { useParams } from 'react-router-dom';
import { getPlayer } from '../services/playerService';
import { getBetHistory, getGameHistory } from '../services/bettingService';
import { getPlayerAnalytics } from '../services/analyticsService';
import { getFraudCheck } from '../services/fraudService';
import PlayerCard from '../components/PlayerCard';
import DataTable from '../components/DataTable';
import LoadingSpinner from '../components/LoadingSpinner';
import { fmt } from '../utils/format';

export default function PlayerProfile() {
  const { id } = useParams();
  const [player, setPlayer] = useState(null);
  const [bets, setBets] = useState([]);
  const [games, setGames] = useState([]);
  const [analytics, setAnalytics] = useState(null);
  const [fraud, setFraud] = useState(null);
  const [activeTab, setActiveTab] = useState('bets');
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    async function load() {
      setLoading(true);
      try { setPlayer(await getPlayer(id)); } catch {}
      try { setBets(await getBetHistory(id)); } catch {}
      try { setGames(await getGameHistory(id)); } catch {}
      try { setAnalytics(await getPlayerAnalytics(id)); } catch {}
      try { setFraud(await getFraudCheck(id)); } catch {}
      setLoading(false);
    }
    load();
  }, [id]);

  if (loading) return <LoadingSpinner text="Loading profile..." />;
  if (!player) return <p className="text-red-400">Player not found</p>;

  return (
    <div className="space-y-6">
      <h1 className="text-2xl font-bold">Player Profile</h1>
      <PlayerCard player={player} />

      {fraud && (
        <div className={`rounded-xl border p-4 ${fraud.flagged ? 'bg-red-900 border-red-500' : 'bg-green-900 border-green-500'}`}>
          <p className="text-sm font-medium">Fraud Score: {(fraud.fraudScore * 100).toFixed(1)}% {fraud.flagged && 'FLAGGED'}</p>
        </div>
      )}

      {analytics && (
        <div className="bg-slate-800 rounded-xl border border-slate-600 p-4 space-y-2">
          <h2 className="font-semibold">Strategy Report</h2>
          <div className="grid grid-cols-2 sm:grid-cols-3 gap-3 text-sm">
            <div><span className="text-slate-400">Strategy</span><br /><span className="font-medium">{analytics.recommendedStrategy}</span></div>
            <div><span className="text-slate-400">Expected Return</span><br /><span className="font-medium">{fmt(analytics.expectedReturn)} Credits</span></div>
            <div><span className="text-slate-400">Risk</span><br /><span className="font-medium">{(analytics.risk * 100).toFixed(1)}%</span></div>
            <div><span className="text-slate-400">DP Analysis</span><br /><span className="font-medium">{analytics.dpAnalysis}</span></div>
            <div><span className="text-slate-400">Greedy</span><br /><span className="font-medium">{analytics.greedyAnalysis}</span></div>
            <div><span className="text-slate-400">Kelly</span><br /><span className="font-medium">{analytics.kellyAnalysis}</span></div>
          </div>
        </div>
      )}

      <div className="flex gap-2 mb-2">
        <button onClick={() => setActiveTab('bets')} className={`px-4 py-1.5 rounded-lg text-sm ${activeTab === 'bets' ? 'bg-indigo-600 text-white' : 'bg-slate-700 text-slate-300'}`}>Bets</button>
        <button onClick={() => setActiveTab('games')} className={`px-4 py-1.5 rounded-lg text-sm ${activeTab === 'games' ? 'bg-indigo-600 text-white' : 'bg-slate-700 text-slate-300'}`}>Games</button>
      </div>

      {activeTab === 'bets' && (
        <div className="bg-slate-800 rounded-xl border border-slate-600 p-4">
          <h2 className="font-semibold mb-3">Bet History ({bets.length})</h2>
          <DataTable
            columns={[
              { key: 'betId', label: 'Bet ID' },
              { key: 'betType', label: 'Type' },
              { key: 'betAmount', label: 'Amount', render: (v) => `${fmt(v)} Credits` },
              { key: 'expectedValue', label: 'Expected', render: (v) => `${fmt(v)} Credits` },
              { key: 'risk', label: 'Risk' },
            ]}
            data={bets}
          />
        </div>
      )}

      {activeTab === 'games' && (
        <div className="bg-slate-800 rounded-xl border border-slate-600 p-4">
          <h2 className="font-semibold mb-3">Game History ({games.length})</h2>
          <DataTable
            columns={[
              { key: 'gameId', label: 'Game' },
              { key: 'betType', label: 'Type' },
              { key: 'symbols', label: 'Symbols', render: (v) => v?.join(' ') || '-' },
              { key: 'betAmount', label: 'Bet', render: (v) => `${fmt(v)} Credits` },
              { key: 'win', label: 'Result', render: (v) => v ? 'Win' : 'Loss' },
              { key: 'payout', label: 'Payout', render: (v) => `${fmt(v)} Credits` },
            ]}
            data={games}
          />
        </div>
      )}
    </div>
  );
}
