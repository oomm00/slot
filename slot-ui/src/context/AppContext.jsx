import { createContext, useContext, useState, useCallback, useEffect } from 'react';
import api from '../services/api';

const AppContext = createContext();

export function AppProvider({ children }) {
  const [user, setUser] = useState(() => {
    const stored = localStorage.getItem('user');
    return stored ? JSON.parse(stored) : null;
  });
  const [players, setPlayers] = useState([]);
  const [playersLoading, setPlayersLoading] = useState(false);
  const [lb, setLb] = useState([]);
  const [error, setError] = useState(null);

  const token = user?.token;
  const role = user?.role;

  const updateUserBalance = useCallback((newBalance) => {
    setUser((prev) => {
      if (!prev) return prev;
      const updated = { ...prev, balance: newBalance };
      localStorage.setItem('user', JSON.stringify(updated));
      return updated;
    });
  }, []);

  const login = useCallback(async (username, password) => {
    const res = await api.post('/login', { username, password });
    const data = res.data;
    const userData = { ...data, token: data.token };
    setUser(userData);
    localStorage.setItem('user', JSON.stringify(userData));
    localStorage.setItem('token', data.token);
    return userData;
  }, []);

  const register = useCallback(async (name, username, password, confirmPassword, age) => {
    const res = await api.post('/register', { name, username, password, confirmPassword, age });
    const data = res.data;
    const userData = { ...data, token: data.token };
    setUser(userData);
    localStorage.setItem('user', JSON.stringify(userData));
    localStorage.setItem('token', data.token);
    return userData;
  }, []);

  const logout = useCallback(() => {
    setUser(null);
    localStorage.removeItem('user');
    localStorage.removeItem('token');
  }, []);

  const loadPlayers = useCallback(async () => {
    setPlayersLoading(true);
    try {
      const data = await api.get('/players').then((r) => r.data);
      setPlayers(data);
    } catch (e) { setError(e.message); }
    finally { setPlayersLoading(false); }
  }, []);

  const loadLeaderboard = useCallback(async (n = 10) => {
    try {
      const data = await api.get(`/leaderboard?n=${n}`).then((r) => r.data);
      setLb(data);
    } catch (e) { setError(e.message); }
  }, []);

  const clearError = useCallback(() => setError(null), []);

  useEffect(() => {
    const stored = localStorage.getItem('user');
    if (stored) setUser(JSON.parse(stored));
  }, []);

  return (
    <AppContext.Provider value={{
      user, token, role, login, register, logout, updateUserBalance,
      players, playersLoading, loadPlayers,
      lb, loadLeaderboard,
      error, setError, clearError,
    }}>
      {children}
    </AppContext.Provider>
  );
}

export const useApp = () => useContext(AppContext);
