import { useEffect, useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { useApp } from '../context/AppContext';
import { depositFunds, withdrawFunds } from '../services/playerService';
import DataTable from '../components/DataTable';
import StatCard from '../components/StatCard';
import LoadingSpinner from '../components/LoadingSpinner';
import { fmt } from '../utils/format';

export default function Players() {
  const { user, players, playersLoading, loadPlayers, updateUserBalance } = useApp();
  const navigate = useNavigate();
  const isAdmin = user?.role === 'admin';
  const [funds, setFunds] = useState({ id: user?.playerId || '', amount: 100, action: 'deposit' });
  const [msg, setMsg] = useState('');
  const [msgType, setMsgType] = useState('red');

  useEffect(() => {
    loadPlayers();
    if (user?.playerId && !isAdmin) {
      setFunds((prev) => ({ ...prev, id: user.playerId }));
    }
  }, []);

  useEffect(() => {
    if (user?.playerId && !isAdmin) {
      setFunds((prev) => ({ ...prev, id: user.playerId }));
    }
  }, [user?.playerId]);

  const handleFunds = async (e) => {
    e.preventDefault();
    setMsg('');
    if (!funds.id.trim()) { setMsg('Enter a player ID'); setMsgType('red'); return; }
    if (funds.amount <= 0) { setMsg('Amount must be positive'); setMsgType('red'); return; }
    try {
      let result;
      if (funds.action === 'deposit') result = await depositFunds(funds.id, funds.amount);
      else result = await withdrawFunds(funds.id, funds.amount);
      if (funds.id === user?.playerId) updateUserBalance(result.balance);
      await loadPlayers();
      setMsg(`${funds.action === 'deposit' ? 'Deposited' : 'Withdrew'} ${fmt(funds.amount)} Credits`);
      setMsgType('green');
    } catch (err) {
      setMsg(err.message);
      setMsgType('red');
    }
  };

  const totalBal = players.reduce((s, p) => s + p.balance, 0);

  if (playersLoading) return <LoadingSpinner text="Loading players..." />;

  return (
    <div className="space-y-6">
      <h1 className="text-2xl font-bold">Players</h1>

      <StatCard label="Total Balance" value={`${fmt(totalBal)} Credits`} color="bg-indigo-800" />

      {msg && (
        <div className={`rounded-lg px-4 py-2.5 text-sm ${msgType === 'red' ? 'bg-red-900/30 text-red-300 border border-red-700/30' : 'bg-green-900/30 text-green-300 border border-green-700/30'}`}>
          {msg}
        </div>
      )}

      <form onSubmit={handleFunds} className="bg-slate-800 border border-slate-600 rounded-xl p-4 space-y-3">
        <h2 className="font-semibold">Deposit / Withdraw</h2>
        <div className="flex gap-3 flex-wrap items-end">
          <div className="flex-1 min-w-[120px]">
            <label className="block text-xs text-slate-400 mb-1">Player ID</label>
            <input
              placeholder="Player ID"
              value={funds.id}
              onChange={(e) => setFunds({ ...funds, id: e.target.value })}
              required
              readOnly={!isAdmin}
              className={`w-full bg-slate-700 border border-slate-600 rounded-lg px-3 py-2 text-sm ${!isAdmin ? 'opacity-70 cursor-not-allowed' : ''}`}
            />
          </div>
          <div className="w-28">
            <label className="block text-xs text-slate-400 mb-1">Amount</label>
            <input type="number" min="1" placeholder="Amount" value={funds.amount} onChange={(e) => setFunds({ ...funds, amount: +e.target.value })} required className="w-full bg-slate-700 border border-slate-600 rounded-lg px-3 py-2 text-sm" />
          </div>
          <div className="w-28">
            <label className="block text-xs text-slate-400 mb-1">Action</label>
            <select value={funds.action} onChange={(e) => setFunds({ ...funds, action: e.target.value })} className="w-full bg-slate-700 border border-slate-600 rounded-lg px-3 py-2 text-sm">
              <option value="deposit">Deposit</option>
              <option value="withdraw">Withdraw</option>
            </select>
          </div>
          <button type="submit" className="bg-indigo-600 hover:bg-indigo-500 text-white px-5 py-2 rounded-lg text-sm">Submit</button>
        </div>
      </form>

      <div className="bg-slate-800 rounded-xl border border-slate-600 p-4">
        <h2 className="font-semibold mb-3">All Players</h2>
        <DataTable
          columns={[
            { key: 'playerId', label: 'ID' },
            { key: 'name', label: 'Name' },
            { key: 'balance', label: 'Balance', render: (v) => `${fmt(v)} Credits` },
            { key: 'gamesPlayed', label: 'Games' },
            { key: 'winRate', label: 'Win Rate', render: (v) => (v * 100).toFixed(1) + '%' },
            { key: 'fraudFlagged', label: 'Status', render: (v) => v ? 'Flagged' : 'Clean' },
          ]}
          data={players}
          onRowClick={(row) => navigate(`/players/${row.playerId}`)}
        />
      </div>
    </div>
  );
}
