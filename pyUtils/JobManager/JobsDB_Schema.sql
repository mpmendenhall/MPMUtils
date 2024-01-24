-- sqlite3 database schema for job submission management

PRAGMA foreign_keys = ON;
--PRAGMA journal_mode = WAL; -- for local jobs to report back

CREATE TABLE jobs (
    job_id INTEGER PRIMARY KEY, -- unique internal job ID number
    jtype  INTEGER,             -- type, 0: shell script, 1: bundle of sub-jobs
    status INTEGER,             -- processing status. -1: hold; 0: waiting; 1: submitted; 2: queued; 3: running; 4: done; >= 5: special/undefined
    queue_id INTEGER,           -- submission queue identifier
    name TEXT,                  -- user-assigned grouping name
    q_name TEXT,                -- job queue name
    q_acct TEXT,                -- submission account
    jobscript TEXT,             -- job script or other data
    associated INTEGER,         -- associated job for bundling
    t_submit REAL,              -- submission timestamp
    use_walltime REAL,          -- actual used wall time [s]
    return_code INTEGER,        -- job system return code
    FOREIGN KEY(associated) REFERENCES jobs(job_id) ON DELETE CASCADE
);
CREATE INDEX idx_jobs_qid ON jobs(queue_id);
CREATE INDEX idx_jobs_name ON jobs(name,status);
CREATE INDEX idx_jobs_status ON jobs(status);
CREATE INDEX idx_jobs_jid ON jobs(job_id);

-- Resource pools needed by jobs
CREATE TABLE resources (
    resource_id INTEGER PRIMARY KEY, -- primary key for object identification
    name TEXT,          -- name (short identifier)
    descrip TEXT,       -- description (more verbose information)
    available REAL      -- resource amount available for all jobs
);
CREATE UNIQUE INDEX idx_resources ON resources(name);
INSERT INTO resources(name,descrip,available) VALUES ("walltime","job walltime [s]",1e9);

-- Resources requested by a job
CREATE TABLE resource_use (
   job_id INTEGER,              -- job requesting resource
   resource_id INTEGER,         -- resource being requested
   quantity REAL,               -- amount of resource needed
   FOREIGN KEY(job_id) REFERENCES jobs(job_id) ON DELETE CASCADE,
   FOREIGN KEY(resource_id) REFERENCES resources(resource_id) ON DELETE CASCADE
);
CREATE UNIQUE INDEX idx_resource_use ON resource_use(job_id, resource_id);
CREATE INDEX idx_rsrcuse_2 ON resource_use(resource_id);
