import { useEffect, useState } from 'react';
import { useApp } from '../context/AppContext';
import { placeBet, getBetTypes } from '../services/bettingService';
import SlotMachineDisplay from '../components/SlotMachineDisplay';
import { fmt } from '../utils/format';

const SYMBOLS = ['CHERRY', 'LEMON', 'ORANGE', 'SEVEN', 'BELL', 'BAR', 'GRAPE'];

function SymbolPicker({ label, value, onChange }) {
  return (
    <div>
      <label className="block text-xs text-slate-400 mb-1.5 font-medium">{label}</label>
      <select value={value} onChange={(e) => onChange(e.target.value)}
        className="w-full bg-slate-700 border border-slate-600 rounded-lg px-3 py-2.5 text-sm text-white focus:outline-none focus:ring-2 focus:ring-indigo-500 focus:border-transparent transition-all">
        <option value="">Select symbol</option>
        {SYMBOLS.map((s) => (
          <option key={s} value={s}>{s}</option>
        ))}
      </select>
    </div>
  );
}

const BET_TYPE_LABELS = {
  EXACT: 'Exact Prediction',
  TRIPLE_SYMBOL: 'Triple Symbol',
  PAIR: 'Pair Prediction',
  SYMBOL_APPEARANCE: 'Symbol Appearance',
  ANY_PAIR: 'Any Pair',
  ANY_TRIPLE: 'Any Triple',
};

