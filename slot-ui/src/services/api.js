import axios from 'axios';

// In dev, Vite proxy handles /api -> localhost:8080
// In production (Vercel), set VITE_API_URL to your Railway backend URL (no trailing /api)
const API_BASE = import.meta.env.VITE_API_URL || '';
const api = axios.create({
  baseURL: API_BASE + '/api',
});

api.interceptors.request.use((config) => {
  const token = localStorage.getItem('token');
  if (token) config.headers.Authorization = `Bearer ${token}`;
  return config;
});

api.interceptors.response.use(
  (res) => res,
  (err) => {
    const msg = err.response?.data?.error || err.message || 'Request failed';
    return Promise.reject(new Error(msg));
  }
);

export default api;
