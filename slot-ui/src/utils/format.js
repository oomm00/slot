export const fmt = (n) => {
  if (n == null) return '\u2014';
  return Number(n).toLocaleString('en-US', {
    minimumFractionDigits: 2,
    maximumFractionDigits: 2,
  });
};

export const fmtPercent = (n) => {
  if (n == null) return '\u2014';
  return (Number(n) * 100).toFixed(1) + '%';
};
