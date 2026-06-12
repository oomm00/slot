export default function LoadingSpinner({ text }) {
  return (
    <div className="flex items-center justify-center py-12">
      <div className="flex flex-col items-center gap-3">
        <div className="w-8 h-8 border-4 border-indigo-400 border-t-transparent rounded-full animate-spin" />
        {text && <p className="text-sm text-slate-400">{text}</p>}
      </div>
    </div>
  );
}
