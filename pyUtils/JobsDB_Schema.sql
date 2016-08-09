-- sqlite3 database schema for job submission management
-- sqlite3 Jobs.db < $MPMUTILS/pyUtils/JobManager/JobsDB.sql

CREATE TABLE jobs (
    job_id INTEGER PRIMARY KEY, -- unique internal job ID number
    status INTEGER,             -- processing status. -1: hold; 0: waiting; 1: sumitted; 2: queued; 3: running; 4: done; >= 5: special/undefined
    queue_id INTEGER,           -- submission queue identifier
    name TEXT,                  -- user-assigned (group) name
    jobfile TEXT,               -- job commands file
    outlog TEXT,                -- output logfile path
    associated INTEGER,         -- associated job for bundling
    t_submit DATETIME,          -- submission time
    n_nodes INTEGER,            -- required number of nodes
    est_walltime REAL,          -- estimated wall time [s]
    use_walltime REAL,          -- actual used wall time [s]
    return_code INTEGER,        -- job system return code
    FOREIGN KEY(associated) REFERENCES jobs(job_id) ON DELETE CASCADE
);
CREATE UNIQUE INDEX idx_jobs_qid ON jobs(queue_id);
CREATE INDEX idx_jobs_name ON jobs(name,status);
CREATE INDEX idx_jobs_status ON jobs(status);
