-- sqlite3 database schema for job submission management
-- sqlite3 Jobs.db < $MPMUTILS/pyUtils/JobManager/JobsDB.sql

CREATE TABLE jobs (
    job_id INTEGER PRIMARY KEY, -- unique internal job ID number
    status INTEGER,             -- processing status. -1: hold; 0: waiting; 1: sumitted; 2: queued; 3: running; 4: done; >= 5: special/undefined
    queue_id INTEGER,           -- submission queue identifier
    name TEXT,                  -- user-assigned (group) name
    jobfile TEXT,               -- job commands file
    outlog TEXT,                -- output logfile path
    t_submit DATETIME,          -- submission time
    n_nodes INTEGER,            -- required number of nodes
    est_walltime REAL           -- estimated wall time in s
);
CREATE UNIQUE INDEX idx_jobs_qid ON jobs(queue_id);
CREATE INDEX idx_jobs_name ON jobs(name,status);
CREATE INDEX idx_jobs_status ON jobs(status);
