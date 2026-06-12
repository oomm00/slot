export default function SlotMachineDisplay({ symbols, spinning }) {
  const symMap = { CHERRY: '🍒', LEMON: '🍋', ORANGE: '🍊', SEVEN: '7️⃣', BELL: '🔔', BAR: '📀', GRAPE: '🍇' };

  if (spinning) {
    return (
      <div className="flex justify-center gap-4 py-6">
        {[0, 1, 2].map((i) => (
          <div key={i} className="w-20 h-20 bg-slate-700 rounded-xl flex items-center justify-center text-3xl animate-pulse">
            🎰
          </div>
        ))}
      </div>
    );
  }

  return (
    <div className="flex justify-center gap-4 py-4">
      {symbols?.map((s, i) => (
        <div key={i} className="w-20 h-20 bg-slate-700 rounded-xl flex items-center justify-center text-3xl border-2 border-indigo-500">
          {symMap[s] || '❓'}
        </div>
      ))}
    </div>
  );
}
