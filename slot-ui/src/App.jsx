import { BrowserRouter, Routes, Route, Navigate } from 'react-router-dom'
import { AppProvider, useApp } from './context/AppContext'
import MainLayout from './layouts/MainLayout'
import Login from './pages/Login'
import Register from './pages/Register'
import Dashboard from './pages/Dashboard'
import Players from './pages/Players'
import PlayerProfile from './pages/PlayerProfile'
import Betting from './pages/Betting'
import Analytics from './pages/Analytics'
import FraudDetection from './pages/FraudDetection'
import Leaderboard from './pages/Leaderboard'
import Search from './pages/Search'
import Admin from './pages/Admin'
import './App.css'

function AuthGuard({ children }) {
  const { user } = useApp();
  if (!user) return <Navigate to="/login" replace />;
  return children;
}

function AdminGuard({ children }) {
  const { role } = useApp();
  if (role !== 'admin') return <Navigate to="/" replace />;
  return children;
}

function App() {
  return (
    <AppProvider>
      <BrowserRouter>
        <Routes>
          <Route path="/login" element={<Login />} />
          <Route path="/register" element={<Register />} />
          <Route path="/" element={<AuthGuard><MainLayout /></AuthGuard>}>
            <Route index element={<Navigate to="/dashboard" replace />} />
            <Route path="dashboard" element={<Dashboard />} />
            <Route path="players" element={<Players />} />
            <Route path="players/:id" element={<PlayerProfile />} />
            <Route path="betting" element={<Betting />} />
            <Route path="analytics" element={<Analytics />} />
            <Route path="fraud" element={<AdminGuard><FraudDetection /></AdminGuard>} />
            <Route path="leaderboard" element={<Leaderboard />} />
            <Route path="search" element={<Search />} />
            <Route path="admin" element={<AdminGuard><Admin /></AdminGuard>} />
          </Route>
        </Routes>
      </BrowserRouter>
    </AppProvider>
  )
}

export default App
