import api from './api';

export const getPlayers = () => api.get('/players').then((r) => r.data);
export const getPlayer = (id) => api.get(`/players/${id}`).then((r) => r.data);
export const depositFunds = (id, amount) =>
  api.put(`/players/${id}/deposit`, { amount }).then((r) => r.data);
export const withdrawFunds = (id, amount) =>
  api.put(`/players/${id}/withdraw`, { amount }).then((r) => r.data);
