import { fmt, fmtPercent } from '../utils/format';

export default function PlayerCard({ player }) {
  if (!player) return null;
  return (
    <div className="bg-slate-800 rounded-xl border border-slate-600 p-5 space-y-3">
      <div className="flex items-center justify-between">
        <div>
          <h3 className="text-lg font-semibold text-white">{player.name}</h3>
          <p className="text-xs text-slate-400">{player.playerId} (@{player.username})</p>
        </div>
        {player.fraudFlagged && (
          <span className="bg-red-900 text-red-200 text-xs px-2 py-1 rounded-full">Flagged</span>
        )}
      </div>
      <div className="grid grid-cols-2 gap-2 text-sm">
        <div><span className="text-slate-400">Balance</span><br /><span className="text-white font-medium">{fmt(player.balance)} Credits</span></div>
        <div><span className="text-slate-400">Win Rate</span><br /><span className="text-white font-medium">{fmtPercent(player.winRate)}</span></div>
        <div><span className="text-slate-400">Games</span><br /><span className="text-white font-medium">{player.gamesPlayed}</span></div>
        <div><span className="text-slate-400">Risk</span><br /><span className="text-white font-medium">{fmtPercent(player.riskScore)}</span></div>
      </div>
    </div>
  );
}
