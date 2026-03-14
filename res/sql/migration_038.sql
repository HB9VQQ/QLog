UPDATE modes SET submodes = '["FSQCALL", "FST4", "FST4W", "FT2", "FT4", "JS8", "JTMS", "MFSK4", "MFSK8", "MFSK11", "MFSK16", "MFSK22", "MFSK31", "MFSK32", "MFSK64", "MFSK64L", "MFSK128", "MFSK128L", "Q65"]' WHERE name = 'MFSK';
UPDATE modes SET submodes = '["VARA HF", "VARA SATELLITE", "VARA FM 1200", "VARA FM 9600", "FREEDATA"]' WHERE name = 'DYNAMIC';

INSERT INTO modes (name, submodes, rprt, dxcc, enabled) VALUES ('OFDM', '["RIBBIT_PIX", "RIBBIT_SMS"]', NULL, 'DIGITAL', 0);
