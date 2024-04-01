-- Track data processing status

PRAGMA foreign_keys = ON;

-- Named entities to perform actions on
CREATE TABLE entity (
    entity_id INTEGER PRIMARY KEY,  -- primary key for object identification
    entity_name TEXT,               -- name (short identifier)
    entity_descrip TEXT             -- description (more verbose information)
);
CREATE UNIQUE INDEX idx_entity ON entity(entity_name);

-- Processes to perform on entities
CREATE TABLE process (
    process_id INTEGER PRIMARY KEY, -- primary key for object identification
    process_name TEXT,              -- name (short identifier)
    process_descrip TEXT            -- description (more verbose information)
);
CREATE UNIQUE INDEX idx_process ON process(process_name);

-- Processing status
CREATE TABLE status (
    entity_id INTEGER,  -- entity being processed
    process_id INTEGER, -- process being applied
    state INTEGER,      -- state of processing: 0 = "setup", 1 = "in progress", 2 = "done", 3 = "failed"
    job_id INTEGER,     -- process identifier from external job launcher system
    stattime REAL,      -- time when state specified [UNIX timestamp]
    calctime REAL,      -- processing runtime [s]
    input_size REAL,    -- input files size [B]
    output_size REAL,   -- output files size [B]
    FOREIGN KEY(entity_id) REFERENCES entity(entity_id) ON DELETE CASCADE,
    FOREIGN KEY(process_id) REFERENCES process(process_id) ON DELETE CASCADE
);
CREATE UNIQUE INDEX idx_status ON status(entity_id,process_id);
CREATE INDEX idx_status_2 ON status(state,process_id);

/*
check status for a particular process:
SELECT * FROM status NATURAL JOIN entity WHERE process_id = 2;

show failed steps:
SELECT * FROM entity NATURAL JOIN status WHERE state = 3;

clear all failed steps (for retry):
DELETE FROM status WHERE state = 3;

remove "failed" or "setup" status to re-try step:
DELETE FROM status WHERE (state = 0 OR state = 3) AND process_id = 2;

remove entities failing a process step:
delete from entity where entity_id in (select entity_id from status where state=3 and p"rocess_id=2);
*/
