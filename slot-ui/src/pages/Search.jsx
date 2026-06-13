import { useState } from 'react';
import { searchPlayers, searchGame, searchBet, searchByDateRange } from '../services/searchService';
import DataTable from '../components/DataTable';
import { fmt } from '../utils/format';

const tabs = [
  { key: 'players', label: 'Players' },
  { key: 'game', label: 'Game' },
  { key: 'bet', label: 'Bet' },
  { key: 'date', label: 'Date Range' },
];

export default function Search() {
  const [tab, setTab] = useState('players');
  const [query, setQuery] = useState('');
  const [results, setResults] = useState(null);
  const [error, setError] = useState('');
  const [loading, setLoading] = useState(false);
  const [dates, setDates] = useState({ start: '', end: '' });

  const handleSearch = async (e) => {
    e.preventDefault();
    setError('');
    setResults(null);
    if (!query.trim() && tab !== 'date') { setError('Enter a query'); return; }
    setLoading(true);

    try {
      let data;
      switch (tab) {
        case 'players': 
          // Always use client-side case-insensitive search for players
          // Backend search is case-sensitive and compiled server doesn't have the fix
          try {
            const response = await fetch('/api/players', {
              headers: {
                'Authorization': `Bearer ${localStorage.getItem('token')}`
              }
            });
            if (response.ok) {
              const allPlayers = await response.json();
              const searchLower = query.toLowerCase().trim();
              data = allPlayers.filter(p => 
                p.name?.toLowerCase().includes(searchLower) ||
                p.username?.toLowerCase().includes(searchLower) ||
                p.playerId?.toLowerCase().includes(searchLower)
              );
            } else {
              throw new Error('Failed to fetch players');
            }
          } catch (err) {
            setError('Failed to search players: ' + err.message);
            data = [];
          }
          break;
        case 'game': data = [await searchGame(query)]; break;
        case 'bet': data = [await searchBet(query)]; break;
        case 'date':
          if (!dates.start || !dates.end) { setError('Start and end dates required'); setLoading(false); return; }
          data = await searchByDateRange(dates.start, dates.end);
          break;
      }
      setResults(data);
    } catch (err) { 
      setError(err.message);
      setResults([]);
    }
    setLoading(false);
  };

  const playerCols = [
    { key: 'playerId', label: 'ID' },
    { key: 'name', label: 'Name' },
    { key: 'balance', label: 'Balance', render: (v) => `${fmt(v)} Credits` },
  ];

  const gameCols = [
    { key: 'gameId', label: 'Game ID' },
    { key: 'playerId', label: 'Player' },
    { key: 'betType', label: 'Type' },
    { key: 'betAmount', label: 'Bet', render: (v) => `${fmt(v)} Credits` },
    { key: 'win', label: 'Result', render: (v) => v ? 'Win' : 'Loss' },
    { key: 'payout', label: 'Payout', render: (v) => `${fmt(v)} Credits` },
  ];

  const betCols = [
    { key: 'betId', label: 'Bet ID' },
    { key: 'playerId', label: 'Player' },
    { key: 'betType', label: 'Type' },
    { key: 'prediction', label: 'Prediction', render: (v) => v?.join(' ') || '-' },
    { key: 'betAmount', label: 'Amount', render: (v) => `${fmt(v)} Credits` },
    { key: 'expectedValue', label: 'Expected', render: (v) => `${fmt(v)} Credits` },
  ];

  return (
    <div className="space-y-6 max-w-3xl">
      <h1 className="text-2xl font-bold">Search</h1>

      <div className="flex gap-2 flex-wrap">
        {tabs.map((t) => (
          <button
            key={t.key}
            onClick={() => { setTab(t.key); setResults(null); setError(''); }}
            className={`px-4 py-2 rounded-lg text-sm font-medium transition-colors ${
              tab === t.key ? 'bg-indigo-600 text-white' : 'bg-slate-700 text-slate-300 hover:bg-slate-600'
            }`}
          >
            {t.label}
          </button>
        ))}
      </div>

      <form onSubmit={handleSearch} className="flex gap-3 flex-wrap">
        {tab === 'date' ? (
          <>
            <input type="date" value={dates.start} onChange={(e) => setDates({ ...dates, start: e.target.value })} className="flex-1 min-w-[120px] bg-slate-700 border border-slate-600 rounded-lg px-3 py-2 text-sm" />
            <input type="date" value={dates.end} onChange={(e) => setDates({ ...dates, end: e.target.value })} className="flex-1 min-w-[120px] bg-slate-700 border border-slate-600 rounded-lg px-3 py-2 text-sm" />
          </>
        ) : (
          <input
            placeholder={tab === 'players' ? 'Search by name...' : tab === 'game' ? 'Game ID...' : 'Bet ID...'}
            value={query}
            onChange={(e) => setQuery(e.target.value)}
            className="flex-1 min-w-[200px] bg-slate-700 border border-slate-600 rounded-lg px-3 py-2 text-sm"
          />
        )}
        <button type="submit" disabled={loading} className="bg-indigo-600 hover:bg-indigo-500 disabled:bg-slate-600 text-white px-6 py-2 rounded-lg text-sm">
          {loading ? 'Searching...' : 'Search'}
        </button>
      </form>

      {error && <p className="text-sm text-red-400">{error}</p>}

      {results && (
        <div className="bg-slate-800 rounded-xl border border-slate-600 p-4">
          <p className="text-sm text-slate-400 mb-3">{results.length} result(s)</p>
          {tab === 'players' && <DataTable columns={playerCols} data={results} />}
          {tab === 'game' && <DataTable columns={gameCols} data={results} />}
          {tab === 'bet' && <DataTable columns={betCols} data={results} />}
          {tab === 'date' && <DataTable columns={gameCols} data={results} />}
        </div>
      )}
    </div>
  );
}
