-- Track data processing status

PRAGMA foreign_keys = ON;

-- Named entities to perform actions on
CREATE TABLE entity (
    entity_id INTEGER PRIMARY KEY, -- primary key for object identification
    name TEXT,          -- name (short identifier)
    descrip TEXT        -- description (more verbose information)
);
CREATE UNIQUE INDEX idx_entity ON entity(name);

-- Processes to perform on entities
CREATE TABLE process (
    process_id INTEGER PRIMARY KEY, -- primary key for object identification
    name TEXT,          -- name (short identifier)
    descrip TEXT        -- description (more verbose information)
);
CREATE UNIQUE INDEX idx_process ON process(name);

-- Processing status
CREATE TABLE status (
    entity_id INTEGER,  -- entity being processed
    process_id INTEGER, -- process being applied
    state INTEGER,      -- state of processing: 1 = "in progress", 2 = "done", 3 = "failed"
    time REAL,          -- time when state specified
    FOREIGN KEY(entity_id) REFERENCES entity(entity_id) ON DELETE CASCADE,
    FOREIGN KEY(process_id) REFERENCES process(process_id) ON DELETE CASCADE
);
CREATE UNIQUE INDEX idx_status ON status(entity_id,process_id);
CREATE INDEX idx_status_2 ON status(state,process_id);

