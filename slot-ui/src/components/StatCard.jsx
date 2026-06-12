export default function StatCard({ label, value, icon, color }) {
  const bg = color || 'bg-slate-700';
  return (
    <div className={`${bg} rounded-xl p-4 flex items-center gap-4 border border-slate-600`}>
      {icon && <span className="text-2xl">{icon}</span>}
      <div>
        <p className="text-xs text-slate-400 uppercase tracking-wide">{label}</p>
        <p className="text-xl font-semibold text-white">{value ?? '—'}</p>
      </div>
    </div>
  );
}
