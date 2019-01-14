-- sqlite3 database schema for python-parsed ENSDF objects
-- sqlite3 $ENSDFDB < $MPMUTILS/Physics/RefDataDB/ENSDF_DB_Schema.sql

CREATE TABLE ENSDF_Entries (
    entry_id INTEGER PRIMARY KEY,   -- database ID
    A INTEGER,                      -- mass number
    Z INTEGER,                      -- atomic number
    DSID TEXT,                      -- "dataset ID" text description
    pcl BLOB                        -- python pickle object contents
);
CREATE INDEX idx_entries ON ENSDF_Entries(A,Z);

-- Decay chain information
CREATE TABLE Decay_Index (
    parent_id INTEGER,          -- entry_id for parent adopted levels
    child_id  INTEGER,          -- entry_id for child containing decay from parent
    FOREIGN KEY(parent_id) REFERENCES ENSDF_Entries(entry_id) ON DELETE CASCADE,
    FOREIGN KEY(child_id) REFERENCES ENSDF_Entries(entry_id) ON DELETE CASCADE
);
CREATE INDEX idx_decaychain ON Decay_Index(parent_id, child_id)
