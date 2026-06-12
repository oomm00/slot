import api from './api';

export const getLeaderboard = (n = 10) =>
  api.get(`/leaderboard?n=${n}`).then((r) => r.data);

export const getTopBy = (metric, n = 10) =>
  api.get(`/leaderboard/top/${metric}?n=${n}`).then((r) => r.data);
