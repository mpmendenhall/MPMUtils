-- Analysis results database schema
-- sqlite3 [analysis DB] < AnalysisDB_Schema.sql

-- Analysis run identifier
CREATE TABLE analysis_runs (
    run_id INTEGER PRIMARY KEY, -- database ID
    dataname TEXT,      -- identifier for dataset/file input
    anatime REAL        -- timestamp for when analysis was performed
);
CREATE UNIQUE INDEX idx_analysis_runs ON analysis_runs(dataname,anatime);

-- Analysis quantity identifier
CREATE TABLE analysis_vars (
    var_id INTEGER PRIMARY KEY, -- database ID
    name TEXT,          -- name of analysis quantity
    unit TEXT,          -- units for quantity
    descrip TEXT        -- longer description
);
CREATE UNIQUE INDEX idx_analysis_vars ON analysis_vars(name);

-- Analysis results quantities
CREATE TABLE analysis_results (
    run_id INTEGER,     -- from analysis_runs
    var_id INTEGER,     -- from analysis_vars
    val REAL,           -- result value
    err REAL,           -- result uncertainty
    FOREIGN KEY(run_id) REFERENCES analysis_runs(run_id) ON DELETE CASCADE,
    FOREIGN KEY(var_id) REFERENCES analysis_vars(var_id) ON DELETE CASCADE
);
CREATE UNIQUE INDEX idx_analysis_results ON analysis_results(run_id, var_id);
CREATE INDEX idx_anaresults_var ON analysis_results(var_id);

-- Additional freeform text result quantities
CREATE TABLE analysis_xresults (
    run_id INTEGER,     -- from analysis_runs
    var_id INTEGER,     -- from analysis_vars
    val TEXT,           -- result value
    FOREIGN KEY(run_id) REFERENCES analysis_runs(run_id) ON DELETE CASCADE,
    FOREIGN KEY(var_id) REFERENCES analysis_vars(var_id) ON DELETE CASCADE
);
CREATE UNIQUE INDEX idx_analysis_xresults ON analysis_xresults(run_id, var_id);
CREATE INDEX idx_anaxresults_var ON analysis_xresults(var_id);


-- PRAGMA journal_mode=WAL; -- not good on network filesystems
-- PRAGMA foreign_keys=ON; -- needs to be re-done for every connection

---------------------------------------
---------------------------------------
-- query recipes
--
-- tables joined:
-- SELECT * FROM analysis_runs,analysis_vars,analysis_results WHERE analysis_results.run_id = analysis_runs.run_id AND analysis_results.var_id = analysis_vars.var_id;
--
-- newest analysis for every run:
-- SELECT * FROM analysis_runs GROUP BY dataname ORDER BY anatime;
--
-- newest analysis for a particular run:
-- SELECT * FROM analysis_runs WHERE dataname = "<dataset name>" GROUP BY dataname ORDER BY anatime;
--
-- newest results for particular run:
-- SELECT * FROM analysis_runs,analysis_vars,analysis_results WHERE analysis_results.run_id = analysis_runs.run_id AND analysis_results.var_id = analysis_vars.var_id
-- AND analysis_runs.run_id = (SELECT run_id FROM analysis_runs WHERE dataname = "<dataset name>" GROUP BY dataname ORDER BY anatime);
--
