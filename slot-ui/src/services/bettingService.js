import api from './api';

export const placeBet = (betType, prediction, betAmount) =>
  api.post('/spin', { betType, prediction, betAmount }).then((r) => r.data);

export const getBetHistory = (playerId) =>
  api.get(`/bets/history/${playerId}`).then((r) => r.data);

export const getGameHistory = (playerId) =>
  api.get(`/games/history/${playerId}`).then((r) => r.data);

export const getBetTypes = () =>
  api.get('/bet-types').then((r) => r.data);
