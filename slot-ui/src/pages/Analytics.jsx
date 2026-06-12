import { useEffect, useState } from 'react';
import {
  getSystemAnalytics, getDistribution, getRiskMetrics,
  getStrategyComparison, getStrategyRecommendation
} from '../services/analyticsService';
import api from '../services/api';
import LoadingSpinner from '../components/LoadingSpinner';

const SYMBOL_COLORS = {
  CHERRY: '#ef4444', LEMON: '#eab308', ORANGE: '#f97316',
  BELL: '#06b6d4', SEVEN: '#8b5cf6', DIAMOND: '#22d3ee'
};
const STRAT_COLORS = ['#06b6d4', '#f59e0b', '#10b981'];

function Slider({ label, value, onChange, min, max, step }) {
  return (
    <label className="flex flex-col gap-1 text-sm">
      <span className="text-slate-300">{label}: <span className="font-mono text-cyan-300">{value}</span></span>
      <input type="range" min={min} max={max} step={step} value={value}
        onChange={e => onChange(Number(e.target.value))}
        className="accent-cyan-500 w-full" />
    </label>
  );
}

function StatBox({ label, value, color = 'text-white', sub }) {
  return (
    <div className="bg-slate-800 rounded-lg border border-slate-600 p-3">
      <p className="text-xs text-slate-400">{label}</p>
      <p className={`text-lg font-bold font-mono ${color}`}>{value ?? '\u2014'}</p>
      {sub && <p className="text-xs text-slate-500 mt-1">{sub}</p>}
    </div>
  );
}

function Donut({ data, colors, size = 160 }) {
  if (!data || data.length === 0) return null;
  const total = data.reduce((s, d) => s + d.value, 0) || 1;
  const cx = size / 2, cy = size / 2, r = size * 0.38;
  const circumference = 2 * Math.PI * r;
  let offset = 0;
  return (
    <svg viewBox={`0 0 ${size} ${size}`} className="inline-block" style={{ width: size, height: size }}>
      <circle cx={cx} cy={cy} r={r} fill="none" stroke="#1e293b" strokeWidth="20" />
      {data.map((d, i) => {
        const len = circumference * (d.value / total);
        const seg = (
          <circle key={i} cx={cx} cy={cy} r={r} fill="none"
            stroke={colors[i % colors.length]} strokeWidth="20"
            strokeDasharray={`${len} ${circumference - len}`}
            strokeDashoffset={-offset} transform={`rotate(-90 ${cx} ${cy})`}>
            <title>{d.label}: {d.value.toFixed(1)}</title>
          </circle>
        );
        offset += len;
        return seg;
      })}
      <text x={cx} y={cy + 4} textAnchor="middle" fill="#94a3b8" fontSize="11" fontFamily="monospace">
        {total.toFixed(0)}
      </text>
    </svg>
  );
}

function BarChart({ data, label, color = 'bg-cyan-500', height = 150 }) {
  if (!data || data.length === 0) return null;
  const max = Math.max(...data, 0.001);
  return (
    <div>
      <p className="text-xs text-slate-400 mb-1">{label}</p>
      <div className="flex items-end gap-[2px]" style={{ height }}>
        {data.map((v, i) => (
          <div key={i} title={`${i}: ${v.toFixed(4)}`}
            className={`${color} rounded-t`}
            style={{ width: `${100 / data.length}%`, height: `${(v / max) * 100}%`, minHeight: v > 0 ? 1 : 0 }} />
        ))}
      </div>
    </div>
  );
}

function LineChart({ data, label, color = '#06b6d4', height = 180, width = 500 }) {
  if (!data || data.length < 2) return null;
  const max = Math.max(...data, 0.001);
  const min = Math.min(...data, 0);
  const range = max - min || 1;
  const px = (x) => (x / (data.length - 1)) * width;
  const py = (y) => height - ((y - min) / range) * height * 0.9 - height * 0.05;
  const pts = data.map((v, i) => `${px(i)},${py(v)}`).join(' ');
  return (
    <div>
      <p className="text-xs text-slate-400 mb-1">{label}</p>
      <svg viewBox={`0 0 ${width} ${height}`} className="w-full" style={{ maxHeight: height }}>
        <polyline points={pts} fill="none" stroke={color} strokeWidth="2" />
      </svg>
    </div>
  );
}

