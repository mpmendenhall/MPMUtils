-- Analysis results database schema
-- sqlite3 [analysis DB] < AnalysisDB_Schema.sql

-- Analysis code identifier
CREATE TABLE analysis_code (
    code_id INTEGER PRIMARY KEY,    -- database ID; best practice: hash of code_description
    code_description TEXT           -- descriptive string for code version, compile, etc.
);

-- Analysis dataset identifier
CREATE TABLE analysis_data (
    data_id INTEGER PRIMARY KEY,    -- database ID; best practice: hash of data_name
    data_name TEXT                  -- identifier for dataset/file input
);

-- Analysis quantity identifier
CREATE TABLE analysis_vars (
    var_id INTEGER PRIMARY KEY, -- database ID; best practice: hash of name
    name TEXT,                  -- name of analysis quantity
    unit TEXT,                  -- units for quantity
    descrip TEXT                -- longer description
);
CREATE UNIQUE INDEX idx_analysis_vars ON analysis_vars(name);

-- Analysis results identifiers
CREATE TABLE analysis_results (
    result_id INTEGER PRIMARY KEY,  -- database ID; best practice: hash of following items
    data_id INTEGER,                -- from analysis_data
    var_id INTEGER,                 -- from analysis_vars
    code_id INTEGER,                -- from analysis_code
    anatime REAL,                   -- timestamp for when result was entered
    FOREIGN KEY(data_id) REFERENCES analysis_data(data_id) ON DELETE CASCADE,
    FOREIGN KEY(var_id) REFERENCES analysis_vars(var_id) ON DELETE CASCADE,
    FOREIGN KEY(code_id) REFERENCES analysis_code(code_id) ON DELETE CASCADE
);
CREATE UNIQUE INDEX idx_analysis_results ON analysis_results(data_id, var_id);
CREATE INDEX idx_anaresults_var ON analysis_results(var_id);
CREATE INDEX idx_anaresults_code ON analysis_results(code_id);

-------------------------------------------------------------------
-- User's job to associate data structures with results identifiers
-- Some simple tables below:

-- numerical result quantities
CREATE TABLE number_result (
    result_id INTEGER,  -- from analysis_results
    val REAL,           -- result value
    err REAL,           -- result uncertainty
    FOREIGN KEY(result_id) REFERENCES analysis_results(result_id) ON DELETE CASCADE
);
CREATE INDEX idx_numresults ON number_result(result_id);

-- freeform text result quantities
CREATE TABLE text_result (
    result_id INTEGER,  -- from analysis_results
    val TEXT,           -- result value
    FOREIGN KEY(result_id) REFERENCES analysis_results(result_id) ON DELETE CASCADE
);
CREATE INDEX idx_textresults ON text_result(result_id);

-- PRAGMA journal_mode=WAL; -- not good on network filesystems
-- PRAGMA foreign_keys=ON;  -- needs to be re-done for every connection
