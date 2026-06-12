export default function FraudAlertCard({ playerId, score, flagged }) {
  const level = score > 0.7 ? 'high' : score > 0.3 ? 'medium' : 'low';
  const colors = { high: 'bg-red-900 border-red-500', medium: 'bg-yellow-900 border-yellow-500', low: 'bg-green-900 border-green-500' };
  return (
    <div className={`${colors[level]} rounded-xl border p-4 flex items-center justify-between`}>
      <div>
        <p className="text-sm font-medium text-white">{playerId}</p>
        <p className="text-xs text-slate-300">Fraud Score: {(score * 100).toFixed(1)}%</p>
      </div>
      {flagged && <span className="text-red-300 text-xs font-bold">⚠ FLAGGED</span>}
    </div>
  );
}