export default function Analytics() {
  const [sys, setSys] = useState(null);
  const [tab, setTab] = useState('dashboard');
  const [loading, setLoading] = useState(true);
  const [params, setParams] = useState({ balance: 1000, bet: 10, rounds: 50, type: 'ANY_PAIR', riskTolerance: 0.15 });

  const [dist, setDist] = useState(null);
  const [risk, setRisk] = useState(null);
  const [comparison, setComparison] = useState(null);
  const [recommendation, setRecommendation] = useState(null);
  const [strategies, setStrategies] = useState(null);
  const [symbolProbs, setSymbolProbs] = useState(null);

  const defaultWeights = [
    { sym: 'CHERRY', weight: 30, color: SYMBOL_COLORS.CHERRY },
    { sym: 'LEMON', weight: 25, color: SYMBOL_COLORS.LEMON },
    { sym: 'ORANGE', weight: 20, color: SYMBOL_COLORS.ORANGE },
    { sym: 'BELL', weight: 15, color: SYMBOL_COLORS.BELL },
    { sym: 'SEVEN', weight: 8, color: SYMBOL_COLORS.SEVEN },
    { sym: 'DIAMOND', weight: 2, color: SYMBOL_COLORS.DIAMOND },
  ];
  const totalWeight = defaultWeights.reduce((s, sw) => s + sw.weight, 0);
  const defaultProbs = defaultWeights.map(sw => ({ ...sw, prob: (sw.weight / totalWeight * 100).toFixed(1) }));

  const fetchAll = () => {
    setLoading(true);
    const p = { balance: params.balance, bet: params.bet, rounds: params.rounds, type: params.type, riskTolerance: params.riskTolerance };
    Promise.all([
      getSystemAnalytics().then(setSys).catch(() => {}),
      getDistribution(p).then(setDist).catch(() => {}),
      getRiskMetrics(p).then(setRisk).catch(() => {}),
      getStrategyComparison(p).then(setComparison).catch(() => {}),
      getStrategyRecommendation(p).then(r => setRecommendation(r)).catch(() => {}),
      api.get('/analytics/strategies', { params: p }).then(r => setStrategies(r.data)).catch(() => {}),
    ]).finally(() => setLoading(false));
  };

  useEffect(() => { fetchAll(); }, []);

  const tabs = [
    { key: 'dashboard', label: 'Dashboard' },
    { key: 'strategies', label: 'Strategies' },
    { key: 'compare', label: 'Compare' },
    { key: 'distribution', label: 'Distribution' },
  ];

  return (
    <div className="space-y-6">
      <h1 className="text-2xl font-bold">Betting Analyzer & Strategy Lab</h1>

      <div className="bg-slate-800 rounded-xl border border-slate-600 p-4 grid grid-cols-2 md:grid-cols-5 lg:grid-cols-7 gap-4">
        <Slider label="Balance" value={params.balance} onChange={v => setParams(p => ({ ...p, balance: v }))} min={100} max={10000} step={100} />
        <Slider label="Bet" value={params.bet} onChange={v => setParams(p => ({ ...p, bet: v }))} min={1} max={500} step={1} />
        <Slider label="Rounds" value={params.rounds} onChange={v => setParams(p => ({ ...p, rounds: v }))} min={10} max={500} step={10} />
        <label className="flex flex-col gap-1 text-sm">
          <span className="text-slate-300">Bet Type</span>
          <select value={params.type} onChange={e => setParams(p => ({ ...p, type: e.target.value }))}
            className="bg-slate-700 border border-slate-500 rounded px-2 py-1 text-sm">
            <option value="ANY_PAIR">Any Pair (10x)</option>
            <option value="ANY_TRIPLE">Any Triple (25x)</option>
            <option value="SYMBOL_APPEARANCE">Symbol Appearance (3x)</option>
            <option value="PAIR">Pair Prediction (8x)</option>
            <option value="TRIPLE_SYMBOL">Triple Symbol (30x)</option>
            <option value="EXACT_PREDICTION">Exact (50x)</option>
          </select>
        </label>
        <label className="flex flex-col gap-1 text-sm">
          <span className="text-slate-300">Risk Tol.</span>
          <select value={params.riskTolerance} onChange={e => setParams(p => ({ ...p, riskTolerance: Number(e.target.value) }))}
            className="bg-slate-700 border border-slate-500 rounded px-2 py-1 text-sm">
            <option value={0.05}>5%</option>
            <option value={0.1}>10%</option>
            <option value={0.15}>15%</option>
            <option value={0.25}>25%</option>
            <option value={0.5}>50%</option>
          </select>
        </label>
        <button onClick={fetchAll} disabled={loading}
          className="bg-cyan-600 hover:bg-cyan-500 disabled:bg-slate-600 rounded-lg px-4 py-2 font-semibold self-end text-sm">
          {loading ? '\u2026' : 'Analyze'}
        </button>
      </div>

      <div className="flex gap-2 border-b border-slate-700 pb-2">
        {tabs.map(t => (
          <button key={t.key} onClick={() => setTab(t.key)}
            className={`px-4 py-1 rounded-t text-sm font-medium ${tab === t.key ? 'bg-slate-700 text-cyan-300' : 'text-slate-400 hover:text-slate-200'}`}>
            {t.label}
          </button>
        ))}
      </div>

      {loading && <LoadingSpinner text="Running analysis\u2026" />}

      {/* ── DASHBOARD ── */}
      {!loading && tab === 'dashboard' && (
        <div className="space-y-6">
          <div className="grid grid-cols-2 md:grid-cols-3 lg:grid-cols-6 gap-3">
            {sys && <>
              <StatBox label="House Edge" value={`${(sys.houseEdge * 100).toFixed(1)}%`} color="text-red-400" />
              <StatBox label="Win Rate" value={`${sys.overallWinRate?.toFixed(1)}%`} color="text-green-400" />
              <StatBox label="Avg Streak" value={sys.avgStreakLength?.toFixed(1)} color="text-cyan-400" />
              <StatBox label="Ruin Risk" value={`${((sys.avgRuinRisk || 0) * 100).toFixed(1)}%`} color="text-orange-400" />
              <StatBox label="Players" value={sys.totalPlayers} color="text-blue-400" />
              <StatBox label="Spins" value={sys.totalSpins} color="text-purple-400" />
            </>}
          </div>

          {/* Symbol Weights */}
          <div className="bg-slate-800 rounded-xl border border-slate-600 p-4">
            <h2 className="text-lg font-semibold mb-3">Weighted Reel System</h2>
            <div className="grid grid-cols-3 md:grid-cols-6 gap-3">
              {defaultProbs.map(sw => (
                <div key={sw.sym} className="text-center p-2 rounded-lg" style={{ backgroundColor: sw.color + '22', borderColor: sw.color, borderWidth: 1 }}>
                  <p className="text-xs font-semibold" style={{ color: sw.color }}>{sw.sym}</p>
                  <p className="text-lg font-bold font-mono text-white">{sw.weight}</p>
                  <p className="text-xs text-slate-400">{sw.prob}%</p>
                </div>
              ))}
            </div>
          </div>

          {/* Strategy comparison summary */}
          {strategies && (
            <div className="grid md:grid-cols-3 gap-4">
              {strategies.map((s, i) => (
                <div key={i} className="bg-slate-800 rounded-xl border border-slate-600 p-4">
                  <h3 className="font-semibold text-sm mb-2" style={{ color: STRAT_COLORS[i] }}>{s.name}</h3>
                  <div className="space-y-1 text-xs">
                    <div className="flex justify-between"><span className="text-slate-400">Exp. Balance</span><span className="font-mono text-cyan-300">{s.expectedBalance.toFixed(0)}</span></div>
                    <div className="flex justify-between"><span className="text-slate-400">Return</span><span className={`font-mono ${s.expectedValue >= 1 ? 'text-green-400' : 'text-red-400'}`}>{(s.expectedValue * 100).toFixed(0)}%</span></div>
                    <div className="flex justify-between"><span className="text-slate-400">Bust Prob</span><span className="font-mono text-red-400">{(s.bustProbability * 100).toFixed(2)}%</span></div>
                    <div className="flex justify-between"><span className="text-slate-400">Variance</span><span className="font-mono text-yellow-400">{s.variance.toFixed(1)}</span></div>
                  </div>
                  {s.allocations?.length > 0 && (
                    <div className="mt-2">
                      <Donut data={s.allocations.map(a => ({ label: a.betType, value: a.fraction * 100 }))} colors={STRAT_COLORS} size={120} />
                      <div className="text-[10px] mt-1 space-y-0.5">
                        {s.allocations.map((a, ai) => (
                          <div key={ai} className="flex justify-between">
                            <span className="truncate max-w-[100px]">{a.betType}</span>
                            <span className="font-mono">{(a.fraction * 100).toFixed(0)}%</span>
                          </div>
                        ))}
                      </div>
                    </div>
                  )}
                </div>
              ))}
            </div>
          )}

          {/* Risk + Distribution */}
          <div className="grid lg:grid-cols-2 gap-6">
            <div className="bg-slate-800 rounded-xl border border-slate-600 p-4">
              <h2 className="text-lg font-semibold mb-3">Balance Evolution ({params.type})</h2>
              {dist && <LineChart data={dist.expectedBalances} label="Expected Balance" />}
            </div>
            <div className="bg-slate-800 rounded-xl border border-slate-600 p-4">
              <h2 className="text-lg font-semibold mb-3">Risk Metrics</h2>
              {risk && (
                <div className="grid grid-cols-2 gap-3 text-sm">
                  <StatBox label="Bust Prob" value={`${(risk.overallBustProb * 100).toFixed(2)}%`} color="text-red-400" />
                  <StatBox label="Survival" value={`${(risk.overallSurvivalProb * 100).toFixed(2)}%`} color="text-green-400" />
                  <StatBox label="Final Balance" value={risk.finalExpectedBalance?.toFixed(0)} color="text-cyan-400" />
                  <StatBox label="Std Dev" value={risk.balanceStdDev?.toFixed(0)} color="text-yellow-400" />
                </div>
              )}
            </div>
          </div>
        </div>
      )}

      {/* ── STRATEGIES ── */}
      {!loading && tab === 'strategies' && strategies && (
        <div className="space-y-4">
          <div className="grid md:grid-cols-3 gap-4">
            {strategies.map((s, i) => (
              <div key={i} className="bg-slate-800 rounded-xl border border-slate-600 p-4">
                <h3 className="font-bold mb-2" style={{ color: STRAT_COLORS[i] }}>{s.name}</h3>
                <div className="grid grid-cols-2 gap-2 text-sm mb-3">
                  <StatBox label="Exp. Balance" value={s.expectedBalance.toFixed(0)} color="text-cyan-400" sub={`Return: ${(s.expectedValue * 100).toFixed(0)}%`} />
                  <StatBox label="Bust Probability" value={`${(s.bustProbability * 100).toFixed(2)}%`} color={s.bustProbability < 0.01 ? 'text-green-400' : 'text-red-400'} />
                  <StatBox label="Variance" value={s.variance.toFixed(1)} color="text-yellow-400" />
                  <StatBox label="Risk Score" value={s.riskScore.toFixed(4)} color={s.riskScore < 0.3 ? 'text-green-400' : 'text-red-400'} />
                </div>
                {s.allocations?.length > 0 && (
                  <div>
                    <p className="text-xs text-slate-400 mb-2">Allocation</p>
                    <div className="flex justify-center">
                      <Donut data={s.allocations.map(a => ({ label: a.betType, value: a.fraction * 100 }))} colors={STRAT_COLORS} size={160} />
                    </div>
                    <div className="mt-2 space-y-1 text-xs">
                      {s.allocations.map((a, ai) => (
                        <div key={ai} className="flex justify-between items-center border-b border-slate-700 pb-1">
                          <span className="flex items-center gap-1">
                            <span className="w-2 h-2 rounded-full inline-block" style={{ backgroundColor: STRAT_COLORS[ai % 3] }} />
                            <span className="truncate max-w-[140px]">{a.betType}</span>
                          </span>
                          <span className="font-mono text-cyan-300">{(a.fraction * 100).toFixed(0)}%</span>
                          <span className="font-mono text-slate-400">{a.credits.toFixed(0)}cr</span>
                        </div>
                      ))}
                    </div>
                  </div>
                )}
              </div>
            ))}
          </div>
          <div className="bg-slate-800 rounded-xl border border-slate-600 p-4 overflow-x-auto">
            <h3 className="font-semibold mb-3">Greedy vs DP vs Kelly</h3>
            <table className="w-full text-sm">
              <thead><tr className="text-slate-400 border-b border-slate-600">
                <th className="text-left py-2">Metric</th><th className="text-right py-2" style={{color:STRAT_COLORS[0]}}>Greedy</th>
                <th className="text-right py-2" style={{color:STRAT_COLORS[1]}}>DP</th><th className="text-right py-2" style={{color:STRAT_COLORS[2]}}>Kelly</th>
              </tr></thead>
              <tbody>
                {['expectedBalance','expectedValue','bustProbability','variance','riskScore'].map(m => (
                  <tr key={m} className="border-b border-slate-700">
                    <td className="py-1 text-slate-400">{m}</td>
                    {strategies.map((s, i) => {
                      let v = s[m];
                      let color = 'text-white';
                      if (m === 'expectedValue') color = v >= 1 ? 'text-green-400' : 'text-red-400';
                      if (m === 'bustProbability') color = v < 0.01 ? 'text-green-400' : 'text-red-400';
                      return <td key={i} className={`py-1 text-right font-mono ${color}`}>
                        {typeof v === 'number' ? (v < 0.01 && v > 0 ? v.toExponential(2) : v.toFixed(4)) : v}
                      </td>;
                    })}
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        </div>
      )}

      {/* ── COMPARE ── */}
      {!loading && tab === 'compare' && (
        <div className="bg-slate-800 rounded-xl border border-slate-600 p-4">
          <h2 className="text-lg font-semibold mb-3">All Bet Types + Per-Symbol Odds</h2>
          {comparison && (
            <div className="overflow-x-auto">
              <table className="w-full text-sm">
                <thead><tr className="text-slate-400 border-b border-slate-600">
                  <th className="text-left py-2">Bet Type</th>
                  <th className="text-right py-2">Win Prob</th>
                  <th className="text-right py-2">Payout</th>
                  <th className="text-right py-2">Expected Value</th>
                  <th className="text-right py-2">Variance</th>
                  <th className="text-right py-2">Risk</th>
                  <th className="text-right py-2">Bust Prob</th>
                </tr></thead>
                <tbody>
                  {comparison.map((r, i) => {
                    const isSym = r.betType.includes('(');
                    const sym = isSym ? r.betType.match(/\((\w+)\)/)?.[1] : null;
                    const symColor = sym ? SYMBOL_COLORS[sym] || '#94a3b8' : null;
                    const isAny = !isSym && !r.betType.includes('EXACT') && !r.betType.includes('TRIPLE') && !r.betType.includes('PAIR') && !r.betType.includes('SYMBOL');
                    return (
                      <tr key={i} className={`border-b border-slate-700 ${isAny ? 'bg-cyan-900/10' : ''}`}>
                        <td className="py-1 flex items-center gap-1">
                          {symColor && <span className="w-2 h-2 rounded-full inline-block" style={{ backgroundColor: symColor }} />}
                          <span className={isSym ? 'text-xs text-slate-400' : 'font-medium'}>{r.betType}</span>
                        </td>
                        <td className="py-1 text-right font-mono">{(r.winProb * 100).toFixed(2)}%</td>
                        <td className="py-1 text-right font-mono">{r.payoutMultiplier}x</td>
                        <td className={`py-1 text-right font-mono ${r.expectedValue >= 1 ? 'text-green-400' : r.expectedValue > 0.5 ? 'text-yellow-400' : 'text-red-400'}`}>
                          {r.expectedValue.toFixed(4)}
                        </td>
                        <td className="py-1 text-right font-mono">{r.variance.toFixed(2)}</td>
                        <td className="py-1 text-right">{r.riskLevel}</td>
                        <td className="py-1 text-right font-mono text-red-400">{(r.bustProb * 100).toFixed(2)}%</td>
                      </tr>
                    );
                  })}
                </tbody>
              </table>
            </div>
          )}
        </div>
      )}

      {/* ── DISTRIBUTION ── */}
      {!loading && tab === 'distribution' && (
        <div className="space-y-6">
          <div className="bg-slate-800 rounded-xl border border-slate-600 p-4">
            <h2 className="text-lg font-semibold mb-3">Expected Balance Over {params.rounds} Rounds ({params.type})</h2>
            {dist && <LineChart data={dist.expectedBalances} label="Expected Balance" height={250} />}
          </div>
          <div className="bg-slate-800 rounded-xl border border-slate-600 p-4">
            <h2 className="text-lg font-semibold mb-3">Bust Probability Evolution</h2>
            {dist && <LineChart data={dist.bustProbabilities} label="Bust Probability" color="#ef4444" height={200} />}
          </div>
          <div className="grid grid-cols-3 gap-3">
            {dist && <>
              <StatBox label="Final Exp. Balance" value={dist.finalExpectedBalance?.toFixed(1)} color="text-cyan-400" />
              <StatBox label="Bust Prob" value={`${(dist.overallBustProb * 100).toFixed(2)}%`} color="text-red-400" />
              <StatBox label="Survival Prob" value={`${(dist.overallSurvivalProb * 100).toFixed(2)}%`} color="text-green-400" />
            </>}
          </div>
        </div>
      )}
    </div>
  );
}
