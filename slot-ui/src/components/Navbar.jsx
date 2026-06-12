import { Link, useNavigate } from 'react-router-dom';
import { useApp } from '../context/AppContext';

export default function Navbar() {
  const { user, logout } = useApp();
  const navigate = useNavigate();

  const handleLogout = () => {
    logout();
    navigate('/login');
  };

  return (
    <header className="bg-slate-900 border-b border-slate-700 px-6 py-3 flex items-center justify-between">
      <Link to="/" className="text-xl font-bold tracking-tight text-white">
        Slot DAA
      </Link>
      <div className="flex items-center gap-4">
        {user && (
          <>
            <span className="text-sm text-slate-300">{user.username}</span>
            <span className="text-xs bg-slate-700 text-slate-300 px-2 py-0.5 rounded-full">{user.role}</span>
            <button onClick={handleLogout} className="text-sm text-slate-400 hover:text-white transition-colors">
              Sign Out
            </button>
          </>
        )}
      </div>
    </header>
  );
}
