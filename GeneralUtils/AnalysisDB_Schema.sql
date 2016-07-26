-- Analysis results database schema
-- sqlite3 $PROSPECT_ANADB < $MPM_P2X_ANALYSIS/FileIO/AnalysisDB_Schema.sql

-- Analysis run identifier
CREATE TABLE analysis_runs (
    dataname TEXT,      -- identifier for dataset/file input
    anatime REAL        -- timestamp for when analysis was performed
);
CREATE UNIQUE INDEX idx_analysis_runs ON analysis_runs(dataname,anatime);

-- Analysis quantity identifier
CREATE TABLE analysis_vars (
    name TEXT,          -- name of analysis quantity
    unit TEXT,          -- units for quantity
    descrip TEXT        -- longer description
);
CREATE UNIQUE INDEX idx_analysis_vars ON analysis_vars(name,descrip);

-- Analysis results quantities
CREATE TABLE analysis_results (
    run INTEGER,        -- row_id from analysis_runs
    var INTEGER,        -- rowid from analysis_vars
    val REAL,           -- result value
    err REAL            -- result uncertainty
);
CREATE UNIQUE INDEX idx_analysis_results ON analysis_results(run,var);

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
-- SELECT * FROM analysis_runs WHERE dataname = "yale/P50/P50A/series018" GROUP BY dataname ORDER BY anatime;
--
-- newest results for particular run:
-- SELECT * FROM analysis_runs,analysis_vars,analysis_results WHERE run = analysis_runs.rowid AND var = analysis_vars.rowid
-- AND analysis_runs.rowid = (SELECT rowid FROM analysis_runs WHERE dataname = "yale/P50/P50A/series018" GROUP BY dataname ORDER BY anatime);
--