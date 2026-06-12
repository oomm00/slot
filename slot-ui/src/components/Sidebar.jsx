import { NavLink } from 'react-router-dom';
import { useApp } from '../context/AppContext';

export default function Sidebar() {
  const { role } = useApp();
  const links = [
    { to: '/', label: 'Dashboard' },
    { to: '/players', label: 'Players' },
    { to: '/betting', label: 'Game' },
    { to: '/leaderboard', label: 'Leaderboard' },
    { to: '/analytics', label: 'Analytics' },
    { to: '/search', label: 'Search' },
  ];
  if (role === 'admin') {
    links.push({ to: '/fraud', label: 'Fraud Detection' });
    links.push({ to: '/admin', label: 'Admin' });
  }

  return (
    <aside className="w-56 bg-slate-800 border-r border-slate-700 flex flex-col py-4">
      <nav className="flex flex-col gap-1 px-2">
        {links.map((l) => (
          <NavLink
            key={l.to}
            to={l.to}
            end={l.to === '/'}
            className={({ isActive }) =>
              `flex items-center gap-3 px-3 py-2 rounded-lg text-sm transition-colors ${
                isActive
                  ? 'bg-indigo-600 text-white'
                  : 'text-slate-300 hover:bg-slate-700 hover:text-white'
              }`
            }
          >
            {l.label}
          </NavLink>
        ))}
      </nav>
    </aside>
  );
}
