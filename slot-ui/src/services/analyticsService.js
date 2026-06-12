import api from './api';

export const getPlayerAnalytics = (id) =>
  api.get(`/analytics/player/${id}`).then((r) => r.data);

export const getSystemAnalytics = () =>
  api.get('/analytics/system').then((r) => r.data);

export const getDistribution = (params) =>
  api.get('/analytics/distribution', { params }).then((r) => r.data);

export const getMarkovChain = (params) =>
  api.get('/analytics/markov', { params }).then((r) => r.data);

export const getRiskMetrics = (params) =>
  api.get('/analytics/risk', { params }).then((r) => r.data);

export const getStrategyComparison = (params) =>
  api.get('/analytics/compare', { params }).then((r) => r.data);

export const getStrategyRecommendation = (params) =>
  api.get('/analytics/recommend', { params }).then((r) => r.data);