export default function Betting() {
  const { user, updateUserBalance } = useApp();
  const [betTypes, setBetTypes] = useState([]);
  const [selectedType, setSelectedType] = useState('');
  const [betAmount, setBetAmount] = useState(10);
  const [prediction, setPrediction] = useState([]);
  const [result, setResult] = useState(null);
  const [error, setError] = useState('');
  const [spinning, setSpinning] = useState(false);
  const [balance, setBalance] = useState(user?.balance ?? 0);
  const [recentResults, setRecentResults] = useState([]);

  useEffect(() => {
    getBetTypes().then(setBetTypes).catch(() => {});
  }, []);

  const currentType = betTypes.find((t) => t.type === selectedType);

  const updatePrediction = (idx, symbol) => {
    const next = [...prediction];
    next[idx] = symbol;
    setPrediction(next);
  };

  const handleSpin = async (e) => {
    e.preventDefault();
    if (!selectedType) { setError('Select a bet type'); return; }
    if (betAmount <= 0) { setError('Bet must be greater than 0'); return; }
    setError('');
    setResult(null);
    setSpinning(true);

    try {
      const pred = prediction.filter((p) => p);
      const res = await placeBet(selectedType, pred, betAmount);
      setTimeout(() => {
        setResult(res);
        setBalance(res.balanceAfter);
        updateUserBalance(res.balanceAfter);
        setSpinning(false);
        setRecentResults((prev) => [res, ...prev].slice(0, 10));
      }, 800);
    } catch (err) {
      setError(err.message);
      setSpinning(false);
    }
  };

  const renderPredictionInputs = () => {
    switch (selectedType) {
      case 'EXACT':
        return (
          <div className="grid grid-cols-3 gap-3">
            <SymbolPicker label="Reel 1" value={prediction[0] || ''} onChange={(v) => updatePrediction(0, v)} />
            <SymbolPicker label="Reel 2" value={prediction[1] || ''} onChange={(v) => updatePrediction(1, v)} />
            <SymbolPicker label="Reel 3" value={prediction[2] || ''} onChange={(v) => updatePrediction(2, v)} />
          </div>
        );
      case 'TRIPLE_SYMBOL':
      case 'PAIR':
      case 'SYMBOL_APPEARANCE':
        return (
          <SymbolPicker label="Pick a symbol" value={prediction[0] || ''} onChange={(v) => setPrediction([v])} />
        );
      default:
        return null;
    }
  };

  return (
    <div className="max-w-3xl mx-auto space-y-6">
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-2xl font-bold">Game</h1>
          <p className="text-sm text-slate-400 mt-0.5">Place your bets and spin the reels</p>
        </div>
        <div className="bg-slate-800 border border-slate-600 rounded-lg px-4 py-2 text-right">
          <p className="text-xs text-slate-400">Balance</p>
          <p className="text-lg font-bold text-indigo-300">{fmt(balance)} <span className="text-xs font-normal text-slate-400">Credits</span></p>
        </div>
      </div>

      <form onSubmit={handleSpin} className="bg-gradient-to-br from-slate-800 to-slate-850 rounded-xl border border-slate-600 p-6 space-y-5 shadow-lg">
        <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
          <div>
            <label className="block text-xs text-slate-400 mb-1.5 font-medium">Bet Type</label>
            <select value={selectedType} onChange={(e) => { setSelectedType(e.target.value); setPrediction([]); }}
              className="w-full bg-slate-700 border border-slate-600 rounded-lg px-3 py-2.5 text-sm text-white focus:outline-none focus:ring-2 focus:ring-indigo-500 transition-all">
              <option value="">Select bet type</option>
              {betTypes.map((t) => (
                <option key={t.type} value={t.type}>
                  {BET_TYPE_LABELS[t.type] || t.type} ({t.multiplier}x)
                </option>
              ))}
            </select>
          </div>
          <div>
            <label className="block text-xs text-slate-400 mb-1.5 font-medium">Bet Amount (Credits)</label>
            <div className="flex gap-2">
              <input type="number" min="1" value={betAmount} onChange={(e) => setBetAmount(+e.target.value)}
                className="flex-1 bg-slate-700 border border-slate-600 rounded-lg px-3 py-2.5 text-sm text-white focus:outline-none focus:ring-2 focus:ring-indigo-500 transition-all" />
              <button type="button" onClick={() => setBetAmount(Math.max(1, betAmount - 10))}
                className="bg-slate-700 hover:bg-slate-600 text-white px-3 rounded-lg text-sm transition-colors">-10</button>
              <button type="button" onClick={() => setBetAmount(betAmount + 10)}
                className="bg-slate-700 hover:bg-slate-600 text-white px-3 rounded-lg text-sm transition-colors">+10</button>
            </div>
          </div>
        </div>

        {currentType && (
          <div className="bg-slate-700/50 rounded-lg px-4 py-2.5 border border-slate-600/50">
            <p className="text-xs text-slate-400">{currentType.description}</p>
          </div>
        )}

        {renderPredictionInputs() && (
          <div className="border-t border-slate-600/50 pt-4">
            <p className="text-xs text-slate-400 mb-3 font-medium">Your Prediction</p>
            {renderPredictionInputs()}
          </div>
        )}

        {selectedType === 'ANY_PAIR' || selectedType === 'ANY_TRIPLE' ? (
          <div className="bg-amber-900/20 border border-amber-700/30 rounded-lg px-4 py-2.5">
            <p className="text-xs text-amber-300">No prediction needed for this bet type. Any matching symbols will win.</p>
          </div>
        ) : null}

        {error && (
          <div className="bg-red-900/30 border border-red-700/30 rounded-lg px-4 py-2.5">
            <p className="text-sm text-red-300">{error}</p>
          </div>
        )}

        <button type="submit" disabled={spinning || !selectedType || betAmount <= 0}
          className="w-full bg-gradient-to-r from-indigo-600 to-purple-600 hover:from-indigo-500 hover:to-purple-500 disabled:from-slate-700 disabled:to-slate-700 text-white font-bold py-3.5 rounded-xl text-lg transition-all duration-200 shadow-lg disabled:shadow-none disabled:cursor-not-allowed tracking-wide">
          {spinning ? 'Spinning...' : 'SPIN'}
        </button>
      </form>

      {spinning && (
        <div className="bg-slate-800 rounded-xl border border-slate-600 p-8 shadow-lg">
          <SlotMachineDisplay spinning />
        </div>
      )}

      {result && !spinning && (
        <div className={`rounded-xl border p-6 space-y-4 shadow-lg ${
          result.win
            ? 'bg-gradient-to-br from-green-900/40 to-slate-800 border-green-600/50'
            : 'bg-gradient-to-br from-red-900/30 to-slate-800 border-red-600/30'
        }`}>
          <SlotMachineDisplay symbols={result.symbols} />
          <div className="text-center space-y-2">
            <p className={`text-3xl font-bold tracking-tight ${result.win ? 'text-green-400' : 'text-red-400'}`}>
              {result.win ? 'WIN' : 'NO WIN'}
            </p>
            <div className="flex justify-center gap-6 text-sm">
              <div>
                <p className="text-slate-400 text-xs">Multiplier</p>
                <p className="text-white font-semibold">x{result.multiplier}</p>
              </div>
              <div>
                <p className="text-slate-400 text-xs">Bet</p>
                <p className="text-white font-semibold">{fmt(result.betAmount)}</p>
              </div>
              <div>
                <p className="text-slate-400 text-xs">Payout</p>
                <p className={`font-semibold ${result.win ? 'text-green-400' : 'text-slate-400'}`}>{fmt(result.payout)}</p>
              </div>
            </div>
            <p className="text-sm text-slate-400">Balance: {fmt(result.balanceAfter)} Credits</p>
            {result.fraudScore > 0 && (
              <p className="text-xs text-yellow-400">Fraud score: {(result.fraudScore * 100).toFixed(1)}%</p>
            )}
          </div>
        </div>
      )}

      {recentResults.length > 0 && (
        <div className="bg-slate-800 rounded-xl border border-slate-600 p-4">
          <h2 className="text-sm font-semibold text-slate-300 mb-3">Recent Spins</h2>
          <div className="space-y-1.5">
            {recentResults.map((r, i) => (
              <div key={i} className={`flex items-center justify-between px-3 py-2 rounded-lg text-sm ${
                r.win ? 'bg-green-900/20' : 'bg-slate-700/30'
              }`}>
                <div className="flex items-center gap-3">
                  <span className={`w-2 h-2 rounded-full ${r.win ? 'bg-green-400' : 'bg-red-400'}`} />
                  <span className="text-slate-300">{r.symbols?.join(' ')}</span>
                </div>
                <div className="flex items-center gap-4 text-xs">
                  <span className="text-slate-400">{BET_TYPE_LABELS[r.betType] || r.betType}</span>
                  <span className={r.win ? 'text-green-400 font-medium' : 'text-slate-400'}>{fmt(r.payout)}</span>
                </div>
              </div>
            ))}
          </div>
        </div>
      )}
    </div>
  );
}
