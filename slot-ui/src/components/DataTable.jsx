export default function DataTable({ columns, data, onRowClick }) {
  if (!data || data.length === 0) {
    return <p className="text-slate-400 text-sm py-4 text-center">No data</p>;
  }
  return (
    <div className="overflow-x-auto">
      <table className="w-full text-sm">
        <thead>
          <tr className="border-b border-slate-600 text-slate-400 text-left">
            {columns.map((c) => (
              <th key={c.key} className="py-2 px-3 font-medium">{c.label}</th>
            ))}
          </tr>
        </thead>
        <tbody>
          {data.map((row, i) => (
            <tr
              key={row.id ?? i}
              onClick={() => onRowClick?.(row)}
              className={`border-b border-slate-700 text-slate-200 ${
                onRowClick ? 'cursor-pointer hover:bg-slate-700' : ''
              }`}
            >
              {columns.map((c) => (
                <td key={c.key} className="py-2 px-3">{c.render ? c.render(row[c.key], row) : row[c.key]}</td>
              ))}
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
}
