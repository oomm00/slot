import api from './api';

export const getFraudCheck = (id) =>
  api.get(`/fraud/player/${id}`).then((r) => r.data);

export const getFraudAlerts = () =>
  api.get('/fraud/alerts').then((r) => r.data);
