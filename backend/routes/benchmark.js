// benchmark.js 
import express from 'express';
import db from '../db.js';

const router = express.Router();

router.get('/', (req, res) => {
  db.query('SELECT * FROM benchmarks ORDER BY id DESC', (err, results) => {
    if (err) return res.status(500).json({ error: err });
    res.json(results);
  });
});

router.post('/', (req, res) => {
  const { username, cpu_score, gpu_score } = req.body;
  db.query(
    'INSERT INTO benchmarks (username, cpu_score, gpu_score) VALUES (?, ?, ?)',
    [username, cpu_score, gpu_score],
    (err) => {
      if (err) return res.status(500).json({ error: err });
      res.json({ message: 'Benchmark başarıyla eklendi!' });
    }
  );
});

export default router;
