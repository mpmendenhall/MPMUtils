-- sqlite3 database schema for job submission management
-- sqlite3 Jobs.db < $MPMUTILS/pyUtils/JobManager/JobsDB.sql

PRAGMA foreign_keys = ON;

CREATE TABLE jobs (
    job_id INTEGER PRIMARY KEY, -- unique internal job ID number
    status INTEGER,             -- processing status. -1: hold; 0: waiting; 1: sumitted; 2: queued; 3: running; 4: done; >= 5: special/undefined
    queue_id INTEGER,           -- submission queue identifier
    name TEXT,                  -- user-assigned (group) name
    jobfile TEXT,               -- job commands file
    outlog TEXT,                -- output logfile path
    associated INTEGER,         -- associated job for bundling
    t_submit REAL,              -- submission time
    use_walltime REAL,          -- actual used wall time [s]
    return_code INTEGER,        -- job system return code
    FOREIGN KEY(associated) REFERENCES jobs(job_id) ON DELETE CASCADE
);
CREATE INDEX idx_jobs_qid ON jobs(queue_id);
CREATE INDEX idx_jobs_name ON jobs(name,status);
CREATE INDEX idx_jobs_status ON jobs(status);

-- Resource pools needed by jobs
CREATE TABLE resources (
    resource_id INTEGER PRIMARY KEY, -- primary key for object identification
    name TEXT,          -- name (short identifier)
    descrip TEXT,       -- description (more verbose information)
    available REAL      -- resource amount available for all jobs
);
CREATE UNIQUE INDEX idx_resources ON resources(name);

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
