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
CREATE UNIQUE INDEX idx_analysis_vars ON analysis_vars(name,descrip);

-- Analysis results quantities
CREATE TABLE analysis_results (
    run_id INTEGER,     -- from analysis_runs
    var_id INTEGER,     -- rowid from analysis_vars
    val REAL,           -- result value
    err REAL,           -- result uncertainty
    FOREIGN KEY(run_id) REFERENCES analysis_runs(run_id) ON DELETE CASCADE,
    FOREIGN KEY(var_id) REFERENCES analysis_vars(var_id) ON DELETE CASCADE
);
CREATE UNIQUE INDEX idx_analysis_results ON analysis_results(run_id, var_id);
CREATE INDEX idx_anaresults_var ON analysis_results(var_id);

---------------------------------------
---------------------------------------
-- query recipes
--
-- tables joined:
-- SELECT * FROM analysis_runs,analysis_vars,analysis_results WHERE run = analysis_runs.rowid AND var = analysis_vars.rowid;
--
-- newest analysis for every run:
-- SELECT * FROM analysis_runs GROUP BY dataname ORDER BY anatime;
--
-- newest analysis for a particular run:
-- SELECT * FROM analysis_runs WHERE dataname = "<dataset name>" GROUP BY dataname ORDER BY anatime;
--
-- newest results for particular run:
-- SELECT * FROM analysis_runs,analysis_vars,analysis_results WHERE run = analysis_runs.rowid AND var = analysis_vars.rowid
-- AND analysis_runs.rowid = (SELECT rowid FROM analysis_runs WHERE dataname = "<dataset name>" GROUP BY dataname ORDER BY anatime);
--
