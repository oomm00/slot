import api from './api';

export const searchPlayers = (q) =>
  api.get(`/search/player?q=${encodeURIComponent(q)}`).then((r) => r.data);

export const searchGame = (id) =>
  api.get(`/search/game/${id}`).then((r) => r.data);

export const searchBet = (id) =>
  api.get(`/search/bets/${id}`).then((r) => r.data);

export const searchByDateRange = (start, end) =>
  api.get(`/search/date?start=${start}&end=${end}`).then((r) => r.data);
